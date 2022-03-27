#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_KEYWORDDETECTORSTATEOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_KEYWORDDETECTORSTATEOBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class KeyWordDetectorStateObserverInterface {
            public:
                enum class KeyWordDetectorState {
                    ACTIVE,
                    STREAM_CLOSED,
                    ERROR
                };
                virtual ~KeyWordDetectorStateObserverInterface() = default;
                virtual void onStateChanged(KeyWordDetectorState keyWordDetectorState) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_KEYWORDDETECTORSTATEOBSERVERINTERFACE_H_
