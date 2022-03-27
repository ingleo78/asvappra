#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <avs/Initialization/AlexaClientSDKInit.h>
#include <util/network/InternetConnectionMonitor.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/MessageObserverInterface.h>
#include <sdkinterfaces/MessageObserverInterface.h>
#include <acl/AVSConnectionManager.h>
#include "MessageRouterInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            using namespace std;
            using namespace testing;
            using namespace avsCommon;
            using namespace avs;
            using namespace initialization;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace network;
            using namespace test;
            class MockMessageObserver : public MessageObserverInterface {
            public:
                MockMessageObserver() {}
                MOCK_METHOD2(receive, void(const string& contextId, const string& message));
            };
            class MockConnectionStatusObserver : public ConnectionStatusObserverInterface {
            public:
                MockConnectionStatusObserver() {}
                MOCK_METHOD2(onConnectionStatusChanged, void(ConnectionStatusObserverInterface::Status status,
                             ConnectionStatusObserverInterface::ChangedReason reason));
            };
            class MockMessageRouter : public MessageRouterInterface {
            public:
                MockMessageRouter() : MessageRouterInterface{"MockMessageRouter"} {}
                MOCK_METHOD0(enable, void());
                MOCK_METHOD0(disable, void());
                MOCK_METHOD0(doShutdown, void());
                MOCK_METHOD0(getConnectionStatus, MessageRouterInterface::ConnectionStatus());
                MOCK_METHOD1(sendMessage, void(shared_ptr<MessageRequest> request));
                MOCK_METHOD1(setAVSGateway, void(const string& avsGateway));
                MOCK_METHOD0(getAVSGateway, string());
                MOCK_METHOD0(onWakeConnectionRetry, void());
                MOCK_METHOD0(onWakeVerifyConnectivity, void());
                MOCK_METHOD1(setObserver, void(shared_ptr<MessageRouterObserverInterface> observer));
            };
            class AVSConnectionManagerTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                shared_ptr<AVSConnectionManager> m_avsConnectionManager;
                shared_ptr<MockMessageRouter> m_messageRouter;
                shared_ptr<MockConnectionStatusObserver> m_observer;
                shared_ptr<MockMessageObserver> m_messageObserver;
                shared_ptr<InternetConnectionMonitor> m_mockConnectionMonitor;
            };
            void AVSConnectionManagerTest::SetUp() {
                AlexaClientSDKInit::initialize(vector<shared_ptr<istream>>());
                m_messageRouter = make_shared<MockMessageRouter>();
                m_observer = make_shared<MockConnectionStatusObserver>();
                m_messageObserver = make_shared<MockMessageObserver>();
                //m_mockConnectionMonitor = make_shared<InternetConnectionMonitor>();
                m_avsConnectionManager = AVSConnectionManager::create(m_messageRouter,true,
                                                unordered_set<shared_ptr<ConnectionStatusObserverInterface>>(),
                                                      unordered_set<shared_ptr<MessageObserverInterface>>(),
                                                m_mockConnectionMonitor);
                EXPECT_THAT(m_avsConnectionManager, NotNull());
            }
            void AVSConnectionManagerTest::TearDown() {
                AlexaClientSDKInit::uninitialize();
            }
            TEST_F(AVSConnectionManagerTest, test_create) {
                EXPECT_CALL(*m_messageRouter, setObserver(_)).Times(1);
                EXPECT_CALL(*m_messageRouter, enable()).Times(1);
                ASSERT_NE(nullptr, m_avsConnectionManager->create(m_messageRouter, true, {m_observer}, {m_messageObserver}));
            }
            TEST_F(AVSConnectionManagerTest, test_createWithNullMessageRouterAndObservers) {
                ASSERT_EQ(nullptr, m_avsConnectionManager->create(nullptr, true, {m_observer}, {m_messageObserver}));
                ASSERT_EQ(nullptr, m_avsConnectionManager->create(m_messageRouter, true, {nullptr}, {m_messageObserver}));
                ASSERT_EQ(nullptr, m_avsConnectionManager->create(m_messageRouter, true, {m_observer}, {nullptr}));
                ASSERT_NE(nullptr, m_avsConnectionManager->create(m_messageRouter, true, {m_observer}, {m_messageObserver}, nullptr));
                ASSERT_NE(nullptr,m_avsConnectionManager->create(m_messageRouter, true, {m_observer}, {m_messageObserver}, m_mockConnectionMonitor));
                shared_ptr<MockConnectionStatusObserver> validConnectionStatusObserver;
                validConnectionStatusObserver = make_shared<MockConnectionStatusObserver>();
                ASSERT_EQ(nullptr,m_avsConnectionManager->create(m_messageRouter, true, {m_observer, nullptr, validConnectionStatusObserver},
                          {m_messageObserver}));
                shared_ptr<MockMessageObserver> validMessageObserver;
                validMessageObserver = make_shared<MockMessageObserver>();
                ASSERT_EQ(nullptr,m_avsConnectionManager->create(m_messageRouter, true, {m_observer}, {m_messageObserver, nullptr, validMessageObserver}));
                ASSERT_EQ(nullptr, m_avsConnectionManager->create(m_messageRouter, true, {nullptr}, {nullptr}));
                ASSERT_NE(nullptr,m_avsConnectionManager->create(m_messageRouter, true, unordered_set<shared_ptr<ConnectionStatusObserverInterface>>(),
                          {validMessageObserver}));
                ASSERT_NE(nullptr,m_avsConnectionManager->create(m_messageRouter, true, {validConnectionStatusObserver},
                          unordered_set<shared_ptr<MessageObserverInterface>>()));
                ASSERT_NE(nullptr,m_avsConnectionManager->create(m_messageRouter, true, {validConnectionStatusObserver}, {validMessageObserver}));
            }
            TEST_F(AVSConnectionManagerTest, test_addConnectionStatusObserverNull) {
                EXPECT_CALL(*m_messageRouter, getConnectionStatus()).Times(0);
                m_avsConnectionManager->addConnectionStatusObserver(nullptr);
            }
            TEST_F(AVSConnectionManagerTest, test_addConnectionStatusObserverValid) {
                EXPECT_CALL(*m_observer, onConnectionStatusChanged(_, _)).Times(1);
                m_avsConnectionManager->addConnectionStatusObserver(m_observer);
            }
            TEST_F(AVSConnectionManagerTest, test_removeConnectionStatusObserverNull) {
                m_avsConnectionManager->removeConnectionStatusObserver(nullptr);
            }
            TEST_F(AVSConnectionManagerTest, test_addMessageObserverNull) {
                m_avsConnectionManager->addMessageObserver(nullptr);
            }
            TEST_F(AVSConnectionManagerTest, test_removeMessageObserverNull) {
                m_avsConnectionManager->removeMessageObserver(nullptr);
            }
            TEST_F(AVSConnectionManagerTest, test_enableAndDisableFunction) {
                EXPECT_CALL(*m_messageRouter, enable()).Times(1);
                m_avsConnectionManager->enable();
                ASSERT_TRUE(m_avsConnectionManager->isEnabled());
                EXPECT_CALL(*m_messageRouter, disable()).Times(1);
                m_avsConnectionManager->disable();
                ASSERT_FALSE(m_avsConnectionManager->isEnabled());
            }
            TEST_F(AVSConnectionManagerTest, test_sendMessageRequest) {
                EXPECT_CALL(*m_messageRouter, sendMessage(_)).Times(1);
                m_avsConnectionManager->sendMessage(nullptr);
                EXPECT_CALL(*m_messageRouter, sendMessage(_)).Times(1);
                shared_ptr<MessageRequest> messageRequest;
                messageRequest = make_shared<MessageRequest>("Test message");
                m_avsConnectionManager->sendMessage(messageRequest);
            }
            TEST_F(AVSConnectionManagerTest, test_setAVSGateway) {
                EXPECT_CALL(*m_messageRouter, setAVSGateway(_)).Times(1);
                m_avsConnectionManager->setAVSGateway("AVSGateway");
            }
            TEST_F(AVSConnectionManagerTest, getAVSGatewayTest) {
                auto gateway = "AVSGateway";
                EXPECT_CALL(*m_messageRouter, getAVSGateway()).Times(1).WillOnce(Return(gateway));
                ASSERT_EQ(gateway, m_avsConnectionManager->getAVSGateway());
            }
            TEST_F(AVSConnectionManagerTest, test_enabledOnConnectStatusChangedToFalse) {
                auto messageRouter = make_shared<MockMessageRouter>();
                {
                    InSequence dummy;
                    EXPECT_CALL(*messageRouter, enable());
                    EXPECT_CALL(*messageRouter, onWakeVerifyConnectivity());
                }
                m_avsConnectionManager = AVSConnectionManager::create(messageRouter,true,
                                                unordered_set<shared_ptr<ConnectionStatusObserverInterface>>(),
                                                      unordered_set<shared_ptr<MessageObserverInterface>>());
                m_avsConnectionManager->onConnectionStatusChanged(false);
                m_avsConnectionManager.reset();
            }
            TEST_F(AVSConnectionManagerTest, test_enabledOnConnectStatusChangedToTrue) {
                auto messageRouter = std::make_shared<MockMessageRouter>();
                {
                    InSequence dummy;
                    EXPECT_CALL(*messageRouter, enable());
                    EXPECT_CALL(*messageRouter, onWakeConnectionRetry());
                }
                m_avsConnectionManager = AVSConnectionManager::create(messageRouter,true,
                                                unordered_set<shared_ptr<ConnectionStatusObserverInterface>>(),
                                                      unordered_set<shared_ptr<MessageObserverInterface>>());
                m_avsConnectionManager->onConnectionStatusChanged(true);
                m_avsConnectionManager.reset();
            }
            TEST_F(AVSConnectionManagerTest, test_disabledOnConnectStatusChanged) {
                auto messageRouter = make_shared<MockMessageRouter>();
                {
                    InSequence dummy;
                    EXPECT_CALL(*messageRouter, enable()).Times(0);
                    EXPECT_CALL(*messageRouter, disable()).Times(0);
                    EXPECT_CALL(*messageRouter, onWakeVerifyConnectivity()).Times(0);
                    EXPECT_CALL(*messageRouter, onWakeConnectionRetry()).Times(0);
                }
                m_avsConnectionManager = AVSConnectionManager::create(messageRouter,false,
                                                unordered_set<shared_ptr<ConnectionStatusObserverInterface>>(),
                                                      unordered_set<shared_ptr<MessageObserverInterface>>());
                m_avsConnectionManager->onConnectionStatusChanged(true);
                m_avsConnectionManager->onConnectionStatusChanged(false);
                m_avsConnectionManager.reset();
            }
        }
    }
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}