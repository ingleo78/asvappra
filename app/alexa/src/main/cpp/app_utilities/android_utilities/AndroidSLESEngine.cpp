#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <avs/AudioInputStream.h>
#include "AndroidSLESEngine.h"
#include "AndroidSLESMicrophone.h"

static const std::string TAG{"AndroidSLESEngine"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            using namespace std;
            using namespace avsCommon::avs;
            atomic_flag AndroidSLESEngine::m_created = ATOMIC_FLAG_INIT;
            shared_ptr<AndroidSLESEngine> AndroidSLESEngine::create() {
                if (!m_created.test_and_set()) {
                    SLObjectItf slObjectItf;
                    auto result = slCreateEngine(&slObjectItf, 0, nullptr, 0, nullptr, nullptr);
                    if (result != SL_RESULT_SUCCESS) {
                        ACSDK_ERROR(LX("createAndroidSLESEngineFailed").d("result", result));
                        m_created.clear();
                        return nullptr;
                    }
                    auto engineObject = AndroidSLESObject::create(slObjectItf);
                    if (!engineObject) {
                        ACSDK_ERROR(LX("createAndroidSLESEngineFailed"));
                        m_created.clear();
                        return nullptr;
                    }
                    SLEngineItf engineInterface;
                    if (!engineObject->getInterface(SL_IID_ENGINE, &engineInterface)) {
                        ACSDK_ERROR(LX("createRecorderFailed").d("reason", "failed to get engine interface"));
                        m_created.clear();
                        return nullptr;
                    }

                    return shared_ptr<AndroidSLESEngine>(new AndroidSLESEngine(std::move(engineObject), engineInterface));
                }
                ACSDK_ERROR(LX("createEngineFailed").d("reason", "singleton engine has been created already"));
                return nullptr;
            }
            AndroidSLESEngine::AndroidSLESEngine(std::unique_ptr<AndroidSLESObject> object, SLEngineItf engine) : m_object{std::move(object)}, m_engine{engine} {}
            AndroidSLESEngine::~AndroidSLESEngine() {
                m_created.clear();
            }
            unique_ptr<AndroidSLESMicrophone> AndroidSLESEngine::createAndroidMicrophone(shared_ptr<AudioInputStream> stream) {
                auto androidRecorder = AndroidSLESMicrophone::create(shared_from_this(), stream);
                return androidRecorder;
            }
            unique_ptr<AndroidSLESObject> AndroidSLESEngine::createAudioRecorder() {
                auto audioSink = AndroidSLESMicrophone::createSinkConfiguration();
                auto audioSource = AndroidSLESMicrophone::createSourceConfiguration();
                constexpr uint32_t interfaceSize = 1;
                const SLInterfaceID interfaceIDs[interfaceSize] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
                const SLboolean requiredInterfaces[interfaceSize] = {SL_BOOLEAN_TRUE};
                SLObjectItf recorderObject;
                (*m_engine)->CreateAudioRecorder(m_engine, &recorderObject, &audioSource, &audioSink, interfaceSize, interfaceIDs, requiredInterfaces);
                return AndroidSLESObject::create(recorderObject);
            }
            unique_ptr<AndroidSLESObject> AndroidSLESEngine::createOutputMix() const {
                SLObjectItf outputMixObject;
                auto result = (*m_engine)->CreateOutputMix(m_engine, &outputMixObject, 0, nullptr, nullptr);
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("createPlayerFailed").d("reason", "Failed to create output mix.").d("result", result));
                    return nullptr;
                }
                return AndroidSLESObject::create(outputMixObject);
            }
            unique_ptr<AndroidSLESObject> AndroidSLESEngine::createPlayer(SLDataSource& source, SLDataSink& sink, bool requireEqualizer) const {
                constexpr uint32_t interfaceSize = 4;
                const SLInterfaceID interfaceIds[interfaceSize] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PREFETCHSTATUS, SL_IID_EQUALIZER };
                const SLboolean requiredInterfaces[interfaceSize] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE,
                                                                      requireEqualizer ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE };
                SLObjectItf playerObject;
                auto result = (*m_engine)->CreateAudioPlayer(m_engine, &playerObject, &source, &sink, interfaceSize, interfaceIds, requiredInterfaces);
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "createAudioPlayerFailed"));
                    return nullptr;
                }
                return AndroidSLESObject::create(playerObject);
            }
        }
    }
}