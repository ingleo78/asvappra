#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MODECONTROLLER_MODECONTROLLERINTERFACE_H_

#include <util/Optional.h>
#include <avs/AlexaResponseType.h>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include "ModeControllerObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace modeController {
                using namespace std;
                using namespace avs;
                using namespace utils;
                class ModeControllerInterface {
                public:
                    using ModeState = ModeControllerObserverInterface::ModeState;
                    virtual ~ModeControllerInterface() = default;
                    using ModeControllerConfiguration = vector<string>;
                    virtual ModeControllerConfiguration getConfiguration();
                    virtual pair<AlexaResponseType, string> setMode(const string& mode, AlexaStateChangeCauseType cause);
                    virtual pair<AlexaResponseType, string> adjustMode(int modeDelta, AlexaStateChangeCauseType cause);
                    virtual pair<AlexaResponseType, Optional<ModeState>> getMode();
                    virtual bool addObserver(shared_ptr<ModeControllerObserverInterface> observer);
                    virtual void removeObserver(const shared_ptr<ModeControllerObserverInterface>& observer);
                };
            }
        }
    }
}
#endif