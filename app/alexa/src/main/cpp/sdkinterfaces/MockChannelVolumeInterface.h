#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCHANNELVOLUMEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCHANNELVOLUMEINTERFACE_H_

#include <gmock/gmock.h>
#include "ChannelVolumeInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                using namespace testing;
                class MockChannelVolumeManager : public ChannelVolumeInterface {
                public:
                    ChannelVolumeInterface::Type getSpeakerType() const {
                        return m_type;
                    }
                    bool startDucking() {
                        return true;
                    }
                    bool stopDucking() {
                        return true;
                    }
                    bool setUnduckedVolume(int8_t volume) {
                        m_settings.volume = volume;
                        return true;
                    }
                    bool setMute(bool mute) {
                        m_settings.mute = mute;
                        return true;
                    }
                    bool getSpeakerSettings(SpeakerInterface::SpeakerSettings* settings) const {
                        if (!settings) return false;
                        settings->volume = m_settings.volume;
                        settings->mute = m_settings.mute;
                        return true;
                    }
                    MockChannelVolumeManager(avsCommon::sdkInterfaces::ChannelVolumeInterface::Type type) : ChannelVolumeInterface(),
                                             m_settings{speakerConstants::AVS_SET_VOLUME_MIN, false}, m_type{type} {}
                private:
                    SpeakerInterface::SpeakerSettings m_settings;
                    const ChannelVolumeInterface::Type m_type;
                };
                class MockChannelVolumeInterface : public ChannelVolumeInterface {
                public:
                    MOCK_CONST_METHOD0(getSpeakerType, ChannelVolumeInterface::Type());
                    MOCK_METHOD0(startDucking, bool());
                    MOCK_METHOD0(stopDucking, bool());
                    MOCK_METHOD1(setUnduckedVolume, bool(int8_t));
                    MOCK_METHOD1(setMute, bool(bool));
                    MOCK_CONST_METHOD1(getSpeakerSettings, bool(SpeakerInterface::SpeakerSettings*));
                    void DelegateToReal();
                    MockChannelVolumeInterface(ChannelVolumeInterface::Type type = ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME) :
                                               ChannelVolumeInterface(), m_manager{type} {}
                private:
                    MockChannelVolumeManager m_manager;
                };
                inline void MockChannelVolumeInterface::DelegateToReal() {
                    ON_CALL(*this, getSpeakerType()).WillByDefault(Invoke(&m_manager, &ChannelVolumeInterface::getSpeakerType));
                    ON_CALL(*this, startDucking()).WillByDefault(Invoke(&m_manager, &ChannelVolumeInterface::startDucking));
                    ON_CALL(*this, stopDucking()).WillByDefault(Invoke(&m_manager, &ChannelVolumeInterface::stopDucking));
                    ON_CALL(*this, setUnduckedVolume(_)).WillByDefault(Invoke(&m_manager, &ChannelVolumeInterface::setUnduckedVolume));
                    ON_CALL(*this, setMute(_)).WillByDefault(Invoke(&m_manager, &ChannelVolumeInterface::setMute));
                    ON_CALL(*this, getSpeakerSettings(_)).WillByDefault(Invoke(&m_manager, &ChannelVolumeInterface::getSpeakerSettings));
                }
            }
        }
    }
}
#endif