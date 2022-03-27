#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_MICROPHONEINTERFACE_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_MICROPHONEINTERFACE_H_

#include <mutex>
#include <thread>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                class MicrophoneInterface {
                public:
                    virtual bool stopStreamingMicrophoneData();
                    virtual bool startStreamingMicrophoneData();
                    virtual bool isStreaming();
                    virtual ~MicrophoneInterface() = default;
                };
            }
        }
    }
}
#endif