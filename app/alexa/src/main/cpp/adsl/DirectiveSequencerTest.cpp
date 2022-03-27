#include <chrono>
#include <future>
#include <string>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <avs/attachment/AttachmentManager.h>
#include <avs/NamespaceAndName.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include "common/MockDirectiveHandler.h"
#include "DirectiveSequencer.h"

using namespace testing;

namespace alexaClientSDK {
    namespace adsl {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace attachment;
            using namespace sdkInterfaces;
            using namespace utils;
            static const milliseconds LONG_HANDLING_TIME_MS(30000);
            static const string NAMESPACE_TEST("Test");
            static const string NAMESPACE_SPEAKER("Speaker");
            static const string NAMESPACE_SPEECH_SYNTHESIZER("SpeechSynthesizer");
            static const string NAMESPACE_AUDIO_PLAYER("AudioPlayer");
            static const string NAME_DONE("Done");
            static const string NAME_SET_VOLUME("SetVolume");
            static const string NAME_SPEAK("Speak");
            static const string NAME_PLAY("Play");
            static const string NAME_BLOCKING("Blocking");
            static const string NAME_NON_BLOCKING("Non-Blocking");
            static const string NAME_HANDLE_IMMEDIATELY("Handle-Immediately");
            static const string MESSAGE_ID_DONE("Message_Done");
            static const string MESSAGE_ID_0("Message_0");
            static const string MESSAGE_ID_1("Message_1");
            static const string MESSAGE_ID_2("Message_2");
            static const string DIALOG_REQUEST_ID_DONE("DialogRequestId_Done");
            static const string DIALOG_REQUEST_ID_0("DialogRequestId_0");
            static const string DIALOG_REQUEST_ID_1("DialogRequestId_1");
            static const string UNPARSED_DIRECTIVE("unparsedDirectiveForTest");
            static const string PAYLOAD_TEST("payloadForTest");
            static const string DIALOG_REQUEST_ID_2("DialogRequestId_2");
            static const string TEST_ATTACHMENT_CONTEXT_ID("TEST_ATTACHMENT_CONTEXT_ID");
            class MockExceptionEncounteredSender : public ExceptionEncounteredSenderInterface {
            public:
                MOCK_METHOD3(sendExceptionEncountered, void(const string&, ExceptionErrorType, const string&));
            };
            class DirectiveSequencerTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                void setDialogRequestId(string dialogRequestId);
                shared_ptr<MockDirectiveHandler> m_doneHandler;
                shared_ptr<MockExceptionEncounteredSender> m_exceptionEncounteredSender;
                unique_ptr<DirectiveSequencerInterface> m_sequencer;
                shared_ptr<AttachmentManager> m_attachmentManager;
            };
            void DirectiveSequencerTest::SetUp() {
                DirectiveHandlerConfiguration config;
                config[NamespaceAndName{NAMESPACE_TEST, NAME_DONE}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                m_doneHandler = MockDirectiveHandler::create(config, LONG_HANDLING_TIME_MS);
                m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                m_exceptionEncounteredSender = make_shared<NiceMock<MockExceptionEncounteredSender>>();
                m_sequencer = DirectiveSequencer::create(m_exceptionEncounteredSender);
                ASSERT_TRUE(m_sequencer);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(m_doneHandler));
            }
            void DirectiveSequencerTest::TearDown() {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_TEST, NAME_DONE, MESSAGE_ID_DONE, DIALOG_REQUEST_ID_DONE);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST, m_attachmentManager,
                                                                          TEST_ATTACHMENT_CONTEXT_ID);
                EXPECT_CALL(*(m_doneHandler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(m_doneHandler.get()), preHandleDirective(directive, _)).Times(1);
                EXPECT_CALL(*(m_doneHandler.get()), handleDirective(_)).Times(1);
                EXPECT_CALL(*(m_doneHandler.get()), cancelDirective(_)).Times(0);
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_DONE);
                m_sequencer->onDirective(directive);
                m_doneHandler->waitUntilHandling();
                ASSERT_TRUE(m_sequencer->removeDirectiveHandler(m_doneHandler));
                m_sequencer->shutdown();
                m_sequencer.reset();
                m_doneHandler->doHandlingCompleted();
            }
            TEST_F(DirectiveSequencerTest, test_nullptrExceptionSender) {
                ASSERT_TRUE(m_sequencer);
                auto sequencer = DirectiveSequencer::create(nullptr, nullptr);
                ASSERT_FALSE(sequencer);
            }
            TEST_F(DirectiveSequencerTest, test_createAndDoneTrigger) {
                ASSERT_TRUE(m_sequencer);
            }
            TEST_F(DirectiveSequencerTest, test_nullptrDirective) {
                ASSERT_FALSE(m_sequencer->onDirective(nullptr));
            }
            TEST_F(DirectiveSequencerTest, test_unhandledDirective) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEAKER, NAME_SET_VOLUME, MESSAGE_ID_0);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST, m_attachmentManager,
                                                                          TEST_ATTACHMENT_CONTEXT_ID);
                EXPECT_CALL(*(m_exceptionEncounteredSender.get()), sendExceptionEncountered(_, _, _)).Times(1);
                m_sequencer->onDirective(directive);
            }
            TEST_F(DirectiveSequencerTest, test_emptyDialogRequestId) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEAKER, NAME_SET_VOLUME, MESSAGE_ID_0);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST, m_attachmentManager,
                                                                          TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration config;
                config[NamespaceAndName{NAMESPACE_SPEAKER, NAME_SET_VOLUME}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler = MockDirectiveHandler::create(config);
                EXPECT_CALL(*(handler.get()), handleDirectiveImmediately(directive)).Times(0);
                //EXPECT_CALL(*(handler.get()), preHandleDirective(_, _)).Times(1);
                EXPECT_CALL(*(handler.get()), handleDirective(_)).Times(1);
                EXPECT_CALL(*(handler.get()), cancelDirective(_)).Times(0);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler));
                m_sequencer->onDirective(directive);
                ASSERT_TRUE(handler->waitUntilHandling());
            }
            TEST_F(DirectiveSequencerTest, test_handleImmediatelyHandler) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_TEST, NAME_HANDLE_IMMEDIATELY, MESSAGE_ID_0);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST, m_attachmentManager,
                                                                          TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration config;
                config[NamespaceAndName{NAMESPACE_TEST, NAME_HANDLE_IMMEDIATELY}] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                auto handler = MockDirectiveHandler::create(config);
                EXPECT_CALL(*(handler.get()), handleDirectiveImmediately(directive)).Times(0);
                //EXPECT_CALL(*(handler.get()), preHandleDirective(_, _)).Times(1);
                EXPECT_CALL(*(handler.get()), handleDirective(_)).Times(1);
                EXPECT_CALL(*(handler.get()), cancelDirective(_)).Times(0);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler));
                m_sequencer->onDirective(directive);
                ASSERT_TRUE(handler->waitUntilHandling());
            }
            TEST_F(DirectiveSequencerTest, test_removingAndChangingHandlers) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEAKER, NAME_SET_VOLUME, MESSAGE_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST,
                                                          m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_TEST, NAME_NON_BLOCKING, MESSAGE_ID_1);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST,
                                                          m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEAKER, NAME_SET_VOLUME}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler0 = MockDirectiveHandler::create(handler0Config);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_TEST, NAME_NON_BLOCKING}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_TEST, NAME_NON_BLOCKING}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler2 = MockDirectiveHandler::create(handler2Config);
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler0.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(directive1, _)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirective(MESSAGE_ID_1)).Times(1);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler1));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler2));
                m_sequencer->onDirective(directive0);
                m_sequencer->onDirective(directive1);
                ASSERT_TRUE(handler2->waitUntilHandling());
            }
            TEST_F(DirectiveSequencerTest, test_blockingDirective) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST,
                                                         m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler = MockDirectiveHandler::create(handlerConfig, LONG_HANDLING_TIME_MS);
                EXPECT_CALL(*(handler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler.get()), preHandleDirective(directive, _)).Times(1);
                EXPECT_CALL(*(handler.get()), handleDirective(MESSAGE_ID_0)).Times(1);
                EXPECT_CALL(*(handler.get()), cancelDirective(_)).Times(1);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler));
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive);
                ASSERT_TRUE(handler->waitUntilHandling());
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_1);
                ASSERT_TRUE(handler->waitUntilCanceling());
            }
            TEST_F(DirectiveSequencerTest, test_blockingThenNonDialogDirective) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST,
                                                          m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = std::make_shared<AVSMessageHeader>(NAMESPACE_SPEAKER, NAME_SET_VOLUME, MESSAGE_ID_1);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST,
                                                          m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler0 = MockDirectiveHandler::create(handler0Config, LONG_HANDLING_TIME_MS);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_SPEAKER, NAME_SET_VOLUME}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(directive0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0)).Times(1);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(_, _)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirective(_)).Times(1);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive0);
                m_sequencer->onDirective(directive1);
                ASSERT_TRUE(handler1->waitUntilPreHandling());
                ASSERT_TRUE(handler0->waitUntilHandling());
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_1);
                ASSERT_TRUE(handler0->waitUntilCanceling());
            }
            TEST_F(DirectiveSequencerTest, test_bargeIn) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST,
                                                         m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler = MockDirectiveHandler::create(handlerConfig, std::chrono::milliseconds(LONG_HANDLING_TIME_MS));
                EXPECT_CALL(*(handler.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler.get()), preHandleDirective(directive, _)).Times(1);
                EXPECT_CALL(*(handler.get()), handleDirective(MESSAGE_ID_0)).Times(1);
                EXPECT_CALL(*(handler.get()), cancelDirective(MESSAGE_ID_0)).Times(1);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler));
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive);
                ASSERT_TRUE(handler->waitUntilHandling());
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_1);
                ASSERT_TRUE(handler->waitUntilCanceling());
            }
            TEST_F(DirectiveSequencerTest, testTimer_blockingThenNonBockingOnSameDialogId) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_1, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_TEST, NAME_NON_BLOCKING, MESSAGE_ID_2, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive2 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader2, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler0 = MockDirectiveHandler::create(handler0Config);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_TEST, NAME_NON_BLOCKING}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler2 = MockDirectiveHandler::create(handler2Config);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler2));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(directive0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0)).Times(1);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(directive1, _)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_1)).Times(1);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(directive2, _)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirective(MESSAGE_ID_2)).Times(1);
                EXPECT_CALL(*(handler2.get()), cancelDirective(_)).Times(0);
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive0);
                m_sequencer->onDirective(directive1);
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive2);
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(handler1->waitUntilCompleted());
                ASSERT_TRUE(handler2->waitUntilCompleted());
            }
            TEST_F(DirectiveSequencerTest, test_thatBargeInDropsSubsequentDirectives) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_1, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_TEST, NAME_BLOCKING, MESSAGE_ID_2, DIALOG_REQUEST_ID_1);
                shared_ptr<AVSDirective> directive2 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader2, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler0 = MockDirectiveHandler::create(handler0Config, LONG_HANDLING_TIME_MS);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_TEST, NAME_BLOCKING}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler2 = MockDirectiveHandler::create(handler2Config, LONG_HANDLING_TIME_MS);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler2));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(directive0)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(directive0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0)).Times(1);
                EXPECT_CALL(*(handler0.get()), cancelDirective(MESSAGE_ID_0)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(directive1, _)).Times(1);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_1)).Times(0);
                EXPECT_CALL(*(handler1.get()), cancelDirective(MESSAGE_ID_1)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(directive2)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(directive2, _)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirective(MESSAGE_ID_2)).Times(1);
                EXPECT_CALL(*(handler2.get()), cancelDirective(MESSAGE_ID_2)).Times(1);
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive0);
                m_sequencer->onDirective(directive1);
                ASSERT_TRUE(handler0->waitUntilHandling());
                ASSERT_TRUE(handler1->waitUntilPreHandling());
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_1);
                m_sequencer->onDirective(directive2);
                ASSERT_TRUE(handler2->waitUntilHandling());
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_2);
                ASSERT_TRUE(handler2->waitUntilCanceling());
            }
            TEST_F(DirectiveSequencerTest, test_preHandleDirectiveError) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_1, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler0 = MockDirectiveHandler::create(handler0Config, LONG_HANDLING_TIME_MS);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(directive0)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(directive0, _)).WillOnce(Invoke(handler0.get(), &MockDirectiveHandler::doPreHandlingFailed));
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0)).Times(0);
                EXPECT_CALL(*(handler0.get()), cancelDirective(MESSAGE_ID_0)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(directive1, _)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_1)).Times(0);
                EXPECT_CALL(*(handler1.get()), cancelDirective(MESSAGE_ID_1)).Times(0);
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive0);
                m_sequencer->onDirective(directive1);
                ASSERT_TRUE(handler0->waitUntilPreHandling());
            }
            TEST_F(DirectiveSequencerTest, test_handleDirectiveError) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_1, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler0 = MockDirectiveHandler::create(handler0Config, LONG_HANDLING_TIME_MS);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(directive0)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(directive0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0)).WillOnce(Invoke(handler0.get(), &MockDirectiveHandler::doHandlingFailed));
                EXPECT_CALL(*(handler0.get()), cancelDirective(MESSAGE_ID_0)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(directive1, _)).Times(AtMost(1));
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_1)).Times(0);
                EXPECT_CALL(*(handler1.get()), cancelDirective(MESSAGE_ID_1)).Times(AtMost(1));
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive0);
                m_sequencer->onDirective(directive1);
                ASSERT_TRUE(handler0->waitUntilHandling());
            }
            TEST_F(DirectiveSequencerTest, test_addDirectiveHandlersWhileHandlingDirectives) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_1, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_TEST, NAME_NON_BLOCKING, MESSAGE_ID_2, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive2 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader2, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler0 = MockDirectiveHandler::create(handler0Config, LONG_HANDLING_TIME_MS);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler2 = MockDirectiveHandler::create(handler2Config);
                DirectiveHandlerConfiguration handler3Config;
                handler3Config[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler3 = MockDirectiveHandler::create(handler3Config);
                DirectiveHandlerConfiguration handler4Config;
                handler4Config[NamespaceAndName{NAMESPACE_TEST, NAME_NON_BLOCKING}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler4 = MockDirectiveHandler::create(handler4Config);
                auto cancelDirectiveFunction = [this, &handler1, &handler3, &handler4](const std::string& messageId) {
                    ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler1));
                    ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler3));
                    ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler4));
                    handler4->mockCancelDirective(messageId);
                };
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler2));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler4));
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(directive0)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(directive0, _)).Times(1);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0)).Times(1);
                EXPECT_CALL(*(handler0.get()), cancelDirective(MESSAGE_ID_0)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(directive0)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(directive0, _)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirective(MESSAGE_ID_0)).Times(0);
                EXPECT_CALL(*(handler1.get()), cancelDirective(MESSAGE_ID_0)).Times(0);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(directive1, _)).Times(1);
                EXPECT_CALL(*(handler2.get()), handleDirective(MESSAGE_ID_1)).Times(0);
                EXPECT_CALL(*(handler2.get()), cancelDirective(MESSAGE_ID_1)).Times(0);
                EXPECT_CALL(*(handler3.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler3.get()), preHandleDirective(directive1, _)).Times(0);
                EXPECT_CALL(*(handler3.get()), handleDirective(MESSAGE_ID_1)).Times(1);
                EXPECT_CALL(*(handler3.get()), cancelDirective(MESSAGE_ID_1)).Times(0);
                EXPECT_CALL(*(handler4.get()), handleDirectiveImmediately(directive2)).Times(0);
                //EXPECT_CALL(*(handler4.get()), preHandleDirective(directive2, _)).Times(1);
                EXPECT_CALL(*(handler4.get()), handleDirective(MESSAGE_ID_2)).Times(0);
                EXPECT_CALL(*(handler4.get()), cancelDirective(MESSAGE_ID_2)).WillOnce(Invoke(cancelDirectiveFunction));
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive0);
                m_sequencer->onDirective(directive1);
                m_sequencer->onDirective(directive2);
                handler0->waitUntilHandling();
                handler4->waitUntilPreHandling();
                ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler2));
                ASSERT_TRUE(m_sequencer->removeDirectiveHandler(handler4));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler3));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler4));
                handler0->doHandlingCompleted();
                ASSERT_TRUE(handler4->waitUntilCanceling());
            }
            TEST_F(DirectiveSequencerTest, test_handleBlockingThenImmediatelyThenNonBockingOnSameDialogId) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_TEST, NAME_HANDLE_IMMEDIATELY, MESSAGE_ID_1, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive1 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader1, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_2, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive2 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader2, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handler0Config;
                handler0Config[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler0 = MockDirectiveHandler::create(handler0Config);
                DirectiveHandlerConfiguration handler1Config;
                handler1Config[NamespaceAndName{NAMESPACE_TEST, NAME_HANDLE_IMMEDIATELY}] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                auto handler1 = MockDirectiveHandler::create(handler1Config);
                DirectiveHandlerConfiguration handler2Config;
                handler2Config[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler2 = MockDirectiveHandler::create(handler2Config);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler0));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler1));
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler2));
                Sequence s1, s2;
                EXPECT_CALL(*(handler0.get()), handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*(handler0.get()), preHandleDirective(directive0, _)).Times(1).InSequence(s1, s2);
                EXPECT_CALL(*(handler0.get()), handleDirective(MESSAGE_ID_0)).Times(1).InSequence(s1, s2);
                EXPECT_CALL(*(handler0.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler1.get()), handleDirectiveImmediately(directive1)).Times(0);
                //EXPECT_CALL(*(handler1.get()), preHandleDirective(directive1, _)).Times(1).InSequence(s1, s2);
                //EXPECT_CALL(*(handler2.get()), preHandleDirective(directive2, _)).Times(1).InSequence(s2);
                EXPECT_CALL(*(handler1.get()), handleDirective(_)).Times(1).InSequence(s1);
                EXPECT_CALL(*(handler1.get()), cancelDirective(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), handleDirectiveImmediately(_)).Times(0);
                EXPECT_CALL(*(handler2.get()), handleDirective(MESSAGE_ID_2)).Times(1).InSequence(s1, s2);
                EXPECT_CALL(*(handler2.get()), cancelDirective(_)).Times(0);
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                m_sequencer->onDirective(directive0);
                ASSERT_TRUE(handler0->waitUntilCompleted());
                m_sequencer->onDirective(directive1);
                m_sequencer->onDirective(directive2);
                ASSERT_TRUE(handler2->waitUntilCompleted());
            }
            TEST_F(DirectiveSequencerTest, test_addDirectiveAfterDisabled) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler = MockDirectiveHandler::create(handlerConfig, LONG_HANDLING_TIME_MS);
                EXPECT_CALL(*handler, handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*handler, preHandleDirective(directive, _)).Times(0);
                EXPECT_CALL(*handler, handleDirective(MESSAGE_ID_2)).Times(0);
                EXPECT_CALL(*handler, cancelDirective(_)).Times(0);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler));
                m_sequencer->disable();
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_FALSE(m_sequencer->onDirective(directive));
                m_sequencer->enable();
            }
            TEST_F(DirectiveSequencerTest, test_disableCancelsDirective) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader, PAYLOAD_TEST,
                                                         m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[NamespaceAndName{NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                auto handler = MockDirectiveHandler::create(handlerConfig, LONG_HANDLING_TIME_MS);
                EXPECT_CALL(*handler, handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*handler, preHandleDirective(directive, _)).Times(1);
                EXPECT_CALL(*handler, handleDirective(MESSAGE_ID_0)).Times(AtMost(1));
                EXPECT_CALL(*handler, cancelDirective(_)).Times(1);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler));
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_sequencer->onDirective(directive));
                ASSERT_TRUE(handler->waitUntilPreHandling());
                m_sequencer->disable();
                ASSERT_TRUE(handler->waitUntilCanceling());
                m_sequencer->enable();
            }
            TEST_F(DirectiveSequencerTest, test_addDirectiveAfterReEnabled) {
                auto avsMessageHeader0 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_0, DIALOG_REQUEST_ID_0);
                shared_ptr<AVSDirective> directive0 = AVSDirective::create(UNPARSED_DIRECTIVE, avsMessageHeader0, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader1 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_1, DIALOG_REQUEST_ID_1);
                shared_ptr<AVSDirective> ignoredDirective1 = AVSDirective::create("ignoreDirective", avsMessageHeader1, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_2, DIALOG_REQUEST_ID_2);
                shared_ptr<AVSDirective> ignoredDirective2 = AVSDirective::create("anotherIgnored", avsMessageHeader2, PAYLOAD_TEST, m_attachmentManager, TEST_ATTACHMENT_CONTEXT_ID);
                DirectiveHandlerConfiguration handlerConfig;
                handlerConfig[NamespaceAndName{NAMESPACE_AUDIO_PLAYER, NAME_PLAY}] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                auto handler = MockDirectiveHandler::create(handlerConfig);
                EXPECT_CALL(*handler, handleDirectiveImmediately(_)).Times(0);
                //EXPECT_CALL(*handler, preHandleDirective(_, _)).Times(0);
                EXPECT_CALL(*handler, handleDirective(_)).Times(0);
                EXPECT_CALL(*handler, cancelDirective(_)).Times(0);
                //EXPECT_CALL(*handler, preHandleDirective(directive0, _)).Times(1);
                EXPECT_CALL(*handler, handleDirective(MESSAGE_ID_0)).Times(1);
                ASSERT_TRUE(m_sequencer->addDirectiveHandler(handler));
                m_sequencer->disable();
                ASSERT_FALSE(m_sequencer->onDirective(ignoredDirective1));
                ASSERT_FALSE(m_sequencer->onDirective(ignoredDirective2));
                m_sequencer->enable();
                m_sequencer->setDialogRequestId(DIALOG_REQUEST_ID_0);
                ASSERT_TRUE(m_sequencer->onDirective(directive0));
                ASSERT_TRUE(handler->waitUntilCompleted());
            }
        }
    }
}