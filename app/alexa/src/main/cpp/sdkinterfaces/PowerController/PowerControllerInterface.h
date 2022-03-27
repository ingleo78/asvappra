#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POWERCONTROLLER_POWERCONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_POWERCONTROLLER_POWERCONTROLLERINTERFACE_H_

#include <avs/AlexaResponseType.h>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include <sdkinterfaces/PowerController/PowerControllerObserverInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace powerController {
                using namespace std;
                using namespace avs;
                using namespace utils;
                class PowerControllerInterface {
                public:
                    using PowerState = PowerControllerObserverInterface::PowerState;
                    virtual ~PowerControllerInterface() = default;
                    virtual pair<AlexaResponseType, string> setPowerState(bool powerState, AlexaStateChangeCauseType cause);
                    virtual pair<AlexaResponseType, Optional<PowerState>> getPowerState();
                    virtual bool addObserver(shared_ptr<PowerControllerObserverInterface> observer);
                    virtual void removeObserver(const shared_ptr<PowerControllerObserverInterface>& observer);
                };
            }
        }
    }
}
#endif