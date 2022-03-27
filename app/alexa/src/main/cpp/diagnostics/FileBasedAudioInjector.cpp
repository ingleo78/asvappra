#include <memory>
#include <string>
#include <avs/AudioInputStream.h>
#include "AudioInjectorMicrophone.h"
#include "FileBasedAudioInjector.h"

namespace alexaClientSDK {
    namespace diagnostics {
        using namespace logger;
        static const string TAG("FileBasedAudioInjector");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<MicrophoneInterface> FileBasedAudioInjector::getMicrophone(const shared_ptr<AudioInputStream>& stream, const AudioFormat& compatibleAudioFormat) {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_microphone) {
                m_microphone = AudioInjectorMicrophone::create(stream, compatibleAudioFormat);
                if (!m_microphone) { ACSDK_CRITICAL(LX("Failed to create microphone")); }
            }
            return m_microphone;
        }
        bool FileBasedAudioInjector::injectAudio(const string& filepath) {
            ACSDK_DEBUG5(LX(__func__));
            vector<uint16_t> audioData;
            if (!m_microphone) {
                ACSDK_ERROR(LX("No microphone available for audio injection"));
                return false;
            }
            if (readWavFileToBuffer(filepath, &audioData)) {
                m_microphone->injectAudio(audioData);
                return true;
            }
            return false;
        }
        FileBasedAudioInjector::~FileBasedAudioInjector() {
            if (m_microphone) m_microphone.reset();
        }
    }
}