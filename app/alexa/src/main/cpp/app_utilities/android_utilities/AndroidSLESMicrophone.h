#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESMICROPHONE_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESMICROPHONE_H_

#include <memory>
#include <SLES/OpenSLES.h>
#include <app_utilities/resource/audio/MicrophoneInterface.h>
#include "AndroidSLESBufferQueue.h"
#include "AndroidSLESEngine.h"
#include "AndroidSLESObject.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            using namespace std;
            using namespace resources::audio;
            using namespace avsCommon::avs;
            class AndroidSLESBufferQueue;
            class AndroidSLESMicrophone : public MicrophoneInterface {
            public:
                static unique_ptr<AndroidSLESMicrophone> create(shared_ptr<AndroidSLESEngine> engine, shared_ptr<AudioInputStream> stream);
                bool configureRecognizeMode();
                bool stopStreamingMicrophoneData() override;
                bool startStreamingMicrophoneData() override;
                bool isStreaming() override;
                static SLDataSink createSinkConfiguration();
                static SLDataSource createSourceConfiguration();
                ~AndroidSLESMicrophone();
            private:
                AndroidSLESMicrophone(shared_ptr<AndroidSLESEngine> engine, shared_ptr<AudioInputStream::Writer> writer);
                shared_ptr<AndroidSLESEngine> m_engineObject;
                shared_ptr<AudioInputStream::Writer> m_writer;
                shared_ptr<AndroidSLESObject> m_recorderObject;
                SLRecordItf m_recorderInterface;
                unique_ptr<AndroidSLESBufferQueue> m_queue;
                mutex m_mutex;
                bool m_isStreaming;
            };
        }
    }
}
#endif