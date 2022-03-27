#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockPlaybackRouter.h>
#include "BluetoothMediaInputTransformer.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        namespace test {
            using namespace testing;
            using namespace sdkInterfaces::test;
            using namespace avsCommon::sdkInterfaces::bluetooth::services;
            class MockPlaybackRouter : public PlaybackRouterInterface {
            public:
                MOCK_METHOD1(buttonPressed, void(PlaybackButton));
                //MOCK_METHOD2(togglePressed, void(PlaybackToggle, bool));
                MOCK_METHOD1(setHandler, void(shared_ptr<PlaybackHandlerInterface>));
                //MOCK_METHOD2(setHandler, void(shared_ptr<PlaybackHandlerInterface>, shared_ptr<LocalPlaybackHandlerInterface>));
                MOCK_METHOD0(switchToDefaultHandler, void());
            };
            class BluetoothMediaInputTransformerTest : public Test {
            public:
                void SetUp();
            protected:
                shared_ptr<BluetoothEventBus> m_eventBus;
                shared_ptr<MockPlaybackRouter> m_mockRouter;
                shared_ptr<BluetoothMediaInputTransformer> m_mediaInputTransformer;
            };
            void BluetoothMediaInputTransformerTest::SetUp() {
                m_eventBus = make_shared<BluetoothEventBus>();
                //m_mockRouter = make_shared<MockPlaybackRouter>();
                m_mediaInputTransformer = BluetoothMediaInputTransformer::create(m_eventBus, m_mockRouter);
            }
            TEST_F(BluetoothMediaInputTransformerTest, test_createWithNullParams) {
                ASSERT_THAT(BluetoothMediaInputTransformer::create(m_eventBus, nullptr), IsNull());
                ASSERT_THAT(BluetoothMediaInputTransformer::create(nullptr, m_mockRouter), IsNull());
            }
            TEST_F(BluetoothMediaInputTransformerTest, test_handlePlayCommand) {
                EXPECT_CALL(*m_mockRouter, buttonPressed(PlaybackButton::PLAY)).Times(1);
                MediaCommandReceivedEvent event(MediaCommand::PLAY);
                m_eventBus->sendEvent(event);
            }
            TEST_F(BluetoothMediaInputTransformerTest, test_handlePauseCommand) {
                EXPECT_CALL(*m_mockRouter, buttonPressed(PlaybackButton::PAUSE)).Times(1);
                MediaCommandReceivedEvent event(MediaCommand::PAUSE);
                m_eventBus->sendEvent(event);
            }
            TEST_F(BluetoothMediaInputTransformerTest, test_handleNextCommand) {
                EXPECT_CALL(*m_mockRouter, buttonPressed(PlaybackButton::NEXT)).Times(1);
                MediaCommandReceivedEvent event(MediaCommand::NEXT);
                m_eventBus->sendEvent(event);
            }
            TEST_F(BluetoothMediaInputTransformerTest, test_handlePreviousCommand) {
                EXPECT_CALL(*m_mockRouter, buttonPressed(PlaybackButton::PREVIOUS)).Times(1);
                MediaCommandReceivedEvent event(MediaCommand::PREVIOUS);
                m_eventBus->sendEvent(event);
            }
            TEST_F(BluetoothMediaInputTransformerTest, handlePlayPauseCommand) {
                EXPECT_CALL(*m_mockRouter, buttonPressed(PlaybackButton::PLAY)).Times(1);
                MediaCommandReceivedEvent event(MediaCommand::PLAY_PAUSE);
                m_eventBus->sendEvent(event);
            }
            TEST_F(BluetoothMediaInputTransformerTest, test_unrelatedEvent) {
                //auto strictPlaybackRouter = make_shared<StrictMock<MockPlaybackRouter>>();
                //auto mediaInputTransformer = BluetoothMediaInputTransformer::create(m_eventBus, strictPlaybackRouter);
                DeviceDiscoveredEvent event(nullptr);
                m_eventBus->sendEvent(event);
            }
        }
    }
}