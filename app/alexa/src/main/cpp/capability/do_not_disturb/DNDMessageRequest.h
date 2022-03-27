#ifndef ACSDKDONOTDISTURB_DNDMESSAGEREQUEST_H_
#define ACSDKDONOTDISTURB_DNDMESSAGEREQUEST_H_

#include <future>
#include <string>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace doNotDisturb {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            class DNDMessageRequest : public MessageRequest {
            public:
                DNDMessageRequest(const string& jsonContent);
                ~DNDMessageRequest() override;
                void sendCompleted(MessageRequestObserverInterface::Status status) override;
                shared_future<MessageRequestObserverInterface::Status> getCompletionFuture();
            private:
                promise<MessageRequestObserverInterface::Status> m_promise;
                shared_future<MessageRequestObserverInterface::Status> m_future;
                bool m_isCompleted;
            };
        }
    }
}
#endif