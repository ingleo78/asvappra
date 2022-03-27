#include <logger/Logger.h>
#include "ToggleControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace toggleController {
            using namespace configuration;
            using ASCCT = AlexaStateChangeCauseType;
            static const string TAG{"ToggleControllerCapabilityAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE{"Alexa.ToggleController"};
            static const string INTERFACE_VERSION{"3"};
            static const string NAME_TURNON{"TurnOn"};
            static const string NAME_TURNOFF{"TurnOff"};
            static const string TOGGLESTATE_PROPERTY_NAME{"toggleState"};
            static const string TOGGLESTATE_ON{R"("ON")"};
            static const string TOGGLESTATE_OFF{R"("OFF")"};
            static const string CAPABILITY_RESOURCES_KEY{"capabilityResources"};
            static bool isToggleControllerAttributeValid(const ToggleControllerAttributes& toggleControllerAttributes) {
                if (!toggleControllerAttributes.isValid()) {
                    ACSDK_ERROR(LX("isToggleControllerAttributeValidFailed").d("reason", "friendlyNamesInvalid"));
                    return false;
                }
                return true;
            }
            shared_ptr<ToggleControllerCapabilityAgent> ToggleControllerCapabilityAgent::create(const EndpointIdentifier& endpointId, const string& instance,
                                                                                                const ToggleControllerAttributes& toggleControllerAttributes,
                                                                                                shared_ptr<ToggleControllerInterface> toggleController,
                                                                                                shared_ptr<ContextManagerInterface> contextManager,
                                                                                                shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                                                shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                                                bool isProactivelyReported, bool isRetrievable,
                                                                                                bool isNonControllable) {
                if (endpointId.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "emptyEndpointId"));
                    return nullptr;
                }
                if (instance.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "emptyInstance"));
                    return nullptr;
                }
                if (!toggleController) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullToggleContoller"));
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
                auto toggleContollerCapabilityAgent = shared_ptr<ToggleControllerCapabilityAgent>(new ToggleControllerCapabilityAgent(endpointId,
                                                                                                  instance, toggleControllerAttributes,
                                                                                                  toggleController, contextManager,
                                                                                                  responseSender, exceptionSender,
                                                                                                  isProactivelyReported, isRetrievable,
                                                                                                  isNonControllable));
                if (!toggleContollerCapabilityAgent->initialize()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "initializationFailed"));
                    return nullptr;
                }
                return toggleContollerCapabilityAgent;
            }
            ToggleControllerCapabilityAgent::ToggleControllerCapabilityAgent(const EndpointIdentifier& endpointId, const string& instance,
                                                                             const ToggleControllerAttributes& toggleControllerAttributes,
                                                                             shared_ptr<ToggleControllerInterface> toggleController,
                                                                             shared_ptr<ContextManagerInterface> contextManager,
                                                                             shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                             shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                             bool isProactivelyReported, bool isRetrievable, bool isNonControllable) :
                                                                             CapabilityAgent{NAMESPACE, exceptionSender},
                                                                             RequiresShutdown{"ToggleControllerCapabilityAgent"},
                                                                             m_endpointId{endpointId}, m_instance{instance},
                                                                             m_isProactivelyReported{isProactivelyReported},
                                                                             m_isRetrievable{isRetrievable}, m_isNonControllable{isNonControllable},
                                                                             m_toggleControllerAttributes{toggleControllerAttributes},
                                                                             m_toggleController{toggleController}, m_contextManager{contextManager},
                                                                             m_responseSender{responseSender} {}
            bool ToggleControllerCapabilityAgent::initialize() {
                ACSDK_DEBUG5(LX(__func__));
                if (!isToggleControllerAttributeValid(m_toggleControllerAttributes)) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "invalidToggleControllerAttributes"));
                    return false;
                }
                if (m_isProactivelyReported) {
                    if (!m_toggleController->addObserver(shared_from_this())) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "addObserverFailed"));
                        return false;
                    }
                }
                if (m_isRetrievable) {
                    m_contextManager->addStateProvider({NAMESPACE, TOGGLESTATE_PROPERTY_NAME, m_endpointId, m_instance},
                                                       shared_from_this());
                }
                return true;
            }
            void ToggleControllerCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void ToggleControllerCapabilityAgent::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("preHandleDirective"));
            }
            void ToggleControllerCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                m_executor.submit([this, info] {
                    ACSDK_DEBUG5(LX("handleDirectiveInExecutor"));
                    const string directiveName = info->directive->getName();
                    if (!info->directive->getEndpoint().hasValue() || (info->directive->getEndpoint().value().endpointId != m_endpointId) ||
                        (info->directive->getInstance() != m_instance)) {
                        executeUnknownDirective(info, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    if (m_isNonControllable) {
                        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "propertyIsNonControllable"));
                        sendExceptionEncounteredAndReportFailed(info, "propertyIsNonControllable", ExceptionErrorType::UNSUPPORTED_OPERATION);
                        return;
                    }
                    pair<AlexaResponseType, string> result;
                    if (directiveName == NAME_TURNON) result = m_toggleController->setToggleState(true, ASCCT::VOICE_INTERACTION);
                    else if (directiveName == NAME_TURNOFF) result = m_toggleController->setToggleState(false, ASCCT::VOICE_INTERACTION);
                    else {
                        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "unexpectedDirective").d("name", directiveName));
                        executeUnknownDirective(info, ExceptionErrorType::UNSUPPORTED_OPERATION);
                        return;
                    }
                    executeSetHandlingCompleted(info);
                    executeSendResponseEvent(info, result);
                });
            }
            void ToggleControllerCapabilityAgent::provideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken) {
                ACSDK_DEBUG5(LX(__func__).d("contextRequestToken", contextRequestToken).sensitive("stateProviderName", stateProviderName));
                m_executor.submit([this, stateProviderName, contextRequestToken] {
                    ACSDK_DEBUG5(LX("provideStateInExecutor"));
                    executeProvideState(stateProviderName, contextRequestToken);
                });
                return;
            }
            bool ToggleControllerCapabilityAgent::canStateBeRetrieved() {
                ACSDK_DEBUG5(LX(__func__));
                return m_isRetrievable;
            }
            bool ToggleControllerCapabilityAgent::hasReportableStateProperties() {
                ACSDK_DEBUG5(LX(__func__));
                return (m_isRetrievable || m_isProactivelyReported);
            }
            void ToggleControllerCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                if (!info->directive->getEndpoint().hasValue() ||
                    info->directive->getEndpoint().value().endpointId != m_endpointId) {
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "notExpectedEndpointId"));
                    return;
                }
                if (info->directive->getInstance() != m_instance) {
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "notExpectedInstance"));
                    return;
                }
                removeDirective(info);
            }
            DirectiveHandlerConfiguration ToggleControllerCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[{NAMESPACE, NAME_TURNON, m_endpointId, m_instance}] = neitherNonBlockingPolicy;
                configuration[{NAMESPACE, NAME_TURNOFF, m_endpointId, m_instance}] = neitherNonBlockingPolicy;
                return configuration;
            }
            CapabilityConfiguration ToggleControllerCapabilityAgent::getCapabilityConfiguration() {
                ACSDK_DEBUG5(LX(__func__));
                auto additionalConfigurations = CapabilityConfiguration::AdditionalConfigurations();
                additionalConfigurations[CAPABILITY_RESOURCES_KEY] = m_toggleControllerAttributes.toJson();
                CapabilityConfiguration configuration{CapabilityConfiguration::ALEXA_INTERFACE_TYPE, NAMESPACE, INTERFACE_VERSION,
                                                      Optional<std::string>(m_instance),Optional<CapabilityConfiguration::Properties>({m_isRetrievable,
                                                      m_isProactivelyReported,{TOGGLESTATE_PROPERTY_NAME}, Optional<bool>(m_isNonControllable)}),
                                                      additionalConfigurations};
                return configuration;
            }
            void ToggleControllerCapabilityAgent::onToggleStateChanged(const ToggleState& toggleState, const AlexaStateChangeCauseType cause) {
                ACSDK_DEBUG5(LX(__func__));
                if (!m_isProactivelyReported) {
                    ACSDK_ERROR(LX("onToggleStateChangedFailed").d("reason", "invalidOnToggleStateChangedCall"));
                    return;
                }
                m_executor.submit([this, toggleState, cause] {
                    m_contextManager->reportStateChange(CapabilityTag(NAMESPACE, TOGGLESTATE_PROPERTY_NAME, m_endpointId,
                                                        m_instance),buildCapabilityState(toggleState), cause);
                });
            }
            void ToggleControllerCapabilityAgent::doShutdown() {
                if (m_isProactivelyReported) m_toggleController->removeObserver(shared_from_this());
                m_executor.shutdown();
                m_toggleController.reset();
                m_responseSender.reset();
                if (m_isRetrievable) {
                    m_contextManager->removeStateProvider({NAMESPACE, TOGGLESTATE_PROPERTY_NAME, m_endpointId,
                                                           m_instance});
                }
                m_contextManager.reset();
            }
            void ToggleControllerCapabilityAgent::removeDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (info && info->directive) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void ToggleControllerCapabilityAgent::executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                if (info && info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void ToggleControllerCapabilityAgent::executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type) {
                ACSDK_ERROR(LX("executeUnknownDirectiveFailed").d("reason", "unknownDirective").d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));
                const string exceptionMessage = "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
                sendExceptionEncounteredAndReportFailed(info, exceptionMessage, type);
            }
            void ToggleControllerCapabilityAgent::executeProvideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken) {
                bool isError = false;
                if (stateProviderName.endpointId != m_endpointId) {
                    ACSDK_ERROR(LX("provideStateFailed").d("reason", "notExpectedEndpointId").sensitive("endpointId", stateProviderName.endpointId));
                    isError = true;
                }
                if (stateProviderName.name != TOGGLESTATE_PROPERTY_NAME) {
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
                auto result = m_toggleController->getToggleState();
                if (AlexaResponseType::SUCCESS != result.first) {
                    ACSDK_WARN(LX("executeProvideState").m("failedToGetPropertyValue").sensitive("reason", result.first));
                    m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                } else {
                    if (!result.second.hasValue()) {
                        ACSDK_ERROR(LX("executeProvideStateFailed").m("emptyToggleState"));
                        m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                        return;
                    }
                    m_contextManager->provideStateResponse(stateProviderName, buildCapabilityState(result.second.value()),
                                                           contextRequestToken);
                }
            }
            void ToggleControllerCapabilityAgent::executeSendResponseEvent(const shared_ptr<DirectiveInfo> info, const pair<AlexaResponseType, string> result) {
                if (AlexaResponseType::SUCCESS == result.first) {
                    m_responseSender->sendResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(),
                                                        AVSMessageEndpoint(m_endpointId));
                } else {
                    m_responseSender->sendErrorResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(),
                                                             AVSMessageEndpoint(m_endpointId),m_responseSender->alexaResponseTypeToErrorType(result.first),
                                                             result.second);
                }
            }
            CapabilityState ToggleControllerCapabilityAgent::buildCapabilityState(const ToggleState& toggleState) {
                return CapabilityState(toggleState.toggleState ? TOGGLESTATE_ON : TOGGLESTATE_OFF, toggleState.timeOfSample,
                                       toggleState.valueUncertainty.count());
            }
        }
    }
}