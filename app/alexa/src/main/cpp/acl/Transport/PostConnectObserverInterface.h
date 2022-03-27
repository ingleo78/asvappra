#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTOBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace acl {
        class PostConnectObserverInterface {
        public:
            virtual ~PostConnectObserverInterface() = default;
            virtual void onPostConnected() = 0;
            virtual void onUnRecoverablePostConnectFailure() = 0;
        };
    }
}
#endif