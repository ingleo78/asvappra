#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTEROBSERVERINTERFACE_H_

#include <memory>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon::sdkInterfaces;
        class MessageRouterObserverInterface {
        public:
            virtual ~MessageRouterObserverInterface() = default;
            virtual void onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status,
                                                   const ConnectionStatusObserverInterface::ChangedReason reason) = 0;
            virtual void receive(const string& contextId, const string& message) = 0;
        };
    }
}
#endif