#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TOGGLECONTROLLER_TOGGLECONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TOGGLECONTROLLER_TOGGLECONTROLLERINTERFACE_H_

#include <avs/AlexaResponseType.h>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include <sdkinterfaces/ToggleController/ToggleControllerObserverInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace toggleController {
                using namespace std;
                using namespace avs;
                using namespace utils;
                class ToggleControllerInterface {
                public:
                    using ToggleState = ToggleControllerObserverInterface::ToggleState;
                    virtual ~ToggleControllerInterface() = default;
                    virtual pair<AlexaResponseType, string> setToggleState(bool state, AlexaStateChangeCauseType cause);
                    virtual pair<AlexaResponseType, Optional<ToggleState>> getToggleState();
                    virtual bool addObserver(shared_ptr<ToggleControllerObserverInterface> observer);
                    virtual void removeObserver(const shared_ptr<ToggleControllerObserverInterface>& observer);
                };
            }
        }
    }
}
#endif