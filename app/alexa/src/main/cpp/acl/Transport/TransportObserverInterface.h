#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTOBSERVERINTERFACE_H_

#include <memory>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include "TransportInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon::sdkInterfaces;
        class TransportObserverInterface {
        public:
            virtual ~TransportObserverInterface() = default;
            virtual void onConnected(shared_ptr<TransportInterface> transport) = 0;
            virtual void onDisconnected(shared_ptr<TransportInterface> transport, ConnectionStatusObserverInterface::ChangedReason reason) = 0;
            virtual void onServerSideDisconnect(shared_ptr<TransportInterface> transport) = 0;
        };
    }
}
#endif