#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSPEAKERMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSPEAKERMANAGER_H_

#include <gmock/gmock.h>
#include "SpeakerManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                class MockSpeakerManager : public SpeakerManagerInterface {
                public:
                    MOCK_METHOD3(setVolume, future<bool>(ChannelVolumeInterface::Type type, int8_t volume,
                                 const SpeakerManagerInterface::NotificationProperties& properties));
                    MOCK_METHOD3(adjustVolume, future<bool>(ChannelVolumeInterface::Type type, int8_t delta,
                                 const SpeakerManagerInterface::NotificationProperties& properties));
                    MOCK_METHOD3(setMute, future<bool>(ChannelVolumeInterface::Type type, bool mute,
                                 const SpeakerManagerInterface::NotificationProperties& properties));
                    MOCK_METHOD4(setVolume, future<bool>(ChannelVolumeInterface::Type type, int8_t volume, bool forceNoNotifications,
                                 SpeakerManagerObserverInterface::Source source));
                    MOCK_METHOD4(adjustVolume, future<bool>(ChannelVolumeInterface::Type type, int8_t delta, bool forceNoNotifications,
                                 SpeakerManagerObserverInterface::Source source));
                    MOCK_METHOD4(setMute, future<bool>(ChannelVolumeInterface::Type type, bool mute, bool forceNoNotifications,
                                 SpeakerManagerObserverInterface::Source source));
                #ifdef ENABLE_MAXVOLUME_SETTING
                    MOCK_METHOD1(setMaximumVolumeLimit, std::future<bool>(const int8_t maximumVolumeLimit));
                #endif
                    MOCK_METHOD2(getSpeakerSettings, future<bool>(ChannelVolumeInterface::Type type, SpeakerInterface::SpeakerSettings* settings));
                    MOCK_METHOD1(addSpeakerManagerObserver, void(shared_ptr<SpeakerManagerObserverInterface> observer));
                    MOCK_METHOD1(removeSpeakerManagerObserver, void(shared_ptr<SpeakerManagerObserverInterface> observer));
                    MOCK_METHOD1(addChannelVolumeInterface, void(shared_ptr<ChannelVolumeInterface> channelVolumeInterface));
                };
            }
        }
    }
}
#endif