#include <chrono>
#include <memory>
#include <queue>
#include <gtest/gtest.h>
#include <registration_manager/CustomerDataManager.h>
#include <avs/AbstractAVSConnectionManager.h>
#include <avs/Initialization/AlexaClientSDKInit.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <util/PromiseFuturePair.h>
#include "CertifiedSender.h"

namespace alexaClientSDK {
    namespace certifiedSender {
        namespace test {
            using namespace testing;
            using namespace initialization;
            using namespace sdkInterfaces::test;
            using Status = ConnectionStatusObserverInterface::Status;
            using ChangedReason = ConnectionStatusObserverInterface::ChangedReason;
            static const string TEST_MESSAGE = "TEST_MESSAGE";
            static const string TEST_URI = "TEST_URI";
            static const auto TEST_TIMEOUT = seconds(5);
            class MockConnection : public AbstractAVSConnectionManager {
                MOCK_METHOD0(enable, void());
                MOCK_METHOD0(disable, void());
                MOCK_METHOD0(isEnabled, bool());
                MOCK_METHOD0(reconnect, void());
                MOCK_CONST_METHOD0(isConnected, bool());
                MOCK_METHOD0(onWakeConnectionRetry, void());
                MOCK_METHOD0(onWakeVerifyConnectivity, void());
                MOCK_METHOD1(addMessageObserver, void(shared_ptr<MessageObserverInterface> observer));
                MOCK_METHOD1(removeMessageObserver, void(shared_ptr<MessageObserverInterface> observer));
            };
            class MockMessageStorage : public MessageStorageInterface {
            public:
                MOCK_METHOD0(createDatabase, bool());
                MOCK_METHOD0(open, bool());
                MOCK_METHOD0(close, void());
                //MOCK_METHOD2(store, bool(const string& message, int* id));
                MOCK_METHOD3(store, bool(const string& message, const string& uriPathExtension, int* id));
                MOCK_METHOD1(load, bool(queue<StoredMessage>* messageContainer));
                MOCK_METHOD1(erase, bool(int messageId));
                MOCK_METHOD0(clearDatabase, bool());
                virtual ~MockMessageStorage() = default;
            };
            class CertifiedSenderTest : public Test {
            public:
            protected:
                void SetUp() override {
                    static const string CONFIGURATION = R"({"certifiedSender" : {"databaseFilePath":"database.db"}})";
                    auto configuration = shared_ptr<stringstream>(new stringstream());
                    (*configuration) << CONFIGURATION;
                    ASSERT_TRUE(AlexaClientSDKInit::initialize({configuration}));
                    m_customerDataManager = make_shared<registrationManager::CustomerDataManager>();
                    m_mockMessageSender = make_shared<MockMessageSender>();
                    m_connection = make_shared<MockConnection>();
                    //m_storage = make_shared<MockMessageStorage>();
                    EXPECT_CALL(*m_storage, open()).Times(1).WillOnce(Return(true));
                    EXPECT_CALL(*m_storage, load(_)).Times(1).WillOnce(Return(true));
                    m_certifiedSender = CertifiedSender::create(m_mockMessageSender, m_connection, m_storage,
                                                                m_customerDataManager);
                }
                void TearDown() override {
                    if (AlexaClientSDKInit::isInitialized()) AlexaClientSDKInit::uninitialize();
                    m_certifiedSender->shutdown();
                }
                shared_ptr<CertifiedSender> m_certifiedSender;
                shared_ptr<MockMessageStorage> m_storage;
                shared_ptr<MockConnection> m_connection;
                shared_ptr<CustomerDataManager> m_customerDataManager;
                shared_ptr<MockMessageSender> m_mockMessageSender;
            };
            TEST_F(CertifiedSenderTest, test_clearData) {
                EXPECT_CALL(*m_storage, clearDatabase()).Times(1);
                m_certifiedSender->clearData();
            }
            TEST_F(CertifiedSenderTest, test_initFailsWhenStorageMethodsFail) {
                {
                    EXPECT_CALL(*m_storage, open()).Times(1).WillOnce(Return(false));
                    EXPECT_CALL(*m_storage, createDatabase()).Times(1).WillOnce(Return(false));
                    EXPECT_CALL(*m_storage, load(_)).Times(0);
                    auto certifiedSender = CertifiedSender::create(m_mockMessageSender, m_connection, m_storage,
                                                                   m_customerDataManager);
                    ASSERT_EQ(certifiedSender, nullptr);
                }
                {
                    EXPECT_CALL(*m_storage, open()).Times(1).WillOnce(Return(true));
                    EXPECT_CALL(*m_storage, load(_)).Times(1).WillOnce(Return(false));
                    auto certifiedSender = CertifiedSender::create(m_mockMessageSender, m_connection, m_storage,
                                                                   m_customerDataManager);
                    ASSERT_EQ(certifiedSender, nullptr);
                }
            }
            TEST_F(CertifiedSenderTest, testTimer_storedMessagesGetSent) {
                EXPECT_CALL(*m_storage, open()).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*m_storage, load(_)).Times(1)
                    .WillOnce(Invoke([](std::queue<MessageStorageInterface::StoredMessage>* storedMessages) {
                        storedMessages->push(MessageStorageInterface::StoredMessage(1, "testMessage_1"));
                        storedMessages->push(MessageStorageInterface::StoredMessage(2, "testMessage_2"));
                        return true;
                    }));
                avsCommon::utils::PromiseFuturePair<bool> allRequestsSent;
                {
                    InSequence s;
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(1)
                        .WillOnce(Invoke([](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
                            ASSERT_EQ(request->getJsonContent(), "testMessage_1");
                            request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS);
                        }));
                    EXPECT_CALL(*m_storage, erase(1)).WillOnce(Return(true));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(1)
                        .WillOnce(Invoke([](shared_ptr<MessageRequest> request) {
                            ASSERT_EQ(request->getJsonContent(), "testMessage_2");
                            request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS);
                        }));
                    EXPECT_CALL(*m_storage, erase(2)).WillOnce(Invoke([&allRequestsSent](int messageId) {
                        allRequestsSent.setValue(true);
                        return true;
                    }));
                }
                auto certifiedSender = CertifiedSender::create(m_mockMessageSender, m_connection, m_storage,
                                                               m_customerDataManager);
                static_pointer_cast<ConnectionStatusObserverInterface>(certifiedSender)->onConnectionStatusChanged(Status::CONNECTED,
                                                                                                                   ChangedReason::SUCCESS);
                EXPECT_TRUE(allRequestsSent.waitFor(TEST_TIMEOUT));
                certifiedSender->shutdown();
            }
            TEST_F(CertifiedSenderTest, testTimer_SendMessageWithURI) {
                PromiseFuturePair<shared_ptr<MessageRequest>> requestSent;
                EXPECT_CALL(*m_storage, store(_, TEST_URI, _)).WillOnce(Return(true));
                {
                    InSequence s;
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                        .WillOnce(Invoke([&requestSent](shared_ptr<MessageRequest> request) {
                            requestSent.setValue(request);
                            request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS);
                        }));
                    EXPECT_CALL(*m_storage, erase(_)).WillOnce(Return(true));
                }
                static_pointer_cast<ConnectionStatusObserverInterface>(m_certifiedSender)->onConnectionStatusChanged(Status::CONNECTED,
                                                                                                                     ChangedReason::SUCCESS);
                m_certifiedSender->sendJSONMessage(TEST_MESSAGE, TEST_URI);
                EXPECT_TRUE(requestSent.waitFor(TEST_TIMEOUT));
                EXPECT_EQ(requestSent.getValue()->getJsonContent(), TEST_MESSAGE);
                EXPECT_EQ(requestSent.getValue()->getUriPathExtension(), TEST_URI);
            }
        }
    }
}