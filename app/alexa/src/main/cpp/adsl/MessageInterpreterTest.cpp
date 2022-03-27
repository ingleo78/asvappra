#include <avs/attachment/AttachmentManager.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include "DirectiveSequencer.h"
#include "MessageInterpreter.h"
#include "MockDirectiveSequencer.h"

namespace alexaClientSDK {
    namespace adsl {
        namespace test {
            using namespace std;
            using namespace testing;
            using namespace avsCommon;
            using namespace avs;
            using namespace attachment;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            static const string NAMESPACE_TEST = "SpeechSynthesizer";
            static const string NAME_TEST = "Speak";
            static const string MESSAGE_ID_TEST = "testMessageId";
            static const string DIALOG_REQUEST_ID_TEST = "dialogRequestIdTest";
            static const string PAYLOAD_TEST = R"({"url":"cid:testCID","format":"testFormat","token":"testToken"})";
            static const string INVALID_JSON = "invalidTestJSON }}";
            static const string TEST_ATTACHMENT_CONTEXT_ID = "testContextId";
            static const string SPEAK_DIRECTIVE = R"({
                "directive": {
                    "header": {
                        "namespace":")" + NAMESPACE_TEST + R"(",
                        "name": ")" + NAME_TEST + R"(",
                        "messageId": ")" + MESSAGE_ID_TEST + R"(",
                        "dialogRequestId": ")" + DIALOG_REQUEST_ID_TEST + R"("
                    },
                    "payload": )" + PAYLOAD_TEST + R"(
                }
            })";
            static const string DIRECTIVE_INVALID_DIRECTIVE_KEY = R"({
                "Foo_directive": {
                    "header": {
                        "namespace":"namespace_test",
                        "name": "name_test",
                        "messageId": "messageId_test",
                        "dialogRequestId": "dialogRequestId_test"
                    },
                    "payload":{}
                }
            })";
            static const string DIRECTIVE_INVALID_HEADER_KEY = R"({
                "directive": {
                    "Foo_header": {
                        "namespace":"namespace_test",
                        "name": "name_test",
                        "messageId": "messageId_test",
                        "dialogRequestId": "dialogRequestId_test"
                    },
                    "payload":{}
                }
            })";
            static const string DIRECTIVE_INVALID_NAMESPACE_KEY = R"({
                "directive": {
                    "header": {
                        "Foo_namespace":"namespace_test",
                        "name": "name_test",
                        "messageId": "messageId_test",
                        "dialogRequestId": "dialogRequestId_test"
                    },
                    "payload":{}
                }
            })";
            static const string DIRECTIVE_INVALID_NAME_KEY = R"({
                "directive": {
                    "header": {
                        "namespace":"namespace_test",
                        "Foo_name": "name_test",
                        "messageId": "messageId_test",
                        "dialogRequestId": "dialogRequestId_test"
                    },
                    "payload":{}
                }
            })";
            static const string DIRECTIVE_INVALID_MESSAGEID_KEY = R"({
                "directive": {
                    "header": {
                        "namespace":"namespace_test",
                        "name": "name_test",
                        "Foo_messageId": "messageId_test",
                        "dialogRequestId": "dialogRequestId_test"
                    },
                    "payload":{}
                }
            })";
            static const string DIRECTIVE_NO_PAYLOAD = R"({
                "directive": {
                    "header": {
                        "namespace":"namespace_test",
                        "name": "name_test",
                        "messageId": "messageId_test"
                    }
                }
            })";
            static const string DIRECTIVE_INVALID_PAYLOAD_KEY = R"({
                "directive": {
                    "header": {
                        "namespace":"namespace_test",
                        "name": "name_test",
                        "Foo_messageId": "messageId_test",
                        "dialogRequestId": "dialogRequestId_test"
                    },
                    "Foo_payload":{}
                }
            })";
            static const string DIRECTIVE_NO_DIALOG_REQUEST_ID_KEY = R"({
                "directive": {
                    "header": {
                        "namespace":")" + NAMESPACE_TEST + R"(",
                        "name": ")" + NAME_TEST + R"(",
                        "messageId": ")" + MESSAGE_ID_TEST + R"("
                    },
                    "payload": )" + PAYLOAD_TEST + R"(
                }
            })";
            class MessageIntepreterTest : public Test {
            protected:
                void SetUp() override {
                    m_mockExceptionEncounteredSender = make_shared<MockExceptionEncounteredSender>();
                    m_mockDirectiveSequencer = make_shared<MockDirectiveSequencer>();
                    m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                    m_messageInterpreter = make_shared<MessageInterpreter>(m_mockExceptionEncounteredSender, m_mockDirectiveSequencer, m_attachmentManager);
                }
                shared_ptr<MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
                shared_ptr<AttachmentManager> m_attachmentManager;
                shared_ptr<MockDirectiveSequencer> m_mockDirectiveSequencer;
                shared_ptr<MessageInterpreter> m_messageInterpreter;
            };
            TEST_F(MessageIntepreterTest, test_messageIsInValidJSON) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, INVALID_JSON);
            }
            TEST_F(MessageIntepreterTest, test_messageHasInvalidDirectiveKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_INVALID_DIRECTIVE_KEY);
            }
            TEST_F(MessageIntepreterTest, test_messageHasInvalidHeaderKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_INVALID_HEADER_KEY);
            }
            TEST_F(MessageIntepreterTest, test_messageHasInvalidNamespaceKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_INVALID_NAMESPACE_KEY);
            }
            TEST_F(MessageIntepreterTest, test_messageHasInvalidNameKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_INVALID_NAME_KEY);
            }
            TEST_F(MessageIntepreterTest, test_messageHasInvalidMessageIdKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_INVALID_MESSAGEID_KEY);
            }
            TEST_F(MessageIntepreterTest, test_messageHasNoDialogRequestIdKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(0);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_))
                    .Times(1)
                    .WillOnce(Invoke([](shared_ptr<AVSDirective> avsDirective) -> bool {
                        EXPECT_EQ(avsDirective->getNamespace(), NAMESPACE_TEST);
                        EXPECT_EQ(avsDirective->getName(), NAME_TEST);
                        EXPECT_EQ(avsDirective->getMessageId(), MESSAGE_ID_TEST);
                        EXPECT_TRUE(avsDirective->getDialogRequestId().empty());
                        return true;
                    }));
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_NO_DIALOG_REQUEST_ID_KEY);
            }
            TEST_F(MessageIntepreterTest, test_messageHasNoPayloadKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_NO_PAYLOAD);
            }
            TEST_F(MessageIntepreterTest, test_messageHasInvalidPayloadKey) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(1);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_)).Times(0);
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, DIRECTIVE_INVALID_PAYLOAD_KEY);
            }
            TEST_F(MessageIntepreterTest, test_messageIsValidDirective) {
                EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(0);
                EXPECT_CALL(*m_mockDirectiveSequencer, onDirective(_))
                    .Times(1)
                    .WillOnce(Invoke([](shared_ptr<AVSDirective> avsDirective) -> bool {
                        EXPECT_EQ(avsDirective->getNamespace(), NAMESPACE_TEST);
                        EXPECT_EQ(avsDirective->getName(), NAME_TEST);
                        EXPECT_EQ(avsDirective->getMessageId(), MESSAGE_ID_TEST);
                        EXPECT_EQ(avsDirective->getDialogRequestId(), DIALOG_REQUEST_ID_TEST);
                        return true;
                    }));
                m_messageInterpreter->receive(TEST_ATTACHMENT_CONTEXT_ID, SPEAK_DIRECTIVE);
            }
        }
    }
}