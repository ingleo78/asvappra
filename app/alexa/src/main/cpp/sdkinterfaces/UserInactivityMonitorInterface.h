#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_USERINACTIVITYMONITORINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_USERINACTIVITYMONITORINTERFACE_H_

#include <chrono>
#include <memory>
#include "UserInactivityMonitorObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class UserInactivityMonitorInterface {
            public:
                virtual ~UserInactivityMonitorInterface() = default;
                virtual void onUserActive();
                virtual std::chrono::seconds timeSinceUserActivity();
                virtual void addObserver(
                    std::shared_ptr<avsCommon::sdkInterfaces::UserInactivityMonitorObserverInterface> observer);
                virtual void removeObserver(
                    std::shared_ptr<avsCommon::sdkInterfaces::UserInactivityMonitorObserverInterface> observer);
            };
        }
    }
}
#endif