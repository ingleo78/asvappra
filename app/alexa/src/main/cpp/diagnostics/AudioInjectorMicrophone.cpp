#include <logger/Logger.h>
#include "AudioInjectorMicrophone.h"

namespace alexaClientSDK {
    namespace diagnostics {
        using namespace logger;
        static const string TAG("AudioInjectorMicrophone");
        static const milliseconds TIMEOUT_FOR_WRITING{500};
        static constexpr unsigned int MILLISECONDS_PER_SECOND = 1000;
        static inline unsigned int calculateMaxSampleCountPerTimeout(unsigned int sampleRateHz) {
            return (sampleRateHz / MILLISECONDS_PER_SECOND) * TIMEOUT_FOR_WRITING.count();
        }
        #define LX(event) LogEntry(TAG, event)
        unique_ptr<AudioInjectorMicrophone> AudioInjectorMicrophone::create(
            const shared_ptr<AudioInputStream>& stream,
            const AudioFormat& compatibleAudioFormat) {
            if (!stream) {
                ACSDK_ERROR(LX("createFileBasedMicrophoneFailed").d("reason", "invalid stream"));
                return nullptr;
            }
            unique_ptr<AudioInjectorMicrophone> fileBasedMicrophone(new AudioInjectorMicrophone(stream, compatibleAudioFormat));
            if (!fileBasedMicrophone->initialize()) {
                ACSDK_ERROR(LX("createFileBasedMicrophoneFailed").d("reason", "failed to initialize microphone"));
                return nullptr;
            }
            return fileBasedMicrophone;
        }
        AudioInjectorMicrophone::AudioInjectorMicrophone(const shared_ptr<AudioInputStream>& stream, const AudioFormat& compatibleAudioFormat) :
                                                         m_audioInputStream{move(stream)}, m_isStreaming{false},
                                                         m_maxSampleCountPerTimeout{calculateMaxSampleCountPerTimeout(compatibleAudioFormat.sampleRateHz)},
                                                         m_injectionData{vector<uint16_t>()}, m_injectionDataCounter{0} {
            m_silenceBuffer = AudioInputStream::Buffer(m_maxSampleCountPerTimeout);
            fill(m_silenceBuffer.begin(), m_silenceBuffer.end(), 0);
        }
        AudioInjectorMicrophone::~AudioInjectorMicrophone() {
            lock_guard<mutex> lock(m_mutex);
            m_timer.stop();
        }
        bool AudioInjectorMicrophone::initialize() {
            m_writer = m_audioInputStream->createWriter(AudioInputStream::Writer::Policy::BLOCKING);
            if (!m_writer) {
                ACSDK_ERROR(LX("initializeFileBasedMicrophoneFailed").d("reason", "failed to create stream writer"));
                return false;
            }
            return true;
        }
        bool AudioInjectorMicrophone::isStreaming() {
            ACSDK_DEBUG5(LX(__func__).d("isStreaming", m_isStreaming));
            return m_isStreaming;
        }
        bool AudioInjectorMicrophone::startStreamingMicrophoneData() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_isStreaming = true;
            startTimer();
            return true;
        }
        void AudioInjectorMicrophone::startTimer() {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_timer.isActive()) {
                m_timer.start(milliseconds(0),TIMEOUT_FOR_WRITING,Timer::PeriodType::RELATIVE, Timer::getForever(),
                         bind(&AudioInjectorMicrophone::write, this));
            }
        }
        bool AudioInjectorMicrophone::stopStreamingMicrophoneData() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_isStreaming = false;
            resetAudioInjection();
            m_timer.stop();
            return true;
        }
        void AudioInjectorMicrophone::injectAudio(const vector<uint16_t>& audioData) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_injectionData = audioData;
            m_injectionDataCounter = 0;
        }
        void AudioInjectorMicrophone::write() {
            lock_guard<mutex> lock(m_mutex);
            if (m_isStreaming) {
                if (m_injectionData.empty()) {
                    ssize_t writeResult = m_writer->write(static_cast<void*>(m_silenceBuffer.data()), m_maxSampleCountPerTimeout,
                                                          TIMEOUT_FOR_WRITING);
                    if (writeResult <= 0) {
                        switch(writeResult) {
                            case AudioInputStream::Writer::Error::WOULDBLOCK:
                            case AudioInputStream::Writer::Error::INVALID:
                            case AudioInputStream::Writer::Error::CLOSED:
                                ACSDK_ERROR(LX("writeSilenceFailed").d("reason", "failed to write to stream").d("errorCode", writeResult));
                                break;
                            case AudioInputStream::Writer::Error::TIMEDOUT:
                                ACSDK_DEBUG9(LX("writeSilenceTimedOut"));
                                break;
                        }
                    } else { ACSDK_DEBUG9(LX("writeSilence").d("wordsWritten", writeResult)); }
                } else {
                    if (m_injectionDataCounter >= m_injectionData.size()) {
                        ACSDK_ERROR(LX("injectAudioFailed").d("reason", "bufferOverrun").d("overrun", m_injectionDataCounter - m_injectionData.size()));
                        resetAudioInjection();
                        return;
                    }
                    size_t injectionDataLeft = m_injectionData.size() - m_injectionDataCounter;
                    size_t amountToWrite = (m_maxSampleCountPerTimeout > injectionDataLeft) ? injectionDataLeft : m_maxSampleCountPerTimeout;
                    ssize_t writeResult = m_writer->write(static_cast<void*>(m_injectionData.data() + m_injectionDataCounter),
                                                          amountToWrite, TIMEOUT_FOR_WRITING);

                    if (writeResult <= 0) {
                        switch(writeResult) {
                            case AudioInputStream::Writer::Error::WOULDBLOCK:
                            case AudioInputStream::Writer::Error::INVALID:
                            case AudioInputStream::Writer::Error::CLOSED:
                                ACSDK_ERROR(LX("injectAudioFailed").d("error", writeResult));
                                resetAudioInjection();
                                break;
                            case AudioInputStream::Writer::Error::TIMEDOUT:
                                ACSDK_DEBUG9(LX("injectAudioTimedOut"));
                                break;
                        }
                    } else {
                        ACSDK_DEBUG9(LX("injectAudio").d("wordsWritten", writeResult));
                        m_injectionDataCounter += writeResult;
                        if (m_injectionDataCounter == m_injectionData.size()) resetAudioInjection();
                    }
                }
            }
        }
        void AudioInjectorMicrophone::resetAudioInjection() {
            m_injectionData.clear();
            m_injectionDataCounter = 0;
        }
    }
}