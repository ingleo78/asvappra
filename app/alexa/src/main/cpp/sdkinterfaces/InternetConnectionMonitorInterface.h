#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_INTERNETCONNECTIONMONITORINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_INTERNETCONNECTIONMONITORINTERFACE_H_

#include <memory>
#include "InternetConnectionObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class InternetConnectionMonitorInterface {
            public:
                virtual void addInternetConnectionObserver(std::shared_ptr<InternetConnectionObserverInterface> observer);
                virtual void removeInternetConnectionObserver(std::shared_ptr<InternetConnectionObserverInterface> observer);
                virtual ~InternetConnectionMonitorInterface() = default;
            };
        }
    }
}
#endif