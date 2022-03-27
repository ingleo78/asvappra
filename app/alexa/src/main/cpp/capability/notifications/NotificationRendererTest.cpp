#include <future>
#include <memory>
#include <random>
#include <sstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <media_player/MockMediaPlayer.h>
#include <avs/MixingBehavior.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <logger/Logger.h>
#include "NotificationRenderer.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        namespace test {
            using namespace testing;
            using namespace sdkInterfaces::test;
            using namespace mediaPlayer::test;
            static const milliseconds ZERO_TIMEOUT{0};
            static const milliseconds EXPECTED_TIMEOUT{100};
            static const milliseconds UNEXPECTED_TIMEOUT{5000};
            static function<pair<unique_ptr<istream>, const MediaType>()> goodStreamFunction = [] {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<istream>(new stringstream), MediaType::MPEG);
                };
            class MockNotificationRendererObserver : public NotificationRendererObserverInterface {
            public:
                MockNotificationRendererObserver();
                MOCK_METHOD0(onNotificationRenderingFinished, void());
                bool waitForFinished(const milliseconds timeout = UNEXPECTED_TIMEOUT);
            private:
                future<void> m_future;
                promise<void> m_promise;
            };
            MockNotificationRendererObserver::MockNotificationRendererObserver() {
                m_future = m_promise.get_future();
                ON_CALL(*this, onNotificationRenderingFinished()).WillByDefault(InvokeWithoutArgs([this]() {
                    m_promise.set_value();
                }));
            }
            bool MockNotificationRendererObserver::waitForFinished(const milliseconds timeout) {
                return m_future.wait_for(timeout) != std::future_status::timeout;
            }
            struct FuturePromisePair {
            public:
                FuturePromisePair();
                promise<void> promise;
                future<void> future;
            };
            FuturePromisePair::FuturePromisePair() : promise{}, future{promise.get_future()} {}
            class NotificationRendererTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                shared_ptr<MockMediaPlayer> m_player;
                shared_ptr<MockFocusManager> m_focusManager;
                shared_ptr<NotificationRenderer> m_renderer;
                shared_ptr<MockNotificationRendererObserver> m_observer;
            };
            void NotificationRendererTest::SetUp() {
                getConsoleLogger()->setLevel(Level::DEBUG9);
                m_player = MockMediaPlayer::create();
                //m_focusManager = make_shared<MockFocusManager>();
                m_renderer = NotificationRenderer::create(m_player, m_focusManager);
                ASSERT_TRUE(m_renderer);
                m_observer.reset(new MockNotificationRendererObserver());
                m_renderer->addObserver(m_observer);
                /*ON_CALL(*(m_focusManager.get()), acquireChannel(_, _)).WillByDefault(Return(true));
                ON_CALL(*m_focusManager, releaseChannel(_, _)).WillByDefault(InvokeWithoutArgs([] {
                    auto releaseChannelSuccess = std::make_shared<std::promise<bool>>();
                    std::future<bool> returnValue = releaseChannelSuccess->get_future();
                    releaseChannelSuccess->set_value(true);
                    return returnValue;
                }));*/
            }
            void NotificationRendererTest::TearDown() {
                m_player->shutdown();
            }
            TEST_F(NotificationRendererTest, test_createWithNullMediaPlayer) {
                auto renderer = NotificationRenderer::create(nullptr, m_focusManager);
                ASSERT_FALSE(renderer);
            }
            TEST_F(NotificationRendererTest, test_createWithNullFocusManager) {
                auto renderer = NotificationRenderer::create(m_player, nullptr);
                ASSERT_FALSE(renderer);
            }
            TEST_F(NotificationRendererTest, test_playPreferredStream) {
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(1);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(0);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(1);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).Times(1);
                m_renderer->renderNotification(goodStreamFunction, "");
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                m_player->mockFinished(m_player->getCurrentSourceId());
                ASSERT_TRUE(m_observer->waitForFinished());
            }
            TEST_F(NotificationRendererTest, testTimer_playDefaultStream) {
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(1);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(1);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(2);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).Times(1);
                m_renderer->renderNotification(goodStreamFunction, "");
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                m_player->mockError(m_player->getCurrentSourceId());
                m_player->waitUntilPlaybackError();
                ASSERT_FALSE(m_observer->waitForFinished(ZERO_TIMEOUT));
                m_player->waitUntilNextSetSource();
                m_player->mockFinished(m_player->getCurrentSourceId());
                ASSERT_TRUE(m_observer->waitForFinished());
            }
            TEST_F(NotificationRendererTest, test_secondPlayRejected) {
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(1);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(0);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(1);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).Times(1);
                ASSERT_TRUE(m_renderer->renderNotification(goodStreamFunction, ""));
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                ASSERT_FALSE(m_renderer->renderNotification(goodStreamFunction, ""));
                m_player->mockFinished(m_player->getCurrentSourceId());
                ASSERT_TRUE(m_observer->waitForFinished());
            }
            TEST_F(NotificationRendererTest, testTimer_secondPlayWhilePlayingDefaultStream) {
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(1);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(1);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(2);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).Times(1);
                ASSERT_TRUE(m_renderer->renderNotification(goodStreamFunction, ""));
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                m_player->mockError(m_player->getCurrentSourceId());
                m_player->waitUntilPlaybackError();
                ASSERT_FALSE(m_observer->waitForFinished(ZERO_TIMEOUT));
                m_player->waitUntilNextSetSource();
                m_player->waitUntilPlaybackStarted();
                ASSERT_FALSE(m_renderer->renderNotification(goodStreamFunction, ""));
                m_player->mockFinished(m_player->getCurrentSourceId());
                ASSERT_TRUE(m_observer->waitForFinished());
            }
            TEST_F(NotificationRendererTest, test_cancelNotificationRendering) {
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(1);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(0);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(1);
                EXPECT_CALL(*(m_player.get()), stop(_)).Times(1);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).Times(1);
                ASSERT_TRUE(m_renderer->renderNotification(goodStreamFunction, ""));
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                ASSERT_TRUE(m_renderer->cancelNotificationRendering());
                ASSERT_TRUE(m_observer->waitForFinished());
            }
            TEST_F(NotificationRendererTest, test_renderNotificationWhileNotifying) {
                FuturePromisePair signal;
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(2);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(0);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(2);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).WillOnce(InvokeWithoutArgs([&signal] {
                    static int counter = 0;
                    ASSERT_LT(counter, 2);
                    if (0 == counter++) {
                        signal.promise.set_value();
                        this_thread::sleep_for(EXPECTED_TIMEOUT);
                    }
                }));
                ASSERT_TRUE(m_renderer->renderNotification(goodStreamFunction, ""));
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                m_player->mockFinished(m_player->getCurrentSourceId());
                ASSERT_EQ(signal.future.wait_for(UNEXPECTED_TIMEOUT), std::future_status::ready);
                m_renderer->onFocusChanged(FocusState::NONE, MixingBehavior::UNDEFINED);
                ASSERT_TRUE(m_renderer->renderNotification(goodStreamFunction, ""));
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                m_player->waitUntilPlaybackFinished();
            }
            TEST_F(NotificationRendererTest, test_renderWhenAcquireChannelsSucceeds) {
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(1);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(0);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(1);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).Times(1);
                //EXPECT_CALL(*(m_focusManager.get()), acquireChannel(_, _)).Times(1);
                //EXPECT_CALL(*(m_focusManager.get()), releaseChannel(_, _)).Times(1);
                m_renderer->renderNotification(goodStreamFunction, "");
                m_renderer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::UNDEFINED);
                m_player->waitUntilPlaybackStarted();
                m_player->mockFinished(m_player->getCurrentSourceId());
                ASSERT_TRUE(m_observer->waitForFinished());
            }
            TEST_F(NotificationRendererTest, test_renderWhenAcquireChannelsFails) {
                EXPECT_CALL(*(m_player.get()), urlSetSource(_)).Times(1);
                //EXPECT_CALL(*(m_player.get()), streamSetSource(_, _)).Times(0);
                EXPECT_CALL(*(m_player.get()), play(_)).Times(0);
                EXPECT_CALL(*(m_observer.get()), onNotificationRenderingFinished()).Times(0);
                //EXPECT_CALL(*(m_focusManager.get()), acquireChannel(_, _)).Times(1).WillRepeatedly(Return(false));
                ASSERT_FALSE(m_renderer->renderNotification(goodStreamFunction, ""));
            }
            TEST_F(NotificationRendererTest, testShutdown) {
                m_renderer->shutdown();
                m_renderer.reset();
                ASSERT_TRUE(m_player->getObservers().empty());
            }
        }
    }
}