#include <future>
#include <memory>
#include <gmock/gmock.h>
#include <avs/AVSDirective.h>
#include <avs/NamespaceAndName.h>
#include <sdkinterfaces/AlexaEventProcessedObserverInterface.h>
#include <sdkinterfaces/MockAlexaInterfaceMessageSender.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <memory/Memory.h>
#include "AlexaInterfaceCapabilityAgent.h"
#include "AlexaInterfaceMessageSenderInternalInterface.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace alexa {
            namespace test {
                using namespace avs;
                using namespace sdkInterfaces;
                using namespace utils;
                using namespace memory;
                using namespace testing;
                using namespace sdkInterfaces::test;
                static const string NAMESPACE = "Alexa";
                static const string EVENT_PROCESSED_DIRECTIVE_NAME = "EventProcessed";
                static const string REPORT_STATE_DIRECTIVE_NAME = "ReportState";
                static const NamespaceAndName EVENT_PROCESSED{NAMESPACE, EVENT_PROCESSED_DIRECTIVE_NAME};
                static const NamespaceAndName REPORT_STATE{NAMESPACE, REPORT_STATE_DIRECTIVE_NAME};
                static const string TEST_ENDPOINT_ID = "test-endpoint";
                static const string TEST_MESSAGE_ID = "abcdefg";
                static const string TEST_EVENTCORRELATION_TOKEN = "abcdefg";
                static const string VALID_EVENT_PROCESSED_DIRECTIVE = "{\"directive\":{\"header\":{\"namespace\":\"" + NAMESPACE+"\","
                                                                      "\"name\":\""+EVENT_PROCESSED.name+"\",\"messageId\":\"" +
                                                                      TEST_MESSAGE_ID + "\",\"eventCorrelationToken\":\"" +
                                                                      TEST_EVENTCORRELATION_TOKEN+"\"},\"payload\":{}}}";
                static const string UNKNOWN_DIRECTIVE = "{\"directive\":{\"header\":{\"namespace\":\"" + NAMESPACE+"\",\"name\":"
                                                        "\"UnknownDirective\",\"messageId\":\"" + TEST_MESSAGE_ID+"\","
                                                        "\"eventCorrelationToken\":\"" + TEST_EVENTCORRELATION_TOKEN + "\"},"
                                                        "\"payload\":{}}}";
                static const string EVENT_PROCESSED_WITH_NO_EVENT_CORRELATION_TOKEN = "{\"directive\":{\"header\":{\"namespace\":\"" +
                                                                                      NAMESPACE+"\",\"name\":\"" + EVENT_PROCESSED.name+"\","
                                                                                      "\"messageId\":\"" + TEST_MESSAGE_ID+"\"},"
                                                                                      "\"payload\":{}}}";
                static const string VALID_ALEXA_REPORTSTATE_DIRECTIVE = "{\"directive\":{\"header\":{\"namespace\":\""+NAMESPACE+"\","
                                                                        "\"name\":\""+REPORT_STATE.name + "\",\"messageId\":\"" +
                                                                        TEST_MESSAGE_ID+"\",\"correlationToken\":\"" +
                                                                        TEST_EVENTCORRELATION_TOKEN+"\"},\"endpoint\":{\"endpointId\":\"" +
                                                                        TEST_ENDPOINT_ID+"\"},\"payload\":{}}}";
                static const string INVALID_ALEXA_REPORTSTATE_DIRECTIVE_NO_ENDPOINT = "{\"directive\":{\"header\":{\"namespace\":\"" +
                                                                                      NAMESPACE + "\",\"name\":\"" + REPORT_STATE.name+"\","
                                                                                      "\"messageId\":\""+TEST_MESSAGE_ID+"\","
                                                                                      "\"correlationToken\":\""+TEST_EVENTCORRELATION_TOKEN +
                                                                                      "\"},\"payload\":{}}}";
                static milliseconds TIMEOUT(1000);
                class TestEventProcessedObserver : public AlexaEventProcessedObserverInterface {
                public:
                    bool waitForEventProcessed(const string& eventConrrelationToken) {
                        unique_lock<std::mutex> lock{m_mutex};
                        auto predicate = [this, eventConrrelationToken] { return m_eventCorrelationToken == eventConrrelationToken; };
                        return m_wakeTrigger.wait_for(lock, TIMEOUT, predicate);
                    }
                    void onAlexaEventProcessedReceived(const string& eventCorrelationToken) {
                        lock_guard<mutex> lock{m_mutex};
                        m_eventCorrelationToken = eventCorrelationToken;
                        m_wakeTrigger.notify_one();
                    }
                private:
                    mutex m_mutex;
                    condition_variable m_wakeTrigger;
                    string m_eventCorrelationToken;
                };
                class MockAlexaInterfaceMessageSenderInternal : public AlexaInterfaceMessageSenderInternalInterface {
                public:
                    MOCK_METHOD3(sendStateReportEvent, bool(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint));
                    MOCK_METHOD4(sendResponseEvent, bool(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                 const string& jsonPayload));
                    MOCK_METHOD5(sendErrorResponseEvent, bool(const string& instance, const string& correlationToken,
                                 const AVSMessageEndpoint& endpoint, const ErrorResponseType errorType, const string& errorMessage));
                    MOCK_METHOD3(sendDeferredResponseEvent, bool(const string& instance, const string& correlationToken,
                                 const int estimatedDeferralInSeconds));
                    MOCK_METHOD1(alexaResponseTypeToErrorType, ErrorResponseType(const AlexaResponseType& responseType));
                };
                class AlexaInterfaceCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                    void wakeOnSetCompleted();
                    AlexaInterfaceCapabilityAgentTest() : m_wakeSetCompletedPromise{}, m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {}
                protected:
                    promise<void> m_wakeSetCompletedPromise;
                    future<void> m_wakeSetCompletedFuture;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                    shared_ptr<DeviceInfo> m_deviceInfo;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    shared_ptr<MockAlexaInterfaceMessageSenderInternal> m_mockAlexaMessageSender;
                    shared_ptr<AlexaInterfaceCapabilityAgent> m_alexaInterfaceCapabilityAgent;
                };
                void AlexaInterfaceCapabilityAgentTest::SetUp() {
                    m_deviceInfo = DeviceInfo::create("testClientId","testProductId","testSerialNumber",
                                       "testManufacturer","testDescription","testFriendlyName",
                                             "testDeviceType");
                    m_mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    m_mockAlexaMessageSender = make_shared<MockAlexaInterfaceMessageSenderInternal>();
                    m_alexaInterfaceCapabilityAgent = AlexaInterfaceCapabilityAgent::create(*m_deviceInfo, TEST_ENDPOINT_ID,
                                                                                            m_mockExceptionSender, m_mockAlexaMessageSender);
                    ASSERT_NE(m_alexaInterfaceCapabilityAgent, nullptr);
                }
                void AlexaInterfaceCapabilityAgentTest::TearDown() {}
                void AlexaInterfaceCapabilityAgentTest::wakeOnSetCompleted() {
                    m_wakeSetCompletedPromise.set_value();
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, createWithInvalidParameters) {
                    auto alexaInterfaceCA = AlexaInterfaceCapabilityAgent::create(*m_deviceInfo, TEST_ENDPOINT_ID, nullptr, m_mockAlexaMessageSender);
                    ASSERT_EQ(alexaInterfaceCA, nullptr);
                    alexaInterfaceCA = AlexaInterfaceCapabilityAgent::create(*m_deviceInfo, TEST_ENDPOINT_ID, m_mockExceptionSender, nullptr);
                    ASSERT_EQ(alexaInterfaceCA, nullptr);
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testDefaultEndpoint) {
                    auto endpointId = m_deviceInfo->getDefaultEndpointId();
                    auto alexaInterfaceCA = AlexaInterfaceCapabilityAgent::create(*m_deviceInfo, endpointId, m_mockExceptionSender, m_mockAlexaMessageSender);
                    ASSERT_NE(alexaInterfaceCA, nullptr);
                    auto configuration = alexaInterfaceCA->getConfiguration();
                    EXPECT_NE(configuration.find(EVENT_PROCESSED), configuration.end());
                    EXPECT_NE(configuration.find({NAMESPACE, REPORT_STATE_DIRECTIVE_NAME, endpointId}), configuration.end());
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testNonDefaultEndpoint) {
                    auto endpointId = TEST_ENDPOINT_ID;
                    auto alexaInterfaceCA = AlexaInterfaceCapabilityAgent::create(*m_deviceInfo, endpointId, m_mockExceptionSender, m_mockAlexaMessageSender);
                    ASSERT_NE(alexaInterfaceCA, nullptr);
                    auto configuration = alexaInterfaceCA->getConfiguration();
                    EXPECT_EQ(configuration.find(EVENT_PROCESSED), configuration.end());
                    EXPECT_NE(configuration.find({NAMESPACE, REPORT_STATE_DIRECTIVE_NAME, endpointId}), configuration.end());
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testValidUnknownDirective) {
                    auto directivePair = AVSDirective::create(UNKNOWN_DIRECTIVE, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    ASSERT_THAT(directive, NotNull());
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, Eq(ExceptionErrorType::UNSUPPORTED_OPERATION), _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs(this, &AlexaInterfaceCapabilityAgentTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendStateReportEvent(_, _, _)).Times(Exactly(0));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::handleDirective(directive->getMessageId());
                    EXPECT_EQ(m_wakeSetCompletedFuture.wait_for(TIMEOUT), std::future_status::ready);
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testInvalidEventProcessedDirective) {
                    auto directivePair = AVSDirective::create(EVENT_PROCESSED_WITH_NO_EVENT_CORRELATION_TOKEN, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    ASSERT_THAT(directive, NotNull());
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, Eq(ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED), _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs(this, &AlexaInterfaceCapabilityAgentTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendStateReportEvent(_, _, _)).Times(Exactly(0));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::handleDirective(directive->getMessageId());
                    EXPECT_EQ(m_wakeSetCompletedFuture.wait_for(TIMEOUT), std::future_status::ready);
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testValidEventProcessedDirective) {
                    auto directivePair = AVSDirective::create(VALID_EVENT_PROCESSED_DIRECTIVE, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    ASSERT_THAT(directive, NotNull());
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs(this, &AlexaInterfaceCapabilityAgentTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendStateReportEvent(_, _, _)).Times(Exactly(0));
                    auto testObserver = make_shared<TestEventProcessedObserver>();
                    m_alexaInterfaceCapabilityAgent->addEventProcessedObserver(testObserver);
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::handleDirective(directive->getMessageId());
                    EXPECT_EQ(m_wakeSetCompletedFuture.wait_for(TIMEOUT), std::future_status::ready);
                    ASSERT_TRUE(testObserver->waitForEventProcessed(TEST_EVENTCORRELATION_TOKEN));
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testValidReportStateDirective) {
                    auto directivePair = AVSDirective::create(VALID_ALEXA_REPORTSTATE_DIRECTIVE, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    ASSERT_THAT(directive, NotNull());
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendErrorResponseEvent(_, _, _, _, _)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs(this, &AlexaInterfaceCapabilityAgentTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendStateReportEvent(_, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([](const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint) {
                            EXPECT_EQ(correlationToken, TEST_EVENTCORRELATION_TOKEN);
                            EXPECT_EQ(endpoint.endpointId, TEST_ENDPOINT_ID);
                            return true;
                        }));
                    auto testObserver = make_shared<TestEventProcessedObserver>();
                    m_alexaInterfaceCapabilityAgent->addEventProcessedObserver(testObserver);
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::preHandleDirective(directive,move(m_mockDirectiveHandlerResult));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::handleDirective(directive->getMessageId());
                    EXPECT_EQ(m_wakeSetCompletedFuture.wait_for(TIMEOUT),future_status::ready);
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testValidReportStateDirectiveReportStateFailure) {
                    auto directivePair = AVSDirective::create(VALID_ALEXA_REPORTSTATE_DIRECTIVE, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    ASSERT_THAT(directive, NotNull());
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendErrorResponseEvent(_, _, _, Eq(AlexaInterfaceMessageSenderInternalInterface::ErrorResponseType::INTERNAL_ERROR), _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs(this, &AlexaInterfaceCapabilityAgentTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendStateReportEvent(_, _, _)).WillOnce(Return(false));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::handleDirective(directive->getMessageId());
                    EXPECT_EQ(m_wakeSetCompletedFuture.wait_for(TIMEOUT), future_status::ready);
                }
                TEST_F(AlexaInterfaceCapabilityAgentTest, testInvalidReportStateDirectiveNoEndpoint) {
                    auto directivePair = AVSDirective::create(INVALID_ALEXA_REPORTSTATE_DIRECTIVE_NO_ENDPOINT, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    ASSERT_THAT(directive, NotNull());
                    EXPECT_CALL(*m_mockAlexaMessageSender, sendErrorResponseEvent(_, _, _, Eq(AlexaInterfaceMessageSenderInternalInterface::ErrorResponseType::INVALID_DIRECTIVE), _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs(this, &AlexaInterfaceCapabilityAgentTest::wakeOnSetCompleted));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_alexaInterfaceCapabilityAgent->CapabilityAgent::handleDirective(directive->getMessageId());
                    EXPECT_EQ(m_wakeSetCompletedFuture.wait_for(TIMEOUT), future_status::ready);
                }
            }
        }
    }
}