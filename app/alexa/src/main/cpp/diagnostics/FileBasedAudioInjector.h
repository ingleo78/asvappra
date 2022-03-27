#ifndef ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_FILEBASEDAUDIOINJECTOR_H_
#define ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_FILEBASEDAUDIOINJECTOR_H_

#include <memory>
#include <string>
#include <sdkinterfaces/Diagnostics/AudioInjectorInterface.h>
#include <app_utilities/resource/audio/MicrophoneInterface.h>
#include "AudioInjectorMicrophone.h"
#include "DiagnosticsUtils.h"

namespace alexaClientSDK {
    namespace diagnostics {
        using namespace std;
        using namespace applicationUtilities;
        using namespace audio;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace sdkInterfaces::diagnostics;
        class FileBasedAudioInjector : public AudioInjectorInterface {
        public:
            ~FileBasedAudioInjector();
            shared_ptr<MicrophoneInterface> getMicrophone(const shared_ptr<AudioInputStream>& stream, const AudioFormat& compatibleAudioFormat) override;
            bool injectAudio(const string& filepath) override;
        private:
            shared_ptr<AudioInjectorMicrophone> m_microphone;
        };
    }
}
#endif