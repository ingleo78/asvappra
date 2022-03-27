#include <logger/Logger.h>
#include "MessageRequest.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace attachment;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            static const string TAG("MessageRequest");
            #define LX(event) LogEntry(TAG, event)
            MessageRequest::MessageRequest(const string& jsonContent, const string& uriPathExtension) : m_jsonContent{jsonContent}, m_isSerialized{true},
                                                                                                        m_uriPathExtension{uriPathExtension} {}
            MessageRequest::MessageRequest(const string& jsonContent, bool isSerialized, const string& uriPathExtension,
                                           vector<pair<string, string>> headers) : m_jsonContent{jsonContent}, m_isSerialized{isSerialized},
                                           m_uriPathExtension{uriPathExtension}, m_headers(std::move(headers)) {}
            MessageRequest::~MessageRequest() {}
            void MessageRequest::addAttachmentReader(const string& name, shared_ptr<AttachmentReader> attachmentReader) {
                if (!attachmentReader) {
                    ACSDK_ERROR(LX("addAttachmentReaderFailed").d("reason", "nullAttachment"));
                    return;
                }
                auto namedReader = make_shared<MessageRequest::NamedReader>(name, attachmentReader);
                m_readers.push_back(namedReader);
            }
            string MessageRequest::getJsonContent() const {
                return m_jsonContent;
            }
            bool MessageRequest::getIsSerialized() const {
                return m_isSerialized;
            }
            string MessageRequest::getUriPathExtension() const {
                return m_uriPathExtension;
            }
            int MessageRequest::attachmentReadersCount() const {
                return m_readers.size();
            }
            shared_ptr<MessageRequest::NamedReader> MessageRequest::getAttachmentReader(size_t index) {
                if (m_readers.size() <= index) {
                    ACSDK_ERROR(LX("getAttachmentReaderFailed").d("reason", "index out of bound").d("index", index));
                    return nullptr;
                }
                return m_readers[index];
            }
            void MessageRequest::sendCompleted(MessageRequestObserverInterface::Status status) {
                unique_lock<mutex> lock{m_observerMutex};
                auto observers = m_observers;
                lock.unlock();
                for (auto observer : observers) observer->onSendCompleted(status);
            }
            void MessageRequest::exceptionReceived(const string& exceptionMessage) {
                ACSDK_ERROR(LX("onExceptionReceived").d("exception", exceptionMessage));
                unique_lock<mutex> lock{m_observerMutex};
                auto observers = m_observers;
                lock.unlock();
                for (auto observer : observers) observer->onExceptionReceived(exceptionMessage);
            }
            void MessageRequest::addObserver(shared_ptr<MessageRequestObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observerMutex};
                m_observers.insert(observer);
            }
            void MessageRequest::removeObserver(shared_ptr<MessageRequestObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observerMutex};
                m_observers.erase(observer);
            }
            const vector<pair<string, string>>& MessageRequest::getHeaders() const {
                return m_headers;
            }
        }
    }
}