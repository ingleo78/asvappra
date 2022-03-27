#include <gtest/gtest.h>
#include <sdkinterfaces/Audio/MockSystemSoundAudioFactory.h>
#include <media_player/MockMediaPlayer.h>
#include "SystemSoundPlayer.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace systemSoundPlayer {
            namespace test {
                using namespace std;
                using namespace avsCommon;
                using namespace sdkInterfaces;
                using namespace audio;
                using namespace audio::test;
                using namespace utils;
                using namespace mediaPlayer;
                using namespace mediaPlayer::test;
                using namespace ::testing;
                class SystemSoundPlayerTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                    shared_ptr<SystemSoundPlayer> m_systemSoundPlayer;
                    shared_ptr<MockMediaPlayer> m_mockMediaPlayer;
                    shared_ptr<MockSystemSoundAudioFactory> m_mockSystemSoundAudioFactory;
                };
                void SystemSoundPlayerTest::SetUp() {
                    m_mockSystemSoundAudioFactory = MockSystemSoundAudioFactory::create();
                    m_mockMediaPlayer = MockMediaPlayer::create();
                    m_systemSoundPlayer = SystemSoundPlayer::create(m_mockMediaPlayer, m_mockSystemSoundAudioFactory);
                }
                void SystemSoundPlayerTest::TearDown() {
                    m_mockMediaPlayer->shutdown();
                }
                TEST_F(SystemSoundPlayerTest, test_createWithNullPointers) {
                    auto testSystemSoundPlayer = SystemSoundPlayer::create(nullptr, m_mockSystemSoundAudioFactory);
                    EXPECT_EQ(testSystemSoundPlayer, nullptr);
                    testSystemSoundPlayer = SystemSoundPlayer::create(m_mockMediaPlayer, nullptr);
                    EXPECT_EQ(testSystemSoundPlayer, nullptr);
                }
                TEST_F(SystemSoundPlayerTest, test_playWakeWord) {
                    EXPECT_CALL(*m_mockSystemSoundAudioFactory, wakeWordNotificationTone()).Times(1);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), streamSetSource(_, _)).Times(1);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).WillOnce(Return(true));
                    shared_future<bool> playToneFuture = m_systemSoundPlayer->playTone(SystemSoundPlayer::Tone::WAKEWORD_NOTIFICATION);
                    m_mockMediaPlayer->mockFinished(m_mockMediaPlayer->getCurrentSourceId());
                    playToneFuture.wait();
                    ASSERT_TRUE(playToneFuture.get());
                }
                TEST_F(SystemSoundPlayerTest, test_playEndSpeech) {
                    EXPECT_CALL(*m_mockSystemSoundAudioFactory, endSpeechTone()).Times(1);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), streamSetSource(_, _)).Times(1);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).WillOnce(Return(true));
                    shared_future<bool> playToneFuture = m_systemSoundPlayer->playTone(SystemSoundPlayer::Tone::END_SPEECH);
                    m_mockMediaPlayer->mockFinished(m_mockMediaPlayer->getCurrentSourceId());
                    playToneFuture.wait();
                    ASSERT_TRUE(playToneFuture.get());
                }
                TEST_F(SystemSoundPlayerTest, test_failPlayback) {
                    EXPECT_CALL(*m_mockSystemSoundAudioFactory, endSpeechTone()).Times(1);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), streamSetSource(_, _)).Times(1);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).WillOnce(Return(true));
                    shared_future<bool> playToneFuture = m_systemSoundPlayer->playTone(SystemSoundPlayer::Tone::END_SPEECH);
                    m_mockMediaPlayer->mockError(m_mockMediaPlayer->getCurrentSourceId());
                    playToneFuture.wait();
                    ASSERT_FALSE(playToneFuture.get());
                }
                TEST_F(SystemSoundPlayerTest, test_playBeforeFinish) {
                    shared_future<bool> playToneFuture = m_systemSoundPlayer->playTone(SystemSoundPlayer::Tone::END_SPEECH);
                    EXPECT_CALL(*m_mockSystemSoundAudioFactory, endSpeechTone()).Times(0);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), streamSetSource(_, _)).Times(0);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(0);
                    shared_future<bool> playToneFutureFail = m_systemSoundPlayer->playTone(SystemSoundPlayer::Tone::END_SPEECH);
                    playToneFutureFail.wait();
                    ASSERT_FALSE(playToneFutureFail.get());
                    m_mockMediaPlayer->mockFinished(m_mockMediaPlayer->getCurrentSourceId());
                    playToneFuture.wait();
                    ASSERT_TRUE(playToneFuture.get());
                }
                TEST_F(SystemSoundPlayerTest, test_sequentialPlayback) {
                    EXPECT_CALL(*m_mockSystemSoundAudioFactory, endSpeechTone()).Times(2);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), streamSetSource(_, _)).Times(2);
                    EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(2).WillRepeatedly(Return(true));
                    shared_future<bool> playToneFuture = m_systemSoundPlayer->playTone(SystemSoundPlayer::Tone::END_SPEECH);
                    m_mockMediaPlayer->mockFinished(m_mockMediaPlayer->getCurrentSourceId());
                    playToneFuture.wait();
                    ASSERT_TRUE(playToneFuture.get());
                    playToneFuture = m_systemSoundPlayer->playTone(SystemSoundPlayer::Tone::END_SPEECH);
                    m_mockMediaPlayer->mockFinished(m_mockMediaPlayer->getCurrentSourceId());
                    playToneFuture.wait();
                    ASSERT_TRUE(playToneFuture.get());
                }
            }
        }
    }
}