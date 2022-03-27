#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_WAITABLEMESSAGEREQUEST_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_WAITABLEMESSAGEREQUEST_H_

#include <condition_variable>
#include <mutex>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include "MessageRequest.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            class WaitableMessageRequest : public MessageRequest {
            public:
                WaitableMessageRequest(const std::string& jsonContent);
                void sendCompleted(avsCommon::sdkInterfaces::MessageRequestObserverInterface::Status sendMessageStatus);
                avsCommon::sdkInterfaces::MessageRequestObserverInterface::Status waitForCompletion();
                void shutdown();
            private:
                avsCommon::sdkInterfaces::MessageRequestObserverInterface::Status m_sendMessageStatus;
                bool m_responseReceived;
                std::mutex m_requestMutex;
                std::condition_variable m_requestCv;
                bool m_isRequestShuttingDown;
            };
        }
    }
}
#endif