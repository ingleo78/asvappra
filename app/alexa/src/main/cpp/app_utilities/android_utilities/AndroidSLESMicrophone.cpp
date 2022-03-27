#include <SLES/OpenSLES_Android.h>
#include "AndroidSLESMicrophone.h"
#include "AndroidSLESEngine.h"

static const std::string TAG{"AndroidMicrophone"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            using namespace std;
            using namespace avsCommon::avs;
            unique_ptr<AndroidSLESMicrophone> AndroidSLESMicrophone::create(shared_ptr<AndroidSLESEngine> engine, shared_ptr<AudioInputStream> stream) {
                shared_ptr<AudioInputStream::Writer> writer = stream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
                if (!writer) {
                    ACSDK_ERROR(LX("createAndroidSLESMicrophoneFailed").d("reason", "failed to create writer"));
                    return nullptr;
                }
                return unique_ptr<AndroidSLESMicrophone>(new AndroidSLESMicrophone(engine, writer));
            }
            AndroidSLESMicrophone::AndroidSLESMicrophone(shared_ptr<AndroidSLESEngine> engine, shared_ptr<AudioInputStream::Writer> writer) :
                                                         m_engineObject{engine}, m_writer{writer}, m_isStreaming{false} {}
            SLDataSink AndroidSLESMicrophone::createSinkConfiguration() {
                static SLDataLocator_AndroidSimpleBufferQueue loc_bq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, AndroidSLESBufferQueue::NUMBER_OF_BUFFERS };
                static SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM,1, SL_SAMPLINGRATE_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                                       SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
                return SLDataSink{&loc_bq, &format_pcm};
            }
            SLDataSource AndroidSLESMicrophone::createSourceConfiguration() {
                static SLDataLocator_IODevice loc_dev = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT,nullptr };
                return SLDataSource{&loc_dev, nullptr};
            }
            AndroidSLESMicrophone::~AndroidSLESMicrophone() {
                stopStreamingMicrophoneData();
            }
            bool AndroidSLESMicrophone::configureRecognizeMode() {
                SLAndroidConfigurationItf configurationInterface;
                if (!m_recorderObject) {
                    ACSDK_ERROR(LX("configureRecognizeModeFailed").d("reason", "recorderObjectUnavailable"));
                    return false;
                }
                if (!m_recorderObject->getInterface(SL_IID_RECORD, &configurationInterface) || !configurationInterface) {
                    ACSDK_WARN(LX("configureRecognizeModeFailed").d("reason", "configurationInterfaceUnavailable")
                                     .d("configuration", configurationInterface));
                    return false;
                }
                auto presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
                auto result = (*configurationInterface)->SetConfiguration(configurationInterface, SL_ANDROID_KEY_RECORDING_PRESET, &presetValue, sizeof(SLuint32));
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_WARN(LX("configureRecognizeModeFailed").d("reason", "cannot set configuration").d("result", result));
                    return false;
                }
                return true;
            }
            bool AndroidSLESMicrophone::isStreaming() {
                return m_isStreaming;
            }
            bool AndroidSLESMicrophone::startStreamingMicrophoneData() {
                ACSDK_INFO(LX(__func__));
                lock_guard<std::mutex> lock{m_mutex};
                m_recorderObject = m_engineObject->createAudioRecorder();
                if (!m_recorderObject) {
                    ACSDK_ERROR(LX("startStreamingFailed").d("reason", "Failed to create recorder."));
                    return false;
                }
                if (!m_recorderObject->getInterface(SL_IID_RECORD, &m_recorderInterface)) {
                    ACSDK_ERROR(LX("startStreamingFailed").d("reason", "Failed to get recorder interface."));
                    return false;
                }
                m_queue = AndroidSLESBufferQueue::create(m_recorderObject, m_writer);
                if (!m_queue) {
                    ACSDK_ERROR(LX("startStreamingFailed").d("reason", "Failed to create buffer queue."));
                    return false;
                }
                if (!configureRecognizeMode()) {
                    ACSDK_WARN(LX("Failed to set Recognize mode. This might affect the voice recognition."));
                }
                if (!m_queue->enqueueBuffers()) {
                    ACSDK_ERROR(LX("startStreamingFailed").d("reason", "Failed to enqueue buffers."));
                    return false;
                }
                auto result = (*m_recorderInterface)->SetRecordState(m_recorderInterface, SL_RECORDSTATE_RECORDING);
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("startStreamingFailed").d("reason", "failed to set state").d("result", result));
                    return false;
                }
                m_isStreaming = true;
                return true;
            }
            bool AndroidSLESMicrophone::stopStreamingMicrophoneData() {
                ACSDK_INFO(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                if (m_recorderObject && m_recorderInterface) {
                    auto result = (*m_recorderInterface)->SetRecordState(m_recorderInterface, SL_RECORDSTATE_STOPPED);
                    if (result != SL_RESULT_SUCCESS) {
                        ACSDK_ERROR(LX("stopStreamingFailed").d("result", result));
                        return false;
                    }
                    m_recorderObject.reset();
                    m_recorderInterface = nullptr;
                    m_queue.reset();
                    m_isStreaming = false;
                    return true;
                } else {
                    ACSDK_ERROR(LX("stopStreamingFailed").d("reason", "Recorder object or interface not available."));
                    return false;
                }
            }
        }
    }
}