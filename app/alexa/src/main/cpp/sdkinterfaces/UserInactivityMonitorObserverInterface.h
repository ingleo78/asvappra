#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_USERINACTIVITYMONITOROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_USERINACTIVITYMONITOROBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class UserInactivityMonitorObserverInterface {
            public:
                virtual ~UserInactivityMonitorObserverInterface() = default;
                virtual void onUserInactivityReportSent();
            };
        }
    }
}
#endif