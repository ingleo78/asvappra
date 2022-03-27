#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_NOTIFICATIONSAUDIOFACTORY_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_NOTIFICATIONSAUDIOFACTORY_H_

#include <sdkinterfaces/Audio/NotificationsAudioFactoryInterface.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace avsCommon;
                using namespace utils;
                using namespace sdkInterfaces::audio;
                class NotificationsAudioFactory : public NotificationsAudioFactoryInterface {
                public:
                    function<pair<unique_ptr<istream>, const MediaType>()> notificationDefault() const override;
                };
            }
        }
    }
}
#endif