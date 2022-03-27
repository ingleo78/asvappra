#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESENGINE_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESENGINE_H_

#include <SLES/OpenSLES.h>
#include <memory>
#include <avs/AudioInputStream.h>
#include "AndroidSLESObject.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            class AndroidSLESMicrophone;
            class AndroidSLESEngine : public std::enable_shared_from_this<AndroidSLESEngine> {
            public:
                static std::shared_ptr<AndroidSLESEngine> create();
                std::unique_ptr<AndroidSLESMicrophone> createAndroidMicrophone(std::shared_ptr<avsCommon::avs::AudioInputStream> stream);
                std::unique_ptr<AndroidSLESObject> createAudioRecorder();
                void createAudioRecorder(SLObjectItf& recorderObject);
                std::unique_ptr<AndroidSLESObject> createPlayer(SLDataSource& source, SLDataSink& sink, bool requireEqualizer) const;
                std::unique_ptr<AndroidSLESObject> createOutputMix() const;
                ~AndroidSLESEngine();
            private:
                AndroidSLESEngine(std::unique_ptr<AndroidSLESObject> object, SLEngineItf engine);
                std::unique_ptr<AndroidSLESObject> m_object;
                SLEngineItf m_engine;
                static std::atomic_flag m_created;
            };
        }
    }
}
#endif