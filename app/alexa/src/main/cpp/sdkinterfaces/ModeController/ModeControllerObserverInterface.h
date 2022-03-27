#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLEROBSERVERINTERFACE_H_

#include <chrono>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include <timing/TimePoint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace modeController {
                class ModeControllerObserverInterface {
                public:
                    struct ModeState {
                        std::string mode;
                        avsCommon::utils::timing::TimePoint timeOfSample;
                        std::chrono::milliseconds valueUncertainty;
                    };
                    virtual ~ModeControllerObserverInterface() = default;
                    virtual void onModeChanged(const ModeState& mode, AlexaStateChangeCauseType cause);
                };
            }
        }
    }
}
#endif