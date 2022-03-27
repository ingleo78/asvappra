#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RANGECONTROLLER_RANGECONTROLLERINTERFACE_H_

#include <avs/AlexaResponseType.h>
#include <sdkinterfaces/AlexaStateChangeCauseType.h>
#include <sdkinterfaces/RangeController/RangeControllerObserverInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace rangeController {
                using namespace std;
                using namespace avs;
                using namespace utils;
                class RangeControllerInterface {
                public:
                    using RangeState = RangeControllerObserverInterface::RangeState;
                    virtual ~RangeControllerInterface() = default;
                    struct RangeControllerConfiguration {
                        double minimumValue;
                        double maximumValue;
                        double precision;
                    };
                    virtual RangeControllerConfiguration getConfiguration();
                    virtual pair<AlexaResponseType, string> setRangeValue(double range, AlexaStateChangeCauseType cause);
                    virtual pair<AlexaResponseType, string> adjustRangeValue(double deltaRange, AlexaStateChangeCauseType cause);
                    virtual pair<AlexaResponseType, Optional<RangeState>> getRangeState();
                    virtual bool addObserver(shared_ptr<RangeControllerObserverInterface> observer);
                    virtual void removeObserver(const shared_ptr<RangeControllerObserverInterface>& observer);
                };
            }
        }
    }
}
#endif