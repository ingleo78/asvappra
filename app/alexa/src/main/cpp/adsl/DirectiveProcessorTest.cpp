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
#include "DirectiveProcessor.h"

using namespace testing;

namespace alexaClientSDK {
    namespace adsl {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace attachment;
            static const string MESSAGE_ID_0_0("Message_0_0");
            static const string MESSAGE_ID_0_1("Message_0_1");
            static const string MESSAGE_ID_0_2("Message_0_2");
            static const string MESSAGE_ID_0_3("Message_0_3");
            static const string MESSAGE_ID_1_0("Message_1_0");
            static const string DIALOG_REQUEST_ID_0("DialogRequestId_0");
            static const string DIALOG_REQUEST_ID_1("DialogRequestId_1");
            static const string UNPARSED_DIRECTIVE("unparsedDirectiveForTest");
            static const string PAYLOAD_TEST("payloadForTest");
            static const string NAMESPACE_0("namespace_0");
            static const string NAMESPACE_1("namespace_1");
            static const string NAME_0("name_0");
            static const string NAME_1("name_1");
            static const string NAME_2("name_2");
            static const string NAME_3("name_3");
            static const string TEST_ATTACHMENT_CONTEXT_ID("TEST_ATTACHMENT_CONTEXT_ID");
            #define NAMESPACE_AND_NAME_0_0 NAMESPACE_0, NAME_0
            #define NAMESPACE_AND_NAME_0_1 NAMESPACE_0, NAME_1
            #define NAMESPACE_AND_NAME_0_2 NAMESPACE_0, NAME_2
            #define NAMESPACE_AND_NAME_0_3 NAMESPACE_0, NAME_3
            #define NAMESPACE_AND_NAME_1_0 NAMESPACE_1, NAME_0
            static const string NEW_DIALOG_REQUEST_DIRECTIVE_V0 = R"delim(
            {
            "directive": {
                    "header": {
                        "namespace": "InteractionModel",
                        "name": "NewDialogRequest",
                        "messageId": "2120215c-d803-4800-8773-e9505d16354a",
                        "dialogRequestId": ")delim" + DIALOG_REQUEST_ID_0 + R"delim("
                    },
                    "payload": {
                        "dialogRequestId": ")delim" + DIALOG_REQUEST_ID_0 + R"delim("
                    }
                }
            })delim";
            static const string NO_CONTEXT = "";
            static const NamespaceAndName NEW_DIALOG_REQUEST_SIGNATURE{"InteractionModel", "NewDialogRequest"};
            class DirectiveProcessorTest : public Test {
            public:
                void SetUp() override;
                shared_ptr<DirectiveRouter> m_router;
                shared_ptr<DirectiveProcessor> m_processor;
                shared_ptr<AttachmentManager> m_attachmentManager;
                shared_ptr<AVSDirective> m_directive_0_0;
                shared_ptr<AVSDirective> m_directive_0_1;
                shared_ptr<AVSDirective> m_directive_0_2;
                shared_ptr<AVSDirective> m_directive_0_3;
                shared_ptr<AVSDirective> m_directive_1_0;
            };
            void DirectiveProcessorTest::SetUp() {
                m_router = make_shared<DirectiveRouter>();
                m_processor = make_shared<DirectiveProcessor>(m_router.get());
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
                auto avsMessageHeader_0_3 = make_shared<AVSMessageHeader>(NAMESPACE_AND_NAME_0_3, MESSAGE_ID_0_3, DIALOG_REQUEST_ID_0);
                m_directive_0_3 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader_0_3, PAYLOAD_TEST, m_attachmentManager,
                                                       TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader_1_0 = make_shared<AVSMessageHeader>(NAMESPACE_AND_NAME_1_0, MESSAGE_ID_1_0, DIALOG_REQUEST_ID_1);
                m_directive_1_0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader_1_0, PAYLOAD_TEST, m_attachmentManager,
                                                       TEST_ATTACHMENT_CONTEXT_ID);
            }
            TEST_F(DirectiveProcessorTest, test_nullptrDirective) {
                ASSERT_FALSE(m_processor->onDirective(nullptr));
            }
            TEST_F(DirectiveProcessorTest, test_wrongDialogRequestId) {
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(handler0));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(m_directive_0_0, _)).Times(0);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0_0)).Times(0);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_1);
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_0));
            }
            TEST_F(DirectiveProcessorTest, test_sendNonBlocking) {
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(handler0));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(m_directive_0_0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0_0)).Times(1);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_0));
                ASSERT_TRUE(handler0->waitUntilCompleted(std::chrono::milliseconds(1500000)));
            }
            TEST_F(DirectiveProcessorTest, test_sendBlockingThenNonBlocking) {
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
                shared_ptr<MockDirectiveHandler> handler0 = MockDirectiveHandler::create(handler0Config);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                shared_ptr<MockDirectiveHandler> handler1 = MockDirectiveHandler::create(handler1Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_router->addDirectiveHandler(handler1));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(m_directive_0_0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0_0)).Times(1);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(m_directive_0_1, _)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_0_1)).Times(1);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_0));
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_1));
                ASSERT_TRUE(handler1->waitUntilCompleted());
            }
            TEST_F(DirectiveProcessorTest, test_onUnregisteredDirective) {
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                shared_ptr<MockDirectiveHandler> handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_AND_NAME_1_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                shared_ptr<MockDirectiveHandler> handler2 = MockDirectiveHandler::create(handler2Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(handler1));
                ASSERT_TRUE(m_router->addDirectiveHandler(handler2));
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(m_directive_0_1, _)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_0_1)).Times(1);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(m_directive_1_0, _)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirective(MESSAGE_ID_1_0)).Times(1);
                EXPECT_CALL(*(handler2.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_FALSE(m_processor->onDirective(m_directive_0_0));
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_1));
                ASSERT_TRUE(handler1->waitUntilCompleted());
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_1);
                ASSERT_TRUE(m_processor->onDirective(m_directive_1_0));
                ASSERT_TRUE(handler2->waitUntilCompleted());
            }
            TEST_F(DirectiveProcessorTest, test_setDialogRequestIdCancelsOutstandingDirectives) {
                DirectiveHandlerConfiguration longRunningHandlerConfig;
                longRunningHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto longRunningHandler = MockDirectiveHandler::create(longRunningHandlerConfig, MockDirectiveHandler::DEFAULT_DONE_TIMEOUT_MS);
                DirectiveHandlerConfiguration handler1Config;
                auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                handler1Config[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = audioNonBlockingPolicy;
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_AND_NAME_1_0}] = audioNonBlockingPolicy;
                auto handler2 = MockDirectiveHandler::create(handler2Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(longRunningHandler));
                ASSERT_TRUE(m_router->addDirectiveHandler(handler1));
                ASSERT_TRUE(m_router->addDirectiveHandler(handler2));
                EXPECT_CALL(*(longRunningHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(longRunningHandler.get()), preHandleDirective(_, _)).Times(1);
                EXPECT_CALL(*(longRunningHandler.get()), handleDirective(_)).Times(1);
                EXPECT_CALL(*(longRunningHandler.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(m_directive_0_1, _)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_0_1)).Times(0);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(m_directive_1_0, _)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirective(MESSAGE_ID_1_0)).Times(1);
                EXPECT_CALL(*(handler2.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_0));
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_1));
                ASSERT_TRUE(longRunningHandler->waitUntilHandling());
                ASSERT_TRUE(handler1->waitUntilPreHandling());
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_1);
                ASSERT_TRUE(m_processor->onDirective(m_directive_1_0));
                ASSERT_TRUE(handler2->waitUntilCompleted());
            }
            TEST_F(DirectiveProcessorTest, test_addDirectiveWhileDisabled) {
                m_processor->disable();
                ASSERT_FALSE(m_processor->onDirective(m_directive_0_0));
            }
            TEST_F(DirectiveProcessorTest, test_addDirectiveAfterReEnabled) {
                m_processor->disable();
                ASSERT_FALSE(m_processor->onDirective(m_directive_0_0));
                m_processor->enable();
                ASSERT_TRUE(m_processor->onDirective(m_directive_0_0));
            }
            TEST_F(DirectiveProcessorTest, test_audioAndVisualIsBlockingAudio) {
                auto& audioAndVisualBlockingDirective = m_directive_0_0;
                DirectiveHandlerConfiguration audioAndVisualBlockingHandlerConfig;
                auto audioBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
                audioAndVisualBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = audioBlockingPolicy;
                auto audioAndVisualBlockingHandler = MockDirectiveHandler::create(audioAndVisualBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioAndVisualBlockingHandler));
                auto& audioNonBlockingDirective = m_directive_0_1;
                DirectiveHandlerConfiguration audioNonBlockingHandlerConfig;
                auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                audioNonBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = audioNonBlockingPolicy;
                auto audioNonBlockingHandler = MockDirectiveHandler::create(audioNonBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioNonBlockingHandler));
                auto& noneMediumsNoBlockingDirective = m_directive_0_2;
                DirectiveHandlerConfiguration handler3Config;
                auto noneMediums = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                handler3Config[NamespaceAndName{NAMESPACE_AND_NAME_0_2}] = noneMediums;
                auto noneMediumsNonBlockingHandler = MockDirectiveHandler::create(handler3Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(noneMediumsNonBlockingHandler));
                EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), preHandleDirective(audioAndVisualBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), handleDirective(MESSAGE_ID_0_0)).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(audioNonBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioNonBlockingHandler.get()), preHandleDirective(audioNonBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(audioNonBlockingHandler.get()), handleDirective(MESSAGE_ID_0_1)).Times(0);
                EXPECT_CALL(*(audioNonBlockingHandler.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(noneMediumsNonBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(noneMediumsNonBlockingHandler.get()), preHandleDirective(noneMediumsNoBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(noneMediumsNonBlockingHandler.get()), handleDirective(MESSAGE_ID_0_2)).Times(1);
                EXPECT_CALL(*(noneMediumsNonBlockingHandler.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(audioAndVisualBlockingDirective));
                ASSERT_TRUE(m_processor->onDirective(audioNonBlockingDirective));
                ASSERT_TRUE(m_processor->onDirective(noneMediumsNoBlockingDirective));
                noneMediumsNonBlockingHandler->waitUntilCompleted();
            }
            TEST_F(DirectiveProcessorTest, test_differentMediums) {
                auto& audioBlockingDirective = m_directive_0_0;
                DirectiveHandlerConfiguration audioBlockingHandlerConfig;
                audioBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto audioBlockingHandler = MockDirectiveHandler::create(audioBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioBlockingHandler));
                auto& visualBlockingDirective = m_directive_0_1;
                DirectiveHandlerConfiguration visualBlockingHandlerConfig;
                visualBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, true);
                auto visualBlockingHandler = MockDirectiveHandler::create(visualBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(visualBlockingHandler));
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioBlockingHandler.get()), preHandleDirective(audioBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirective(MESSAGE_ID_0_0)).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*(audioBlockingHandler.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(visualBlockingHandler.get()), preHandleDirective(visualBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirective(MESSAGE_ID_0_1)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(audioBlockingDirective));
                ASSERT_TRUE(m_processor->onDirective(visualBlockingDirective));
                visualBlockingHandler->waitUntilCompleted();
            }
            TEST_F(DirectiveProcessorTest, test_releaseOneMedium) {
                auto& audioBlockingDirective = m_directive_0_0;
                DirectiveHandlerConfiguration audioBlockingHandlerConfig;
                audioBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto audioBlockingHandler = MockDirectiveHandler::create(audioBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioBlockingHandler));
                auto& visualBlockingDirective = m_directive_0_1;
                DirectiveHandlerConfiguration visualBlockingHandlerConfig;
                visualBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, true);
                auto visualBlockingHandler = MockDirectiveHandler::create(visualBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(visualBlockingHandler));
                auto& audioBlockingDirective2 = m_directive_0_2;
                DirectiveHandlerConfiguration audioBlockingHandler2Config;
                audioBlockingHandler2Config[NamespaceAndName{NAMESPACE_AND_NAME_0_2}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto audioBlockingHandler2 = MockDirectiveHandler::create(audioBlockingHandler2Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioBlockingHandler2));
                auto& visualBlockingDirective2 = m_directive_0_3;
                DirectiveHandlerConfiguration visualBlockingHandler2Config;
                visualBlockingHandler2Config[NamespaceAndName{NAMESPACE_AND_NAME_0_3}] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, true);
                auto visualBlockingHandler2 = MockDirectiveHandler::create(visualBlockingHandler2Config);
                ASSERT_TRUE(m_router->addDirectiveHandler(visualBlockingHandler2));
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioBlockingHandler.get()), preHandleDirective(audioBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirective(MESSAGE_ID_0_0)).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*(audioBlockingHandler.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(visualBlockingHandler.get()), preHandleDirective(visualBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirective(MESSAGE_ID_0_1)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(audioBlockingHandler2.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioBlockingHandler2.get()), preHandleDirective(audioBlockingDirective2, _)).Times(1);
                EXPECT_CALL(*(audioBlockingHandler2.get()), handleDirective(MESSAGE_ID_0_2)).Times(0);
                EXPECT_CALL(*(audioBlockingHandler2.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler2.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(visualBlockingHandler2.get()), preHandleDirective(visualBlockingDirective2, _)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler2.get()), handleDirective(MESSAGE_ID_0_3)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler2.get()), cancelDirective(_)).Times(0);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(audioBlockingDirective));
                ASSERT_TRUE(m_processor->onDirective(visualBlockingDirective));
                ASSERT_TRUE(m_processor->onDirective(audioBlockingDirective2));
                ASSERT_TRUE(m_processor->onDirective(visualBlockingDirective2));
                visualBlockingHandler->waitUntilCompleted();
                visualBlockingHandler2->waitUntilCompleted();
            }
            TEST_F(DirectiveProcessorTest, test_blockingQueuedDirectivIsBlocking) {
                auto& audioBlockingDirective = m_directive_0_0;
                DirectiveHandlerConfiguration audioBlockingHandlerConfig;
                audioBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto audioBlockingHandler = MockDirectiveHandler::create(audioBlockingHandlerConfig, MockDirectiveHandler::DEFAULT_DONE_TIMEOUT_MS);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioBlockingHandler));
                auto& audioAndVisualBlocking = m_directive_0_1;
                DirectiveHandlerConfiguration audioAndVisualBlockingHandlerConfig;
                audioAndVisualBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = BlockingPolicy(BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
                auto audioAndVisualBlockingHandler = MockDirectiveHandler::create(audioAndVisualBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioAndVisualBlockingHandler));
                auto& visualBlockingDirective = m_directive_0_2;
                DirectiveHandlerConfiguration visualBlockingHandlerConfig;
                visualBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_2}] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, true);
                auto visualBlockingHandler = MockDirectiveHandler::create(visualBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(visualBlockingHandler));
                auto& noneMediumsDirective = m_directive_0_3;
                DirectiveHandlerConfiguration noneMediumsHandlerConfig;
                noneMediumsHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_3}] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                auto noneMediumsHandler = MockDirectiveHandler::create(noneMediumsHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(noneMediumsHandler));
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioBlockingHandler.get()), preHandleDirective(audioBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(audioBlockingHandler.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), preHandleDirective(audioAndVisualBlocking, _)).Times(1);
                EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(visualBlockingHandler.get()), preHandleDirective(visualBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(noneMediumsHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(noneMediumsHandler.get()), preHandleDirective(noneMediumsDirective, _)).Times(1);
                EXPECT_CALL(*(noneMediumsHandler.get()), cancelDirective(_)).Times(0);
                Sequence s1;
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirective(MESSAGE_ID_0_0)).Times(1).InSequence(s1);
                EXPECT_CALL(*(noneMediumsHandler.get()), handleDirective(MESSAGE_ID_0_3)).Times(1).InSequence(s1);
                EXPECT_CALL(*(audioAndVisualBlockingHandler.get()), handleDirective(MESSAGE_ID_0_1)).Times(1).InSequence(s1);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirective(MESSAGE_ID_0_2)).Times(1).InSequence(s1);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(audioBlockingDirective));
                audioBlockingHandler->waitUntilHandling();
                ASSERT_TRUE(m_processor->onDirective(audioAndVisualBlocking));
                audioAndVisualBlockingHandler->waitUntilPreHandling();
                ASSERT_TRUE(m_processor->onDirective(visualBlockingDirective));
                visualBlockingHandler->waitUntilPreHandling();
                ASSERT_TRUE(m_processor->onDirective(noneMediumsDirective));
                noneMediumsHandler->waitUntilHandling();
                audioBlockingHandler->doHandlingCompleted();
                audioAndVisualBlockingHandler->waitUntilCompleted();
                visualBlockingHandler->waitUntilCompleted();
            }
            TEST_F(DirectiveProcessorTest, test_nonBlockingQueuedDirectivIsNotBlocking) {
                auto& audioBlockingDirective = m_directive_0_0;
                DirectiveHandlerConfiguration audioBlockingHandlerConfig;
                audioBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_0}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto audioBlockingHandler = MockDirectiveHandler::create(audioBlockingHandlerConfig, MockDirectiveHandler::DEFAULT_DONE_TIMEOUT_MS);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioBlockingHandler));
                auto& audioAndVisualNonBlocking = m_directive_0_1;
                DirectiveHandlerConfiguration audioAndVisualNonBlockingHandlerConfig;
                audioAndVisualNonBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_1}] = BlockingPolicy(BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, false);
                auto audioAndVisualNonBlockingHandler = MockDirectiveHandler::create(audioAndVisualNonBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(audioAndVisualNonBlockingHandler));
                auto& visualBlockingDirective = m_directive_0_2;
                DirectiveHandlerConfiguration visualBlockingHandlerConfig;
                visualBlockingHandlerConfig[NamespaceAndName{NAMESPACE_AND_NAME_0_2}] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, true);
                auto visualBlockingHandler = MockDirectiveHandler::create(visualBlockingHandlerConfig);
                ASSERT_TRUE(m_router->addDirectiveHandler(visualBlockingHandler));
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioBlockingHandler.get()), preHandleDirective(audioBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(audioBlockingHandler.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(audioAndVisualNonBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(audioAndVisualNonBlockingHandler.get()), preHandleDirective(audioAndVisualNonBlocking, _)).Times(1);
                EXPECT_CALL(*(audioAndVisualNonBlockingHandler.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(visualBlockingHandler.get()), preHandleDirective(visualBlockingDirective, _)).Times(1);
                EXPECT_CALL(*(visualBlockingHandler.get()), cancelDirective(_)).Times(0);
                Sequence s1;
                EXPECT_CALL(*(audioBlockingHandler.get()), handleDirective(MESSAGE_ID_0_0)).Times(1).InSequence(s1);
                EXPECT_CALL(*(visualBlockingHandler.get()), handleDirective(MESSAGE_ID_0_2)).Times(1).InSequence(s1);
                EXPECT_CALL(*(audioAndVisualNonBlockingHandler.get()), handleDirective(MESSAGE_ID_0_1)).Times(1).InSequence(s1);
                m_processor->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_processor->onDirective(audioBlockingDirective));
                audioBlockingHandler->waitUntilHandling();
                ASSERT_TRUE(m_processor->onDirective(audioAndVisualNonBlocking));
                ASSERT_TRUE(m_processor->onDirective(visualBlockingDirective));
                audioAndVisualNonBlockingHandler->waitUntilPreHandling();
                visualBlockingHandler->waitUntilHandling();
                audioBlockingHandler->doHandlingCompleted();
                audioAndVisualNonBlockingHandler->waitUntilCompleted();
                visualBlockingHandler->waitUntilCompleted();
            }
        }
    }
}