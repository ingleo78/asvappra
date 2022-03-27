#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_AUDIOINJECTORINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_AUDIOINJECTORINTERFACE_H_

#include <avs/AudioInputStream.h>
#include <util/AudioFormat.h>
#include <app_utilities/resource/audio/MicrophoneInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace diagnostics {
                using namespace std;
                using namespace applicationUtilities::resources::audio;
                using namespace alexaClientSDK::avsCommon;
                using namespace avs;
                using namespace utils;
                class AudioInjectorInterface {
                public:
                    virtual shared_ptr<MicrophoneInterface> getMicrophone(const shared_ptr<AudioInputStream>& stream, const AudioFormat& compatibleAudioFormat) = 0;
                    virtual bool injectAudio(const string& filepath);
                    virtual ~AudioInjectorInterface() = default;
                };
            }
        }
    }
}

#endif