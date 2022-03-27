#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTINTERFACE_H_

#include <memory>
#include <avs/MessageRequest.h>
#include <util/RequiresShutdown.h>

namespace alexaClientSDK {
    namespace acl {
        class TransportInterface : public avsCommon::utils::RequiresShutdown {
        public:
            TransportInterface();
            virtual bool connect() = 0;
            virtual void disconnect() = 0;
            virtual bool isConnected() = 0;
            virtual void onRequestEnqueued() = 0;
            virtual void onWakeConnectionRetry() = 0;
            virtual void onWakeVerifyConnectivity() = 0;
            TransportInterface(const TransportInterface& rhs) = delete;
            TransportInterface& operator=(const TransportInterface& rhs) = delete;
            virtual ~TransportInterface() = default;
        };
        inline TransportInterface::TransportInterface() : RequiresShutdown{"TransportInterface"} {}
    }
}
#endif