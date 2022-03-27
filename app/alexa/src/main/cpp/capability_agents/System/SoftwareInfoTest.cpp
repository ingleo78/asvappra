#include <chrono>
#include <string>
#include <gtest/gtest.h>
#include <json/document.h>
#include <avs/AbstractAVSConnectionManager.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockMessageSender.h>
#include "SoftwareInfoSender.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            namespace test {
                using namespace testing;
                using namespace attachment;
                using namespace logger;
                using namespace rapidjson;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                static const string NAMESPACE_SYSTEM = "System";
                static const string NAME_SOFTWARE_INFO = "SoftwareInfo";
                static const string NAME_REPORT_SOFTWARE_INFO = "ReportSoftwareInfo";
                static const string MESSAGE_ID = "Message-1";
                static const string DIALOG_REQUEST_ID = "";
                static const string UNPARSED_DIRECTIVE = "";
                static const auto REPORT_SOFTWARE_INFO_DIRECTIVE_HEADER = make_shared<AVSMessageHeader>(NAMESPACE_SYSTEM,
                                                                                                        NAME_REPORT_SOFTWARE_INFO,
                                                                                                        MESSAGE_ID, DIALOG_REQUEST_ID);
                static const string TEST_PAYLOAD = "";
                static const string ATTACHMENT_CONTEXT_ID = "";
                static const milliseconds EXPECTED_TIMEOUT(100);
                static const seconds UNEXPECTED_TIMEOUT(5);
                static const seconds TWO_RETRIES_TIMEOUT(15);
                static const FirmwareVersion FIRST_FIRMWARE_VERSION = 1;
                static const FirmwareVersion SECOND_FIRMWARE_VERSION = 2;
                static const FirmwareVersion THIRD_FIRMWARE_VERSION = 3;
                class MockSoftwareInfoSenderObserver : public SoftwareInfoSenderObserverInterface {
                public:
                    MOCK_METHOD1(onFirmwareVersionAccepted, void(FirmwareVersion));
                };
                class MockConnection : public AbstractAVSConnectionManager {
                public:
                    MockConnection();
                    MOCK_METHOD0(enable, void());
                    MOCK_METHOD0(disable, void());
                    MOCK_METHOD0(isEnabled, bool());
                    MOCK_METHOD0(reconnect, void());
                    MOCK_METHOD0(onWakeConnectionRetry, void());
                    MOCK_METHOD0(onWakeVerifyConnectivity, void());
                    MOCK_METHOD1(addMessageObserver, void(shared_ptr<MessageObserverInterface> observer));
                    MOCK_METHOD1(removeMessageObserver, void(shared_ptr<MessageObserverInterface> observer));
                    bool isConnected() const;
                    void updateConnectionStatus(Status status, ChangedReason reason);
                };
                MockConnection::MockConnection() : AbstractAVSConnectionManager{} {}
                bool MockConnection::isConnected() const {
                    return Status::CONNECTED == m_connectionStatus;
                }
                void MockConnection::updateConnectionStatus(Status status, ChangedReason reason) {
                    AbstractAVSConnectionManager::updateConnectionStatus(status, reason);
                }
                class SoftwareInfoSenderTest : public Test {
                public:
                    void SetUp() override;
                protected:
                    shared_ptr<StrictMock<MockSoftwareInfoSenderObserver>> m_mockObserver;
                    unordered_set<std::shared_ptr<SoftwareInfoSenderObserverInterface>> m_mockObservers;
                    shared_ptr<StrictMock<MockConnection>> m_mockConnection;
                    shared_ptr<StrictMock<MockMessageSender>> m_mockMessageSender;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionEncounteredSender;
                    shared_ptr<StrictMock<MockAttachmentManager>> m_mockAttachmentManager;
                    shared_ptr<AVSDirective> m_reportSoftwareInfoDirective;
                };
                void SoftwareInfoSenderTest::SetUp() {
                    m_mockObserver = make_shared<StrictMock<MockSoftwareInfoSenderObserver>>();
                    m_mockObservers.insert(m_mockObserver);
                    m_mockConnection = make_shared<StrictMock<MockConnection>>();
                    m_mockMessageSender = make_shared<StrictMock<MockMessageSender>>();
                    m_mockExceptionEncounteredSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockAttachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    m_reportSoftwareInfoDirective = AVSDirective::create(UNPARSED_DIRECTIVE, REPORT_SOFTWARE_INFO_DIRECTIVE_HEADER, TEST_PAYLOAD,
                                                                         m_mockAttachmentManager, ATTACHMENT_CONTEXT_ID);
                }
                TEST_F(SoftwareInfoSenderTest, test_createFailedInvalidFirmwareVersion) {
                    ASSERT_FALSE(SoftwareInfoSender::create(INVALID_FIRMWARE_VERSION, true, m_mockObservers, m_mockConnection, m_mockMessageSender,
                                 m_mockExceptionEncounteredSender));
                }
                TEST_F(SoftwareInfoSenderTest, test_createSuccessWithsendSoftwareInfoUponConnectFalse) {
                    auto softwareInfoSender = SoftwareInfoSender::create(FIRST_FIRMWARE_VERSION,false,m_mockObservers,
                                                                         m_mockConnection,m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_createSuccessWithObserverNull) {
                    unordered_set<shared_ptr<SoftwareInfoSenderObserverInterface>> observers;
                    auto softwareInfoSender = SoftwareInfoSender::create(1, true, observers, m_mockConnection,
                                                                         m_mockMessageSender, m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_createFailedConnectionNull) {
                    ASSERT_FALSE(SoftwareInfoSender::create(1, true, m_mockObservers, nullptr, m_mockMessageSender, m_mockExceptionEncounteredSender));
                }
                TEST_F(SoftwareInfoSenderTest, test_createFailedMessageSenderNull) {
                    ASSERT_FALSE(SoftwareInfoSender::create(1, true, m_mockObservers, m_mockConnection, nullptr, m_mockExceptionEncounteredSender));
                }
                TEST_F(SoftwareInfoSenderTest, test_createFailedExceptionEncounteredSenderNull) {
                    ASSERT_FALSE(SoftwareInfoSender::create(1, true, m_mockObservers, m_mockConnection, m_mockMessageSender, nullptr));
                }
                TEST_F(SoftwareInfoSenderTest, test_noSoftwareInfoEventSentByDefault) {
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(_)).Times(0);
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(0);
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(FIRST_FIRMWARE_VERSION,false,m_mockObservers,
                                                                         m_mockConnection,m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::DISCONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_nothingSentBeforeConnected) {
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(_)).Times(0);
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(0);
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(1, true, m_mockObservers,
                                                                         m_mockConnection, m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_softwareInfoSentUponConnectIfSendSetTrueBeforeConnect) {
                    promise<void> versionAcceptedPromise;
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs([&versionAcceptedPromise]() { versionAcceptedPromise.set_value(); }));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(1)
                        .WillOnce(Invoke([](shared_ptr<MessageRequest> request) {
                            request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT);
                        }));
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(
                        1, true, m_mockObservers, m_mockConnection, m_mockMessageSender, m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::DISCONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    auto versionAcceptedFuture = versionAcceptedPromise.get_future();
                    ASSERT_NE(versionAcceptedFuture.wait_for(UNEXPECTED_TIMEOUT), std::future_status::timeout);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_reportSoftwareInfoReceived) {
                    promise<void> versionAcceptedPromise;
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs([&versionAcceptedPromise]() { versionAcceptedPromise.set_value(); }));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(1)
                        .WillOnce(Invoke([](shared_ptr<avsCommon::avs::MessageRequest> request) {
                            request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT);
                        }));
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(FIRST_FIRMWARE_VERSION,false,m_mockObservers,
                                                                         m_mockConnection,m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    softwareInfoSender->handleDirectiveImmediately(m_reportSoftwareInfoDirective);
                    auto versionAcceptedFuture = versionAcceptedPromise.get_future();
                    ASSERT_NE(versionAcceptedFuture.wait_for(UNEXPECTED_TIMEOUT), std::future_status::timeout);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_reportSoftwareInfoCancellsPreviousDirective) {
                    atomic<MessageRequestObserverInterface::Status> status(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                    promise<void> versionAcceptedPromise;
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs([&versionAcceptedPromise]() { versionAcceptedPromise.set_value(); }));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(2)
                        .WillRepeatedly(Invoke([&status](std::shared_ptr<MessageRequest> request) {
                            this_thread::sleep_for(EXPECTED_TIMEOUT);
                            request->sendCompleted(status);
                        }));
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(FIRST_FIRMWARE_VERSION,false,m_mockObservers,
                                                                         m_mockConnection,m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    softwareInfoSender->handleDirectiveImmediately(m_reportSoftwareInfoDirective);
                    status = MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT;
                    softwareInfoSender->handleDirectiveImmediately(m_reportSoftwareInfoDirective);
                    this_thread::sleep_for(4 * EXPECTED_TIMEOUT);
                    auto versionAcceptedFuture = versionAcceptedPromise.get_future();
                    ASSERT_NE(versionAcceptedFuture.wait_for(UNEXPECTED_TIMEOUT), future_status::timeout);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_delayedReportSoftwareInfoNotifiesOnce) {
                    promise<void> versionAcceptedPromise;
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(_)).Times(2)
                        .WillOnce(InvokeWithoutArgs([&versionAcceptedPromise]() { versionAcceptedPromise.set_value(); }));
                    promise<void> messageSentTwicePromise;
                    int sentCounter = 0;
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(2)
                        .WillRepeatedly(Invoke([&messageSentTwicePromise, &sentCounter](shared_ptr<MessageRequest> request) {
                            request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT);
                            if (++sentCounter == 2) messageSentTwicePromise.set_value();
                        }));
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(FIRST_FIRMWARE_VERSION,false,m_mockObservers,
                                                                         m_mockConnection,m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    softwareInfoSender->handleDirectiveImmediately(m_reportSoftwareInfoDirective);
                    auto versionAcceptedFuture = versionAcceptedPromise.get_future();
                    ASSERT_NE(versionAcceptedFuture.wait_for(UNEXPECTED_TIMEOUT), std::future_status::timeout);
                    softwareInfoSender->handleDirectiveImmediately(m_reportSoftwareInfoDirective);
                    auto messageSentTwiceFuture = messageSentTwicePromise.get_future();
                    ASSERT_NE(messageSentTwiceFuture.wait_for(UNEXPECTED_TIMEOUT), std::future_status::timeout);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, testSlow_verifySendRetries) {
                    promise<void> versionAcceptedPromise;
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs([&versionAcceptedPromise]() { versionAcceptedPromise.set_value(); }));
                    int callCount = 0;
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(3)
                        .WillRepeatedly(Invoke([&callCount](std::shared_ptr<MessageRequest> request) {
                            request->sendCompleted(++callCount == 3 ? MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT :
                                                   MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                        }));
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(1, true, m_mockObservers,
                                                                         m_mockConnection, m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    auto versionAcceptedFuture = versionAcceptedPromise.get_future();
                    ASSERT_NE(versionAcceptedFuture.wait_for(TWO_RETRIES_TIMEOUT), future_status::timeout);
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_setInvalidFirmwareVersion) {
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(0);
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(FIRST_FIRMWARE_VERSION,false,m_mockObservers,
                                                                         m_mockConnection,m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    ASSERT_FALSE(softwareInfoSender->setFirmwareVersion(INVALID_FIRMWARE_VERSION));
                    softwareInfoSender->shutdown();
                }
                TEST_F(SoftwareInfoSenderTest, test_setFirmwareVersionCancellsPreviousSetting) {
                    atomic<MessageRequestObserverInterface::Status> status(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                    promise<void> versionAcceptedPromise;
                    EXPECT_CALL(*(m_mockObserver.get()), onFirmwareVersionAccepted(THIRD_FIRMWARE_VERSION)).Times(1)
                        .WillOnce(InvokeWithoutArgs([&versionAcceptedPromise]() { versionAcceptedPromise.set_value(); }));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(2)
                        .WillRepeatedly(Invoke([&status](std::shared_ptr<MessageRequest> request) {
                            this_thread::sleep_for(EXPECTED_TIMEOUT);
                            request->sendCompleted(status);
                        }));
                    EXPECT_CALL(*(m_mockExceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(0);
                    auto softwareInfoSender = SoftwareInfoSender::create(FIRST_FIRMWARE_VERSION,false,m_mockObservers,
                                                                         m_mockConnection,m_mockMessageSender,
                                                                         m_mockExceptionEncounteredSender);
                    ASSERT_TRUE(softwareInfoSender);
                    m_mockConnection->updateConnectionStatus(Status::PENDING, ChangedReason::ACL_CLIENT_REQUEST);
                    m_mockConnection->updateConnectionStatus(Status::CONNECTED, ChangedReason::ACL_CLIENT_REQUEST);
                    softwareInfoSender->setFirmwareVersion(SECOND_FIRMWARE_VERSION);
                    status = MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT;
                    softwareInfoSender->setFirmwareVersion(THIRD_FIRMWARE_VERSION);
                    this_thread::sleep_for(4 * EXPECTED_TIMEOUT);
                    auto versionAcceptedFuture = versionAcceptedPromise.get_future();
                    ASSERT_NE(versionAcceptedFuture.wait_for(UNEXPECTED_TIMEOUT),future_status::timeout);
                    softwareInfoSender->shutdown();
                }
            }
        }
    }
}