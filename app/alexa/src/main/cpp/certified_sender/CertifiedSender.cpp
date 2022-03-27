#include <avs/MessageRequest.h>
#include <logger/Logger.h>
#include <registration_manager/CustomerDataManager.h>
#include <queue>
#include "CertifiedSender.h"

namespace alexaClientSDK {
    namespace certifiedSender {
        using namespace configuration;
        using MROI = MessageRequestObserverInterface;
        using CS = CertifiedSender;
        static const string TAG("CertifiedSender");
        #define LX(event) LogEntry(TAG, event)
        static const vector<int> EXPONENTIAL_BACKOFF_RETRY_TABLE = { 10000, 50000, 250000, 1250000 };
        CS::CertifiedMessageRequest::CertifiedMessageRequest(const string& jsonContent, int dbId, const string& uriPathExtension) :
                                                             MessageRequest{jsonContent, uriPathExtension}, m_responseReceived{false},
                                                             m_dbId{dbId}, m_isRequestShuttingDown{false} {}
        void CS::CertifiedMessageRequest::exceptionReceived(const string& exceptionMessage) {
            lock_guard<mutex> lock(m_requestMutex);
            m_sendMessageStatus = MROI::Status::SERVER_INTERNAL_ERROR_V2;
            m_responseReceived = true;
            m_requestCv.notify_all();
        }
        void CS::CertifiedMessageRequest::sendCompleted(MROI::Status sendMessageStatus) {
            lock_guard<mutex> lock(m_requestMutex);
            if (!m_responseReceived) {
                m_sendMessageStatus = sendMessageStatus;
                m_responseReceived = true;
                m_requestCv.notify_all();
            }
        }
        MROI::Status CS::CertifiedMessageRequest::waitForCompletion() {
            unique_lock<mutex> lock(m_requestMutex);
            m_requestCv.wait(lock, [this]() { return m_isRequestShuttingDown || m_responseReceived; });
            if (m_isRequestShuttingDown) {
                return MessageRequestObserverInterface::Status::TIMEDOUT;
            }
            return m_sendMessageStatus;
        }
        int CertifiedSender::CertifiedMessageRequest::getDbId() {
            return m_dbId;
        }
        void CertifiedSender::CertifiedMessageRequest::shutdown() {
            lock_guard<mutex> lock(m_requestMutex);
            m_isRequestShuttingDown = true;
            m_requestCv.notify_all();
        }
        shared_ptr<CertifiedSender> CertifiedSender::create(shared_ptr<MessageSenderInterface> messageSender,
                                                            shared_ptr<AVSConnectionManagerInterface> connection,
                                                            shared_ptr<MessageStorageInterface> storage,
                                                            shared_ptr<CustomerDataManager> dataManager) {
            auto certifiedSender = shared_ptr<CertifiedSender>(new CertifiedSender(messageSender, connection, storage, dataManager));
            if (!certifiedSender->init()) {
                ACSDK_ERROR(LX("createFailed").m("Could not initialize certifiedSender."));
                return nullptr;
            }
            connection->addConnectionStatusObserver(certifiedSender);
            return certifiedSender;
        }
        CertifiedSender::CertifiedSender(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<AVSConnectionManagerInterface> connection,
                                         shared_ptr<MessageStorageInterface> storage, shared_ptr<CustomerDataManager> dataManager,
                                         int queueSizeWarnLimit, int queueSizeHardLimit) : RequiresShutdown("CertifiedSender"),
                                         CustomerDataHandler(dataManager), m_queueSizeWarnLimit{queueSizeWarnLimit},
                                         m_queueSizeHardLimit{queueSizeHardLimit}, m_isShuttingDown{false}, m_isConnected{false},
                                         m_retryTimer(EXPONENTIAL_BACKOFF_RETRY_TABLE), m_messageSender{messageSender},
                                         m_connection{connection}, m_storage{storage} {}
        CertifiedSender::~CertifiedSender() {
            unique_lock<mutex> lock(m_mutex);
            m_isShuttingDown = true;
            if (m_currentMessage) m_currentMessage->shutdown();
            lock.unlock();
            m_workerThreadCV.notify_one();
            m_backoffWaitCV.notify_one();
            if (m_workerThread.joinable()) m_workerThread.join();
        }
        bool CertifiedSender::init() {
            if (m_queueSizeWarnLimit < 0 || m_queueSizeHardLimit <= 0 || m_queueSizeHardLimit < m_queueSizeWarnLimit) {
                ACSDK_ERROR(LX("initFailed").d("warnSizeLimit", m_queueSizeWarnLimit).d("hardSizeLimit", m_queueSizeHardLimit)
                    .m("Limit values are invalid."));
                return false;
            }
            if (!m_storage->open()) {
                ACSDK_INFO(LX("init : Database file does not exist.  Creating."));
                if (!m_storage->createDatabase()) {
                    ACSDK_ERROR(LX("initFailed").m("Could not create database file."));
                    return false;
                }
            }
            queue<MessageStorageInterface::StoredMessage> storedMessages;
            if (!m_storage->load(&storedMessages)) {
                ACSDK_ERROR(LX("initFailed").m("Could not load messages from database file."));
                return false;
            }
            while(!storedMessages.empty()) {
                auto storedMessage = storedMessages.front();
                {
                    lock_guard<mutex> lock{m_mutex};
                    m_messagesToSend.push_back(make_shared<CertifiedMessageRequest>(storedMessage.message, storedMessage.id,
                                                                                    storedMessage.uriPathExtension));
                }
                storedMessages.pop();
            }
            m_workerThread = thread(&CertifiedSender::mainloop, this);
            return true;
        }
        inline bool shouldRetryTransmission(MROI::Status status) {
            switch(status) {
                case MROI::Status::SUCCESS: case MROI::Status::SUCCESS_ACCEPTED: case MROI::Status::SUCCESS_NO_CONTENT:
                case MROI::Status::SERVER_INTERNAL_ERROR_V2: case MROI::Status::CANCELED: case MROI::Status::SERVER_OTHER_ERROR:
                case MROI::Status::BAD_REQUEST:
                    return false;
                case MROI::Status::THROTTLED: case MROI::Status::PENDING: case MROI::Status::NOT_CONNECTED:
                case MROI::Status::NOT_SYNCHRONIZED: case MROI::Status::TIMEDOUT: case MROI::Status::PROTOCOL_ERROR:
                case MROI::Status::INTERNAL_ERROR: case MROI::Status::REFUSED: case MROI::Status::INVALID_AUTH:
                    return true;
            }
            return false;
        }
        void CertifiedSender::mainloop() {
            int failedSendRetryCount = 0;
            while(true) {
                unique_lock<mutex> lock(m_mutex);
                m_currentMessage.reset();
                if ((!m_isConnected || m_messagesToSend.empty()) && !m_isShuttingDown) {
                    m_workerThreadCV.wait(lock, [this]() { return ((m_isConnected && !m_messagesToSend.empty()) || m_isShuttingDown); });
                }
                if (m_isShuttingDown) {
                    ACSDK_DEBUG9(LX("CertifiedSender worker thread done.  Exiting mainloop."));
                    return;
                }
                m_currentMessage = m_messagesToSend.front();
                lock.unlock();
                m_messageSender->sendMessage(m_currentMessage);
                auto status = m_currentMessage->waitForCompletion();
                if (shouldRetryTransmission(status)) {
                    lock.lock();
                    m_messagesToSend.pop_front();
                    m_messagesToSend.push_front(make_shared<CertifiedMessageRequest>(m_currentMessage->getJsonContent(),
                                                m_currentMessage->getDbId(), m_currentMessage->getUriPathExtension()));
                    lock.unlock();
                    auto timeout = m_retryTimer.calculateTimeToRetry(failedSendRetryCount);
                    ACSDK_DEBUG5(LX(__func__).d("failedSendRetryCount", failedSendRetryCount).d("timeout", timeout.count()));
                    failedSendRetryCount++;
                    lock.lock();
                    m_backoffWaitCV.wait_for(lock, timeout, [this]() { return m_isShuttingDown; });
                    if (m_isShuttingDown) {
                        ACSDK_DEBUG9(LX("CertifiedSender worker thread done.  Exiting mainloop."));
                        return;
                    }
                    lock.unlock();
                } else {
                    lock.lock();
                    if (!m_storage->erase(m_currentMessage->getDbId())) {
                        ACSDK_ERROR(LX("mainloop : could not erase message from storage."));
                    }
                    m_messagesToSend.pop_front();
                    lock.unlock();
                    failedSendRetryCount = 0;
                }
            }
        }
        void CertifiedSender::onConnectionStatusChanged(Status status, ChangedReason reason) {
            unique_lock<mutex> lock(m_mutex);
            m_isConnected = (Status::CONNECTED == status);
            lock.unlock();
            m_workerThreadCV.notify_one();
        }
        future<bool> CertifiedSender::sendJSONMessage(const string& jsonMessage, const string& uriPathExtension) {
            return m_executor.submit([this, jsonMessage, uriPathExtension]() { return executeSendJSONMessage(jsonMessage, uriPathExtension); });
        }
        bool CertifiedSender::executeSendJSONMessage(string jsonMessage, const string& uriPathExtension) {
            unique_lock<mutex> lock(m_mutex);
            int queueSize = static_cast<int>(m_messagesToSend.size());
            if (queueSize >= m_queueSizeHardLimit) {
                ACSDK_ERROR(LX("executeSendJSONMessage").m("Queue size is at max limit.  Cannot add message to send."));
                return false;
            }
            if (queueSize >= m_queueSizeWarnLimit) {
                ACSDK_WARN(LX("executeSendJSONMessage").m("Warning : queue size has exceeded the warn limit."));
            }
            int messageId = 0;
            if (!m_storage->store(jsonMessage, uriPathExtension, &messageId)) {
                ACSDK_ERROR(LX("executeSendJSONMessage").m("Could not store message."));
                return false;
            }
            m_messagesToSend.push_back(make_shared<CertifiedMessageRequest>(jsonMessage, messageId, uriPathExtension));
            lock.unlock();
            m_workerThreadCV.notify_one();
            return true;
        }
        void CertifiedSender::doShutdown() {
            m_connection->removeConnectionStatusObserver(shared_from_this());
        }
        void CertifiedSender::clearData() {
            auto result = m_executor.submit([this]() {
                unique_lock<std::mutex> lock(m_mutex);
                m_messagesToSend.clear();
                m_storage->clearDatabase();
            });
            result.wait();
        }
    }
}