#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <logger/Logger.h>
#include <util/WaitEvent.h>
#include "ContextManager.h"

namespace alexaClientSDK {
    namespace contextManager {
        namespace test {
            using namespace testing;
            using namespace rapidjson;
            static const string TAG("ContextManagerTest");
            #define LX(event) LogEntry(TAG, event)
            class MockStateProvider : public StateProviderInterface {
            public:
                //MOCK_METHOD2(provideState, void(const CapabilityTag& stateProviderName, const ContextRequestToken stateRequestToken));
                MOCK_METHOD0(hasReportableStateProperties, bool());
            };
            struct MockLegacyStateProvider : public StateProviderInterface {
                //MOCK_METHOD2(provideState, void(const NamespaceAndName& stateProviderName, const ContextRequestToken stateRequestToken));
            };
            struct MockContextRequester : public ContextRequesterInterface {
                MOCK_METHOD3(onContextAvailable, void(const string& endpointId, const AVSContext& endpointContext, ContextRequestToken requestToken));
                //MOCK_METHOD2(onContextFailure, void(const ContextRequestError error, ContextRequestToken token));
            };
            struct MockContextObserver : public ContextManagerObserverInterface {
                MOCK_METHOD3(onStateChanged, void(const CapabilityTag& identifier, const CapabilityState& state, AlexaStateChangeCauseType cause));
            };
            class ContextManagerTest : public Test {
            protected:
                void SetUp() override;
                shared_ptr<ContextManagerInterface> m_contextManager;
            };
            void ContextManagerTest::SetUp() {
                auto deviceInfo = DeviceInfo::create("clientId","productId","1234","manufacturer",
                                                     "my device", "friendlyName", "deviceType");
                ASSERT_NE(deviceInfo, nullptr);
                m_contextManager = ContextManager::createContextManagerInterface(std::move(deviceInfo));
            }
            TEST_F(ContextManagerTest, test_setStateForLegacyRegisteredProvider) {
                auto provider = make_shared<MockLegacyStateProvider>();
                auto capability = NamespaceAndName("Namespace", "Name");
                m_contextManager->setStateProvider(capability, provider);
                string payload{R"({"state":"value"})"};
                ASSERT_EQ(m_contextManager->setState(capability, payload, StateRefreshPolicy::ALWAYS), SetStateResult::SUCCESS);
            }
            TEST_F(ContextManagerTest, test_setStateForUnregisteredLegacyProvider) {
                auto provider = make_shared<MockLegacyStateProvider>();
                auto capability = NamespaceAndName("Namespace", "Name");
                string payload{R"({"state":"value"})"};
                ASSERT_EQ(m_contextManager->setState(capability, payload, StateRefreshPolicy::ALWAYS), SetStateResult::SUCCESS);
            }
            TEST_F(ContextManagerTest, test_getContextLegacyProvider) {
                auto provider = make_shared<MockLegacyStateProvider>();
                auto capability = NamespaceAndName("Namespace", "Name");
                string payload{R"({"state":"value"})"};
                m_contextManager->setStateProvider(capability, provider);
                promise<ContextRequestToken> tokenPromise;
                /*EXPECT_CALL(*provider, provideState(_, _))
                    .WillOnce(WithArg<1>(Invoke([&tokenPromise](const ContextRequestToken token) { tokenPromise.set_value(token); })));*/
                auto requester = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                auto requestToken = m_contextManager->getContext(requester);
                const milliseconds timeout{100};
                auto expectedTokenFuture = tokenPromise.get_future();
                ASSERT_EQ(expectedTokenFuture.wait_for(timeout), future_status::ready);
                EXPECT_EQ(requestToken, expectedTokenFuture.get());
                ASSERT_EQ(m_contextManager->setState(capability, payload, StateRefreshPolicy::ALWAYS, requestToken),SetStateResult::SUCCESS);
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), future_status::ready);
                EXPECT_EQ(statesFuture.get()[capability].valuePayload, payload);
            }
            TEST_F(ContextManagerTest, test_setLegacyStateProviderSetStateTwiceShouldFail) {
                auto provider = make_shared<MockLegacyStateProvider>();
                auto capability = NamespaceAndName("Namespace", "Name");
               string payload{R"({"state":"value"})"};
                m_contextManager->setStateProvider(capability, provider);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*provider, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                auto requester = make_shared<MockContextRequester>();
                WaitEvent stateAvailableEvent;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _)).WillOnce((InvokeWithoutArgs([&stateAvailableEvent] {
                    stateAvailableEvent.wakeUp();
                })));
                auto requestToken = m_contextManager->getContext(requester);
                const milliseconds timeout{100};
                ASSERT_TRUE(provideStateEvent.wait(timeout));
                ASSERT_EQ(m_contextManager->setState(capability, payload, StateRefreshPolicy::ALWAYS, requestToken),SetStateResult::SUCCESS);
                ASSERT_TRUE(stateAvailableEvent.wait(timeout));
                EXPECT_EQ(m_contextManager->setState(capability, payload, StateRefreshPolicy::ALWAYS, requestToken),SetStateResult::STATE_TOKEN_OUTDATED);
            }
            TEST_F(ContextManagerTest, test_provideStateTimeout) {
                auto provider = make_shared<MockLegacyStateProvider>();
                auto capability = NamespaceAndName("Namespace", "Name");
                string payload{R"({"state":"value"})"};
                m_contextManager->setStateProvider(capability, provider);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*provider, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                const milliseconds timeout{100};
                auto requester = make_shared<MockContextRequester>();
                auto token = m_contextManager->getContext(requester, capability.endpointId, timeout);
                WaitEvent stateFailureEvent;
                /*EXPECT_CALL(*requester, onContextFailure(ContextRequestError::STATE_PROVIDER_TIMEDOUT, token))
                    .WillOnce((InvokeWithoutArgs([&stateFailureEvent] { stateFailureEvent.wakeUp(); })));*/
                ASSERT_TRUE(provideStateEvent.wait(timeout));
                ASSERT_TRUE(stateFailureEvent.wait(timeout + timeout));
            }
            TEST_F(ContextManagerTest, test_incorrectToken) {
                auto provider = make_shared<MockLegacyStateProvider>();
                auto capability = NamespaceAndName("Namespace", "Name");
                string payload{R"({"state":"value"})"};
                m_contextManager->setStateProvider(capability, provider);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*provider, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                auto requester = make_shared<MockContextRequester>();
                auto requestToken = m_contextManager->getContext(requester);
                const milliseconds timeout{100};
                ASSERT_TRUE(provideStateEvent.wait(timeout));
                EXPECT_EQ(m_contextManager->setState(capability, payload, StateRefreshPolicy::ALWAYS, requestToken + 1),SetStateResult::STATE_TOKEN_OUTDATED);
            }
            TEST_F(ContextManagerTest, test_sometimesProviderWithValidState) {
                auto sometimesProvider = make_shared<MockLegacyStateProvider>();
                auto sometimesCapability = NamespaceAndName("Namespace", "Name");
                string sometimesPayload{R"({"state":"value"})"};
                m_contextManager->setStateProvider(sometimesCapability, sometimesProvider);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*sometimesProvider, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                auto requester = std::make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                auto requestToken = m_contextManager->getContext(requester);
                const milliseconds timeout{100};
                provideStateEvent.wait(timeout);
                ASSERT_EQ(m_contextManager->setState(sometimesCapability, sometimesPayload, StateRefreshPolicy::SOMETIMES, requestToken),
                     SetStateResult::SUCCESS);
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), future_status::ready);
                EXPECT_EQ(statesFuture.get()[sometimesCapability].valuePayload, sometimesPayload);
            }
            TEST_F(ContextManagerTest, test_sometimesProviderWithEmptyState) {
                auto sometimesProvider = make_shared<MockLegacyStateProvider>();
                auto sometimesCapability = NamespaceAndName("Namespace", "Name");
                m_contextManager->setStateProvider(sometimesCapability, sometimesProvider);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*sometimesProvider, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                auto requester = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                auto requestToken = m_contextManager->getContext(requester);
                const milliseconds timeout{100};
                provideStateEvent.wait(timeout);
                ASSERT_EQ(m_contextManager->setState(sometimesCapability, "", StateRefreshPolicy::SOMETIMES, requestToken),SetStateResult::SUCCESS);
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), std::future_status::ready);
                auto states = statesFuture.get();
                EXPECT_EQ(states.find(sometimesCapability), states.end());
            }
            TEST_F(ContextManagerTest, test_neverProvider) {
                auto neverProvider = make_shared<StrictMock<MockLegacyStateProvider>>();
                auto neverCapability = NamespaceAndName("Namespace", "Name");
                string neverPayload{R"({"state":"value"})"};
                m_contextManager->setStateProvider(neverCapability, neverProvider);
                ASSERT_EQ(m_contextManager->setState(neverCapability, neverPayload, StateRefreshPolicy::NEVER), SetStateResult::SUCCESS);
                auto requester = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                m_contextManager->getContext(requester);
                const milliseconds timeout{100};
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), future_status::ready);
                EXPECT_EQ(statesFuture.get()[neverCapability].valuePayload, neverPayload);
            }
            TEST_F(ContextManagerTest, test_getEndpointContextShouldIncludeOnlyRelevantStates) {
                auto providerForTargetEndpoint = make_shared<MockStateProvider>();
                auto capabilityForTarget = CapabilityTag("TargetNamespace", "TargetName", "TargetEndpointId");
                CapabilityState stateForTarget{R"({"state":"target"})"};
                EXPECT_CALL(*providerForTargetEndpoint, hasReportableStateProperties()).WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capabilityForTarget, providerForTargetEndpoint);
                auto providerForOtherEndpoint = make_shared<StrictMock<MockStateProvider>>();
                auto capabilityForOther = CapabilityTag("OtherNamespace", "OtherName", "OtherEndpointId");
                EXPECT_CALL(*providerForOtherEndpoint, hasReportableStateProperties()).WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capabilityForOther, providerForOtherEndpoint);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*providerForTargetEndpoint, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                auto requester = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                auto requestToken = m_contextManager->getContext(requester, capabilityForTarget.endpointId);
                const milliseconds timeout{100};
                provideStateEvent.wait(timeout);
                m_contextManager->provideStateResponse(capabilityForTarget, stateForTarget, requestToken);
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), future_status::ready);
                auto states = statesFuture.get();
                EXPECT_EQ(states[capabilityForTarget].valuePayload, stateForTarget.valuePayload);
                EXPECT_EQ(states.find(capabilityForOther), states.end());
            }
            TEST_F(ContextManagerTest, test_getContextWhenStateAndCacheAreUnavailableShouldFail) {
                auto provider = make_shared<MockStateProvider>();
                auto capability = CapabilityTag("Namespace", "Name", "EndpointId");
                EXPECT_CALL(*provider, hasReportableStateProperties()).WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capability, provider);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*provider, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                auto requester = make_shared<MockContextRequester>();
                auto requestToken = m_contextManager->getContext(requester, capability.endpointId);
                WaitEvent contextFailureEvent;
                /*EXPECT_CALL(*requester, onContextFailure(ContextRequestError::BUILD_CONTEXT_ERROR, requestToken))
                    .WillOnce(InvokeWithoutArgs([&contextFailureEvent] { contextFailureEvent.wakeUp(); }));*/
                const milliseconds timeout{100};
                ASSERT_TRUE(provideStateEvent.wait(timeout));
                m_contextManager->provideStateUnavailableResponse(capability, requestToken, false);
                EXPECT_TRUE(contextFailureEvent.wait(timeout));
            }
            TEST_F(ContextManagerTest, test_getContextWhenStateUnavailableShouldReturnCache) {
                auto provider = make_shared<MockStateProvider>();
                auto capability = CapabilityTag("Namespace", "Name", "EndpointId");
                CapabilityState state{R"({"state":"target"})"};
                EXPECT_CALL(*provider, hasReportableStateProperties()).WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capability, provider);
                m_contextManager->reportStateChange(capability, state, AlexaStateChangeCauseType::APP_INTERACTION);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*provider, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent] {
                    provideStateEvent.wakeUp();
                })));*/
                auto requester = make_shared<MockContextRequester>();
                auto requestToken = m_contextManager->getContext(requester, capability.endpointId);
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                const milliseconds timeout{100};
                ASSERT_TRUE(provideStateEvent.wait(timeout));
                m_contextManager->provideStateUnavailableResponse(capability, requestToken, false);
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), std::future_status::ready);
                EXPECT_EQ(statesFuture.get()[capability].valuePayload, state.valuePayload);
            }
            TEST_F(ContextManagerTest, test_reportStateChangeShouldNotifyObserver) {
                auto provider = make_shared<MockStateProvider>();
                auto capability = CapabilityTag("Namespace", "Name", "EndpointId");
                CapabilityState state{R"({"state":"target"})"};
                m_contextManager->setStateProvider(capability, provider);
                auto observer = make_shared<MockContextObserver>();
                m_contextManager->addContextManagerObserver(observer);
                WaitEvent notificationEvent;
                auto cause = AlexaStateChangeCauseType::APP_INTERACTION;
                EXPECT_CALL(*observer, onStateChanged(capability, state, cause)).WillOnce(InvokeWithoutArgs([&notificationEvent] {
                    notificationEvent.wakeUp();
                }));
                m_contextManager->reportStateChange(capability, state, cause);
                const milliseconds timeout{100};
                EXPECT_TRUE(notificationEvent.wait(timeout));
            }
            TEST_F(ContextManagerTest, test_getContextInParallelShouldSucceed) {
                auto providerForEndpoint1 = make_shared<MockStateProvider>();
                auto capabilityForEndpoint1 = CapabilityTag("Namespace", "Name", "EndpointId1");
                CapabilityState stateForEndpoint1{R"({"state":1})"};
                EXPECT_CALL(*providerForEndpoint1, hasReportableStateProperties()).WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capabilityForEndpoint1, providerForEndpoint1);
                auto providerForEndpoint2 = make_shared<MockStateProvider>();
                auto capabilityForEndpoint2 = CapabilityTag("Namespace", "Name", "EndpointId2");
                CapabilityState stateForEndpoint2{R"({"state":2})"};
                EXPECT_CALL(*providerForEndpoint2, hasReportableStateProperties()).WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capabilityForEndpoint2, providerForEndpoint2);
                WaitEvent provideStateEvent1;
                /*EXPECT_CALL(*providerForEndpoint1, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent1] {
                    provideStateEvent1.wakeUp();
                })));*/
                WaitEvent provideStateEvent2;
                /*EXPECT_CALL(*providerForEndpoint2, provideState(_, _)).WillOnce((InvokeWithoutArgs([&provideStateEvent2] {
                    provideStateEvent2.wakeUp();
                })));*/
                auto requester1 = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise1;
                EXPECT_CALL(*requester1, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise1](const AVSContext& context) {
                        contextStatesPromise1.set_value(context.getStates());
                    })));
                auto requester2 = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise2;
                EXPECT_CALL(*requester2, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise2](const AVSContext& context) {
                        contextStatesPromise2.set_value(context.getStates());
                    })));
                auto requestToken1 = m_contextManager->getContext(requester1, capabilityForEndpoint1.endpointId);
                auto requestToken2 = m_contextManager->getContext(requester2, capabilityForEndpoint2.endpointId);
                const milliseconds timeout{100};
                ASSERT_TRUE(provideStateEvent1.wait(timeout));
                ASSERT_TRUE(provideStateEvent2.wait(timeout));
                m_contextManager->provideStateResponse(capabilityForEndpoint1, stateForEndpoint1, requestToken1);
                m_contextManager->provideStateResponse(capabilityForEndpoint2, stateForEndpoint2, requestToken2);
                auto statesFuture1 = contextStatesPromise1.get_future();
                ASSERT_EQ(statesFuture1.wait_for(timeout), std::future_status::ready);
                auto statesForEndpoint1 = statesFuture1.get();
                EXPECT_EQ(statesForEndpoint1[capabilityForEndpoint1].valuePayload, stateForEndpoint1.valuePayload);
                EXPECT_EQ(statesForEndpoint1.find(capabilityForEndpoint2), statesForEndpoint1.end());
                auto statesFuture2 = contextStatesPromise2.get_future();
                ASSERT_EQ(statesFuture2.wait_for(timeout), std::future_status::ready);
                auto statesForEndpoint2 = statesFuture2.get();
                EXPECT_EQ(statesForEndpoint2[capabilityForEndpoint2].valuePayload, stateForEndpoint2.valuePayload);
                EXPECT_EQ(statesForEndpoint2.find(capabilityForEndpoint1), statesForEndpoint2.end());
            }
            TEST_F(ContextManagerTest, test_getContextWithoutReportableStateProperties) {
                auto providerWithReportableStateProperties = make_shared<MockStateProvider>();
                auto capability1 = CapabilityTag("Namespace", "Name1", "");
                CapabilityState state1{R"({"state1":"target1"})"};
                EXPECT_CALL(*providerWithReportableStateProperties, hasReportableStateProperties()).WillRepeatedly(Return(true));
                m_contextManager->setStateProvider(capability1, providerWithReportableStateProperties);
                auto providerWithoutReportableStateProperties = make_shared<MockStateProvider>();
                auto capability2 = CapabilityTag("Namespace", "Name2", "");
                CapabilityState state2{R"({"state2":"target2"})"};
                EXPECT_CALL(*providerWithoutReportableStateProperties, hasReportableStateProperties())
                    .WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capability2, providerWithoutReportableStateProperties);
                //EXPECT_CALL(*providerWithReportableStateProperties, provideState(_, _)).Times(0);
                WaitEvent provideStateEvent;
                /*EXPECT_CALL(*providerWithoutReportableStateProperties, provideState(_, _))
                    .WillOnce((InvokeWithoutArgs([&provideStateEvent] { provideStateEvent.wakeUp(); })));*/
                auto requester = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                auto requestToken = m_contextManager->getContextWithoutReportableStateProperties(requester);
                const milliseconds timeout{100};
                provideStateEvent.wait(timeout);
                m_contextManager->provideStateResponse(capability2, state2, requestToken);
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), future_status::ready);
                auto states = statesFuture.get();
                EXPECT_EQ(states[capability2].valuePayload, state2.valuePayload);
                EXPECT_EQ(states.find(capability1), states.end());
            }
            TEST_F(ContextManagerTest, test_getContextWithReportableStateProperties) {
                auto providerWithReportableStateProperties = make_shared<MockStateProvider>();
                auto capability1 = CapabilityTag("Namespace", "Name1", "");
                CapabilityState state1{R"({"state1":"target1"})"};
                EXPECT_CALL(*providerWithReportableStateProperties, hasReportableStateProperties()).WillRepeatedly(Return(true));
                m_contextManager->setStateProvider(capability1, providerWithReportableStateProperties);
                auto providerWithoutReportableStateProperties = make_shared<MockStateProvider>();
                auto capability2 = CapabilityTag("Namespace", "Name2", "");
                CapabilityState state2{R"({"state2":"target2"})"};
                EXPECT_CALL(*providerWithoutReportableStateProperties, hasReportableStateProperties())
                    .WillRepeatedly(Return(false));
                m_contextManager->setStateProvider(capability2, providerWithoutReportableStateProperties);
                WaitEvent provideStateEvent1;
                /*EXPECT_CALL(*providerWithReportableStateProperties, provideState(_, _))
                    .WillOnce((InvokeWithoutArgs([&provideStateEvent1] { provideStateEvent1.wakeUp(); })));*/
                WaitEvent provideStateEvent2;
                /*EXPECT_CALL(*providerWithoutReportableStateProperties, provideState(_, _))
                    .WillOnce((InvokeWithoutArgs([&provideStateEvent2] { provideStateEvent2.wakeUp(); })));*/
                auto requester = make_shared<MockContextRequester>();
                promise<AVSContext::States> contextStatesPromise;
                EXPECT_CALL(*requester, onContextAvailable(_, _, _))
                    .WillOnce(WithArg<1>(Invoke([&contextStatesPromise](const AVSContext& context) {
                        contextStatesPromise.set_value(context.getStates());
                    })));
                auto requestToken = m_contextManager->getContext(requester);
                const milliseconds timeout{100};
                provideStateEvent1.wait(timeout);
                provideStateEvent2.wait(timeout);
                m_contextManager->provideStateResponse(capability2, state2, requestToken);
                m_contextManager->provideStateResponse(capability1, state1, requestToken);
                auto statesFuture = contextStatesPromise.get_future();
                ASSERT_EQ(statesFuture.wait_for(timeout), future_status::ready);
                auto states = statesFuture.get();
                EXPECT_EQ(states[capability2].valuePayload, state2.valuePayload);
                EXPECT_EQ(states[capability1].valuePayload, state1.valuePayload);
            }
        }
    }
}