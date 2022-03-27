#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockAlexaInterfaceMessageSender.h>
#include <sdkinterfaces/PowerController/PowerControllerInterface.h>
#include <memory/Memory.h>
#include <timing/TimePoint.h>
#include <util/WaitEvent.h>
#include "PowerControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace powerController {
            namespace test {
                using namespace attachment;
                using namespace memory;
                using namespace testing;
                using namespace timing;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                static milliseconds TIMEOUT(1000);
                static const string NAMESPACE{"Alexa.PowerController"};
                static const string INTERFACE_VERSION{"3"};
                static const string NAME_TURNON{"TurnOn"};
                static const string NAME_TURNOFF{"TurnOff"};
                static const string POWERSTATE_PROPERTY_NAME{"powerState"};
                static const string POWERSTATE_ON{R"("ON")"};
                static const string POWERSTATE_OFF{R"("OFF")"};
                static const string UNKNOWN_DIRECTIVE{"Unknown"};
                static const EndpointIdentifier TEST_ENDPOINT_ID("testEndpointId");
                static const string EVENT("event");
                static const string HEADER("header");
                static const string MESSAGE_ID("messageId");
                static const string MESSAGE_ID_TEST("MessageId_Test");
                static const string DIALOG_REQUEST_ID("dialogRequestId");
                static const string DIALOG_REQUEST_ID_TEST("DialogRequestId_Test");
                static const string CORRELATION_TOKEN("correlationToken");
                static const string CORRELATION_TOKEN_TEST("CorrelationToken_Test");
                static const string EVENT_CORRELATION_TOKEN("eventCorrelationToken");
                static const string EVENT_CORRELATION_TOKEN_TEST("EventCorrelationToken_Test");
                static const string TIME_OF_SAMPLE("timeOfSample");
                static const string TIME_OF_SAMPLE_TEST("2017-02-03T16:20:50.523Z");
                class MockPowerControllerInterface : public PowerControllerInterface {
                public:
                    using SetPowerStateResult = pair<AlexaResponseType, string>;
                    using GetPowerStateResult = pair<AlexaResponseType, Optional<PowerState>>;
                    //MOCK_METHOD2(setPowerState, SetPowerStateResult(const bool state, const AlexaStateChangeCauseType cause));
                    MOCK_METHOD0(getPowerState, GetPowerStateResult());
                    MOCK_METHOD1(addObserver, bool(shared_ptr<PowerControllerObserverInterface>));
                    MOCK_METHOD1(removeObserver, void(const shared_ptr<PowerControllerObserverInterface>&));
                };
                class PowerControllerCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                    PowerControllerCapabilityAgentTest() {}
                protected:
                    shared_ptr<PowerControllerCapabilityAgent> createCapabilityAgentAndSetExpects(bool proactivelyReported, bool retrievable);
                    PowerControllerInterface::PowerState m_testPowerStateOn;
                    shared_ptr<StrictMock<MockPowerControllerInterface>> m_mockPowerController;
                    shared_ptr<PowerControllerObserverInterface> m_observer;
                    shared_ptr<StrictMock<MockContextManager>> m_mockContextManager;
                    shared_ptr<MockAlexaInterfaceMessageSender> m_mockResponseSender;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                };
                void PowerControllerCapabilityAgentTest::SetUp() {
                    m_mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    //m_mockPowerController = make_shared<StrictMock<MockPowerControllerInterface>>();
                    m_mockContextManager = make_shared<StrictMock<MockContextManager>>();
                    m_mockResponseSender = make_shared<MockAlexaInterfaceMessageSender>();
                    auto timePoint = TimePoint();
                    timePoint.setTime_ISO_8601(TIME_OF_SAMPLE_TEST);
                    m_testPowerStateOn = PowerControllerInterface::PowerState{true, timePoint,milliseconds(0)};
                }
                void PowerControllerCapabilityAgentTest::TearDown() {}
                shared_ptr<PowerControllerCapabilityAgent> PowerControllerCapabilityAgentTest::createCapabilityAgentAndSetExpects(bool proactivelyReported,
                                                                                                                                  bool retrievable) {
                    if (retrievable) {
                        //EXPECT_CALL(*m_mockContextManager, addStateProvider(_, NotNull()));
                        EXPECT_CALL(*m_mockContextManager, removeStateProvider(_));
                    }
                    if (proactivelyReported) {
                        EXPECT_CALL(*m_mockPowerController, addObserver(_))
                            .WillOnce(WithArg<0>(Invoke([this](shared_ptr<PowerControllerObserverInterface> observer) {
                                m_observer = observer;
                                return true;
                            })));
                        EXPECT_CALL(*m_mockPowerController, removeObserver(_))
                            .WillOnce(WithArg<0>(Invoke([this](shared_ptr<PowerControllerObserverInterface> observer) {
                                m_observer = nullptr;
                            })));
                    }
                    auto powerControllerCapabilityAgent = PowerControllerCapabilityAgent::create(TEST_ENDPOINT_ID,m_mockPowerController,
                                                                                    m_mockContextManager,m_mockResponseSender,
                                                                                    m_mockExceptionSender, proactivelyReported,
                                                                                                  retrievable);
                    return powerControllerCapabilityAgent;
                }
                shared_ptr<AVSDirective> buildAVSDirective(string directiveName) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE, directiveName, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST,
                                                                          CORRELATION_TOKEN_TEST, INTERFACE_VERSION);
                    auto avsMessageEndpoint = AVSMessageEndpoint(TEST_ENDPOINT_ID);
                    return AVSDirective::create("", avsMessageHeader, "", attachmentManager, "",
                                                avsMessageEndpoint);
                }
                TEST_F(PowerControllerCapabilityAgentTest, test_givenInvalidParameters_create_shouldFail) {
                    ASSERT_THAT(PowerControllerCapabilityAgent::create("", m_mockPowerController, m_mockContextManager, m_mockResponseSender,
                                m_mockExceptionSender, true, true),IsNull());
                    ASSERT_THAT(PowerControllerCapabilityAgent::create(TEST_ENDPOINT_ID, nullptr, m_mockContextManager, m_mockResponseSender,
                                m_mockExceptionSender, true, true),IsNull());
                    ASSERT_THAT(PowerControllerCapabilityAgent::create(TEST_ENDPOINT_ID, m_mockPowerController, nullptr, m_mockResponseSender,
                                m_mockExceptionSender, true, true),IsNull());
                    ASSERT_THAT(PowerControllerCapabilityAgent::create(TEST_ENDPOINT_ID, m_mockPowerController, m_mockContextManager, nullptr,
                                m_mockExceptionSender, true, true),IsNull());
                    ASSERT_THAT(PowerControllerCapabilityAgent::create(TEST_ENDPOINT_ID, m_mockPowerController, m_mockContextManager,
                                m_mockResponseSender, nullptr, true, true),IsNull());
                }
                TEST_F(PowerControllerCapabilityAgentTest, test_turnOnDirective_successCase) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    //EXPECT_CALL(*m_mockPowerController, setPowerState(true, _))
                    //    .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) { return std::make_pair(AlexaResponseType::SUCCESS, ""); })));
                    EXPECT_CALL(*m_mockResponseSender, sendResponseEvent(_, _, _, _));
                    auto powerControllerCapabilityAgent = createCapabilityAgentAndSetExpects(false, true);
                    ASSERT_THAT(powerControllerCapabilityAgent, NotNull());
                    powerControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_TURNON),move(m_mockDirectiveHandlerResult));
                    powerControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    powerControllerCapabilityAgent->shutdown();
                }
                TEST_F(PowerControllerCapabilityAgentTest, test_turnOnDirective_errorCase) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*XPECT_CALL(*m_mockPowerController, setPowerState(true, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) {
                            return make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE, "TestEndpointNotReachable");
                        })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendErrorResponseEvent(_, _, _, _, _));
                    auto powerControllerCapabilityAgent = createCapabilityAgentAndSetExpects(true, true);
                    ASSERT_THAT(powerControllerCapabilityAgent, NotNull());
                    powerControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_TURNON), move(m_mockDirectiveHandlerResult));
                    powerControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    powerControllerCapabilityAgent->shutdown();
                }
                TEST_F(PowerControllerCapabilityAgentTest, test_reportStateChange_successCase) {
                    WaitEvent waitEvent;
                    auto powerControllerCapabilityAgent = createCapabilityAgentAndSetExpects(true, true);
                    ASSERT_THAT(powerControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockPowerController, setPowerState(true, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onPowerStateChanged(m_testPowerStateOn, cause);
                            return std::make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([powerControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            powerControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, POWERSTATE_PROPERTY_NAME,
                                                                         TEST_ENDPOINT_ID), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockPowerController, getPowerState())
                        .WillOnce(InvokeWithoutArgs([this]() -> MockPowerControllerInterface::GetPowerStateResult {
                            return make_pair(AlexaResponseType::SUCCESS,Optional<PowerControllerInterface::PowerState>(m_testPowerStateOn));
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateResponse(_, _, _)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    m_mockPowerController->setPowerState(true, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    powerControllerCapabilityAgent->shutdown();
                }
                TEST_F(PowerControllerCapabilityAgentTest, test_reportStateChange_errorCase) {
                    WaitEvent waitEvent;
                    auto powerControllerCapabilityAgent = createCapabilityAgentAndSetExpects(true, true);
                    ASSERT_THAT(powerControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockPowerController, setPowerState(true, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onPowerStateChanged(m_testPowerStateOn, cause);
                            return std::make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([powerControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            powerControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, POWERSTATE_PROPERTY_NAME,
                                                                         TEST_ENDPOINT_ID), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockPowerController, getPowerState())
                        .WillOnce(InvokeWithoutArgs([]() -> MockPowerControllerInterface::GetPowerStateResult {
                            return make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE,Optional<PowerControllerInterface::PowerState>());
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateUnavailableResponse(_, _, _))
                        .WillOnce(InvokeWithoutArgs([&waitEvent]() { waitEvent.wakeUp(); }));
                    m_mockPowerController->setPowerState(true, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    powerControllerCapabilityAgent->shutdown();
                }
                TEST_F(PowerControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableTrue) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    auto powerControllerCapabilityAgent = createCapabilityAgentAndSetExpects(true, true);
                    ASSERT_THAT(powerControllerCapabilityAgent, NotNull());
                    powerControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE),move(m_mockDirectiveHandlerResult));
                    powerControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    powerControllerCapabilityAgent->shutdown();
                }
                TEST_F(PowerControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableFalse) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    auto powerControllerCapabilityAgent = createCapabilityAgentAndSetExpects(false, false);
                    ASSERT_THAT(powerControllerCapabilityAgent, NotNull());
                    powerControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE),move(m_mockDirectiveHandlerResult));
                    powerControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    powerControllerCapabilityAgent->shutdown();
                }
            }
        }
    }
}