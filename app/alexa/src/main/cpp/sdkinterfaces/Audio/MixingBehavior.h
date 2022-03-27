#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_MIXINGBEHAVIOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_MIXINGBEHAVIOR_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                enum class MixingBehavior {
                    BEHAVIOR_PAUSE,
                    BEHAVIOR_DUCK,
                };
            }
        }
    }
}
#endif