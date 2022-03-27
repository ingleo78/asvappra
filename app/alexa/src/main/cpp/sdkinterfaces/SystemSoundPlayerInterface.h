#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMSOUNDPLAYERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMSOUNDPLAYERINTERFACE_H_

#include <future>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class SystemSoundPlayerInterface {
            public:
                virtual ~SystemSoundPlayerInterface() = default;
                enum class Tone {
                    WAKEWORD_NOTIFICATION,
                    END_SPEECH
                };
                virtual std::shared_future<bool> playTone(Tone tone) = 0;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SYSTEMSOUNDPLAYERINTERFACE_H_
