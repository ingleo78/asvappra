#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLEROBSERVERINTERFACE_H_

#include <chrono>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include <timing/TimePoint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace rangeController {
                class RangeControllerObserverInterface {
                public:
                    virtual ~RangeControllerObserverInterface() = default;
                    struct RangeState {
                        double value;
                        avsCommon::utils::timing::TimePoint timeOfSample;
                        std::chrono::milliseconds valueUncertainty;
                    };
                    virtual void onRangeChanged(const RangeState& rangeState, AlexaStateChangeCauseType cause);
                };
            }
        }
    }
}
#endif