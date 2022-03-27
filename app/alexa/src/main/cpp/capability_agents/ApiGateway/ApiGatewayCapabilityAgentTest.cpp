#include <memory>
#include <gmock/gmock.h>
#include <avs/AVSDirective.h>
#include <avs/NamespaceAndName.h>
#include <sdkinterfaces/MockAVSGatewayManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <memory/Memory.h>
#include "ApiGatewayCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace apiGateway {
            namespace test {
                using namespace memory;
                using namespace testing;
                using namespace sdkInterfaces::test;
                static const string NAMESPACE = "Alexa.ApiGateway";
                static const NamespaceAndName SET_GATEWAY_REQUEST{NAMESPACE, "SetGateway"};
                static const string TEST_GATEWAY_URL = "https://avs-alexa-na.amazon.com";
                static const string SET_GATEWAY_DIRECTIVE_JSON_STRING = R"({"directive": {"header": {"namespace": ")" + NAMESPACE + R"(","name": ")" +
                                                                        SET_GATEWAY_REQUEST.name + R"(","messageId": "12345"},"payload": {
                                                                        "gateway": ")" + TEST_GATEWAY_URL + R"("}}})";
                static const string UNKNOWN_DIRECTIVE_JSON_STRING = R"({"directive": {"header": {"namespace": ")" + NAMESPACE + R"(",
                                                                    "name": "NewDialogRequest1","messageId": "12345"},"payload": {
                                                                    "gateway": ")" + TEST_GATEWAY_URL + R"("}}})";
                static const string NO_PAYLOAD_SET_GATEWAY_DIRECTIVE_JSON_STRING = R"({"directive": {"header": {"namespace": ")" + NAMESPACE + R"(",
                                                                                   "name": ")" + SET_GATEWAY_REQUEST.name + R"(","messageId": "12345"
                                                                                   },"payload": {}}})";
                static const string INVALID_PAYLOAD_SET_GATEWAY_DIRECTIVE_JSON_STRING = R"({"directive": {"header": {"namespace": ")" + NAMESPACE + R"(",
                                                                                        "name": ")" + SET_GATEWAY_REQUEST.name + R"(","messageId": "12345"
                                                                                        },"payload": {"gateway": 2}}})";
                static milliseconds TIMEOUT(1000);
                class ApiGatewayCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                    void wakeOnSetCompleted();
                    ApiGatewayCapabilityAgentTest() :
                            m_wakeSetCompletedPromise{},
                            m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {
                    }
                protected:
                    promise<void> m_wakeSetCompletedPromise;
                    future<void> m_wakeSetCompletedFuture;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                    shared_ptr<StrictMock<MockAVSGatewayManager>> m_mockAVSGatewayManager;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    shared_ptr<ApiGatewayCapabilityAgent> m_apiGatewayCA;
                };
                void ApiGatewayCapabilityAgentTest::SetUp() {
                    m_mockExceptionSender = std::make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockAVSGatewayManager = std::make_shared<StrictMock<MockAVSGatewayManager>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    m_apiGatewayCA = ApiGatewayCapabilityAgent::create(m_mockAVSGatewayManager, m_mockExceptionSender);
                    ASSERT_NE(m_apiGatewayCA, nullptr);
                }
                void ApiGatewayCapabilityAgentTest::TearDown() {
                    if (m_apiGatewayCA) {
                        m_apiGatewayCA->shutdown();
                        m_apiGatewayCA.reset();
                    }
                }
                void ApiGatewayCapabilityAgentTest::wakeOnSetCompleted() {
                    m_wakeSetCompletedPromise.set_value();
                }
                TEST_F(ApiGatewayCapabilityAgentTest, createNoGatewayManager) {
                    auto apiGatewayCA = ApiGatewayCapabilityAgent::create(nullptr, m_mockExceptionSender);
                    ASSERT_EQ(apiGatewayCA, nullptr);
                }
                TEST_F(ApiGatewayCapabilityAgentTest, createNoAVSGatewayManager) {
                    auto apiGatewayCA = ApiGatewayCapabilityAgent::create(m_mockAVSGatewayManager, nullptr);
                    ASSERT_EQ(apiGatewayCA, nullptr);
                }
                TEST_F(ApiGatewayCapabilityAgentTest, testNullDirective) {
                    EXPECT_CALL(*m_mockAVSGatewayManager, setGatewayURL(_)).Times(Exactly(0));
                    m_apiGatewayCA->handleDirectiveImmediately(nullptr);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(ApiGatewayCapabilityAgentTest, testValidUnknownDirective) {
                    auto directivePair = AVSDirective::create(UNKNOWN_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    EXPECT_CALL(*m_mockAVSGatewayManager, setGatewayURL(_)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([](const string& unparsedDirective, ExceptionErrorType errorType, const string& errorDescription) {
                            EXPECT_EQ(errorType, ExceptionErrorType::UNSUPPORTED_OPERATION);
                        }));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &ApiGatewayCapabilityAgentTest::wakeOnSetCompleted));
                    m_apiGatewayCA->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_apiGatewayCA->CapabilityAgent::handleDirective(directive->getMessageId());
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(ApiGatewayCapabilityAgentTest, testValidDirectiveWithNoPayload) {
                    auto directivePair = AVSDirective::create(NO_PAYLOAD_SET_GATEWAY_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    EXPECT_CALL(*m_mockAVSGatewayManager, setGatewayURL(_)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([](const string& unparsedDirective, ExceptionErrorType errorType, const string& errorDescription) {
                            EXPECT_EQ(errorType, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        }));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &ApiGatewayCapabilityAgentTest::wakeOnSetCompleted));
                    m_apiGatewayCA->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_apiGatewayCA->CapabilityAgent::handleDirective(directive->getMessageId());
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(ApiGatewayCapabilityAgentTest, testValidDirectiveWithInvalidPayload) {
                    auto directivePair = AVSDirective::create(INVALID_PAYLOAD_SET_GATEWAY_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    EXPECT_CALL(*m_mockAVSGatewayManager, setGatewayURL(_)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([](const string& unparsedDirective, ExceptionErrorType errorType, const string& errorDescription) {
                            EXPECT_EQ(errorType, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        }));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &ApiGatewayCapabilityAgentTest::wakeOnSetCompleted));
                    m_apiGatewayCA->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_apiGatewayCA->CapabilityAgent::handleDirective(directive->getMessageId());
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(ApiGatewayCapabilityAgentTest, testValidSetGatewayDirective) {
                    auto directivePair = AVSDirective::create(SET_GATEWAY_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    EXPECT_CALL(*m_mockAVSGatewayManager, setGatewayURL(_)).Times(Exactly(1))
                        .WillOnce(Invoke([](const string& gatewayURL) {
                            EXPECT_EQ(gatewayURL, TEST_GATEWAY_URL);
                            return true;
                        }));
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &ApiGatewayCapabilityAgentTest::wakeOnSetCompleted));
                    m_apiGatewayCA->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_apiGatewayCA->CapabilityAgent::handleDirective(directive->getMessageId());
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
            }
        }
    }
}