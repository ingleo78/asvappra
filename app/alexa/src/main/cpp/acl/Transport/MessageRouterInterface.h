#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTERINTERFACE_H_

#include <memory>
#include <string>
#include <utility>
#include <threading/Executor.h>
#include <util/RequiresShutdown.h>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include "MessageRouterObserverInterface.h"
#include "TransportInterface.h"
#include "TransportObserverInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace sdkInterfaces;
        class MessageRouterInterface : public MessageSenderInterface, public RequiresShutdown {
        public:
            using ConnectionStatus = pair<ConnectionStatusObserverInterface::Status, ConnectionStatusObserverInterface::ChangedReason>;
            MessageRouterInterface(const string& name);
            virtual void enable() = 0;
            virtual void disable() = 0;
            virtual ConnectionStatus getConnectionStatus() = 0;
            virtual void setAVSGateway(const string& avsGateway) = 0;
            virtual string getAVSGateway() = 0;
            virtual void onWakeConnectionRetry() = 0;
            virtual void onWakeVerifyConnectivity() = 0;
            virtual void setObserver(shared_ptr<MessageRouterObserverInterface> observer) = 0;
            virtual ~MessageRouterInterface() = default;
        };
        inline MessageRouterInterface::MessageRouterInterface(const string& name) : RequiresShutdown(name) {}
    }
}
#endif