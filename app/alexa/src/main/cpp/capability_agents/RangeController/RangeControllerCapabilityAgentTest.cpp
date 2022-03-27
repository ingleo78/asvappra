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
#include <sdkinterfaces/RangeController/RangeControllerAttributes.h>
#include <sdkinterfaces/RangeController/RangeControllerInterface.h>
#include <memory/Memory.h>
#include <timing/TimePoint.h>
#include <util/WaitEvent.h>
#include "RangeControllerAttributeBuilder.h"
#include "RangeControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace rangeController {
            namespace test {
                using namespace resources;
                using namespace attachment;
                using namespace memory;
                using namespace testing;
                using namespace timing;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                using EndpointIdentifier = endpoints::EndpointIdentifier;
                static milliseconds TIMEOUT(1000);
                static const string NAMESPACE{"Alexa.RangeController"};
                static const string INTERFACE_VERSION{"3"};
                static const string NAME_SETRANGEVALUE{"SetRangeValue"};
                static const string NAME_ADJUSTRANGEVALUE{"AdjustRangeValue"};
                static const string RANGEVALUE_PROPERTY_NAME{"rangeValue"};
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
                static double RANGE_VALUE_MINUMUM = 0;
                static double RANGE_VALUE_MAXIMUM = 100;
                static double RANGE_VALUE_MEDIUM = 50;
                static double RANGE_PRECISION = 1;
                static const string SET_RANGE_VALUE_PAYLOAD_TEST = R"({"rangeValue":)" + to_string(RANGE_VALUE_MEDIUM) + R"(})";
                static const string ADJUST_RANGE_VALUE_PAYLOAD_TEST = R"({"rangeValueDelta":)" + to_string(RANGE_PRECISION) + R"(,
                                                                      "rangeValueDeltaDefault":false})";
                class MockRangeControllerInterface : public RangeControllerInterface {
                public:
                    using RangeValueResult = pair<AlexaResponseType, string>;
                    using GetRangeValueResult = pair<AlexaResponseType, Optional<RangeState>>;
                    MOCK_METHOD0(getConfiguration, RangeControllerConfiguration());
                    //MOCK_METHOD2(setRangeValue, RangeValueResult(const double value, const AlexaStateChangeCauseType cause));
                    //MOCK_METHOD2(adjustRangeValue, RangeValueResult(const double rangeValueDelta, const AlexaStateChangeCauseType cause));
                    MOCK_METHOD0(getRangeState, GetRangeValueResult());
                    MOCK_METHOD1(addObserver, bool(shared_ptr<RangeControllerObserverInterface>));
                    MOCK_METHOD1(removeObserver, void(const shared_ptr<RangeControllerObserverInterface>&));
                };
                class RangeControllerCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                    RangeControllerCapabilityAgentTest() {}
                    Optional<RangeControllerAttributes> buildRangeControllerAttribute(const CapabilityResources& capabilityResources) {
                        auto rangeControllerAttributeBuilder = RangeControllerAttributeBuilder::create();
                        PresetResources minimumPresetResource;
                        if (!minimumPresetResource.addFriendlyNameWithAssetId(ASSET_ALEXA_VALUE_MINIMUM) ||
                            !minimumPresetResource.addFriendlyNameWithText("Low", "en-US")) {
                            return Optional<RangeControllerAttributes>();
                        }
                        PresetResources mediumPresetResource;
                        if (!mediumPresetResource.addFriendlyNameWithAssetId(ASSET_ALEXA_VALUE_MEDIUM)) {
                            return Optional<RangeControllerAttributes>();
                        }
                        PresetResources maximumPresetResource;
                        if (!maximumPresetResource.addFriendlyNameWithAssetId(ASSET_ALEXA_VALUE_MAXIMUM) ||
                            !maximumPresetResource.addFriendlyNameWithText("High", "en-US")) {
                            return Optional<RangeControllerAttributes>();
                        }
                        rangeControllerAttributeBuilder->withCapabilityResources(capabilityResources).withUnitOfMeasure(ALEXA_UNIT_ANGLE_DEGREES)
                            .addPreset(make_pair(RANGE_VALUE_MINUMUM, minimumPresetResource)).addPreset(make_pair(RANGE_VALUE_MEDIUM, mediumPresetResource))
                            .addPreset(make_pair(RANGE_VALUE_MAXIMUM, maximumPresetResource));
                        return rangeControllerAttributeBuilder->build();
                    }
                protected:
                    shared_ptr<RangeControllerCapabilityAgent> createCapabilityAgentAndSetExpects(const RangeControllerAttributes& rangeControllerAttributes,
                                                                                                  bool proactivelyReported, bool retrievable,
                                                                                                  bool nonControllable);
                    RangeControllerInterface::RangeState m_testRangeValueMedium;
                    shared_ptr<StrictMock<MockRangeControllerInterface>> m_mockRangeController;
                    shared_ptr<RangeControllerObserverInterface> m_observer;
                    shared_ptr<StrictMock<MockContextManager>> m_mockContextManager;
                    shared_ptr<MockAlexaInterfaceMessageSender> m_mockResponseSender;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                };
                void RangeControllerCapabilityAgentTest::SetUp() {
                    m_mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    //m_mockRangeController = make_shared<StrictMock<MockRangeControllerInterface>>();
                    m_mockContextManager = make_shared<StrictMock<MockContextManager>>();
                    m_mockResponseSender = make_shared<MockAlexaInterfaceMessageSender>();
                    auto timePoint = TimePoint();
                    timePoint.setTime_ISO_8601(TIME_OF_SAMPLE_TEST);
                    m_testRangeValueMedium = RangeControllerInterface::RangeState{RANGE_VALUE_MEDIUM, timePoint, milliseconds(0)};
                }
                shared_ptr<RangeControllerCapabilityAgent> RangeControllerCapabilityAgentTest::createCapabilityAgentAndSetExpects(const RangeControllerAttributes& rangeControllerAttributes,
                                                                                                                                  bool proactivelyReported, bool retrievable, bool nonControllable) {
                    EXPECT_CALL(*m_mockRangeController, getConfiguration())
                        .WillOnce(InvokeWithoutArgs([]() -> RangeControllerInterface::RangeControllerConfiguration {
                            return {RANGE_VALUE_MINUMUM, RANGE_VALUE_MAXIMUM, RANGE_PRECISION};
                        }));
                    if (retrievable) {
                        //EXPECT_CALL(*m_mockContextManager, addStateProvider(_, ::testing::NotNull()));
                        EXPECT_CALL(*m_mockContextManager, removeStateProvider(_));
                    }
                    if (proactivelyReported) {
                        EXPECT_CALL(*m_mockRangeController, addObserver(_))
                            .WillOnce(WithArg<0>(Invoke([this](shared_ptr<RangeControllerObserverInterface> observer) {
                                m_observer = observer;
                                return true;
                            })));
                        EXPECT_CALL(*m_mockRangeController, removeObserver(_))
                            .WillOnce(WithArg<0>(Invoke([this](shared_ptr<RangeControllerObserverInterface> observer) { m_observer = nullptr; })));
                    }
                    auto rangeControllerCapabilityAgent = RangeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, rangeControllerAttributes,
                                                                                    m_mockRangeController, m_mockContextManager, m_mockResponseSender,
                                                                                   m_mockExceptionSender, proactivelyReported, retrievable,
                                                                                                 nonControllable);
                    return rangeControllerCapabilityAgent;
                }
                static shared_ptr<AVSDirective> buildAVSDirective(string directiveName, string payload) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE, directiveName, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST,
                                                                          CORRELATION_TOKEN_TEST, EVENT_CORRELATION_TOKEN_TEST, INTERFACE_VERSION,
                                                                          TEST_INSTANCE);
                    auto avsMessageEndpoint = AVSMessageEndpoint(TEST_ENDPOINT_ID);
                    return AVSDirective::create("", avsMessageHeader, payload, attachmentManager, "", avsMessageEndpoint);
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_givenInvalidParameters_create_shouldFail) {
                    CapabilityResources emptyResource;
                    auto emptyRangeControllerAttribute = buildRangeControllerAttribute(emptyResource);
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    EXPECT_THAT(RangeControllerCapabilityAgent::create("", "", emptyRangeControllerAttribute.value(), nullptr, nullptr, nullptr, nullptr, true, true),
                        IsNull());
                    EXPECT_THAT(RangeControllerCapabilityAgent::create("", TEST_INSTANCE, rangeControllerAttributes.value(), m_mockRangeController,
                                m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(RangeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, "", rangeControllerAttributes.value(), m_mockRangeController,
                                m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(RangeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, rangeControllerAttributes.value(),
                                nullptr, m_mockContextManager, m_mockResponseSender, m_mockExceptionSender, true, true), IsNull());
                    EXPECT_THAT(RangeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, rangeControllerAttributes.value(),
                                m_mockRangeController, nullptr, m_mockResponseSender, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(RangeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, rangeControllerAttributes.value(),
                                m_mockRangeController, m_mockContextManager, nullptr, m_mockExceptionSender, true, true),IsNull());
                    EXPECT_THAT(RangeControllerCapabilityAgent::create(TEST_ENDPOINT_ID, TEST_INSTANCE, rangeControllerAttributes.value(),
                                m_mockRangeController, m_mockContextManager, m_mockResponseSender, nullptr, true, true),IsNull());
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_setRangeValueDirective_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    //EXPECT_CALL(*m_mockRangeController, setRangeValue(RANGE_VALUE_MEDIUM, _))
                    //    .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) { return std::make_pair(AlexaResponseType::SUCCESS, ""); })));
                    EXPECT_CALL(*m_mockResponseSender, sendResponseEvent(_, _, _, _));
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), false, true, false);
                    ASSERT_THAT(rangeControllerCapabilityAgent, NotNull());
                    rangeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_SETRANGEVALUE, SET_RANGE_VALUE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    rangeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_setRangeValueDirective_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockRangeController, setRangeValue(RANGE_VALUE_MEDIUM, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) {
                            return std::make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE, "TestEndpointNotReachable");
                        })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendErrorResponseEvent(_, _, _, _, _));
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(rangeControllerCapabilityAgent, NotNull());
                    rangeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_SETRANGEVALUE, SET_RANGE_VALUE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    rangeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_adjustRangeValueDirective_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    //EXPECT_CALL(*m_mockRangeController, adjustRangeValue(RANGE_PRECISION, _))
                    //    .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) { return std::make_pair(AlexaResponseType::SUCCESS, ""); })));
                    EXPECT_CALL(*m_mockResponseSender, sendResponseEvent(_, _, _, _));
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), false, true, false);
                    ASSERT_THAT(rangeControllerCapabilityAgent, NotNull());
                    rangeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_ADJUSTRANGEVALUE, ADJUST_RANGE_VALUE_PAYLOAD_TEST),
                                                                                  move(m_mockDirectiveHandlerResult));
                    rangeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_adjustRangeValueDirective_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    /*EXPECT_CALL(*m_mockRangeController, adjustRangeValue(RANGE_PRECISION, _))
                        .WillOnce(WithArg<1>(Invoke([](AlexaStateChangeCauseType cause) {
                            return std::make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE, "TestEndpointNotReachable");
                        })));*/
                    EXPECT_CALL(*m_mockResponseSender, sendErrorResponseEvent(_, _, _, _, _));
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(rangeControllerCapabilityAgent, NotNull());
                    rangeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(NAME_ADJUSTRANGEVALUE, ADJUST_RANGE_VALUE_PAYLOAD_TEST),
                                                                                  move(m_mockDirectiveHandlerResult));
                    rangeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_reportStateChange_successCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(rangeControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockRangeController, setRangeValue(RANGE_VALUE_MEDIUM, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onRangeChanged(m_testRangeValueMedium, cause);
                            return std::make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([rangeControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            rangeControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, RANGEVALUE_PROPERTY_NAME, TEST_ENDPOINT_ID, TEST_INSTANCE), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockRangeController, getRangeState())
                        .WillOnce(InvokeWithoutArgs([this]() -> MockRangeControllerInterface::GetRangeValueResult {
                            return make_pair(AlexaResponseType::SUCCESS,Optional<RangeControllerInterface::RangeState>(m_testRangeValueMedium));
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateResponse(_, _, _)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    m_mockRangeController->setRangeValue(RANGE_VALUE_MEDIUM, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_reportStateChange_errorCase) {
                    WaitEvent waitEvent;
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(rangeControllerCapabilityAgent, NotNull());
                    /*EXPECT_CALL(*m_mockRangeController, setRangeValue(RANGE_VALUE_MEDIUM, _))
                        .WillOnce(WithArg<1>(Invoke([this](AlexaStateChangeCauseType cause) {
                            m_observer->onRangeChanged(m_testRangeValueMedium, cause);
                            return std::make_pair(AlexaResponseType::SUCCESS, "");
                        })));*/
                    EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _))
                        .WillOnce(InvokeWithoutArgs([rangeControllerCapabilityAgent] {
                            const unsigned int stateRequestToken = 1;
                            rangeControllerCapabilityAgent->provideState(CapabilityTag(NAMESPACE, RANGEVALUE_PROPERTY_NAME, TEST_ENDPOINT_ID, TEST_INSTANCE), stateRequestToken);
                        }));
                    EXPECT_CALL(*m_mockRangeController, getRangeState())
                        .WillOnce(InvokeWithoutArgs([]() -> MockRangeControllerInterface::GetRangeValueResult {
                            return make_pair(AlexaResponseType::ENDPOINT_UNREACHABLE,Optional<RangeControllerInterface::RangeState>());
                        }));
                    EXPECT_CALL(*m_mockContextManager, provideStateUnavailableResponse(_, _, _))
                        .WillOnce(InvokeWithoutArgs([&waitEvent]() { waitEvent.wakeUp(); }));
                    m_mockRangeController->setRangeValue(RANGE_VALUE_MEDIUM, AlexaStateChangeCauseType::APP_INTERACTION);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableTrue) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), true, true, false);
                    ASSERT_THAT(rangeControllerCapabilityAgent, NotNull());
                    rangeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE, SET_RANGE_VALUE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    rangeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
                TEST_F(RangeControllerCapabilityAgentTest, test_unknownDirectiveWithProactivelyReportedAndRetrievableFalse) {
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).WillOnce(InvokeWithoutArgs([&waitEvent]() {
                        waitEvent.wakeUp();
                    }));
                    CapabilityResources resource;
                    ASSERT_TRUE(resource.addFriendlyNameWithText("range", "en-US"));
                    auto rangeControllerAttributes = buildRangeControllerAttribute(resource);
                    ASSERT_TRUE(rangeControllerAttributes.hasValue());
                    auto rangeControllerCapabilityAgent = createCapabilityAgentAndSetExpects(rangeControllerAttributes.value(), false, false, false);
                    rangeControllerCapabilityAgent->CapabilityAgent::preHandleDirective(buildAVSDirective(UNKNOWN_DIRECTIVE, SET_RANGE_VALUE_PAYLOAD_TEST), std::move(m_mockDirectiveHandlerResult));
                    rangeControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
                    rangeControllerCapabilityAgent->shutdown();
                }
            }
        }
    }
}