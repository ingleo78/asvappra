#ifndef ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_AUDIOINJECTORMICROPHONE_H_
#define ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_AUDIOINJECTORMICROPHONE_H_

#include <memory>
#include <mutex>
#include <thread>
#include <avs/AudioInputStream.h>
#include <util/AudioFormat.h>
#include <timing/Timer.h>
#include <app_utilities/resource/audio/MicrophoneInterface.h>
#include "DiagnosticsUtils.h"

namespace alexaClientSDK {
    namespace diagnostics {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace avsCommon::utils;
        using namespace applicationUtilities;
        using namespace resources;
        using namespace audio;
        using namespace timing;
        using AudioFormat = avsCommon::utils::AudioFormat;
        class AudioInjectorMicrophone : public MicrophoneInterface {
        public:
            static unique_ptr<AudioInjectorMicrophone> create(const shared_ptr<AudioInputStream>& stream, const AudioFormat& compatibleAudioFormat);
            bool stopStreamingMicrophoneData() override;
            bool startStreamingMicrophoneData() override;
            bool isStreaming() override;
            void injectAudio(const vector<uint16_t>& audioData);
            ~AudioInjectorMicrophone();
        private:
            AudioInjectorMicrophone(const shared_ptr<AudioInputStream>& stream, const AudioFormat& compatibleAudioFormat);
            void resetAudioInjection();
            bool initialize();
            void startTimer();
            void write();
            const shared_ptr<AudioInputStream> m_audioInputStream;
            mutex m_mutex;
            shared_ptr<AudioInputStream::Writer> m_writer;
            bool m_isStreaming;
            Timer m_timer;
            unsigned int m_maxSampleCountPerTimeout;
            AudioInputStream::Buffer m_silenceBuffer;
            vector<uint16_t> m_injectionData;
            unsigned long m_injectionDataCounter;
        };
    }
}
#endif