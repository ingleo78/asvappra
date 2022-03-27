#include <chrono>
#include <memory>
#include <thread>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/AVSDirective.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <timing/TimePoint.h>
#include "AlexaInterfaceMessageSender.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace alexa {
            namespace test {
                using namespace testing;
                using namespace timing;
                using namespace rapidjson;
                using namespace sdkInterfaces::test;
                static const seconds MY_WAIT_TIMEOUT(2);
                static const string NAME_POWER_CONTROLLER("PowerController");
                static const string NAMESPACE_POWER_CONTROLLER("Alexa.PowerController");
                static const string NAME_TURN_ON("TurnOn");
                static const string POWER_STATE("powerState");
                static const string POWER_STATE_ON("\"ON\"");
                static const string EVENT("event");
                static const string HEADER("header");
                static const string MESSAGE_ID("messageId");
                static const string MESSAGE_ID_TEST("MessageId_Test");
                static const string CORRELATION_TOKEN("correlationToken");
                static const string CORRELATION_TOKEN_TEST("CorrelationToken_Test");
                static const string EVENT_CORRELATION_TOKEN("eventCorrelationToken");
                static const string EVENT_CORRELATION_TOKEN_TEST("EventCorrelationToken_Test");
                static const string PAYLOAD_VERSION("version");
                static const string PAYLOAD_VERSION_TEST("3");
                static const string TIME_OF_SAMPLE("timeOfSample");
                static const string TIME_OF_SAMPLE_TEST("2017-02-03T16:20:50.523Z");
                static const string ENDPOINT_ID("endpointId");
                static const string ENDPOINT_ID_TEST("EndpointId_Test");
                static const string ERROR_ENDPOINT_UNREACHABLE("ENDPOINT_UNREACHABLE");
                static const string ERROR_ENDPOINT_UNREACHABLE_MESSAGE("Endpoint unreachable message");
                static const string PAYLOAD("payload");
                static const string PAYLOAD_TEST("payload_Test");
                static const string STATE_REPORT_CONTEXT = "\"context\":{\"properties\":[{\"namespace\":\"Alexa.PowerController\","
                                                           "\"name\":\"powerState\",\"value\":\"ON\",\"timeOfSample\":\"" +
                                                           TIME_OF_SAMPLE_TEST + "\",\"uncertaintyInMilliseconds\": 0}]}";
                static const string STATE_REPORT_EVENT_JSON_STRING = "{\"event\":{\"header\":{\"namespace\":\"Alexa\",\"name\":\""
                                                                     "StateReport\",\"messageId\":\"" + MESSAGE_ID_TEST + "\","
                                                                     "\"correlationToken\":\"" + CORRELATION_TOKEN_TEST + "\","
                                                                     "\"eventCorrelationToken\":\"" + EVENT_CORRELATION_TOKEN_TEST + "\","
                                                                     "\"payloadVersion\":\""+PAYLOAD_VERSION_TEST+"\""
                                                                     "},\"endpoint\":{\"endpointId\":\"" + ENDPOINT_ID_TEST + "\"},"
                                                                     "\"payload\":{}}," + STATE_REPORT_CONTEXT + "}";
                static const string STATE_REPORT_EVENT_NO_CONTEXT_JSON_STRING = "{\"event\":{\"header\":{\"namespace\":\"Alexa\","
                                                                                "\"name\":\"StateReport\",\"messageId\":\"" +
                                                                                MESSAGE_ID_TEST+"\",\"correlationToken\":\"" +
                                                                                CORRELATION_TOKEN_TEST + "\",\"eventCorrelationToken\":\"" +
                                                                                EVENT_CORRELATION_TOKEN_TEST + "\",\"payloadVersion\":\"" +
                                                                                PAYLOAD_VERSION_TEST + "\"},\"endpoint\":{\"endpointId\":\"" +
                                                                                ENDPOINT_ID_TEST + "\"},\"payload\":{}}}";
                static const string TURNON_PROPERTIES_STRING = "\"properties\":[{\"namespace\":\"Alexa.PowerController\",\"name\":\""
                                                               "powerState\",\"value\":\"ON\",\"timeOfSample\":\"" + TIME_OF_SAMPLE_TEST +
                                                               "\",\"uncertaintyInMilliseconds\": 0}]";
                static const string TURNON_CONTEXT_STRING = "\"context\":{" + TURNON_PROPERTIES_STRING + "}";
                static const string TURNON_RESPONSE_EVENT_STRING = "\"event\":{\"header\":{\"namespace\":\"Alexa\",\"name\":\"Response\","
                                                                   "\"messageId\":\"" + MESSAGE_ID_TEST + "\",\"correlationToken\":\"" +
                                                                   CORRELATION_TOKEN_TEST + "\",\"eventCorrelationToken\":\"" +
                                                                   EVENT_CORRELATION_TOKEN_TEST + "\",\"payloadVersion\":\"" +
                                                                   PAYLOAD_VERSION_TEST + "\"},\"endpoint\":{\"endpointId\":\"" +
                                                                   ENDPOINT_ID_TEST + "\"},\"payload\":{}}";
                static const string TURNON_RESPONSE_EVENT_WITH_CONTEXT_STRING = "{" + TURNON_RESPONSE_EVENT_STRING + "," +
                                                                                TURNON_CONTEXT_STRING + "}";
                static const string TURNON_RESPONSE_EVENT_WITHOUT_CONTEXT_STRING = "{" + TURNON_RESPONSE_EVENT_STRING + "}";
                static const string ERROR_RESPONSE_EVENT_STRING = "{\"event\":{\"header\":{\"namespace\":\"Alexa\",\"name\":"
                                                                  "\"ErrorResponse\",\"messageId\":\"" + MESSAGE_ID_TEST + "\","
                                                                  "\"correlationToken\":\"" + CORRELATION_TOKEN_TEST + "\","
                                                                  "\"eventCorrelationToken\":\"" + EVENT_CORRELATION_TOKEN_TEST + "\","
                                                                  "\"payloadVersion\":\"" + PAYLOAD_VERSION_TEST + "\"},\"endpoint\":{"
                                                                  "\"endpointId\":\"" + ENDPOINT_ID_TEST + "\"},\"payload\":{"
                                                                  "\"type\":\"" + ERROR_ENDPOINT_UNREACHABLE + "\",\"message\":\"" +
                                                                  ERROR_ENDPOINT_UNREACHABLE_MESSAGE + "\"}}}";
                static const string DEFERRED_RESPONSE_EVENT_STRING = "{\"event\":{\"header\":{\"namespace\":\"Alexa\",""\"name\":"
                                                                     "\"DeferredResponse\",\"messageId\":\"" + MESSAGE_ID_TEST + "\","
                                                                     "\"correlationToken\":\"" + CORRELATION_TOKEN_TEST + "\","
                                                                     "\"eventCorrelationToken\":\"" + EVENT_CORRELATION_TOKEN_TEST + "\","
                                                                     "\"payloadVersion\":\""+PAYLOAD_VERSION_TEST+"\"},\"payload\":{"
                                                                     "\"estimatedDeferralInSeconds\":7}}}";
                static const string TURNON_CHANGE_REPORT_WITH_CHANGE_EVENT_STRING = "{\"context\":{\"properties\":[]},\"event\":{"
                                                                                    "\"header\":{\"namespace\":\"Alexa\",\"name\":"
                                                                                    "\"ChangeReport\",\"messageId\":\"" + MESSAGE_ID_TEST +
                                                                                    "\",\"eventCorrelationToken\":\"" +
                                                                                    EVENT_CORRELATION_TOKEN_TEST + "\",\"payloadVersion\":\"" +
                                                                                    PAYLOAD_VERSION_TEST + "\"},\"endpoint\":{\"endpointId\":"
                                                                                    "\""+ENDPOINT_ID_TEST+"\"},\"payload\":{\"change\":{"
                                                                                    "\"cause\":{\"type\":\"ALEXA_INTERACTION\"}," +
                                                                                    TURNON_PROPERTIES_STRING + "}}}}";
                class AlexaInterfaceMessageSenderTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                public:
                    AVSMessageEndpoint buildTestEndpoint(void);
                    CapabilityTag buildTestTag(void);
                    CapabilityState buildTestState(void);
                    std::shared_ptr<AlexaInterfaceMessageSender> createMessageSender(void);
                    bool removeMessageId(Document* document, string* messageId);
                    bool removeEventCorrelationToken(Document* document, string* eventCorrelationToken);
                    bool expectEventSent(const shared_ptr<AlexaInterfaceMessageSender>& sender, const string& jsonEventString,
                                         MessageRequestObserverInterface::Status sendStatus, function<void()> triggerOperation);
                    bool expectEventSentOnInvalidContext(const shared_ptr<AlexaInterfaceMessageSender>& sender, const string& jsonEventString,
                                                         MessageRequestObserverInterface::Status sendStatus, function<void()> triggerOperation);
                    bool expectEventSentWithoutContext(const shared_ptr<AlexaInterfaceMessageSender>& sender, const string& jsonEventString,
                                                       MessageRequestObserverInterface::Status sendStatus, function<void()> triggerOperation);
                    bool expectEventNotSentOnInvalidContext(const shared_ptr<AlexaInterfaceMessageSender>& sender, const string& jsonEventString,
                                                            MessageRequestObserverInterface::Status sendStatus, function<void()> triggerOperation);
                    bool checkEventJson(string jsonEventString, string testEventString);
                protected:
                    shared_ptr<MockContextManager> m_mockContextManager;
                    shared_ptr<MockMessageSender> m_messageSender;
                    AVSContext m_context;
                };
                void AlexaInterfaceMessageSenderTest::SetUp() {
                    m_mockContextManager = make_shared<StrictMock<MockContextManager>>();
                    m_messageSender = make_shared<StrictMock<MockMessageSender>>();
                    m_context.addState(buildTestTag(), buildTestState());
                }
                void AlexaInterfaceMessageSenderTest::TearDown() {
                    m_messageSender.reset();
                    m_mockContextManager.reset();
                }
                AVSMessageEndpoint AlexaInterfaceMessageSenderTest::buildTestEndpoint(void) {
                    return AVSMessageEndpoint(ENDPOINT_ID_TEST);
                }
                CapabilityTag AlexaInterfaceMessageSenderTest::buildTestTag(void) {
                    return CapabilityTag(NAMESPACE_POWER_CONTROLLER, POWER_STATE, ENDPOINT_ID_TEST);
                }
                CapabilityState AlexaInterfaceMessageSenderTest::buildTestState(void) {
                    TimePoint timePoint;
                    timePoint.setTime_ISO_8601(TIME_OF_SAMPLE_TEST);
                    return CapabilityState(POWER_STATE_ON, timePoint, 0);
                }
                bool AlexaInterfaceMessageSenderTest::removeMessageId(Document* document, std::string* messageId) {
                    if (!document || !messageId) return false;
                    auto it = document->FindMember(EVENT.c_str());
                    if (it == document->MemberEnd()) return false;
                    it = it->value.FindMember(HEADER.c_str());
                    if (it == document->MemberEnd()) return false;
                    auto& eventHeader = it->value;
                    it = it->value.FindMember(MESSAGE_ID.c_str());
                    if (it == document->MemberEnd()) return false;
                    *messageId = it->value.GetString();
                    eventHeader.RemoveMember(it);
                    return true;
                }
                bool AlexaInterfaceMessageSenderTest::removeEventCorrelationToken(Document* document, string* eventCorrelationToken) {
                    if (!document || !eventCorrelationToken) return false;
                    auto it = document->FindMember(EVENT.c_str());
                    if (it == document->MemberEnd()) return false;
                    it = it->value.FindMember(HEADER.c_str());
                    if (it == document->MemberEnd()) return false;
                    auto& eventHeader = it->value;
                    it = it->value.FindMember(EVENT_CORRELATION_TOKEN.c_str());
                    if (it == document->MemberEnd()) return false;
                    *eventCorrelationToken = it->value.GetString();
                    eventHeader.RemoveMember(it);
                    return true;
                }
                std::shared_ptr<AlexaInterfaceMessageSender> AlexaInterfaceMessageSenderTest::createMessageSender(void) {
                    EXPECT_CALL(*m_mockContextManager, addContextManagerObserver(_)).Times(1);
                    auto sender = AlexaInterfaceMessageSender::create(m_mockContextManager, m_messageSender);
                    EXPECT_THAT(sender, NotNull());
                    return sender;
                }
                bool AlexaInterfaceMessageSenderTest::expectEventSent(const shared_ptr<AlexaInterfaceMessageSender>& sender, const string& jsonEventString,
                                                                      MessageRequestObserverInterface::Status sendStatus,
                                                                      function<void()> triggerOperation) {
                    promise<bool> eventPromise;
                    promise<bool> contextPromise;
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillOnce(InvokeWithoutArgs([this, sender, &contextPromise] {
                        const ContextRequestToken token = 1;
                        sender->onContextAvailable(ENDPOINT_ID_TEST, m_context, token);
                        contextPromise.set_value(true);
                        return token;
                    }));
                    EXPECT_CALL(*m_messageSender, sendMessage(_)).Times(1)
                        .WillOnce(Invoke([this, sendStatus, &jsonEventString, &eventPromise](shared_ptr<MessageRequest> request) {
                            if (checkEventJson(request->getJsonContent(), jsonEventString)) {
                                request->sendCompleted(sendStatus);
                                eventPromise.set_value(true);
                            } else eventPromise.set_value(false);
                        }));
                    triggerOperation();
                    EXPECT_TRUE(contextPromise.get_future().wait_for(MY_WAIT_TIMEOUT) == future_status::ready);
                    auto sendFuture = eventPromise.get_future();
                    bool isSendFutureReady = false;
                    isSendFutureReady = sendFuture.wait_for(MY_WAIT_TIMEOUT) == future_status::ready;
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_messageSender.get()));
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_mockContextManager.get()));
                    this_thread::sleep_for(milliseconds(10));
                    if (!isSendFutureReady) return false;
                    return sendFuture.get();
                }
                bool AlexaInterfaceMessageSenderTest::expectEventSentOnInvalidContext(const shared_ptr<AlexaInterfaceMessageSender>& sender,
                                                                                      const string& jsonEventString,
                                                                                      MessageRequestObserverInterface::Status sendStatus,
                                                                                      function<void()> triggerOperation) {
                    promise<bool> eventPromise;
                    promise<bool> contextPromise;
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillOnce(InvokeWithoutArgs([sender, &contextPromise] {
                        const ContextRequestToken token = 1;
                        sender->onContextFailure(ContextRequestError::STATE_PROVIDER_TIMEDOUT, token);
                        contextPromise.set_value(false);
                        return token;
                    }));
                    EXPECT_CALL(*m_messageSender, sendMessage(_)).Times(1)
                        .WillOnce(Invoke([this, sendStatus, &jsonEventString, &eventPromise](shared_ptr<MessageRequest> request) {
                            if (checkEventJson(request->getJsonContent(), jsonEventString)) {
                                request->sendCompleted(sendStatus);
                                eventPromise.set_value(true);
                            } else eventPromise.set_value(false);
                        }));
                    triggerOperation();
                    EXPECT_TRUE(contextPromise.get_future().wait_for(MY_WAIT_TIMEOUT) == future_status::ready);
                    auto sendFuture = eventPromise.get_future();
                    bool isSendFutureReady = false;
                    isSendFutureReady = sendFuture.wait_for(MY_WAIT_TIMEOUT) == future_status::ready;
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_messageSender.get()));
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_mockContextManager.get()));
                    this_thread::sleep_for(milliseconds(10));
                    if (!isSendFutureReady) return false;
                    return sendFuture.get();
                }
                bool AlexaInterfaceMessageSenderTest::expectEventSentWithoutContext(const shared_ptr<AlexaInterfaceMessageSender>& sender,
                                                                                    const string& jsonEventString,
                                                                                    MessageRequestObserverInterface::Status sendStatus,
                                                                                    function<void()> triggerOperation) {
                    promise<bool> eventPromise;
                    promise<bool> contextPromise;
                    EXPECT_CALL(*m_messageSender, sendMessage(_)).Times(1)
                        .WillOnce(Invoke([this, sendStatus, &jsonEventString, &eventPromise](shared_ptr<MessageRequest> request) {
                            if (checkEventJson(request->getJsonContent(), jsonEventString)) {
                                request->sendCompleted(sendStatus);
                                eventPromise.set_value(true);
                            } else eventPromise.set_value(false);
                        }));
                    triggerOperation();
                    auto sendFuture = eventPromise.get_future();
                    bool isSendFutureReady = false;
                    isSendFutureReady = sendFuture.wait_for(MY_WAIT_TIMEOUT) == future_status::ready;
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_messageSender.get()));
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_mockContextManager.get()));
                    this_thread::sleep_for(milliseconds(10));
                    if (!isSendFutureReady) return false;
                    return sendFuture.get();
                }
                bool AlexaInterfaceMessageSenderTest::expectEventNotSentOnInvalidContext(const shared_ptr<AlexaInterfaceMessageSender>& sender,
                                                                                         const string& jsonEventString,
                                                                                         MessageRequestObserverInterface::Status sendStatus,
                                                                                         function<void()> triggerOperation) {
                    promise<bool> eventPromise;
                    promise<bool> contextPromise;
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillOnce(InvokeWithoutArgs([sender, &contextPromise] {
                        const ContextRequestToken token = 1;
                        sender->onContextFailure(ContextRequestError::STATE_PROVIDER_TIMEDOUT, token);
                        contextPromise.set_value(false);
                        return token;
                    }));
                    triggerOperation();
                    EXPECT_TRUE(contextPromise.get_future().wait_for(MY_WAIT_TIMEOUT) == std::future_status::ready);
                    auto sendFuture = eventPromise.get_future();
                    bool isSendFutureReady = false;
                    isSendFutureReady = sendFuture.wait_for(MY_WAIT_TIMEOUT) == future_status::ready;
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_messageSender.get()));
                    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_mockContextManager.get()));
                    this_thread::sleep_for(milliseconds(10));
                    if (!isSendFutureReady) return false;
                    return sendFuture.get();
                }
                bool AlexaInterfaceMessageSenderTest::checkEventJson(string jsonEventString, string testEventString) {
                    Document expected, actual;
                    expected.Parse(testEventString.data());
                    actual.Parse(jsonEventString.data());
                    string value;
                    EXPECT_TRUE(removeMessageId(&expected, &value));
                    EXPECT_TRUE(removeMessageId(&actual, &value));
                    EXPECT_TRUE(removeEventCorrelationToken(&expected, &value));
                    EXPECT_TRUE(removeEventCorrelationToken(&actual, &value));
                    EXPECT_EQ(expected, actual) << "Expected: " << testEventString << "\nValue: " << jsonEventString;
                    return true;
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_givenInvalidParameters_create_shouldFail) {
                    auto handler = AlexaInterfaceMessageSender::create(nullptr, m_messageSender);
                    EXPECT_THAT(handler, IsNull());
                    handler = AlexaInterfaceMessageSender::create(m_mockContextManager, nullptr);
                    EXPECT_THAT(handler, IsNull());
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendReportState_shouldSucceedAndSend) {
                    auto sender = createMessageSender();
                    bool result = false;
                    bool directiveResponseEventSent = expectEventSent(sender, STATE_REPORT_EVENT_JSON_STRING,MessageRequestObserverInterface::Status::SUCCESS,
                                                        [this, sender, &result]() {
                                                                          result = sender->sendStateReportEvent("", CORRELATION_TOKEN_TEST, buildTestEndpoint());
                                                                      });
                    ASSERT_TRUE(result);
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendReportState_missingContext_shouldSucceedAndSend) {
                    auto sender = createMessageSender();
                    bool result = false;
                    bool directiveResponseEventSent = expectEventSentOnInvalidContext(sender, STATE_REPORT_EVENT_NO_CONTEXT_JSON_STRING,
                                                                            MessageRequestObserverInterface::Status::SUCCESS,
                                                                        [this, sender, &result]() {
                                                                                          result = sender->sendStateReportEvent("", CORRELATION_TOKEN_TEST,
                                                                                                                        buildTestEndpoint());
                                                                                      });
                    ASSERT_TRUE(result);
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendResponse_shouldSend) {
                    auto sender = createMessageSender();
                    bool result = false;
                    bool directiveResponseEventSent = expectEventSent(sender, TURNON_RESPONSE_EVENT_WITH_CONTEXT_STRING,
                                                            MessageRequestObserverInterface::Status::SUCCESS,
                                                        [this, sender, &result]() {
                                                                          result = sender->sendResponseEvent("", CORRELATION_TOKEN_TEST, buildTestEndpoint());
                                                                      });
                    ASSERT_TRUE(result);
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendResponse_noContext_shouldSend) {
                    auto sender = createMessageSender();
                    bool result = false;
                    bool directiveResponseEventSent = expectEventSentOnInvalidContext(sender, TURNON_RESPONSE_EVENT_WITHOUT_CONTEXT_STRING,
                                                                            MessageRequestObserverInterface::Status::SUCCESS,
                                                                        [this, sender, &result]() {
                                                                                          result = sender->sendResponseEvent("", CORRELATION_TOKEN_TEST,
                                                                                                                     buildTestEndpoint());
                                                                                      });
                    ASSERT_TRUE(result);
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendErrorResponse_shouldSend) {
                    auto sender = createMessageSender();
                    bool result = false;
                    bool directiveResponseEventSent = expectEventSentWithoutContext(sender, ERROR_RESPONSE_EVENT_STRING,
                                                                          MessageRequestObserverInterface::Status::SUCCESS,
                                                                      [this, sender, &result]() {
                                                                                        result = sender->sendErrorResponseEvent("", CORRELATION_TOKEN_TEST, buildTestEndpoint(),
                                                                                                                       AlexaInterfaceMessageSenderInterface::ErrorResponseType::ENDPOINT_UNREACHABLE,
                                                                                                                                ERROR_ENDPOINT_UNREACHABLE_MESSAGE);
                                                                                    });
                    ASSERT_TRUE(result);
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendDeferredResponse_shouldSend) {
                    auto sender = createMessageSender();
                    bool result = false;
                    bool directiveResponseEventSent = expectEventSentWithoutContext(sender, DEFERRED_RESPONSE_EVENT_STRING,
                                                                          MessageRequestObserverInterface::Status::SUCCESS,
                                                                                    [sender, &result]() {
                                                                                        result = sender->sendDeferredResponseEvent("", CORRELATION_TOKEN_TEST, 7);
                                                                                    });
                    ASSERT_TRUE(result);
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendResponse_withChange_shouldSend) {
                    auto sender = createMessageSender();
                    bool directiveResponseEventSent = expectEventSent(sender, TURNON_CHANGE_REPORT_WITH_CHANGE_EVENT_STRING,
                                                            MessageRequestObserverInterface::Status::SUCCESS,
                                                                      [this, sender]() {
                                                                          sender->onStateChanged(buildTestTag(), buildTestState(), AlexaStateChangeCauseType::ALEXA_INTERACTION);
                                                                      });
                    ASSERT_TRUE(directiveResponseEventSent);
                }
                TEST_F(AlexaInterfaceMessageSenderTest, test_sendResponse_withChange_withoutContext_shouldNotSend) {
                    auto sender = createMessageSender();
                    bool directiveResponseEventSent = expectEventNotSentOnInvalidContext(sender, "",MessageRequestObserverInterface::Status::SUCCESS,
                                                                           [this, sender]() {
                                                                                             sender->onStateChanged(buildTestTag(), buildTestState(), AlexaStateChangeCauseType::ALEXA_INTERACTION);
                                                                                         });
                    ASSERT_FALSE(directiveResponseEventSent);
                }
            }
        }
    }
}