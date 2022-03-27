#include "AudioFactory.h"
#include "AlertsAudioFactory.h"
#include "CommunicationsAudioFactory.h"
#include "NotificationsAudioFactory.h"
#include "SystemSoundAudioFactory.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace avsCommon::sdkInterfaces::audio;
                shared_ptr<AlertsAudioFactoryInterface> AudioFactory::alerts() const {
                    return std::make_shared<AlertsAudioFactory>();
                }
                shared_ptr<NotificationsAudioFactoryInterface> AudioFactory::notifications() const {
                    return make_shared<NotificationsAudioFactory>();
                }
                shared_ptr<CommunicationsAudioFactoryInterface> AudioFactory::communications()const {
                    return make_shared<CommunicationsAudioFactory>();
                }
                shared_ptr<SystemSoundAudioFactoryInterface> AudioFactory::systemSounds() const {
                    return make_shared<SystemSoundAudioFactory>();
                }
            }
        }
    }
}