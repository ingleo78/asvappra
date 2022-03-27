#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <avs/attachment/AttachmentManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include "common/MockDirectiveHandler.h"
#include "DirectiveRouter.h"

using namespace testing;

namespace alexaClientSDK {
    namespace adsl {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace attachment;
            using namespace directiveRoutingRule;
            using namespace logger;
            static const string MESSAGE_ID_0_0("Message_0_0");
            static const string MESSAGE_ID_0_1("Message_0_1");
            static const string MESSAGE_ID_0_2("Message_0_2");
            static const string MESSAGE_ID_1_0("Message_1_0");
            static const string MESSAGE_ID_2_0("Message_2_0");
            static const string DIALOG_REQUEST_ID_0("DialogRequestId_0");
            static const string UNPARSED_DIRECTIVE("unparsedDirectiveForTest");
            static const string PAYLOAD_TEST("payloadForTest");
            static const string NAMESPACE_0("namespace_0");
            static const string NAMESPACE_1("namespace_1");
            static const string NAMESPACE_2("namespace_2");
            static const string NAME_0("name_0");
            static const string NAME_1("name_1");
            static const string NAME_2("name_2");
            static const string NAME_ANY("*");
            static const string TEST_ATTACHMENT_CONTEXT_ID("TEST_ATTACHMENT_CONTEXT_ID");
            #define NAMESPACE_AND_NAME_0_0 NAMESPACE_0, NAME_0
            #define NAMESPACE_AND_NAME_0_1 NAMESPACE_0, NAME_1
            #define NAMESPACE_AND_NAME_0_2 NAMESPACE_0, NAME_2
            #define NAMESPACE_AND_NAME_0_ANY NAMESPACE_0, NAME_ANY
            #define NAMESPACE_AND_NAME_1_0 NAMESPACE_1, NAME_0
            #define NAMESPACE_AND_NAME_2_0 NAMESPACE_2, NAME_0
            #define NAMESPACE_AND_NAME_2_ANY NAMESPACE_2, NAME_ANY
            static const seconds LONG_TIMEOUT(15);
            static const string WILDCARD = "*";
            class DirectiveRouterTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                shared_ptr<AVSDirective> createDirective(const string& endpointId, const Optional<string>& instance, const string& nameSpace,
                                                         const string& name) const;
                DirectiveRouter m_router;
                shared_ptr<AttachmentManager> m_attachmentManager;
                shared_ptr<AVSDirective> m_directive_0_0;
                shared_ptr<AVSDirective> m_directive_0_1;
                shared_ptr<AVSDirective> m_directive_0_2;
                shared_ptr<AVSDirective> m_directive_1_0;
                shared_ptr<AVSDirective> m_directive_2_0;
            };
            void DirectiveRouterTest::SetUp() {
                m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                auto avsMessageHeader_0_0 = make_shared<AVSMessageHeader>(NAMESPACE_AND_NAME_0_0, MESSAGE_ID_0_0, DIALOG_REQUEST_ID_0);
                m_directive_0_0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader_0_0, PAYLOAD_TEST, m_attachmentManager,
                                                       TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader_0_1 = make_shared<AVSMessageHeader>(NAMESPACE_AND_NAME_0_1, MESSAGE_ID_0_1, DIALOG_REQUEST_ID_0);
                m_directive_0_1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader_0_1, PAYLOAD_TEST, m_attachmentManager,
                                                       TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader_0_2 = make_shared<AVSMessageHeader>(NAMESPACE_AND_NAME_0_2, MESSAGE_ID_0_2, DIALOG_REQUEST_ID_0);
                m_directive_0_2 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader_0_2, PAYLOAD_TEST, m_attachmentManager,
                                                       TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader_1_0 = make_shared<AVSMessageHeader>(NAMESPACE_AND_NAME_1_0, MESSAGE_ID_1_0, DIALOG_REQUEST_ID_0);
                m_directive_1_0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader_1_0, PAYLOAD_TEST, m_attachmentManager,
                                                       TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader_2_0 = make_shared<AVSMessageHeader>(NAMESPACE_AND_NAME_2_0, MESSAGE_ID_2_0, DIALOG_REQUEST_ID_0);
                m_directive_2_0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader_2_0, PAYLOAD_TEST, m_attachmentManager,
                                                       TEST_ATTACHMENT_CONTEXT_ID);
            }
            void DirectiveRouterTest::TearDown() {
                m_router.shutdown();
            }
            shared_ptr<AVSDirective> DirectiveRouterTest::createDirective(const string& endpointId, const Optional<string>& instance, const string& nameSpace,
                                                                          const string& name) const {
                auto header = make_shared<AVSMessageHeader>(nameSpace, name, "messageId", "dialogId", "token", "eventToken", "3.0",instance.valueOr(""));
                AVSMessageEndpoint endpoint{endpointId};
                return AVSDirective::create(UNPARSED_DIRECTIVE, header, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID, endpoint);
            }
            TEST_F(DirectiveRouterTest, test_unroutedDirective) {
                ASSERT_FALSE(m_router.handleDirectiveImmediately(m_directive_0_0));
            }
            TEST_F(DirectiveRouterTest, test_settingADirectiveHandler) {
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler0));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(m_directive_0_0)).Times(1);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler0.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), onDeregistered()).Times(1);
                ASSERT_TRUE(m_router.handleDirectiveImmediately(m_directive_0_0));
            }
            TEST_F(DirectiveRouterTest, test_registeringMultipeHandler) {
                DirectiveHandlerConfiguration handler0Config;
                auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_AND_NAME_0_ANY}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler2 = MockDirectiveHandler::create(handler2Config);
                DirectiveHandlerConfiguration handler3Config;
                handler3Config[NamespaceAndName{NAMESPACE_AND_NAME_1_0}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler3 = MockDirectiveHandler::create(handler3Config);
                DirectiveHandlerConfiguration handler4Config;
                handler4Config[NamespaceAndName{NAMESPACE_AND_NAME_2_ANY}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler4 = MockDirectiveHandler::create(handler4Config);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler0));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler1));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler2));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler3));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler4));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(m_directive_0_0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), onDeregistered()).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(m_directive_0_1, _)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), onDeregistered()).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(m_directive_0_2, _)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), onDeregistered()).Times(1);
                EXPECT_CALL(*(handler3.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler3.get()), preHandleDirective(m_directive_1_0, _)).Times(1);
                EXPECT_CALL(*(handler3.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler3.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler3.get()), onDeregistered()).Times(1);
                EXPECT_CALL(*(handler4.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler4.get()), preHandleDirective(m_directive_2_0, _)).Times(1);
                EXPECT_CALL(*(handler4.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler4.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler4.get()), onDeregistered()).Times(1);
                ASSERT_TRUE(m_router.preHandleDirective(m_directive_0_0, nullptr));
                ASSERT_TRUE(m_router.preHandleDirective(m_directive_0_1, nullptr));
                ASSERT_TRUE(m_router.preHandleDirective(m_directive_0_2, nullptr));
                ASSERT_TRUE(m_router.preHandleDirective(m_directive_1_0, nullptr));
                ASSERT_TRUE(m_router.preHandleDirective(m_directive_2_0, nullptr));
            }
            TEST_F(DirectiveRouterTest, test_removingChangingAndNotChangingHandlers) {
                DirectiveHandlerConfiguration handler0Config;
                auto audioBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_AND_NAME_1_0}] = audioNonBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler2 = MockDirectiveHandler::create(handler2Config);
                DirectiveHandlerConfiguration handler3Config;
                handler3Config[NamespaceAndName{NAMESPACE_AND_NAME_1_0}] = audioBlockingPolicy;
                shared_ptr<MockDirectiveHandler> handler3 = MockDirectiveHandler::create(handler3Config);
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler0.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), onDeregistered()).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_0_1)).WillOnce(Return(true));
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), onDeregistered()).Times(2);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler2.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), onDeregistered()).Times(1);
                EXPECT_CALL(*(handler3.get()), handleDirective(MESSAGE_ID_1_0)).WillOnce(Return(true));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler0));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler1));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler2));
                ASSERT_TRUE(m_router.removeDirectiveHandler(handler0));
                ASSERT_TRUE(m_router.removeDirectiveHandler(handler1));
                ASSERT_TRUE(m_router.removeDirectiveHandler(handler2));
                ASSERT_FALSE(m_router.removeDirectiveHandler(handler0));
                ASSERT_FALSE(m_router.removeDirectiveHandler(handler1));
                ASSERT_FALSE(m_router.removeDirectiveHandler(handler2));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler1));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler3));
                auto policy = m_router.getPolicy(m_directive_0_0);
                ASSERT_FALSE(m_router.handleDirective(m_directive_0_0));
                ASSERT_FALSE(policy.isValid());
                policy = m_router.getPolicy(m_directive_0_1);
                ASSERT_TRUE(m_router.handleDirective(m_directive_0_1));
                ASSERT_EQ(policy, audioNonBlockingPolicy);
                policy = m_router.getPolicy(m_directive_1_0);
                ASSERT_TRUE(m_router.handleDirective(m_directive_1_0));
                ASSERT_EQ(policy, audioBlockingPolicy);
            }
            TEST_F(DirectiveRouterTest, test_resultOfHandleDirectiveFailure) {
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler1 = MockDirectiveHandler::create(handler1Config);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler0));
                ASSERT_TRUE(m_router.addDirectiveHandler(handler1));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0_0)).WillOnce(Return(false));
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), onDeregistered()).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_0_1)).WillOnce(Return(false));
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), onDeregistered()).Times(1);
                ASSERT_FALSE(m_router.handleDirective(m_directive_0_0));
                ASSERT_FALSE(m_router.handleDirective(m_directive_0_1));
            }
            TEST_F(DirectiveRouterTest, test_handlerMethodsCanRunConcurrently) {
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler0));
                promise<void> waker;
                auto sleeper = waker.get_future();
                auto sleeperFunction = [&sleeper]() {
                    ASSERT_EQ(sleeper.wait_for(LONG_TIMEOUT), std::future_status::ready)
                    << "ERROR: Timeout reached while waiting for concurrent handler.";
                };
                auto wakerFunction = [&waker]() {
                    waker.set_value();
                    return true;
                };
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(m_directive_0_0, _)).WillOnce(InvokeWithoutArgs(sleeperFunction));
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0_0)).WillOnce(InvokeWithoutArgs(wakerFunction));
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), onDeregistered()).Times(1);
                std::thread sleeperThread([this]() { ASSERT_TRUE(m_router.preHandleDirective(m_directive_0_0, nullptr)); });
                ASSERT_TRUE(m_router.handleDirective(m_directive_0_0));
                auto policy = m_router.getPolicy(m_directive_0_0);
                ASSERT_EQ(policy, BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true));
                sleeperThread.join();
            }
            TEST_F(DirectiveRouterTest, test_perDirectiveRuleMatching) {
                auto rule = routingRulePerDirective("endpointId", std::string("instance"), "namespace", "name");
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[rule] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler = MockDirectiveHandler::create(handlerConfig);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler));
                auto directive = createDirective(rule.endpointId, rule.instance, rule.nameSpace, rule.name);
                EXPECT_CALL(*handler, handleDirectiveImmediately(directive)).Times(1);
                EXPECT_CALL(*handler, onDeregistered()).Times(1);
                EXPECT_TRUE(m_router.handleDirectiveImmediately(directive));
            }
            TEST_F(DirectiveRouterTest, test_perNamespaceRuleMatching) {
                auto rule = routingRulePerNamespace("endpointId", std::string("instance"), "namespace");
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[rule] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler = MockDirectiveHandler::create(handlerConfig);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler));
                const string directiveName = "RandomName";
                EXPECT_NE(rule.name, directiveName);
                auto directive = createDirective(rule.endpointId, rule.instance, rule.nameSpace, directiveName);
                EXPECT_CALL(*handler, handleDirectiveImmediately(directive)).Times(1);
                EXPECT_CALL(*handler, onDeregistered()).Times(1);
                EXPECT_TRUE(m_router.handleDirectiveImmediately(directive));
            }
            TEST_F(DirectiveRouterTest, test_perInstanceRuleMatching) {
                auto rule = routingRulePerInstance("endpointId", std::string("instance"));
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[rule] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler = MockDirectiveHandler::create(handlerConfig);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler));
                const string directiveName = "RandomName";
                const string directiveNameSpace = "RandomNamespace";
                EXPECT_NE(rule.name, directiveName);
                EXPECT_NE(rule.nameSpace, directiveNameSpace);
                auto directive = createDirective(rule.endpointId, rule.instance, directiveNameSpace, directiveName);
                EXPECT_CALL(*handler, handleDirectiveImmediately(directive)).Times(1);
                EXPECT_CALL(*handler, onDeregistered()).Times(1);
                EXPECT_TRUE(m_router.handleDirectiveImmediately(directive));
            }
            TEST_F(DirectiveRouterTest, test_perNamespaceAnyInstanceRuleMatching) {
                auto rule = routingRulePerNamespaceAnyInstance("endpointId", "nameSpace");
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[rule] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler = MockDirectiveHandler::create(handlerConfig);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler));
                const string directiveName = "RandomName";
                const string directiveInstance = "RandomInstance";
                EXPECT_NE(rule.name, directiveName);
                EXPECT_NE(rule.instance.valueOr(""), directiveInstance);
                auto directive = createDirective(rule.endpointId, directiveInstance, rule.nameSpace, directiveName);
                EXPECT_CALL(*handler, handleDirectiveImmediately(directive)).Times(1);
                EXPECT_CALL(*handler, onDeregistered()).Times(1);
                EXPECT_TRUE(m_router.handleDirectiveImmediately(directive));
            }
            TEST_F(DirectiveRouterTest, test_perEndpointMatching) {
                auto rule = routingRulePerEndpoint("endpoint");
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[rule] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler = MockDirectiveHandler::create(handlerConfig);
                ASSERT_TRUE(m_router.addDirectiveHandler(handler));
                const string directiveName = "RandomName";
                const string directiveNameSpace = "RandomNamespace";
                const string directiveInstance = "RandomInstance";
                EXPECT_NE(rule.name, directiveName);
                EXPECT_NE(rule.nameSpace, directiveNameSpace);
                EXPECT_NE(rule.instance.valueOr(""), directiveInstance);
                auto directive = createDirective(rule.endpointId, directiveInstance, directiveNameSpace, directiveName);
                EXPECT_CALL(*handler, handleDirectiveImmediately(directive)).Times(1);
                EXPECT_CALL(*handler, onDeregistered()).Times(1);
                EXPECT_TRUE(m_router.handleDirectiveImmediately(directive));
            }
            TEST_F(DirectiveRouterTest, test_addDirectiveHandlerWithInvalidRoutingRuleShouldFail) {
                auto rule = routingRulePerEndpoint(WILDCARD);
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[rule] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                shared_ptr<MockDirectiveHandler> handler = MockDirectiveHandler::create(handlerConfig);
                EXPECT_FALSE(m_router.addDirectiveHandler(handler));
            }
        }
    }
}