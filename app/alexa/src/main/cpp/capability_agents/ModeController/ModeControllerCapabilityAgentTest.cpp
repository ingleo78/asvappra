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
#include <sdkinterfaces/ModeController/ModeControllerAttributes.h>
#include <sdkinterfaces/ModeController/ModeControllerInterface.h>
#include <memory/Memory.h>
#include <timing/TimePoint.h>
#include <util/WaitEvent.h>
#include "ModeControllerAttributeBuilder.h"
#include "ModeControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace modeController {
            namespace test {
                using namespace attachment;
                using namespace memory;
                using namespace testing;
                using namespace timing;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                using EndpointIdentifier = endpoints::EndpointIdentifier;
                static milliseconds TIMEOUT(1000);
                static const string NAMESPACE{"Alexa.ModeController"};
                static const string INTERFACE_VERSION{"3"};
                static const string NAME_SETMODE{"SetMode"};
                static const string NAME_ADJUSTMODE{"AdjustMode"};
                static const string MODE_PROPERTY_NAME{"mode"};
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
                static string MODE_OPEN = "open";
                static string MODE_CLOSED = "close";
                static int MODE_DELTA = 1;
                static const string SET_MODE_PAYLOAD_TEST = R"({"mode":")" + MODE_OPEN + R"("})";
                static const string ADJUST_MODE_PAYLOAD_TEST = R"({"modeDelta":1})";
                class MockModeControllerInterface : public ModeControllerInterface {
                public:
                    using ModeResult = pair<AlexaResponseType, string>;
                    using GetModeResult = pair<AlexaResponseType, Optional<ModeState>>;
                    MOCK_METHOD0(getConfiguration, ModeControllerConfiguration());
                    //MOCK_METHOD2(setMode, ModeResult(const string& mode, const AlexaStateChangeCauseType cause));
                    //MOCK_METHOD2(adjustMode, ModeResult(const int modeDelta, const AlexaStateChangeCauseType cause));
                    MOCK_METHOD0(getMode, GetModeResult());
                    MOCK_METHOD1(addObserver, bool(shared_ptr<ModeControllerObserverInterface>));
                    MOCK_METHOD1(removeObserver, void(const shared_ptr<ModeControllerObserverInterface>&));
                };
                class ModeControllerCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                    ModeControllerCapabilityAgentTest() {}
                    Optional<ModeControllerAttributes> buildModeControllerAttribute(const CapabilityResources& capabilityResources) {
                        auto modeControllerAttributeBuilder = ModeControllerAttributeBuilder::create();
                        ModeResources closeModeResource;
                        if (!closeModeResource.addFriendlyNameWithText("Close", "en-US")) {
                            return Optional<ModeControllerAttributes>();
                        }
                        ModeResources openModeResource;
                        if (!openModeResource.addFriendlyNameWithText("Open", "en-US")) {
                            return Optional<ModeControllerAttributes>();
                        }
                        modeControllerAttributeBuilder->withCapabilityResources(capabilityResources).setOrdered(true)
                            .addMode(MODE_CLOSED, closeModeResource).addMode(MODE_OPEN, openModeResource);
                        return modeControllerAttributeBuilder->build();
                    }
                protected:
                    shared_ptr<ModeControllerCapabilityAgent> createCapabilityAgentAndSetExpects(const ModeControllerAttributes& modeControllerAttributes,
                                                                                                 bool proactivelyReported, bool retrievable,
                                                                                                 bool nonControllable);
                    ModeControllerInterface::ModeState m_testModeOpen;
                    shared_ptr<StrictMock<MockModeControllerInterface>> m_mockModeController;
                    shared_ptr<ModeControllerObserverInterface> m_observer;
                    shared_ptr<StrictMock<MockContextManager>> m_mockContextManager;
                    shared_ptr<MockAlexaInterfaceMessageSender> m_mockResponseSender;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                };
                void ModeControllerCapabilityAgentTest::SetUp() {
                    m_mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    m_mockModeController = make_shared<StrictMock<MockModeControllerInterface>>();
                    m_mockContextManager = make_shared<StrictMock<MockContextManager>>();
                    m_mockResponseSender = make_shared<MockAlexaInterfaceMessageSender>();
                    auto timePoint = TimePoint();
                    timePoint.setTime_ISO_8601(TIME_OF_SAMPLE_TEST);
                    m_testModeOpen = ModeControllerInterface::ModeState{MODE_OPEN, timePoint, milliseconds(0)};
                }
                shared_ptr<ModeControllerCapabilityAgent> ModeControllerCapabilityAgentTest::createCapabilityAgentAndSetExpects(const ModeControllerAttributes& modeControllerAttributes,
                                                                                                                                bool proactivelyReported,
                                                                                                                                bool retrievable,
                                                                                                                                bool nonControllable) {
                    EXPECT_CALL(*m_mockModeController, getConfiguration())
                        .WillOnce(InvokeWithoutArgs([]() -> ModeControllerInterface::ModeControllerConfiguration {
                            return {MODE_OPEN, MODE_CLOSED};
                        }));
                    if (retrievable) {
                        //EXPECT_CALL(*m_mockContextManager, addStateProvider(_, NotNull()));
                        EXPECT_CALL(*m_mockContextManager, removeStateProvider(_));
                    }
                    if (proactivelyReported) {
                        EXPECT_CALL(*m_mockModeController, addObserver(_))
                            .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ModeControllerObserverInterface> observer) {
                                m_observer = observer;
                                return true;
                            })));
                        EXPECT_CALL(*m_mockModeController, removeObserver(_)).WillOnce(WithArg<0>(
                                Invoke([this](shared_ptr<ModeControllerObserverInterface> observer) { m_observer = nullptr; })));
                    }
                    auto modeControllerCapabilityAgent = ModeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, modeControllerAttributes,
                                                                                  m_mockModeController, m_mockContextManager,
                                                                                  m_mockResponseSender, m_mockExceptionSender,
                                                                                               proactivelyReported, retrievable, nonControllable);
                    return modeControllerCapabilityAgent;
                }
                static shared_ptr<AVSDirective> buildAVSDirective(string directiveName, string payload) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE, directiveName, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST,
                                                                          CORRELATION_TOKEN_TEST, EVENT_CORRELATION_TOKEN_TEST, INTERFACE_VERSION,
                                                                          TEST_INSTANCE);
                    auto avsMessageEndpoint = AVSMessageEndpoint(TEST_ENDPOINT_ID);
                    return AVSDirective::create("", avsMessageHeader, payload, attachmentManager, "", avsMessageEndpoint);
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_givenInvalidParameters_create_shouldFail) {
                    CapabilityResources emptyResource;
                    auto emptyModeControllerAttribute = buildModeControllerAttribute(emptyResource);
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    EXPECT_THAT(ModeControllerCapabilityAgent::create("", "", emptyModeControllerAttribute.value(), nullptr, nullptr,
                                nullptr, nullptr, true, true),IsNull());
                    EXPECT_THAT(ModeControllerCapabilityAgent::create("", TEST_INSTANCE, modeControllerAttributes.value(), m_mockModeController,
                                m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(ModeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, "", modeControllerAttributes.value(),
                                m_mockModeController, m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),
                        IsNull());
                    EXPECT_THAT(ModeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, modeControllerAttributes.value(),
                                nullptr, m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(ModeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, modeControllerAttributes.value(),
                                m_mockModeController, nullptr, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(ModeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE,modeControllerAttributes.value(),
                                m_mockModeController, m_mockContextManager, nullptr, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(ModeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, modeControllerAttributes.value(),
                                m_mockModeController, m_mockContextManager, m_mockResponseSender, nullptr, true, true),IsNull());
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_setModeDirective_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    //EXPECT_CALL(*m_mockModeController, setMode(MODE_OPEN, _))
                    //    .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) { return make_pair(AlexaResponseType::SUCCESS, ""); })));
                    EXPECT_CALL(*m_mockResponseSender, sendResponseEvent(_, _, _, _));
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), false, true, false);
                    ASSERT_THAT(modeControllerCapabilityAgent, NotNull());
                    modeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_SETMODE, SET_MODE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    modeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_setModeDirective_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockModeController, setMode(MODE_OPEN, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) {
                            return make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE, "TestEndpointNotReachable");
                        })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendErrorResponseEvent(_, _, _, _, _));
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(modeControllerCapabilityAgent, NotNull());
                    modeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_SETMODE, SET_MODE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    modeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_adjustModeValueDirective_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockModeController, adjustMode(MODE_DELTA, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) { return make_pair(AlexaResponseType::SUCCESS, ""); })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendResponseEvent(_, _, _, _));
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), false, true, false);
                    ASSERT_THAT(modeControllerCapabilityAgent, NotNull());
                    modeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_ADJUSTMODE, ADJUST_MODE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    modeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_adjustModeValueDirective_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockModeController, adjustMode(MODE_DELTA, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) {
                            return make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE, "TestEndpointNotReachable");
                        })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendErrorResponseEvent(_, _, _, _, _));
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(modeControllerCapabilityAgent, NotNull());
                    modeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_ADJUSTMODE, ADJUST_MODE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    modeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_reportStateChange_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(modeControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockModeController, setMode(MODE_OPEN, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onModeChanged(m_testModeOpen, cause);
                            return make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([modeControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            modeControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, MODE_PROPERTY_NAME, TEST_ENDPOINT_ID, TEST_INSTANCE), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockModeController, getMode())
                        .WillOnce(InvokeWithoutArgs([this]() -> MockModeControllerInterface::GetModeResult {
                            return make_pair(AlexaResponseType::SUCCESS,Optional<ModeControllerInterface::ModeState>(m_testModeOpen));
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateResponse(_, _, _)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    m_mockModeController->setMode(MODE_OPEN, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_reportStateChange_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(modeControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockModeController, setMode(MODE_OPEN, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onModeChanged(m_testModeOpen, cause);
                            return make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([modeControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            modeControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, MODE_PROPERTY_NAME, TEST_ENDPOINT_ID, TEST_INSTANCE), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockModeController, getMode())
                        .WillOnce(InvokeWithoutArgs([]() -> MockModeControllerInterface::GetModeResult {
                            return make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE,Optional<ModeControllerInterface::ModeState>());
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateUnavailableResponse(_, _, _))
                        .WillOnce(InvokeWithoutArgs([&waitEvent]() { waitEvent.wakeUp(); }));
                    m_mockModeController->setMode(MODE_OPEN, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableTrue) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(modeControllerCapabilityAgent, NotNull());
                    modeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE, SET_MODE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    modeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
                TEST_F(ModeControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableFalse) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("mode", "en-US"));
                    auto modeControllerAttributes = buildModeControllerAttribute(resource);
                    ASSERT_TRUE(modeControllerAttributes.hasValue());
                    auto modeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(modeControllerAttributes.value(), false, false, false);
                    modeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE, SET_MODE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    modeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    modeControllerCapabilityAgent->shutdown();
                }
            }
        }
    }
}