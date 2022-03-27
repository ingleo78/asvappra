#include <chrono>
#include <condition_variable>
#include <thread>
#include <gtest/gtest.h>
#include <json/document.h>
#include <adsl/DirectiveSequencer.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockRevokeAuthorizationObserver.h>
#include "RevokeAuthorizationHandler.h"

using namespace testing;

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            namespace test {
                using namespace adsl;
                using namespace attachment;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                static const string REVOKE_NAMESPACE = "System";
                static const string REVOKE_DIRECTIVE_NAME = "RevokeAuthorization";
                static const string REVOKE_PAYLOAD = "{}";
                static const string REVOKE_MESSAGE_ID = "ABC123DEF";
                static const nanoseconds SHORT_DIRECTIVE_DELAY = duration_cast<nanoseconds>(milliseconds(50));
                condition_variable exitTrigger;
                static void notifyExit() {
                    exitTrigger.notify_all();
                }
                static shared_ptr<AVSDirective> createDirective() {
                    auto revokeDirectiveHeader = make_shared<AVSMessageHeader>(REVOKE_NAMESPACE, REVOKE_DIRECTIVE_NAME, REVOKE_MESSAGE_ID);
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    return AVSDirective::create("", revokeDirectiveHeader, REVOKE_PAYLOAD, attachmentManager, "");
                }
                class RevokeAuthorizationHandlerTest : public Test {
                public:
                    void SetUp() override;
                protected:
                    shared_ptr<StrictMock<MockRevokeAuthorizationObserver>> m_mockRevokeAuthorizationObserver;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionEncounteredSender;
                };
                void RevokeAuthorizationHandlerTest::SetUp() {
                    m_mockRevokeAuthorizationObserver = make_shared<StrictMock<MockRevokeAuthorizationObserver>>();
                    m_mockExceptionEncounteredSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_createSuccessfully) {
                    ASSERT_NE(nullptr, RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender));
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_createWithError) {
                    ASSERT_EQ(nullptr, RevokeAuthorizationHandler::create(nullptr));
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_handleDirectiveProperly) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    ASSERT_TRUE(revokeHandler->addObserver(m_mockRevokeAuthorizationObserver));
                    ASSERT_FALSE(revokeHandler->addObserver(m_mockRevokeAuthorizationObserver));
                    auto directiveSequencer = DirectiveSequencer::create(m_mockExceptionEncounteredSender);
                    directiveSequencer->addDirectiveHandler(revokeHandler);
                    std::mutex exitMutex;
                    std::unique_lock<mutex> exitLock(exitMutex);
                    EXPECT_CALL(*m_mockRevokeAuthorizationObserver, onRevokeAuthorization()).WillOnce(InvokeWithoutArgs(notifyExit));
                    directiveSequencer->onDirective(createDirective());
                    ASSERT_EQ(cv_status::no_timeout, exitTrigger.wait_for(exitLock, SHORT_DIRECTIVE_DELAY));
                    directiveSequencer->shutdown();
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_handleDirectiveImmediatelyProperly) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    ASSERT_TRUE(revokeHandler->addObserver(m_mockRevokeAuthorizationObserver));
                    EXPECT_CALL(*m_mockRevokeAuthorizationObserver, onRevokeAuthorization());
                    revokeHandler->handleDirectiveImmediately(createDirective());
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_handleDirectiveImmediatelyNullDirective) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    ASSERT_TRUE(revokeHandler->addObserver(m_mockRevokeAuthorizationObserver));
                    revokeHandler->handleDirectiveImmediately(nullptr);
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_handleDirectiveNullDirectiveInfo) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    ASSERT_TRUE(revokeHandler->addObserver(m_mockRevokeAuthorizationObserver));
                    revokeHandler->handleDirective(nullptr);
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_cancelDirectiveNullDirectiveInfo) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    ASSERT_TRUE(revokeHandler->addObserver(m_mockRevokeAuthorizationObserver));
                    revokeHandler->cancelDirective(nullptr);
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_removeObserverSuccessfully) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    revokeHandler->addObserver(m_mockRevokeAuthorizationObserver);
                    auto directiveSequencer = DirectiveSequencer::create(m_mockExceptionEncounteredSender);
                    directiveSequencer->addDirectiveHandler(revokeHandler);
                    ASSERT_TRUE(revokeHandler->removeObserver(m_mockRevokeAuthorizationObserver));
                    ASSERT_FALSE(revokeHandler->removeObserver(m_mockRevokeAuthorizationObserver));
                    mutex exitMutex;
                    unique_lock<mutex> exitLock(exitMutex);
                    directiveSequencer->onDirective(createDirective());
                    this_thread::sleep_for(SHORT_DIRECTIVE_DELAY);
                    directiveSequencer->shutdown();
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_preHandleDirective) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    revokeHandler->preHandleDirective(nullptr);
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_addObserverIgnoreNullPtr) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    ASSERT_FALSE(revokeHandler->addObserver(nullptr));
                }
                TEST_F(RevokeAuthorizationHandlerTest, test_removeObserverIgnoreNullPtr) {
                    auto revokeHandler = RevokeAuthorizationHandler::create(m_mockExceptionEncounteredSender);
                    ASSERT_NE(nullptr, revokeHandler);
                    ASSERT_FALSE(revokeHandler->removeObserver(nullptr));
                }
            }
        }
    }
}