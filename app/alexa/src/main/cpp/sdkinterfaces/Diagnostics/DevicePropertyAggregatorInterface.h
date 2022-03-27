#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_DEVICEPROPERTYAGGREGATORINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_DEVICEPROPERTYAGGREGATORINTERFACE_H_

#include <string>
#include <unordered_map>
#include <acsdk_alerts_interfaces/AlertObserverInterface.h>
#include <acsdk_alerts_interfaces/AudioPlayerObserverInterface.h>
#include <acsdk_alerts_interfaces/NotificationsObserverInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <sdkinterfaces/SpeakerManagerInterface.h>
#include <sdkinterfaces/SpeakerManagerObserverInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace diagnostics {
                using namespace std;
                using namespace acsdkAlertsInterfaces;
                using namespace acsdkAudioPlayerInterfaces;
                using namespace acsdkNotificationsInterfaces;
                using namespace sdkInterfaces;
                using namespace utils;
                class DevicePropertyAggregatorInterface : public AlertObserverInterface , public AudioPlayerObserverInterface,
                                                          public ConnectionStatusObserverInterface , public ContextRequesterInterface,
                                                          public NotificationsObserverInterface, public SpeakerManagerObserverInterface,
                                                          public DialogUXStateObserverInterface {
                public:
                    static constexpr const char* DEVICE_CONTEXT = "DeviceContext";
                    static constexpr const char* ALERT_TYPE_AND_STATE = "AlertTypeAndState";
                    static constexpr const char* AUDIO_PLAYER_STATE = "AudioPlayerState";
                    static constexpr const char* CONTENT_ID = "ContentId";
                    static constexpr const char* CONNECTION_STATE = "ConnectionState";
                    static constexpr const char* NOTIFICATION_INDICATOR = "NotificationIndicator";
                    static constexpr const char* TTS_PLAYER_STATE = "TTSPlayerState";
                    static constexpr const char* AVS_SPEAKER_VOLUME = "AVSSpeakerVolume";
                    static constexpr const char* AVS_SPEAKER_MUTE = "AVSSpeakerMute";
                    static constexpr const char* AVS_ALERTS_VOLUME = "AVSAlertsVolume";
                    static constexpr const char* AVS_ALERTS_MUTE = "AVSAlertsMute";
                    virtual Optional<string> getDeviceProperty(const string& propertyKey);
                    virtual unordered_map<string, string> getAllDeviceProperties();
                    virtual void setContextManager(shared_ptr<ContextManagerInterface> contextManager);
                    virtual void initializeVolume(shared_ptr<SpeakerManagerInterface> speakerManager);
                };
            }
        }
    }
}
#endif