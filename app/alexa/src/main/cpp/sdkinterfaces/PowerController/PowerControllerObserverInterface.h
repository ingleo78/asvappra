#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POWERCONTROLLER_POWERCONTROLLEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POWERCONTROLLER_POWERCONTROLLEROBSERVERINTERFACE_H_

#include <chrono>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include <timing/TimePoint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace powerController {
                class PowerControllerObserverInterface {
                public:
                    struct PowerState {
                        bool powerState;
                        avsCommon::utils::timing::TimePoint timeOfSample;
                        std::chrono::milliseconds valueUncertainty;
                    };
                    virtual ~PowerControllerObserverInterface() = default;
                    virtual void onPowerStateChanged(const PowerState& powerState, AlexaStateChangeCauseType cause);
                };
            }
        }
    }
}
#endif