#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_AUDIOFACTORY_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_AUDIOFACTORY_H_

#include <sdkinterfaces/Audio/AudioFactoryInterface.h>
#include <sdkinterfaces/Audio/AlertsAudioFactoryInterface.h>
#include <sdkinterfaces/Audio/CommunicationsAudioFactoryInterface.h>
#include <sdkinterfaces/Audio/NotificationsAudioFactoryInterface.h>
#include <sdkinterfaces/Audio/SystemSoundAudioFactoryInterface.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace avsCommon;
                using namespace sdkInterfaces::audio;
                class AudioFactory : public AudioFactoryInterface {
                public:
                    shared_ptr<AlertsAudioFactoryInterface> alerts() const override;
                    shared_ptr<NotificationsAudioFactoryInterface> notifications() const override;
                    shared_ptr<CommunicationsAudioFactoryInterface> communications() const override;
                    shared_ptr<SystemSoundAudioFactoryInterface> systemSounds() const override;
                };
            }
        }
    }
}
#endif