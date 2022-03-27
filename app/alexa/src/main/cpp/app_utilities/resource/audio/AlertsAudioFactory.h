#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_ALERTSAUDIOFACTORY_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_ALERTSAUDIOFACTORY_H_

#include <sdkinterfaces/Audio/AlertsAudioFactoryInterface.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace avsCommon;
                using namespace sdkInterfaces::audio;
                using namespace utils;
                class AlertsAudioFactory : public AlertsAudioFactoryInterface {
                public:
                    function<pair<unique_ptr<istream>, const MediaType>()> alarmDefault() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> alarmShort() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> timerDefault() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> timerShort() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> reminderDefault() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> reminderShort() const override;
                };
            }
        }
    }
}
#endif