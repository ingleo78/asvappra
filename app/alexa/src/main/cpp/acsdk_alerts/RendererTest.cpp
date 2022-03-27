#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sdkinterfaces/MockSpeakerManager.h>
#include <media_player/MockMediaPlayer.h>
#include <media_player/SourceConfig.h>
#include <registration_manager/CustomerDataManager.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Settings/MockSetting.h>
#include "Renderer/Renderer.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace renderer {
            namespace test {
                using namespace std;
                using namespace chrono;
                using namespace avsCommon;
                using namespace avs;
                using namespace attachment;
                using namespace sdkInterfaces;
                using namespace sdkInterfaces::test;
                using namespace utils;
                using namespace mediaPlayer;
                using namespace settings;
                using namespace mediaPlayer::test;
                using namespace settings::types;
                using namespace settings::test;
                using namespace testing;
                static const milliseconds TEST_TIMEOUT{100};
                static const MediaPlayerState DEFAULT_MEDIA_PLAYER_STATE = {milliseconds(0)};
                static const MediaPlayerInterface::SourceId TEST_SOURCE_ID_GOOD = 1234;
                static const MediaPlayerInterface::SourceId TEST_SOURCE_ID_BAD = 5678;
                static const string TEST_URL1 = "fake.url.one";
                static const string TEST_URL2 = "fake.url.two";
                static const milliseconds TEST_LOOP_PAUSE{100};
                static const int TEST_LOOP_COUNT = 2;
                static const auto TEST_BACKGROUND_LOOP_PAUSE = seconds(1);
                static const auto TEST_BACKGROUND_TIMEOUT = seconds(5);
                static const string ALARM_NAME = "ALARM";
                class MockRendererObserver : public RendererObserverInterface {
                public:
                    bool waitFor(RendererObserverInterface::State newState) {
                        unique_lock<std::mutex> lock(m_mutex);
                        return m_conditionVariable.wait_for(lock, TEST_TIMEOUT, [this, newState] { return m_state == newState; });
                    }
                    bool waitFor(RendererObserverInterface::State newState, std::chrono::milliseconds maxWait) {
                        unique_lock<mutex> lock(m_mutex);
                        return m_conditionVariable.wait_for(lock, maxWait, [this, newState] { return m_state == newState; });
                    }
                    void onRendererStateChange(RendererObserverInterface::State newState, const string& reason) {
                        lock_guard<mutex> lock(m_mutex);
                        m_state = newState;
                        m_conditionVariable.notify_all();
                    }
                private:
                    mutex m_mutex;
                    condition_variable m_conditionVariable;
                    RendererObserverInterface::State m_state;
                };
                class TestMediaPlayer : public MockMediaPlayer {
                public:
                    TestMediaPlayer() {
                        m_sourceIdRetVal = TEST_SOURCE_ID_GOOD;
                        m_playRetVal = true;
                        m_stopRetVal = true;
                    }
                    static shared_ptr<NiceMock<TestMediaPlayer>> create() {
                        return make_shared<NiceMock<TestMediaPlayer>>();
                    }
                    bool play(SourceId id) override {
                        return m_playRetVal;
                    }
                    bool stop(SourceId id) override {
                        return m_stopRetVal;
                    }
                    SourceId setSource(const string& url, milliseconds offset = milliseconds::zero(), const SourceConfig& config = emptySourceConfig(),
                                       bool repeat = false, const PlaybackContext& playbackContext = PlaybackContext()) override {
                        lock_guard<mutex> lock(m_mutex);
                        m_sourceConfig = config;
                        m_sourceChanged.notify_one();
                        return m_sourceIdRetVal;
                    }
                    SourceId setSource(shared_ptr<istream> stream, bool repeat, const SourceConfig& config, MediaType format) override {
                        lock_guard<mutex> lock(m_mutex);
                        m_sourceConfig = config;
                        m_sourceChanged.notify_one();
                        return m_sourceIdRetVal;
                    }
                    SourceId setSource(shared_ptr<AttachmentReader> attachmentReader, const AudioFormat* audioFormat, const SourceConfig& config) override {
                        return m_sourceIdRetVal;
                    }
                    void setSourceRetVal(SourceId sourceRetVal) {
                        m_sourceIdRetVal = sourceRetVal;
                    }
                    void setPlayRetVal(bool playRetVal) {
                        m_playRetVal = playRetVal;
                    }
                    void setStopRetVal(bool stopRetVal) {
                        m_stopRetVal = stopRetVal;
                    }
                    pair<bool, SourceConfig> waitForSourceConfig(
                        milliseconds timeout) {
                        unique_lock<mutex> lock(m_mutex);
                        if (m_sourceChanged.wait_for(lock, timeout) != cv_status::timeout) return make_pair(true, m_sourceConfig);
                        else return make_pair(false, emptySourceConfig());
                    }
                private:
                    SourceId m_sourceIdRetVal;
                    bool m_playRetVal;
                    bool m_stopRetVal;
                    mutex m_mutex;
                    condition_variable m_sourceChanged;
                    SourceConfig m_sourceConfig;
                };
                class RendererTest : public Test {
                public:
                    RendererTest();
                    ~RendererTest();
                    void SetUpTest();
                    void TearDown() override;
                protected:
                    shared_ptr<MockRendererObserver> m_observer;
                    shared_ptr<TestMediaPlayer> m_mediaPlayer;
                    shared_ptr<Renderer> m_renderer;
                    static pair<unique_ptr<istream>, const MediaType> audioFactoryFunc() {
                        return pair<unique_ptr<istream>, const MediaType>(unique_ptr<istream>(new stringstream()),MediaType::MPEG);
                    }
                };
                RendererTest::RendererTest() : m_observer{make_shared<MockRendererObserver>()}, m_mediaPlayer{TestMediaPlayer::create()},
                                               m_renderer{Renderer::create(m_mediaPlayer, nullptr)} {}
                RendererTest::~RendererTest() {
                    m_mediaPlayer.reset();
                }
                void RendererTest::SetUpTest() {
                    function<pair<unique_ptr<istream>, const MediaType>()> audioFactory = RendererTest::audioFactoryFunc;
                    vector<string> urls = {TEST_URL1, TEST_URL2};
                    m_renderer->start(m_observer, audioFactory, true, urls, TEST_LOOP_COUNT, TEST_LOOP_PAUSE);
                }
                void RendererTest::TearDown() {
                    m_mediaPlayer->setSourceRetVal(TEST_SOURCE_ID_GOOD);
                    m_mediaPlayer->setPlayRetVal(true);
                    m_mediaPlayer->setStopRetVal(true);
                }
                TEST_F(RendererTest, test_create) {
                    ASSERT_NE(m_renderer, nullptr);
                    ASSERT_EQ(Renderer::create(nullptr, nullptr), nullptr);
                }
                TEST_F(RendererTest, test_start) {
                    SetUpTest();
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::UNSET));
                    m_mediaPlayer->shutdown();
                }
                TEST_F(RendererTest, test_stop) {
                    SetUpTest();
                    m_renderer->stop();
                    ASSERT_FALSE(m_observer->waitFor(RendererObserverInterface::State::ERROR));
                }
                TEST_F(RendererTest, test_restart) {
                    function<pair<unique_ptr<istream>, const MediaType>()> audioFactory = RendererTest::audioFactoryFunc;
                    vector<string> urls = {TEST_URL1, TEST_URL2};
                    m_renderer->start(m_observer, audioFactory, true, urls, TEST_LOOP_COUNT, TEST_LOOP_PAUSE);
                    m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STARTED));
                    m_renderer->stop();
                    ASSERT_FALSE(m_observer->waitFor(RendererObserverInterface::State::ERROR));
                    m_renderer->onPlaybackStopped(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STOPPED));
                    m_renderer->start(m_observer, audioFactory, true, urls, TEST_LOOP_COUNT, TEST_LOOP_PAUSE);
                    m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STARTED));
                }
                TEST_F(RendererTest, test_stopError) {
                    SetUpTest();
                    m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STARTED));
                    m_mediaPlayer->setStopRetVal(false);
                    const ErrorType& errorType = ErrorType::MEDIA_ERROR_INVALID_REQUEST;
                    string errorMsg = "testError";
                    m_renderer->stop();
                    m_renderer->onPlaybackError(TEST_SOURCE_ID_GOOD, errorType, errorMsg, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::ERROR));
                }
                TEST_F(RendererTest, test_onPlaybackStarted) {
                    SetUpTest();
                    m_renderer->onPlaybackStarted(TEST_SOURCE_ID_BAD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_FALSE(m_observer->waitFor(RendererObserverInterface::State::STARTED));
                    m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STARTED));
                }
                TEST_F(RendererTest, test_onPlaybackStopped) {
                    SetUpTest();
                    m_renderer->onPlaybackStopped(TEST_SOURCE_ID_BAD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_FALSE(m_observer->waitFor(RendererObserverInterface::State::STOPPED));
                    m_renderer->onPlaybackStopped(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STOPPED));
                }
                TEST_F(RendererTest, test_onPlaybackFinishedError) {
                    SetUpTest();
                    m_mediaPlayer->setSourceRetVal(MediaPlayerInterface::ERROR);
                    m_renderer->onPlaybackFinished(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_FALSE(m_observer->waitFor(RendererObserverInterface::State::STOPPED));
                    m_mediaPlayer->setSourceRetVal(TEST_SOURCE_ID_GOOD);
                    m_mediaPlayer->setPlayRetVal(false);
                    m_renderer->onPlaybackFinished(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_FALSE(m_observer->waitFor(RendererObserverInterface::State::STOPPED));
                }
                TEST_F(RendererTest, test_onPlaybackError) {
                    const ErrorType& errorType = ErrorType::MEDIA_ERROR_INVALID_REQUEST;
                    string errorMsg = "testError";
                    SetUpTest();
                    m_renderer->onPlaybackError(TEST_SOURCE_ID_BAD, errorType, errorMsg, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_FALSE(m_observer->waitFor(RendererObserverInterface::State::ERROR));
                    m_renderer->onPlaybackError(TEST_SOURCE_ID_GOOD, errorType, errorMsg, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::ERROR));
                }
                TEST_F(RendererTest, testTimer_emptyURLNonZeroLoopPause) {
                    function<pair<unique_ptr<istream>, const MediaType>()> audioFactory = RendererTest::audioFactoryFunc;
                    vector<string> urls;
                    m_renderer->start(m_observer, audioFactory, true, urls, TEST_LOOP_COUNT, TEST_BACKGROUND_LOOP_PAUSE);
                    m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    auto now = high_resolution_clock::now();
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STARTED, TEST_BACKGROUND_TIMEOUT));
                    m_renderer->onPlaybackFinished(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    m_renderer->onPlaybackFinished(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::STARTED, TEST_BACKGROUND_TIMEOUT));
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::COMPLETED, TEST_BACKGROUND_TIMEOUT));
                    auto elapsed = high_resolution_clock::now() - now;
                    ASSERT_TRUE((elapsed >= TEST_BACKGROUND_LOOP_PAUSE) && (elapsed < TEST_BACKGROUND_TIMEOUT));
                }
                TEST_F(RendererTest, test_alarmVolumeRampRendering) {
                    function<pair<unique_ptr<std::istream>, const MediaType>()> audioFactory = RendererTest::audioFactoryFunc;
                    vector<string> urls;
                    const auto loopPause = seconds(1);
                    thread sourceConfigObserver([this, loopPause]() {
                        SourceConfig config;
                        bool ok;
                        tie(ok, config) = m_mediaPlayer->waitForSourceConfig(6 * loopPause);
                        ASSERT_TRUE(ok);
                        ASSERT_EQ(config.fadeInConfig.startGain, 0);
                        m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                        m_renderer->onPlaybackFinished(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                        tie(ok, config) = m_mediaPlayer->waitForSourceConfig(6 * loopPause);
                        ASSERT_TRUE(ok);
                        ASSERT_GT(config.fadeInConfig.startGain, 0);
                        m_renderer->onPlaybackStarted(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                        m_renderer->onPlaybackFinished(TEST_SOURCE_ID_GOOD, DEFAULT_MEDIA_PLAYER_STATE);
                    });
                    constexpr int testLoopCount = 2;
                    m_renderer->start(m_observer, audioFactory, true, urls, testLoopCount, loopPause);
                    sourceConfigObserver.join();
                    ASSERT_TRUE(m_observer->waitFor(RendererObserverInterface::State::COMPLETED));
                }
            }
        }
    }
}