#include <logger/Logger.h>
#include "DNDMessageRequest.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace doNotDisturb {
            using namespace logger;
            static const string TAG("DNDMessageRequest");
            #define LX(event) LogEntry(TAG, event)
            DNDMessageRequest::DNDMessageRequest(const string& jsonContent) : MessageRequest(jsonContent, ""), m_isCompleted{false} {
                m_future = m_promise.get_future();
            }
            void DNDMessageRequest::sendCompleted(MessageRequestObserverInterface::Status status) {
                MessageRequest::sendCompleted(status);
                if (!m_isCompleted) {
                    ACSDK_DEBUG9(LX(__func__).d("Completed with status", status));
                    m_promise.set_value(status);
                } else { ACSDK_ERROR(LX("sendCompletedFailed").d("reason", "sendCompleted must be called only once.")); }
                m_isCompleted = true;
            }
            shared_future<MessageRequestObserverInterface::Status> DNDMessageRequest::getCompletionFuture() {
                return m_future;
            }
            DNDMessageRequest::~DNDMessageRequest() {
                if (!m_isCompleted) {
                    ACSDK_WARN(LX(__func__).m("Destroying while message delivery has not been completed yet."));
                    m_promise.set_value(MessageRequestObserverInterface::Status::CANCELED);
                }
            }
        }
    }
}