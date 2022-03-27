#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMCLOCKMONITOROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMCLOCKMONITOROBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class SystemClockMonitorObserverInterface {
            public:
                virtual ~SystemClockMonitorObserverInterface() = default;
                virtual void onSystemClockSynchronized() = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMCLOCKMONITOROBSERVERINTERFACE_H_
