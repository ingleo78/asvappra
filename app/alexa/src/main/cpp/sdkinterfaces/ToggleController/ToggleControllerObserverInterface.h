#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TOGGLECONTROLLER_TOGGLECONTROLLEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TOGGLECONTROLLER_TOGGLECONTROLLEROBSERVERINTERFACE_H_

#include <chrono>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include <timing/TimePoint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace toggleController {
                using namespace std;
                using namespace chrono;
                using namespace utils;
                using namespace timing;
                class ToggleControllerObserverInterface {
                public:
                    struct ToggleState {
                        bool toggleState;
                        TimePoint timeOfSample;
                        milliseconds valueUncertainty;
                    };
                    virtual ~ToggleControllerObserverInterface() = default;
                    virtual void onToggleStateChanged(const ToggleState& toggleState, AlexaStateChangeCauseType cause);
                };
            }
        }
    }
}
#endif