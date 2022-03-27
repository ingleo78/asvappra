#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_STATESYNCHRONIZEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_STATESYNCHRONIZEROBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class StateSynchronizerObserverInterface {
            public:
                enum class State {
                    NOT_SYNCHRONIZED,
                    SYNCHRONIZED
                };
                virtual ~StateSynchronizerObserverInterface() = default;
                virtual void onStateChanged(State newState) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_STATESYNCHRONIZEROBSERVERINTERFACE_H_
