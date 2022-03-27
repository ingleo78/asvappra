#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockAlexaInterfaceMessageSender.h>
#include <sdkinterfaces/ToggleController/ToggleControllerAttributes.h>
#include <sdkinterfaces/ToggleController/ToggleControllerInterface.h>
#include <memory/Memory.h>
#include <timing/TimePoint.h>
#include <util/WaitEvent.h>
#include "ToggleControllerAttributeBuilder.h"
#include "ToggleControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace toggleController {
            namespace test {
                using namespace memory;
                using namespace rapidjson;
                using namespace testing;
                using namespace timing;
                using namespace avs::attachment::test;
                using namespace sdkInterfaces::toggleController;
                using namespace sdkInterfaces::test;
                using EndpointIdentifier = endpoints::EndpointIdentifier;
                using ToggleState = ToggleControllerInterface::ToggleState;
                static milliseconds TIMEOUT(1000);
                static const string NAMESPACE{"Alexa.ToggleController"};
                static const string INTERFACE_VERSION{"3"};
                static const string NAME_TURNON{"TurnOn"};
                static const string NAME_TURNOFF{"TurnOff"};
                static const string TOGGLESTATE_PROPERTY_NAME{"toggleState"};
                static const string UNKNOWN_DIRECTIVE{"Unknown"};
                static const EndpointIdentifier TEST_ENDPOINT_ID("testEndpointId");
                static const EndpointIdentifier TEST_INSTANCE("testInstance");
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
                class MockToggleControllerInterface : public ToggleControllerInterface {
                public:
                    using SetToggleStateResult = pair<AlexaResponseType, string>;
                    using GetToggleStateResult = pair<AlexaResponseType, Optional<ToggleState>>;
                    //MOCK_METHOD2(setToggleState, SetToggleStateResult(const bool state, const AlexaStateChangeCauseType cause));
                    MOCK_METHOD0(getToggleState, GetToggleStateResult());
                    MOCK_METHOD1(addObserver, bool(shared_ptr<ToggleControllerObserverInterface>));
                    MOCK_METHOD1(removeObserver, void(const shared_ptr<ToggleControllerObserverInterface>&));
                };
                class ToggleControllerCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                    ToggleControllerCapabilityAgentTest() {}
                    Optional<ToggleControllerAttributes> buildToggleControllerAttribute(const CapabilityResources& capabilityResources) {
                        auto toggleControllerAttributeBuilder = ToggleControllerAttributeBuilder::create();
                        toggleControllerAttributeBuilder->withCapabilityResources(capabilityResources);
                        return toggleControllerAttributeBuilder->build();
                    }
                protected:
                    shared_ptr<ToggleControllerCapabilityAgent> createCapabilityAgentAndSetExpects(const ToggleControllerAttributes& toggleControllerAttributes,
                                                                                                   bool proactivelyReported, bool retrievable,
                                                                                                   bool nonControllable);
                    ToggleControllerInterface::ToggleState TEST_POWER_STATE_ON;
                    ToggleControllerInterface::ToggleState TEST_POWER_STATE_OFF;
                    shared_ptr<StrictMock<MockToggleControllerInterface>> m_mockToggleController;
                    shared_ptr<ToggleControllerObserverInterface> m_observer;
                    shared_ptr<StrictMock<MockContextManager>> m_mockContextManager;
                    shared_ptr<MockAlexaInterfaceMessageSender> m_mockResponseSender;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                };
                using TCCAT = ToggleControllerCapabilityAgentTest;
                void ToggleControllerCapabilityAgentTest::SetUp() {
                    m_mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    m_mockToggleController = make_shared<StrictMock<MockToggleControllerInterface>>();
                    m_mockContextManager = make_shared<testing::StrictMock<MockContextManager>>();
                    m_mockResponseSender = make_shared<MockAlexaInterfaceMessageSender>();
                    auto timePoint = TimePoint();
                    timePoint.setTime_ISO_8601(TIME_OF_SAMPLE_TEST);
                    TEST_POWER_STATE_ON = ToggleControllerInterface::ToggleState{true, timePoint,milliseconds(0)};
                    TEST_POWER_STATE_OFF = ToggleControllerInterface::ToggleState{false, timePoint,milliseconds(0)};
                }
                shared_ptr<ToggleControllerCapabilityAgent> TCCAT::createCapabilityAgentAndSetExpects(const ToggleControllerAttributes& toggleControllerAttributes,
                                                                                                      bool proactivelyReported, bool retrievable,
                                                                                                      bool nonControllable) {
                    if (retrievable) {
                        //EXPECT_CALL(*m_mockContextManager, addStateProvider(_, NotNull()));
                        EXPECT_CALL(*m_mockContextManager, removeStateProvider(_));
                    }
                    if (proactivelyReported) {
                        EXPECT_CALL(*m_mockToggleController, addObserver(_))
                            .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ToggleControllerObserverInterface> observer) {
                                m_observer = observer;
                                return true;
                            })));
                        EXPECT_CALL(*m_mockToggleController, removeObserver(_))
                            .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ToggleControllerObserverInterface> observer) {
                                          m_observer = nullptr;
                                      })));
                    }
                    auto toggleControllerCapabilityAgent = ToggleControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, toggleControllerAttributes,
                                                                                                   m_mockToggleController,m_mockContextManager,
                                                                                                   m_mockResponseSender,m_mockExceptionSender,
                                                                                                   proactivelyReported, retrievable,
                                                                                                   nonControllable);
                    return toggleControllerCapabilityAgent;
                }
                static shared_ptr<AVSDirective> buildAVSDirective(string directiveName) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE, directiveName, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST,
                                                                          CORRELATION_TOKEN_TEST, EVENT_CORRELATION_TOKEN_TEST,
                                                                          INTERFACE_VERSION, TEST_INSTANCE);
                    auto avsMessageEndpoint = AVSMessageEndpoint(TEST_ENDPOINT_ID);
                    return AVSDirective::create("", avsMessageHeader, "", attachmentManager, "",
                                                avsMessageEndpoint);
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_givenInvalidParameters_create_shouldFail) {
                    CapabilityResources emptyResource;
                    auto emptyToggleControllerAttribute = buildToggleControllerAttribute(emptyResource);
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create("", "", emptyToggleControllerAttribute.value(), nullptr, nullptr,
                                nullptr, nullptr, true, true),IsNull());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create("", TEST_INSTANCE, toggleControllerAttributes.value(),
                                m_mockToggleController, m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),
                                IsNull());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create(TEST_ENDPOINT_ID, "", toggleControllerAttributes.value(),
                                m_mockToggleController, m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),
                                IsNull());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, emptyToggleControllerAttribute.value(),
                                m_mockToggleController, m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),
                                IsNull());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, toggleControllerAttributes.value(),
                                nullptr, m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, toggleControllerAttributes.value(),
                                m_mockToggleController, nullptr, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, toggleControllerAttributes.value(),
                                m_mockToggleController, m_mockContextManager, nullptr, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(ToggleControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, toggleControllerAttributes.value(),
                                m_mockToggleController, m_mockContextManager, m_mockResponseSender, nullptr, true, true),IsNull());
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_turnOnDirective_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockToggleController, setToggleState(true, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) { return make_pair(AlexaResponseType::SUCCESS, ""); })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendResponseEvent(_, _, _, _));
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), false, true, false);
                    ASSERT_THAT(toggleControllerCapabilityAgent, NotNull());
                    toggleControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_TURNON),move(m_mockDirectiveHandlerResult));
                    toggleControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_turnOnDirective_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockToggleController, setToggleState(true, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) {
                            return std::make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE, "TestEndpointNotReachable");
                        })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendErrorResponseEvent(_, _, _, _, _));
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(toggleControllerCapabilityAgent, NotNull());
                    toggleControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_TURNON),move(m_mockDirectiveHandlerResult));
                    toggleControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_turnOffDirective_successCase) {
                    avsCommon::utils::WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockToggleController, setToggleState(false, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) { return make_pair(AlexaResponseType::SUCCESS, ""); })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendResponseEvent(_, _, _, _));
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), false, true, false);
                    ASSERT_THAT(toggleControllerCapabilityAgent, NotNull());
                    toggleControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_TURNOFF),move(m_mockDirectiveHandlerResult));
                    toggleControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_turnOffDirective_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockToggleController, setToggleState(false, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) {
                            return std::make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE, "TestEndpointNotReachable");
                        })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendErrorResponseEvent(_, _, _, _, _));
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(toggleControllerCapabilityAgent, NotNull());
                    toggleControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_TURNOFF),move(m_mockDirectiveHandlerResult));
                    toggleControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_reportStateChange_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(toggleControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockToggleController, setToggleState(true, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onToggleStateChanged(TEST_POWER_STATE_ON, cause);
                            return make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([toggleControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            toggleControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, TOGGLESTATE_PROPERTY_NAME,
                                                                          TEST_ENDPOINT_ID, TEST_INSTANCE), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockToggleController, getToggleState())
                        .WillOnce(InvokeWithoutArgs([this]() -> MockToggleControllerInterface::GetToggleStateResult {
                            return make_pair(AlexaResponseType::SUCCESS, Optional<ToggleState>(TEST_POWER_STATE_ON));
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateResponse(_, _, _)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    m_mockToggleController->setToggleState(true, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_reportStateChange_errorCase) {
                    avsCommon::utils::WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(toggleControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockToggleController, setToggleState(true, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onToggleStateChanged(TEST_POWER_STATE_ON, cause);
                            return std::make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([toggleControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            toggleControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, TOGGLESTATE_PROPERTY_NAME,
                                                                          TEST_ENDPOINT_ID, TEST_INSTANCE), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockToggleController, getToggleState())
                        .WillOnce(InvokeWithoutArgs([]() -> MockToggleControllerInterface::GetToggleStateResult {
                            return make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE,Optional<ToggleControllerInterface::ToggleState>());
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateUnavailableResponse(_, _, _))
                        .WillOnce(InvokeWithoutArgs([&waitEvent]() { waitEvent.wakeUp(); }));
                    m_mockToggleController->setToggleState(true, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableTrue) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(toggleControllerCapabilityAgent, NotNull());
                    toggleControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE), move(m_mockDirectiveHandlerResult));
                    toggleControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
                TEST_F(ToggleControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableFalse) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("light", "en-US"));
                    auto toggleControllerAttributes = buildToggleControllerAttribute(resource);
                    ASSERT_TRUE(toggleControllerAttributes.hasValue());
                    auto toggleControllerCapabilityAgent = createCapabilityAgentAndSetExpects(toggleControllerAttributes.value(), false, false, false);
                    toggleControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE), move(m_mockDirectiveHandlerResult));
                    toggleControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    toggleControllerCapabilityAgent->shutdown();
                }
            }
        }
    }
}