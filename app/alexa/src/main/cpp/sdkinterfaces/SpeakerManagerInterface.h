#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEAKERMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_SPEAKERMANAGERINTERFACE_H_

#include <future>
#include <memory>
#include "ChannelVolumeInterface.h"
#include "SpeakerManagerObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class SpeakerManagerInterface {
            public:
                struct NotificationProperties {
                    NotificationProperties(SpeakerManagerObserverInterface::Source source = SpeakerManagerObserverInterface::Source::LOCAL_API,
                                           bool notifyAVS = true, bool notifyObservers = true) : notifyAVS{notifyAVS},
                                           notifyObservers{notifyObservers}, source{source} {}
                    bool notifyAVS;
                    bool notifyObservers;
                    SpeakerManagerObserverInterface::Source source;
                };
                virtual std::future<bool> setVolume(ChannelVolumeInterface::Type type, int8_t volume, const NotificationProperties& properties);
                virtual std::future<bool> adjustVolume(ChannelVolumeInterface::Type type, int8_t delta, const NotificationProperties& properties);
                virtual std::future<bool> setMute(ChannelVolumeInterface::Type type, bool mute, const NotificationProperties& properties);
                virtual std::future<bool> setVolume(ChannelVolumeInterface::Type type, int8_t volume, bool forceNoNotifications = false,
                                                    SpeakerManagerObserverInterface::Source source = SpeakerManagerObserverInterface::Source::LOCAL_API);
                virtual std::future<bool> adjustVolume(ChannelVolumeInterface::Type type, int8_t delta, bool forceNoNotifications = false,
                                                       SpeakerManagerObserverInterface::Source source = SpeakerManagerObserverInterface::Source::LOCAL_API);
                virtual std::future<bool> setMute(ChannelVolumeInterface::Type type, bool mute, bool forceNoNotifications = false,
                                                  SpeakerManagerObserverInterface::Source source = SpeakerManagerObserverInterface::Source::LOCAL_API) ;
            #ifdef ENABLE_MAXVOLUME_SETTING
                virtual std::future<bool> setMaximumVolumeLimit(const int8_t maximumVolumeLimit);
            #endif
                virtual std::future<bool> getSpeakerSettings(ChannelVolumeInterface::Type type, SpeakerInterface::SpeakerSettings* settings);
                virtual void addSpeakerManagerObserver(std::shared_ptr<SpeakerManagerObserverInterface> observer);
                virtual void removeSpeakerManagerObserver(std::shared_ptr<SpeakerManagerObserverInterface> observer);
                virtual void addChannelVolumeInterface(std::shared_ptr<ChannelVolumeInterface> channelVolumeInterface);
                virtual ~SpeakerManagerInterface() = default;
            };
        }
    }
}
#endif