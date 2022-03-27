#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockSpeakerInterface.h>
#include "ChannelVolumeManager.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            namespace test {
                using namespace testing;
                using namespace avs::speakerConstants;
                using namespace sdkInterfaces::test;
                static const SpeakerInterface::SpeakerSettings INITIAL_SETTINGS{AVS_SET_VOLUME_MAX / 2, false};
                class ChannelVolumeManagerTest : public Test {
                public:
                    void SetUp();
                    void TearDown();
                    int8_t getCurrentVolume() {
                        SpeakerInterface::SpeakerSettings settings;
                        if (m_speaker->getSpeakerSettings(&settings)) return settings.volume;
                        return AVS_SET_VOLUME_MIN;
                    }
                protected:
                    shared_ptr<MockSpeakerInterface> m_speaker;
                    shared_ptr<ChannelVolumeManager> unit;
                };
                void ChannelVolumeManagerTest::SetUp() {
                    m_speaker = make_shared<NiceMock<MockSpeakerInterface>>();
                    m_speaker->DelegateToReal();
                    m_speaker->setVolume(INITIAL_SETTINGS.volume);
                    m_speaker->setMute(INITIAL_SETTINGS.mute);
                    unit = ChannelVolumeManager::create(m_speaker);
                }
                void ChannelVolumeManagerTest::TearDown() {
                    unit.reset();
                    m_speaker.reset();
                }
                static int8_t defaultVolumeCurve(int8_t currentVolume) {
                    const float lowerBreakPointFraction = 0.20, upperBreakPointFraction = 0.40;
                    const int8_t lowerBreakPoint = static_cast<int8_t>(AVS_SET_VOLUME_MAX * lowerBreakPointFraction);
                    const int8_t upperBreakPoint = static_cast<int8_t>(AVS_SET_VOLUME_MAX * upperBreakPointFraction);
                    if (currentVolume >= upperBreakPoint) return lowerBreakPoint;
                    else if (currentVolume >= lowerBreakPoint && currentVolume <= upperBreakPoint) return (currentVolume - lowerBreakPoint);
                    else return avsCommon::avs::speakerConstants::AVS_SET_VOLUME_MIN;
                }
                TEST_F(ChannelVolumeManagerTest, test_createTest) {
                    ASSERT_EQ(nullptr, ChannelVolumeManager::create(nullptr));
                    auto instance = ChannelVolumeManager::create(m_speaker);
                    ASSERT_EQ(ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME, instance->getSpeakerType());
                }
                TEST_F(ChannelVolumeManagerTest, test_startDuckingCallAttenuatesVolume) {
                    auto currentVolume = getCurrentVolume();
                    auto desiredAttenuatedVolume = defaultVolumeCurve(currentVolume);
                    EXPECT_CALL(*m_speaker, setVolume(desiredAttenuatedVolume)).Times(1);
                    ASSERT_TRUE(unit->startDucking());
                    ASSERT_EQ(desiredAttenuatedVolume, getCurrentVolume());
                }
                TEST_F(ChannelVolumeManagerTest, test_stopDuckingCallRestoresVolume) {
                    auto currentVolume = getCurrentVolume();
                    ASSERT_TRUE(unit->startDucking());
                    EXPECT_CALL(*m_speaker, setVolume(currentVolume)).Times(1);
                    ASSERT_TRUE(unit->stopDucking());
                }
                TEST_F(ChannelVolumeManagerTest, test_getSpeakerSettingsReturnsUnduckedVolume) {
                    auto currentVolume = getCurrentVolume();
                    auto desiredAttenuatedVolume = defaultVolumeCurve(currentVolume);
                    EXPECT_CALL(*m_speaker, setVolume(desiredAttenuatedVolume)).Times(1);
                    ASSERT_TRUE(unit->startDucking());
                    SpeakerInterface::SpeakerSettings settings;
                    ASSERT_EQ(true, unit->getSpeakerSettings(&settings));
                    ASSERT_EQ(settings.volume, INITIAL_SETTINGS.volume);
                }
                TEST_F(ChannelVolumeManagerTest, test_volumeIsRestoredToLatestUnduckedVolume) {
                    auto currentVolume = getCurrentVolume();
                    auto desiredAttenuatedVolume = defaultVolumeCurve(currentVolume);
                    EXPECT_CALL(*m_speaker, setVolume(desiredAttenuatedVolume)).Times(1);
                    ASSERT_TRUE(unit->startDucking());
                    auto newUnduckedVolume = INITIAL_SETTINGS.volume * 2;
                    ASSERT_EQ(true, unit->setUnduckedVolume(newUnduckedVolume));
                    EXPECT_CALL(*m_speaker, setVolume(newUnduckedVolume)).Times(1);
                    ASSERT_TRUE(unit->stopDucking());
                    ASSERT_EQ(newUnduckedVolume, getCurrentVolume());
                }
                TEST_F(ChannelVolumeManagerTest, test_startDuckingWhenAlreadyDucked) {
                    auto currentVolume = getCurrentVolume();
                    auto desiredAttenuatedVolume = defaultVolumeCurve(currentVolume);
                    EXPECT_CALL(*m_speaker, setVolume(desiredAttenuatedVolume)).Times(1);
                    ASSERT_TRUE(unit->startDucking());
                    EXPECT_CALL(*m_speaker, setVolume(_)).Times(0);
                    ASSERT_TRUE(unit->startDucking());
                }
                TEST_F(ChannelVolumeManagerTest, test_stopDuckingWhenAlreadyUnducked) {
                    auto currentVolume = getCurrentVolume();
                    auto desiredAttenuatedVolume = defaultVolumeCurve(currentVolume);
                    EXPECT_CALL(*m_speaker, setVolume(desiredAttenuatedVolume)).Times(1);
                    ASSERT_TRUE(unit->startDucking());
                    EXPECT_CALL(*m_speaker, setVolume(currentVolume)).Times(1);
                    ASSERT_TRUE(unit->stopDucking());
                    EXPECT_CALL(*m_speaker, setVolume(_)).Times(0);
                    ASSERT_TRUE(unit->stopDucking());
                }
            }
        }
    }
}