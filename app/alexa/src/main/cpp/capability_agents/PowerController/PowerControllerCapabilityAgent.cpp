#include <logger/Logger.h>
#include "PowerControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace powerController {
            using namespace utils::configuration;
            static const string TAG{"PowerControllerCapabilityAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE{"Alexa.PowerController"};
            static const string INTERFACE_VERSION{"3"};
            static const string NAME_TURNON{"TurnOn"};
            static const string NAME_TURNOFF{"TurnOff"};
            static const string POWERSTATE_PROPERTY_NAME{"powerState"};
            static const string POWERSTATE_ON{R"("ON")"};
            static const string POWERSTATE_OFF{R"("OFF")"};
            shared_ptr<PowerControllerCapabilityAgent> PowerControllerCapabilityAgent::create(const EndpointIdentifier& endpointId,
                                                                                              shared_ptr<PowerControllerInterface> powerController,
                                                                                              shared_ptr<ContextManagerInterface> contextManager,
                                                                                              shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                                              shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                                              bool isProactivelyReported, bool isRetrievable) {
                if (endpointId.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "emptyEndpointId"));
                    return nullptr;
                }
                if (!powerController) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullPowerContoller"));
                    return nullptr;
                }
                if (!contextManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                    return nullptr;
                }
                if (!responseSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullResponseSender"));
                    return nullptr;
                }
                if (!exceptionSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
                    return nullptr;
                }
                auto powerContollerCapabilityAgent = shared_ptr<PowerControllerCapabilityAgent>(new PowerControllerCapabilityAgent(endpointId,
                                                                                                powerController, contextManager, responseSender,
                                                                                                exceptionSender, isProactivelyReported,
                                                                                                isRetrievable));
                if (!powerContollerCapabilityAgent) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "instantiationFailed"));
                    return nullptr;
                }
                if (!powerContollerCapabilityAgent->initialize()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "initializationFailed"));
                    return nullptr;
                }
                return powerContollerCapabilityAgent;
            }
            PowerControllerCapabilityAgent::PowerControllerCapabilityAgent(const EndpointIdentifier& endpointId, shared_ptr<PowerControllerInterface> powerController,
                                                                           shared_ptr<ContextManagerInterface> contextManager,
                                                                           shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                           shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                           bool isProactivelyReported, bool isRetrievable) :
                                                                           CapabilityAgent{NAMESPACE, exceptionSender},
                                                                           RequiresShutdown{"PowerControllerCapabilityAgent"},
                                                                           m_endpointId{endpointId}, m_isProactivelyReported{isProactivelyReported},
                                                                           m_isRetrievable{isRetrievable}, m_powerController{powerController},
                                                                           m_contextManager{contextManager}, m_responseSender{responseSender} {}
            bool PowerControllerCapabilityAgent::initialize() {
                ACSDK_DEBUG5(LX(__func__));
                if (m_isRetrievable) {
                    m_contextManager->addStateProvider({NAMESPACE, POWERSTATE_PROPERTY_NAME, m_endpointId}, shared_from_this());
                }
                if (m_isProactivelyReported) {
                    if (!m_powerController->addObserver(shared_from_this())) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "addObserverFailed"));
                        return false;
                    }
                }
                return true;
            }
            void PowerControllerCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void PowerControllerCapabilityAgent::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("preHandleDirective"));
            }
            void PowerControllerCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                m_executor.submit([this, info] {
                    ACSDK_DEBUG5(LX("handleDirectiveInExecutor"));
                    const string directiveName = info->directive->getName();
                    if (!info->directive->getEndpoint().hasValue() ||
                        (info->directive->getEndpoint().value().endpointId != m_endpointId)) {
                        executeUnknownDirective(info, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    pair<AlexaResponseType, std::string> result;
                    if (directiveName == NAME_TURNON) {
                        result = m_powerController->setPowerState(true, AlexaStateChangeCauseType::VOICE_INTERACTION);
                    } else if (directiveName == NAME_TURNOFF) {
                        result = m_powerController->setPowerState(false, AlexaStateChangeCauseType::VOICE_INTERACTION);
                    } else {
                        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "unexpectedDirective").d("name", directiveName));
                        executeUnknownDirective(info, ExceptionErrorType::UNSUPPORTED_OPERATION);
                        return;
                    }
                    executeSetHandlingCompleted(info);
                    executeSendResponseEvent(info, result);
                });
            }
            void PowerControllerCapabilityAgent::provideState(
                const CapabilityTag& stateProviderName,
                const ContextRequestToken contextRequestToken) {
                ACSDK_DEBUG5(LX(__func__).d("contextRequestToken", contextRequestToken).sensitive("stateProviderName", stateProviderName));
                m_executor.submit([this, stateProviderName, contextRequestToken] {
                    ACSDK_DEBUG5(LX("provideStateInExecutor"));
                    executeProvideState(stateProviderName, contextRequestToken);
                });
                return;
            }
            bool PowerControllerCapabilityAgent::canStateBeRetrieved() {
                ACSDK_DEBUG5(LX(__func__));
                return m_isRetrievable;
            }
            bool PowerControllerCapabilityAgent::hasReportableStateProperties() {
                ACSDK_DEBUG5(LX(__func__));
                return m_isRetrievable || m_isProactivelyReported;
            }
            void PowerControllerCapabilityAgent::cancelDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                if (!info->directive->getEndpoint().hasValue() ||
                    info->directive->getEndpoint().value().endpointId != m_endpointId) {
                    ACSDK_WARN(LX("cancelDirective").d("reason", "notExpectedEndpointId"));
                }
                removeDirective(info);
            }
            DirectiveHandlerConfiguration PowerControllerCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[{NAMESPACE, NAME_TURNON, m_endpointId}] = neitherNonBlockingPolicy;
                configuration[{NAMESPACE, NAME_TURNOFF, m_endpointId}] = neitherNonBlockingPolicy;
                return configuration;
            }
            CapabilityConfiguration PowerControllerCapabilityAgent::getCapabilityConfiguration() {
                CapabilityConfiguration configuration{CapabilityConfiguration::ALEXA_INTERFACE_TYPE, NAMESPACE, INTERFACE_VERSION,
                                                      Optional<std::string>(),Optional<CapabilityConfiguration::Properties>({m_isRetrievable,
                                                      m_isProactivelyReported, {POWERSTATE_PROPERTY_NAME}})};
                return configuration;
            }
            void PowerControllerCapabilityAgent::onPowerStateChanged(const PowerState& powerState, AlexaStateChangeCauseType cause) {
                ACSDK_DEBUG5(LX(__func__));
                if (!m_isProactivelyReported) {
                    ACSDK_ERROR(LX("onPowerStateChangedFailed").d("reason", "invalidOnPowerStateChangedCall"));
                    return;
                }
                m_executor.submit([this, powerState, cause] {
                    ACSDK_DEBUG5(LX("onPowerStateChangedInExecutor"));
                    m_contextManager->reportStateChange(CapabilityTag(NAMESPACE, POWERSTATE_PROPERTY_NAME, m_endpointId),
                                            buildCapabilityState(powerState), cause);
                });
            }

            void PowerControllerCapabilityAgent::doShutdown() {
                if (m_isProactivelyReported) m_powerController->removeObserver(shared_from_this());
                m_executor.shutdown();
                m_powerController.reset();
                m_responseSender.reset();
                if (m_isRetrievable) m_contextManager->removeStateProvider({NAMESPACE, POWERSTATE_PROPERTY_NAME, m_endpointId});
                m_contextManager.reset();
            }
            void PowerControllerCapabilityAgent::removeDirective(shared_ptr<DirectiveInfo> info) {
                if (info->directive && info->result) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void PowerControllerCapabilityAgent::executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                if (info && info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void PowerControllerCapabilityAgent::executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type) {
                ACSDK_ERROR(LX("executeUnknownDirectiveFailed").d("reason", "unknownDirective").d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));
                const string exceptionMessage = "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
                sendExceptionEncounteredAndReportFailed(info, exceptionMessage, type);
            }
            void PowerControllerCapabilityAgent::executeProvideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken) {
                ACSDK_DEBUG3(LX("executeProvideState"));
                bool isError = false;
                if (stateProviderName.endpointId != m_endpointId) {
                    ACSDK_ERROR(LX("provideStateFailed").d("reason", "notExpectedEndpointId").sensitive("endpointId", stateProviderName.endpointId));
                    isError = true;
                }
                if (stateProviderName.name != POWERSTATE_PROPERTY_NAME) {
                    ACSDK_ERROR(LX("provideStateFailed").d("reason", "notExpectedName").d("name", stateProviderName.name));
                    isError = true;
                }
                if (!m_isRetrievable) {
                    ACSDK_ERROR(LX("provideStateFailed").d("reason", "provideStateOnNotRetrievableProperty"));
                    isError = true;
                }
                if (isError) {
                    m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, false);
                    return;
                }
                auto result = m_powerController->getPowerState();
                if (AlexaResponseType::SUCCESS != result.first) {
                    ACSDK_WARN(LX("executeProvideState").m("failedToGetPropertyValue").sensitive("reason", result.first));
                    m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                } else {
                    if (!result.second.hasValue()) {
                        ACSDK_ERROR(LX("executeProvideStateFailed").m("emptyPowerState"));
                        m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                        return;
                    }
                    m_contextManager->provideStateResponse(stateProviderName, buildCapabilityState(result.second.value()), contextRequestToken);
                }
            }
            void PowerControllerCapabilityAgent::executeSendResponseEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info,
                                                                          const pair<AlexaResponseType, string> result) {
                if (AlexaResponseType::SUCCESS == result.first) {
                    m_responseSender->sendResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(), AVSMessageEndpoint(m_endpointId));
                } else {
                    m_responseSender->sendErrorResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(),
                                                     AVSMessageEndpoint(m_endpointId), m_responseSender->alexaResponseTypeToErrorType(result.first),
                                                             result.second);
                }
            }
            CapabilityState PowerControllerCapabilityAgent::buildCapabilityState(const PowerState& powerState) {
                return CapabilityState(powerState.powerState ? POWERSTATE_ON : POWERSTATE_OFF, powerState.timeOfSample, powerState.valueUncertainty.count());
            }
        }
    }
}