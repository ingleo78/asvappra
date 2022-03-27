#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_INTERNETCONNECTIONOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_INTERNETCONNECTIONOBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class InternetConnectionObserverInterface {
            public:
                virtual ~InternetConnectionObserverInterface() = default;
                virtual void onConnectionStatusChanged(bool connected) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_INTERNETCONNECTIONOBSERVERINTERFACE_H_
