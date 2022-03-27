#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockDirectiveSequencer.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <settings/Settings/MockDeviceSettingStorage.h>
#include "DoNotDisturbCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace doNotDisturb {
            namespace test {
                using namespace std;
                using namespace chrono;
                using namespace testing;
                using namespace avsCommon;
                using namespace avs;
                using namespace sdkInterfaces;
                using namespace utils;
                using namespace settings;
                using namespace storage;
                using namespace registrationManager;
                using namespace sdkInterfaces::test;
                using namespace storage::test;
                static const seconds MY_WAIT_TIMEOUT(2);
                static const string SETDNDMODE_DIRECTIVE_VALID_JSON_STRING = R"delim({"directive": {"header": {"namespace": "DoNotDisturb","name": "SetDoNotDisturb",
                                                                             "messageId": "12345"},"payload": {"enabled": true}}})delim";
                static const string DND_REPORT_EVENT = "ReportDoNotDisturb";
                static const string DND_CHANGE_EVENT = "DoNotDisturbChanged";
                class DoNotDisturbCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                protected:
                    void TearDown() override;
                public:
                    bool expectEventSend(const string& eventName, MessageRequestObserverInterface::Status statusReported, function<void()> triggerOperation);
                protected:
                    shared_ptr<DoNotDisturbCapabilityAgent> m_dndCA;
                    shared_ptr<MockMessageSender> m_messageSender;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
                    shared_ptr<MockDeviceSettingStorage> m_settingsStorage;
                };
                void DoNotDisturbCapabilityAgentTest::SetUp() {
                    m_messageSender = make_shared<MockMessageSender>();
                    m_mockExceptionEncounteredSender = make_shared<MockExceptionEncounteredSender>();
                    //m_settingsStorage = std::make_shared<MockDeviceSettingStorage>();
                    EXPECT_CALL(*m_settingsStorage, storeSetting(_, _, _)).WillRepeatedly(Return(true));
                    //EXPECT_CALL(*m_settingsStorage, updateSettingStatus(_, _)).WillRepeatedly(Return(true));
                    EXPECT_CALL(*m_settingsStorage, loadSetting(_)).WillRepeatedly(Return(make_pair(SettingStatus::SYNCHRONIZED, "true")));
                    m_dndCA = DoNotDisturbCapabilityAgent::create(m_mockExceptionEncounteredSender, m_messageSender, m_settingsStorage);
                    ASSERT_NE(m_dndCA, nullptr);
                }
                bool DoNotDisturbCapabilityAgentTest::expectEventSend(const string& eventName, MessageRequestObserverInterface::Status statusReported,
                                                                      function<void()> triggerOperation) {
                    promise<bool> eventPromise;
                    EXPECT_CALL(*m_messageSender, sendMessage(_)).Times(1)
                        .WillOnce(Invoke([statusReported, &eventName, &eventPromise](shared_ptr<MessageRequest> request) {
                            if (request->getJsonContent().find(eventName) != string::npos) {
                                request->sendCompleted(statusReported);
                                eventPromise.set_value(true);
                                return;
                            }
                            eventPromise.set_value(false);
                        }));
                    triggerOperation();
                    auto future = eventPromise.get_future();
                    bool isFutureReady = future.wait_for(MY_WAIT_TIMEOUT) == future_status::ready;
                    EXPECT_TRUE(isFutureReady);
                    if (!isFutureReady) return false;
                    return future.get();
                }
                void DoNotDisturbCapabilityAgentTest::TearDown() {
                    if (m_dndCA) m_dndCA->shutdown();
                }
                TEST_F(DoNotDisturbCapabilityAgentTest, test_givenInvalidParameters_create_shouldFail) {
                    auto dndCA = DoNotDisturbCapabilityAgent::create(nullptr, m_messageSender, m_settingsStorage);
                    EXPECT_THAT(dndCA, IsNull());
                    dndCA = DoNotDisturbCapabilityAgent::create(m_mockExceptionEncounteredSender, nullptr, m_settingsStorage);
                    EXPECT_THAT(dndCA, IsNull());
                    dndCA = DoNotDisturbCapabilityAgent::create(m_mockExceptionEncounteredSender, m_messageSender, nullptr);
                    EXPECT_THAT(dndCA, IsNull());
                }
                TEST_F(DoNotDisturbCapabilityAgentTest, test_givenValidSetDNDDirective_handleDirective_shouldSucceed) {
                    bool initialReportSent = expectEventSend(DND_REPORT_EVENT, MessageRequestObserverInterface::Status::SUCCESS, [this]() {
                                                 m_dndCA->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                                                                                    ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                                             });
                    ASSERT_TRUE(initialReportSent);
                    auto directivePair = AVSDirective::create(SETDNDMODE_DIRECTIVE_VALID_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    bool directiveResponseEventSent = expectEventSend(DND_REPORT_EVENT, MessageRequestObserverInterface::Status::SUCCESS,
                                                       [this, &directive]() { m_dndCA->handleDirectiveImmediately(directive); });
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(DoNotDisturbCapabilityAgentTest, test_beingOnline_applyLocalChange_shouldSendReport) {
                    bool initialReportSent = expectEventSend(DND_REPORT_EVENT, MessageRequestObserverInterface::Status::SUCCESS, [this]() {
                                                                 m_dndCA->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                                                                                             ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                                                             });
                    ASSERT_TRUE(initialReportSent);
                    bool changeEventSent = expectEventSend(DND_CHANGE_EVENT, MessageRequestObserverInterface::Status::SUCCESS, [this]() {
                                                               m_dndCA->sendChangedEvent("true");
                                                           });

                    ASSERT_TRUE(changeEventSent);
                }
                TEST_F(DoNotDisturbCapabilityAgentTest, test_beingOffline_applyLocalChangeAndBecomeOnline_shouldSendChanged) {
                    m_dndCA->sendChangedEvent("true");
                    bool changeEventSent = expectEventSend(DND_CHANGE_EVENT, MessageRequestObserverInterface::Status::SUCCESS, [this]() {
                                                               m_dndCA->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                                                                                           ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                                                           });
                    ASSERT_TRUE(changeEventSent);
                }
                TEST_F(DoNotDisturbCapabilityAgentTest, test_whileSendingChangedEvent_sendChangedFail_shouldSendReport) {
                    bool initialReportSent = expectEventSend(DND_REPORT_EVENT, MessageRequestObserverInterface::Status::SUCCESS, [this]() {
                                                                 m_dndCA->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                                                                                             ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                                                             });
                    ASSERT_TRUE(initialReportSent);
                    int eventMask = 0;
                    promise<bool> eventPromise;
                    EXPECT_CALL(*m_messageSender, sendMessage(_)).Times(2)
                        .WillRepeatedly(Invoke([&eventMask, &eventPromise](shared_ptr<MessageRequest> request) {
                            if (request->getJsonContent().find(DND_CHANGE_EVENT) != string::npos) {
                                eventMask <<= 1;
                                eventMask |= 1;
                                request->sendCompleted(MessageRequestObserverInterface::Status::INTERNAL_ERROR);
                                return;
                            } else if (request->getJsonContent().find(DND_REPORT_EVENT) != string::npos) {
                                eventMask <<= 1;
                                eventPromise.set_value(true);
                                return;
                            }
                            eventPromise.set_value(false);
                        }));
                    m_dndCA->sendChangedEvent("true");
                    auto future = eventPromise.get_future();
                    bool isFutureReady = future.wait_for(MY_WAIT_TIMEOUT) == future_status::ready;
                    EXPECT_TRUE(isFutureReady);
                    EXPECT_EQ(eventMask, 2);
                }
            }
        }
    }
}