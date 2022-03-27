#ifndef ALEXA_CLIENT_SDK_CERTIFIEDSENDER_INCLUDE_CERTIFIEDSENDER_CERTIFIEDSENDER_H_
#define ALEXA_CLIENT_SDK_CERTIFIEDSENDER_INCLUDE_CERTIFIEDSENDER_CERTIFIEDSENDER_H_


#include <deque>
#include <memory>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/RequiresShutdown.h>
#include <util/RetryTimer.h>
#include <threading/Executor.h>
#include <registration_manager/CustomerDataHandler.h>
#include <registration_manager/CustomerDataManager.h>
#include <certified_sender/MessageStorageInterface.h>

namespace alexaClientSDK {
    namespace certifiedSender {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace registrationManager;
        using namespace threading;
        using MROI = MessageRequestObserverInterface;
        using CSOI = ConnectionStatusObserverInterface;
        static const int CERTIFIED_SENDER_QUEUE_SIZE_WARN_LIMIT = 25;
        static const int CERTIFIED_SENDER_QUEUE_SIZE_HARD_LIMIT = 50;
        class CertifiedSender : public RequiresShutdown, public ConnectionStatusObserverInterface, public enable_shared_from_this<CertifiedSender>,
                                public CustomerDataHandler {
        public:
            static shared_ptr<CertifiedSender> create(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<AVSConnectionManagerInterface> connection,
                                                      shared_ptr<MessageStorageInterface> storage, shared_ptr<CustomerDataManager> dataManager);
            virtual ~CertifiedSender();
            future<bool> sendJSONMessage(const string& jsonMessage, const string& uriPathExtension = "");
            void clearData() override;
        private:
            class CertifiedMessageRequest : public MessageRequest {
            public:
                CertifiedMessageRequest(const string& jsonContent, int dbId, const string& uriPathExtension = "");
                void exceptionReceived(const string& exceptionMessage) override;
                void sendCompleted(MROI::Status sendMessageStatus);
                MROI::Status waitForCompletion();
                int getDbId();
                void shutdown();
            private:
                MROI::Status m_sendMessageStatus;
                bool m_responseReceived;
                mutex m_requestMutex;
                condition_variable m_requestCv;
                int m_dbId;
                bool m_isRequestShuttingDown;
            };
            CertifiedSender(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<AVSConnectionManagerInterface> connection,
                            shared_ptr<MessageStorageInterface> storage, shared_ptr<CustomerDataManager> dataManager,
                            int queueSizeWarnLimit = CERTIFIED_SENDER_QUEUE_SIZE_WARN_LIMIT,
                            int queueSizeHardLimit = CERTIFIED_SENDER_QUEUE_SIZE_HARD_LIMIT);
            void onConnectionStatusChanged(const CSOI::Status status, const CSOI::ChangedReason reason) override;
            bool init();
            bool executeSendJSONMessage(string jsonMessage, const string& uriPathExtension);
            void doShutdown() override;
            void mainloop();
            int m_queueSizeWarnLimit;
            int m_queueSizeHardLimit;
            thread m_workerThread;
            bool m_isShuttingDown;
            mutex m_mutex;
            condition_variable m_workerThreadCV;
            bool m_isConnected;
            RetryTimer m_retryTimer;
            deque<shared_ptr<CertifiedMessageRequest>> m_messagesToSend;
            shared_ptr<MessageSenderInterface> m_messageSender;
            shared_ptr<CertifiedMessageRequest> m_currentMessage;
            shared_ptr<AVSConnectionManagerInterface> m_connection;
            shared_ptr<MessageStorageInterface> m_storage;
            Executor m_executor;
            condition_variable m_backoffWaitCV;
        };
    }
}
#endif