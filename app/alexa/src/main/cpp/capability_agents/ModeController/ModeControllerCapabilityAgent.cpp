#include <algorithm>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/CapabilityResources.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "ModeControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace modeController {
            using namespace configuration;
            using namespace json;
            using namespace logger;
            static const string TAG{"ModeControllerCapabilityAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE{"Alexa.ModeController"};
            static const string INTERFACE_VERSION{"3"};
            static const string NAME_SETMODE{"SetMode"};
            static const string NAME_ADJUSTMODE{"AdjustMode"};
            static const string MODEVALUE_PROPERTY_NAME{"mode"};
            static const string CAPABILITY_RESOURCES_KEY{"capabilityResources"};
            static const string CAPABILITY_CONFIGURATION_KEY{"configuration"};
            static const char MODE_KEY[] = "mode";
            static const char MODE_DELTA_KEY[] = "modeDelta";
            static bool isModeControllerAttributesValid(const ModeControllerAttributes& modeControllerAttributes,
                                                        const ModeControllerInterface::ModeControllerConfiguration& modeControllerConfiguration) {
                if (!modeControllerAttributes.capabilityResources.isValid()) {
                    ACSDK_ERROR(LX("isModeControllerAttributeValidFailed").d("reason", "capabilityResourcesInvalid"));
                    return false;
                }
                for (const auto& mode : modeControllerAttributes.modes) {
                    if (!mode.second.isValid()) {
                        ACSDK_ERROR(LX("isModeControllerAttributeValidFailed").d("reason", "modeResourcesInvalid"));
                        return false;
                    }
                }
                if (modeControllerConfiguration.size() != modeControllerAttributes.modes.size()) {
                    ACSDK_ERROR(LX("isModeControllerAttributeValidFailed").d("reason", "modeControllerConfigurationSizeNotMatching"));
                    return false;
                }
                for (const auto& supportedMode : modeControllerConfiguration) {
                    if (modeControllerAttributes.modes.find(supportedMode) == modeControllerAttributes.modes.end()) {
                        ACSDK_ERROR(LX("isModeControllerAttributeValidFailed").d("reason", "supportedModeNotFoundInAttributes")
                            .sensitive("supportedMode", supportedMode));
                        return false;
                    }
                }
                return true;
            }
            static bool parseDirectivePayload(const string& payload, Document* document) {
                ACSDK_DEBUG5(LX(__func__));
                if (!document) {
                    ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", "nullDocument"));
                    return false;
                }
                ParseResult result = document->Parse(payload.data());
                if (!result) {
                    ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", "parseFailed").d("error", GetParseError_En(result.Code()))
                        .d("offset", result.Offset()));
                    return false;
                }
                return true;
            }
            shared_ptr<ModeControllerCapabilityAgent> ModeControllerCapabilityAgent::create(const EndpointIdentifier& endpointId, const string& instance,
                                                                                            const ModeControllerAttributes& modeControllerAttributes,
                                                                                            shared_ptr<ModeControllerInterface> modeController,
                                                                                            shared_ptr<ContextManagerInterface> contextManager,
                                                                                            shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                                            shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                                            const bool isProactivelyReported,
                                                                                            const bool isRetrievable,
                                                                                            const bool isNonControllable) {
                if (endpointId.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "emptyEndpointId"));
                    return nullptr;
                }
                if (instance.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "emptyInstance"));
                    return nullptr;
                }
                if (!modeController) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullModeContoller"));
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
                auto modeContollerCapabilityAgent = shared_ptr<ModeControllerCapabilityAgent>(new ModeControllerCapabilityAgent(endpointId, instance,
                                                                                              modeControllerAttributes, modeController, contextManager,
                                                                                              responseSender, exceptionSender, isProactivelyReported,
                                                                                              isRetrievable, isNonControllable));
                if (!modeContollerCapabilityAgent->initialize()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "initializationFailed"));
                    return nullptr;
                }
                return modeContollerCapabilityAgent;
            }
            ModeControllerCapabilityAgent::ModeControllerCapabilityAgent(const EndpointIdentifier& endpointId, const string& instance,
                                                                         const ModeControllerAttributes& modeControllerAttributes,
                                                                         shared_ptr<ModeControllerInterface> modeController,
                                                                         shared_ptr<ContextManagerInterface> contextManager,
                                                                         shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                         shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                         const bool isProactivelyReported, const bool isRetrievable,
                                                                         const bool isNonControllable) : CapabilityAgent{NAMESPACE, exceptionSender},
                                                                         RequiresShutdown{"ModeControllerCapabilityAgent"}, m_endpointId{endpointId},
                                                                         m_instance{instance}, m_isProactivelyReported{isProactivelyReported},
                                                                         m_isRetrievable{isRetrievable}, m_isNonControllable{isNonControllable},
                                                                         m_modeControllerAttributes{modeControllerAttributes},
                                                                         m_modeController{modeController}, m_contextManager{contextManager},
                                                                         m_responseSender{responseSender} {}
            bool ModeControllerCapabilityAgent::initialize() {
                ACSDK_DEBUG5(LX(__func__));
                m_modeControllerConfiguration = m_modeController->getConfiguration();
                if (!isModeControllerAttributesValid(m_modeControllerAttributes, m_modeControllerConfiguration)) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "invalidModeControllerAttributes"));
                    return false;
                }
                if (m_isProactivelyReported) {
                    if (!m_modeController->addObserver(shared_from_this())) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "addObserverFailed"));
                        return false;
                    }
                }
                if (m_isRetrievable) {
                    m_contextManager->addStateProvider({NAMESPACE, MODEVALUE_PROPERTY_NAME, m_endpointId, m_instance},
                                           shared_from_this());
                }
                return true;
            }
            void ModeControllerCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediately").d("reason", "nullDirectiveInfo"));
                    return;
                }
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void ModeControllerCapabilityAgent::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
            }
            void ModeControllerCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
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
                    Document payload(kObjectType);
                    if (!parseDirectivePayload(info->directive->getPayload(), &payload)) {
                        sendExceptionEncounteredAndReportFailed(info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    if (directiveName == NAME_SETMODE) executeSetModeDirective(info, payload);
                    else if (directiveName == NAME_ADJUSTMODE) executeAdjustModeDirective(info, payload);
                    else {
                        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "unexpectedDirective").d("name", directiveName));
                        executeUnknownDirective(info, ExceptionErrorType::UNSUPPORTED_OPERATION);
                    }
                });
            }
            void ModeControllerCapabilityAgent::provideState(
                const CapabilityTag& stateProviderName,
                const ContextRequestToken contextRequestToken) {
                ACSDK_DEBUG5(LX(__func__).d("contextRequestToken", contextRequestToken).sensitive("stateProviderName", stateProviderName));
                m_executor.submit([this, stateProviderName, contextRequestToken] {
                    ACSDK_DEBUG5(LX("provideStateInExecutor"));
                    executeProvideState(stateProviderName, contextRequestToken);
                });
                return;
            }
            bool ModeControllerCapabilityAgent::canStateBeRetrieved() {
                ACSDK_DEBUG5(LX(__func__));
                return m_isRetrievable;
            }
            bool ModeControllerCapabilityAgent::hasReportableStateProperties() {
                ACSDK_DEBUG5(LX(__func__));
                return (m_isRetrievable || m_isProactivelyReported);
            }
            void ModeControllerCapabilityAgent::cancelDirective(shared_ptr<DirectiveInfo> info) {
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
            DirectiveHandlerConfiguration ModeControllerCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[{NAMESPACE, NAME_SETMODE, m_endpointId, m_instance}] = neitherNonBlockingPolicy;
                configuration[{NAMESPACE, NAME_ADJUSTMODE, m_endpointId, m_instance}] = neitherNonBlockingPolicy;
                return configuration;
            }
            string ModeControllerCapabilityAgent::buildModeConfigurationJson() {
                JsonGenerator jsonGenerator;
                jsonGenerator.addMember("ordered", m_modeControllerAttributes.ordered);
                jsonGenerator.startArray("supportedModes");
                auto modes = m_modeControllerAttributes.modes;
                for (const auto& supportedMode : m_modeControllerConfiguration) {
                    if (modes.find(supportedMode) != modes.end()) {
                        jsonGenerator.startArrayElement();
                        jsonGenerator.addMember("value", supportedMode);
                        jsonGenerator.addRawJsonMember("modeResources", modes.at(supportedMode).toJson());
                        jsonGenerator.finishArrayElement();
                    }
                }
                jsonGenerator.finishArray();
                ACSDK_DEBUG5(LX(__func__).sensitive("configuration", jsonGenerator.toString()));
                return jsonGenerator.toString();
            }
            CapabilityConfiguration ModeControllerCapabilityAgent::getCapabilityConfiguration() {
                ACSDK_DEBUG5(LX(__func__));
                auto additionalConfigurations = CapabilityConfiguration::AdditionalConfigurations();
                additionalConfigurations[CAPABILITY_RESOURCES_KEY] = m_modeControllerAttributes.capabilityResources.toJson();
                additionalConfigurations[CAPABILITY_CONFIGURATION_KEY] = buildModeConfigurationJson();
                return {CapabilityConfiguration::ALEXA_INTERFACE_TYPE, NAMESPACE, INTERFACE_VERSION,Optional<string>(m_instance),
                        Optional<CapabilityConfiguration::Properties>({m_isRetrievable, m_isProactivelyReported,{MODEVALUE_PROPERTY_NAME},
                        Optional<bool>(m_isNonControllable)}), additionalConfigurations};
            }
            void ModeControllerCapabilityAgent::onModeChanged(const ModeState& mode, const AlexaStateChangeCauseType cause) {
                ACSDK_DEBUG5(LX(__func__));
                if (!m_isProactivelyReported) {
                    ACSDK_ERROR(LX("onModeChangedFailed").d("reason", "invalidOnModeChangedCall"));
                    return;
                }
                m_executor.submit([this, mode, cause] {
                    m_contextManager->reportStateChange(
                        CapabilityTag(NAMESPACE, MODEVALUE_PROPERTY_NAME, m_endpointId, m_instance),
                        buildCapabilityState(mode),
                        cause);
                });
            }
            void ModeControllerCapabilityAgent::doShutdown() {
                if (m_isProactivelyReported) m_modeController->removeObserver(shared_from_this());
                m_executor.shutdown();
                m_modeController.reset();
                m_responseSender.reset();
                if (m_isRetrievable) {
                    m_contextManager->removeStateProvider({NAMESPACE, MODEVALUE_PROPERTY_NAME, m_endpointId, m_instance});
                }
                m_contextManager.reset();
            }
            void ModeControllerCapabilityAgent::removeDirective(shared_ptr<DirectiveInfo> info) {
                if (info && info->directive) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void ModeControllerCapabilityAgent::executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                if (info && info->result) info->result->setCompleted();
                removeDirective(info);
            }

            bool ModeControllerCapabilityAgent::validateMode(const string& mode) {
                if (mode.empty() || find(m_modeControllerConfiguration.begin(),
               m_modeControllerConfiguration.end(), mode) == m_modeControllerConfiguration.end()) {
                    return false;
                }
                return true;
            }
            void ModeControllerCapabilityAgent::executeSetModeDirective(shared_ptr<DirectiveInfo> info, Document& payload) {
                ACSDK_DEBUG5(LX(__func__));
                string mode;
                if (!jsonUtils::retrieveValue(payload, MODE_KEY, &mode)) {
                    string errorMessage = "modeKeyNotFound";
                    ACSDK_ERROR(LX("executeSetModeDirectiveFailed").m(errorMessage));
                    sendExceptionEncounteredAndReportFailed(info, errorMessage);
                    return;
                }
                if (!validateMode(mode)) {
                    string errorMessage = "invalidModeReceived";
                    ACSDK_ERROR(LX("executeSetModeDirectiveFailed").m(errorMessage));
                    sendExceptionEncounteredAndReportFailed(info, errorMessage);
                    return;
                }
                auto result = m_modeController->setMode(mode, AlexaStateChangeCauseType::VOICE_INTERACTION);
                executeSendResponseEvent(info, result);
                executeSetHandlingCompleted(info);
            }
            void ModeControllerCapabilityAgent::executeAdjustModeDirective(shared_ptr<DirectiveInfo> info, Document& payload) {
                ACSDK_DEBUG5(LX(__func__));
                int64_t modeDelta = 0;
                if (!jsonUtils::retrieveValue(payload, MODE_DELTA_KEY, &modeDelta)) {
                    string errorMessage = "deltaModeKeyNotFound";
                    ACSDK_ERROR(LX("executeAdjustModeDirectiveFailed").m(errorMessage));
                    sendExceptionEncounteredAndReportFailed(info, errorMessage);
                    return;
                }
                auto result = m_modeController->adjustMode(modeDelta, AlexaStateChangeCauseType::VOICE_INTERACTION);
                executeSendResponseEvent(info, result);
                executeSetHandlingCompleted(info);
            }
            void ModeControllerCapabilityAgent::executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type) {
                ACSDK_ERROR(LX("executeUnknownDirectiveFailed").d("reason", "unknownDirective").d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));
                const string exceptionMessage = "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
                sendExceptionEncounteredAndReportFailed(info, exceptionMessage, type);
            }
            void ModeControllerCapabilityAgent::executeProvideState(
                const CapabilityTag& stateProviderName,
                const ContextRequestToken contextRequestToken) {
                bool isError = false;
                if (stateProviderName.endpointId != m_endpointId) {
                    ACSDK_ERROR(LX("provideStateFailed").d("reason", "notExpectedEndpointId").sensitive("endpointId", stateProviderName.endpointId));
                    isError = true;
                }
                if (stateProviderName.name != MODEVALUE_PROPERTY_NAME) {
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
                auto result = m_modeController->getMode();
                if (AlexaResponseType::SUCCESS != result.first) {
                    ACSDK_WARN(LX("executeProvideState").m("failedToGetPropertyValue").sensitive("reason", result.first));
                    m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                } else {
                    if (!result.second.hasValue()) {
                        ACSDK_ERROR(LX("executeProvideStateFailed").m("emptyModeState"));
                        m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                        return;
                    }
                    m_contextManager->provideStateResponse(stateProviderName, buildCapabilityState(result.second.value()), contextRequestToken);
                }
            }
            void ModeControllerCapabilityAgent::executeSendResponseEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info,
                                                                         const pair<AlexaResponseType, string> result) {
                if (AlexaResponseType::SUCCESS == result.first) {
                    m_responseSender->sendResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(), AVSMessageEndpoint(m_endpointId));
                } else {
                    m_responseSender->sendErrorResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(),
                                                     AVSMessageEndpoint(m_endpointId),m_responseSender->alexaResponseTypeToErrorType(result.first),
                                                              result.second);
                }
            }
            CapabilityState ModeControllerCapabilityAgent::buildCapabilityState(const ModeState& modeState) {
                return CapabilityState("\"" + modeState.mode + "\"", modeState.timeOfSample, modeState.valueUncertainty.count());
            }
        }
    }
}