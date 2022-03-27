#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSCONNECTIONMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AVSCONNECTIONMANAGERINTERFACE_H_

#include <memory>
#include "ConnectionStatusObserverInterface.h"
#include "MessageObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AVSConnectionManagerInterface {
            public:
                using ConnectionStatusObserverInterface = avsCommon::sdkInterfaces::ConnectionStatusObserverInterface;
                virtual ~AVSConnectionManagerInterface() = default;
                virtual void enable();
                virtual void disable();
                virtual bool isEnabled();
                virtual void reconnect();
                virtual bool isConnected() const;
                virtual void onWakeConnectionRetry();
                virtual void addMessageObserver(std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> observer);
                virtual void removeMessageObserver(std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> observer);
                virtual void addConnectionStatusObserver(std::shared_ptr<ConnectionStatusObserverInterface> observer);
                virtual void removeConnectionStatusObserver(std::shared_ptr<ConnectionStatusObserverInterface> observer);
            };
        }
    }
}
#endif