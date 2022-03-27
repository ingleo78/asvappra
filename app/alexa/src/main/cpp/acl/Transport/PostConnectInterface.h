#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTINTERFACE_H_

#include <memory>
#include <sdkinterfaces/MessageSenderInterface.h>
#include "PostConnectObserverInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        class PostConnectInterface {
        public:
            virtual bool doPostConnect(shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver) = 0;
            virtual void onDisconnect() = 0;
            virtual ~PostConnectInterface() = default;
        };
    }
}
#endif