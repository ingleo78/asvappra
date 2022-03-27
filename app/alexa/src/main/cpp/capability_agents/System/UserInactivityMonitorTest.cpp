#include <gtest/gtest.h>
#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/MessageRequest.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockUserInactivityMonitorObserver.h>
#include <json/JSONUtils.h>
#include <adsl/DirectiveSequencer.h>
#include "UserInactivityMonitor.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            namespace test {
                using namespace adsl;
                using namespace testing;
                using namespace json;
                using namespace logger;
                using namespace rapidjson;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                using testing::InSequence;
                static const string USER_INACTIVITY_RESET_NAMESPACE = "System";
                static const string USER_INACTIVITY_RESET_NAME = "ResetUserInactivity";
                static const string USER_INACTIVITY_MESSAGE_ID = "ABC123DEF";
                static const string USER_INACTIVITY_PAYLOAD_KEY = "inactiveTimeInSeconds";
                static const milliseconds USER_INACTIVITY_REPORT_PERIOD{100};
                condition_variable exitTrigger;
                static bool checkMessageRequest(shared_ptr<MessageRequest> messageRequest) {
                    Document jsonContent(rapidjson::kObjectType);
                    if (jsonContent.Parse(messageRequest->getJsonContent().data()).HasParseError()) return false;
                    Value::ConstMemberIterator eventNode;
                    rapidjson::Value _jsonContent{jsonContent.GetString(), strlen(jsonContent.GetString())};
                    if (!jsonUtils::findNode(_jsonContent, "event", &eventNode)) return false;
                    rapidjson::Value::ConstMemberIterator payloadNode;
                    if (!jsonUtils::findNode(eventNode->value, "payload", &payloadNode)) return false;
                    rapidjson::Value::ConstMemberIterator inactivityNode;
                    if (!jsonUtils::findNode(payloadNode->value, USER_INACTIVITY_PAYLOAD_KEY, &inactivityNode)) return false;
                    return inactivityNode->value.IsUint64();
                }
                static bool checkMessageRequestAndReleaseTrigger(shared_ptr<MessageRequest> messageRequest) {
                    auto returnValue = checkMessageRequest(messageRequest);
                    exitTrigger.notify_all();
                    return returnValue;
                }
                class UserInactivityMonitorTest : public Test {
                public:
                    void SetUp() override;
                protected:
                    shared_ptr<StrictMock<MockMessageSender>> m_mockMessageSender;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionEncounteredSender;
                };
                void UserInactivityMonitorTest::SetUp() {
                    m_mockMessageSender = make_shared<StrictMock<MockMessageSender>>();
                    m_mockExceptionEncounteredSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                }
                TEST_F(UserInactivityMonitorTest, testTimer_createSuccessfully) {
                    mutex exitMutex;
                    unique_lock<mutex> exitLock(exitMutex);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(ResultOf(&checkMessageRequestAndReleaseTrigger, Eq(true))));
                    auto userInactivityMonitor = UserInactivityMonitor::create(m_mockMessageSender, m_mockExceptionEncounteredSender, USER_INACTIVITY_REPORT_PERIOD);
                    ASSERT_NE(nullptr, userInactivityMonitor);
                    exitTrigger.wait_for(exitLock, USER_INACTIVITY_REPORT_PERIOD + USER_INACTIVITY_REPORT_PERIOD);
                }
                TEST_F(UserInactivityMonitorTest, test_createWithError) {
                    ASSERT_EQ(nullptr, UserInactivityMonitor::create(m_mockMessageSender, nullptr));
                    ASSERT_EQ(nullptr, UserInactivityMonitor::create(nullptr, m_mockExceptionEncounteredSender));
                    ASSERT_EQ(nullptr, UserInactivityMonitor::create(nullptr, nullptr));
                }
                TEST_F(UserInactivityMonitorTest, testTimer_handleDirectiveProperly) {
                    mutex exitMutex;
                    unique_lock<mutex> exitLock(exitMutex);
                    promise<void> notifyObserverPromise1;
                    promise<void> notifyObserverPromise2;
                    auto notifyObserverFuture1 = notifyObserverPromise1.get_future();
                    auto notifyObserverFuture2 = notifyObserverPromise2.get_future();
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(ResultOf(&checkMessageRequestAndReleaseTrigger, Eq(true))));
                    auto userInactivityMonitor = UserInactivityMonitor::create(m_mockMessageSender, m_mockExceptionEncounteredSender,
                                                                               USER_INACTIVITY_REPORT_PERIOD);
                    ASSERT_NE(nullptr, userInactivityMonitor);
                    auto userInactivityObserver1 = make_shared<MockUserInactivityMonitorObserver>();
                    auto userInactivityObserver2 = make_shared<MockUserInactivityMonitorObserver>();
                    EXPECT_CALL(*(userInactivityObserver1.get()), onUserInactivityReportSent())
                        .WillOnce(InvokeWithoutArgs([&notifyObserverPromise1] { notifyObserverPromise1.set_value(); }));
                    EXPECT_CALL(*(userInactivityObserver2.get()), onUserInactivityReportSent())
                        .WillOnce(InvokeWithoutArgs([&notifyObserverPromise2] { notifyObserverPromise2.set_value(); }));
                    userInactivityMonitor->addObserver(userInactivityObserver1);
                    userInactivityMonitor->addObserver(userInactivityObserver2);
                    auto directiveSequencer = DirectiveSequencer::create(m_mockExceptionEncounteredSender);
                    directiveSequencer->addDirectiveHandler(userInactivityMonitor);
                    auto userInactivityDirectiveHeader = make_shared<AVSMessageHeader>(USER_INACTIVITY_RESET_NAMESPACE, USER_INACTIVITY_RESET_NAME,
                                                                                       USER_INACTIVITY_MESSAGE_ID);
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    shared_ptr<AVSDirective> userInactivityDirective =AVSDirective::create("", userInactivityDirectiveHeader, "",
                                                                                           attachmentManager, "");
                    directiveSequencer->onDirective(userInactivityDirective);
                    exitTrigger.wait_for(exitLock, USER_INACTIVITY_REPORT_PERIOD + USER_INACTIVITY_REPORT_PERIOD);
                    notifyObserverFuture1.wait_for(USER_INACTIVITY_REPORT_PERIOD + USER_INACTIVITY_REPORT_PERIOD);
                    notifyObserverFuture2.wait_for(USER_INACTIVITY_REPORT_PERIOD + USER_INACTIVITY_REPORT_PERIOD);
                    directiveSequencer->shutdown();
                }
                TEST_F(UserInactivityMonitorTest, testTimer_sendMultipleReports) {
                    InSequence s;
                    mutex exitMutex;
                    unique_lock<std::mutex> exitLock(exitMutex);
                    int repetitionCount = 3;
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(ResultOf(&checkMessageRequest, Eq(true)))).Times(repetitionCount - 1);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(ResultOf(&checkMessageRequestAndReleaseTrigger, Eq(true)))).Times(1);
                    auto userInactivityMonitor = UserInactivityMonitor::create(m_mockMessageSender, m_mockExceptionEncounteredSender,
                                                                               USER_INACTIVITY_REPORT_PERIOD);
                    ASSERT_NE(nullptr, userInactivityMonitor);
                    exitTrigger.wait_for(exitLock, repetitionCount * USER_INACTIVITY_REPORT_PERIOD + USER_INACTIVITY_REPORT_PERIOD);
                }
                TEST_F(UserInactivityMonitorTest, test_verifyInactivityTime) {
                    auto userInactivityMonitor = UserInactivityMonitor::create(m_mockMessageSender, m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, userInactivityMonitor);
                    auto timeInactive = userInactivityMonitor->timeSinceUserActivity();
                    ASSERT_EQ(0, timeInactive.count());
                    this_thread::sleep_for(USER_INACTIVITY_REPORT_PERIOD);
                    timeInactive = userInactivityMonitor->timeSinceUserActivity();
                    auto msPassed = duration_cast<milliseconds>(timeInactive);
                    ASSERT_GE(msPassed.count(), 0);
                }
                TEST_F(UserInactivityMonitorTest, test_sendMultipleReportsWithReset) {
                    InSequence s;
                    mutex exitMutex;
                    unique_lock<mutex> exitLock(exitMutex);
                    int repetitionCount = 5;
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(ResultOf(&checkMessageRequest, Eq(true))))
                        .Times(AtLeast(repetitionCount - 1));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(ResultOf(&checkMessageRequestAndReleaseTrigger, Eq(true)))).Times(1);
                    auto userInactivityMonitor = UserInactivityMonitor::create(
                        m_mockMessageSender, m_mockExceptionEncounteredSender, USER_INACTIVITY_REPORT_PERIOD);
                    ASSERT_NE(nullptr, userInactivityMonitor);
                    auto directiveSequencer = DirectiveSequencer::create(m_mockExceptionEncounteredSender);
                    directiveSequencer->addDirectiveHandler(userInactivityMonitor);
                    auto userInactivityDirectiveHeader = make_shared<AVSMessageHeader>(USER_INACTIVITY_RESET_NAMESPACE, USER_INACTIVITY_RESET_NAME,
                                                                                       USER_INACTIVITY_MESSAGE_ID);
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    shared_ptr<AVSDirective> userInactivityDirective =AVSDirective::create("", userInactivityDirectiveHeader, "",
                                                                                           attachmentManager, "");
                    this_thread::sleep_for(2 * USER_INACTIVITY_REPORT_PERIOD + USER_INACTIVITY_REPORT_PERIOD / 2);
                    directiveSequencer->onDirective(userInactivityDirective);
                    exitTrigger.wait_for(exitLock, repetitionCount * USER_INACTIVITY_REPORT_PERIOD + USER_INACTIVITY_REPORT_PERIOD);
                    directiveSequencer->shutdown();
                }
            }
        }
    }
}