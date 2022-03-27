#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_SYSTEMSOUNDAUDIOFACTORY_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_SYSTEMSOUNDAUDIOFACTORY_H_

#include <sdkinterfaces/Audio/SystemSoundAudioFactoryInterface.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace avsCommon;
                using namespace utils;
                using namespace sdkInterfaces::audio;
                class SystemSoundAudioFactory : public SystemSoundAudioFactoryInterface {
                public:
                    function<pair<unique_ptr<istream>, const MediaType>()> endSpeechTone() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> wakeWordNotificationTone() const override;
                };
            }
        }
    }
}
#endif