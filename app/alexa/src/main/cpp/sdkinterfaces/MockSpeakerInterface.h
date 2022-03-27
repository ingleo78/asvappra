#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSPEAKERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSPEAKERINTERFACE_H_

#include <gmock/gmock.h>
#include <avs/SpeakerConstants/SpeakerConstants.h>
#include "SpeakerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                using namespace speakerConstants;
                using namespace testing;
                static const bool MUTE(true);
                static const string MUTE_STRING("true");
                static const bool UNMUTE(false);
                static const string UNMUTE_STRING("false");
                static const SpeakerInterface::SpeakerSettings DEFAULT_SETTINGS{AVS_SET_VOLUME_MIN, UNMUTE};
                class MockSpeaker : public SpeakerInterface {
                public:
                    bool setVolume(int8_t volume) override;
                    bool setMute(bool mute) override;
                    bool getSpeakerSettings(SpeakerInterface::SpeakerSettings* settings) override;
                    MockSpeaker();
                private:
                    SpeakerInterface::SpeakerSettings m_settings;
                };
                inline bool MockSpeaker::setVolume(int8_t volume) {
                    m_settings.volume = volume;
                    return true;
                }
                inline bool MockSpeaker::setMute(bool mute) {
                    m_settings.mute = mute;
                    return true;
                }
                inline bool MockSpeaker::getSpeakerSettings(SpeakerInterface::SpeakerSettings* settings) {
                    if (!settings) return false;
                    settings->volume = m_settings.volume;
                    settings->mute = m_settings.mute;
                    return true;
                }
                inline MockSpeaker::MockSpeaker() {
                    m_settings = DEFAULT_SETTINGS;
                }
                class MockSpeakerInterface : public SpeakerInterface {
                public:
                    MOCK_METHOD1(setVolume, bool(int8_t));
                    MOCK_METHOD1(setMute, bool(bool));
                    MOCK_METHOD1(getSpeakerSettings, bool(SpeakerInterface::SpeakerSettings*));
                    void DelegateToReal();
                private:
                    MockSpeaker m_speaker;
                };
                inline void MockSpeakerInterface::DelegateToReal() {
                    ON_CALL(*this, setVolume(_)).WillByDefault(Invoke(&m_speaker, &SpeakerInterface::setVolume));
                    ON_CALL(*this, setMute(_)).WillByDefault(Invoke(&m_speaker, &SpeakerInterface::setMute));
                    ON_CALL(*this, getSpeakerSettings(_)).WillByDefault(Invoke(&m_speaker, &SpeakerInterface::getSpeakerSettings));
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKSPEAKERINTERFACE_H_
