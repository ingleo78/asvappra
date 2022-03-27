#include <ostream>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/CapabilityResources.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "RangeControllerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace rangeController {
            using namespace configuration;
            using namespace json;
            static const string TAG{"RangeControllerCapabilityAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE{"Alexa.RangeController"};
            static const string INTERFACE_VERSION{"3"};
            static const string NAME_SETRANGEVALUE{"SetRangeValue"};
            static const string NAME_ADJUSTRANGEVALUE{"AdjustRangeValue"};
            static const string RANGEVALUE_PROPERTY_NAME{"rangeValue"};
            static const string CAPABILITY_RESOURCES_KEY{"capabilityResources"};
            static const string CAPABILITY_CONFIGURATION_KEY{"configuration"};
            static const char RANGE_VALUE_KEY[] = "rangeValue";
            static const char RANGE_VALUE_DELTA_KEY[] = "rangeValueDelta";
            using RangeControllerConfiguration = RangeControllerInterface::RangeControllerConfiguration;
            static bool isRangeControllerAttributesValid(const RangeControllerAttributes& rangeControllerAttributes,
                                                         const RangeControllerConfiguration& rangeControllerConfiguration) {
                if (!rangeControllerAttributes.capabilityResources.isValid()) {
                    ACSDK_ERROR(LX("isRangeControllerAttributeValidFailed").d("reason", "capabilityResourcesInvalid"));
                    return false;
                }
                if (rangeControllerConfiguration.minimumValue > rangeControllerConfiguration.maximumValue) {
                    ACSDK_ERROR(LX("isRangeControllerAttributesValid").m("minimum cannot be greater than maximum")
                        .sensitive("minimum", rangeControllerConfiguration.minimumValue).sensitive("maximum", rangeControllerConfiguration.maximumValue));
                    return false;
                }
                for (const auto& preset : rangeControllerAttributes.presets) {
                    if (preset.first < rangeControllerConfiguration.minimumValue ||
                        preset.first > rangeControllerConfiguration.maximumValue) {
                        ACSDK_ERROR(LX("isRangeControllerAttributeValidFailed").d("reason", "presetValueOutOfRange")
                            .sensitive("preset", preset.first).sensitive("rangeMaximum", rangeControllerConfiguration.maximumValue)
                            .sensitive("rangeMinimum", rangeControllerConfiguration.minimumValue));
                        return false;
                    }
                    if (!preset.second.isValid()) {
                        ACSDK_ERROR(LX("isRangeControllerAttributeValidFailed").d("reason", "presetResourcesInvalid"));
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
            shared_ptr<RangeControllerCapabilityAgent> RangeControllerCapabilityAgent::create(const EndpointIdentifier& endpointId, const string& instance,
                                                                                              const RangeControllerAttributes& rangeControllerAttributes,
                                                                                              shared_ptr<RangeControllerInterface> rangeController,
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
                if (!rangeController) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullRangeContoller"));
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
                auto rangeContollerCapabilityAgent = shared_ptr<RangeControllerCapabilityAgent>(new RangeControllerCapabilityAgent(endpointId,
                                                                                                instance, rangeControllerAttributes,
                                                                                                rangeController, contextManager, responseSender,
                                                                                                exceptionSender, isProactivelyReported,
                                                                                                isRetrievable, isNonControllable));
                if (!rangeContollerCapabilityAgent->initialize()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "initializationFailed"));
                    return nullptr;
                }
                return rangeContollerCapabilityAgent;
            }
            RangeControllerCapabilityAgent::RangeControllerCapabilityAgent(const EndpointIdentifier& endpointId, const string& instance,
                                                                           const RangeControllerAttributes& rangeControllerAttributes,
                                                                           shared_ptr<RangeControllerInterface> rangeController,
                                                                           shared_ptr<ContextManagerInterface> contextManager,
                                                                           shared_ptr<AlexaInterfaceMessageSenderInterface> responseSender,
                                                                           shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                           bool isProactivelyReported, bool isRetrievable, bool isNonControllable) :
                                                                           CapabilityAgent{NAMESPACE, exceptionSender},
                                                                           RequiresShutdown{"RangeControllerCapabilityAgent"},
                                                                           m_endpointId{endpointId}, m_instance{instance},
                                                                           m_isProactivelyReported{isProactivelyReported},
                                                                           m_isRetrievable{isRetrievable}, m_isNonControllable{isNonControllable},
                                                                           m_rangeControllerAttributes{rangeControllerAttributes},
                                                                           m_rangeController{rangeController}, m_contextManager{contextManager},
                                                                           m_responseSender{responseSender} {}
            bool RangeControllerCapabilityAgent::initialize() {
                ACSDK_DEBUG5(LX(__func__));
                m_rangeControllerConfiguration = m_rangeController->getConfiguration();
                if (!isRangeControllerAttributesValid(m_rangeControllerAttributes, m_rangeControllerConfiguration)) {
                    ACSDK_ERROR(LX("initializeFailed").d("reason", "invalidRangeControllerAttributes"));
                    return false;
                }
                if (m_isProactivelyReported) {
                    if (!m_rangeController->addObserver(shared_from_this())) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "addObserverFailed"));
                        return false;
                    }
                }
                if (m_isRetrievable) {
                    m_contextManager->addStateProvider({NAMESPACE, RANGEVALUE_PROPERTY_NAME, m_endpointId, m_instance},
                                            shared_from_this());
                }
                return true;
            }
            void RangeControllerCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void RangeControllerCapabilityAgent::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("preHandleDirective"));
            }
            void RangeControllerCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
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
                    pair<AlexaResponseType, string> result;
                    if (directiveName == NAME_SETRANGEVALUE) executeSetRangeValueDirective(info, payload);
                    else if (directiveName == NAME_ADJUSTRANGEVALUE) executeAdjustRangeValueDirective(info, payload);
                    else {
                        ACSDK_ERROR(LX("handleDirective").d("reason", "unexpectedDirective").d("name", directiveName));
                        executeUnknownDirective(info, ExceptionErrorType::UNSUPPORTED_OPERATION);
                    }
                });
            }
            void RangeControllerCapabilityAgent::provideState(
                const CapabilityTag& stateProviderName,
                const ContextRequestToken contextRequestToken) {
                ACSDK_DEBUG5(LX(__func__).d("contextRequestToken", contextRequestToken).sensitive("stateProviderName", stateProviderName));
                m_executor.submit([this, stateProviderName, contextRequestToken] {
                    ACSDK_DEBUG5(LX("provideStateInExecutor"));
                    executeProvideState(stateProviderName, contextRequestToken);
                });
                return;
            }
            bool RangeControllerCapabilityAgent::canStateBeRetrieved() {
                ACSDK_DEBUG5(LX(__func__));
                return m_isRetrievable;
            }
            bool RangeControllerCapabilityAgent::hasReportableStateProperties() {
                ACSDK_DEBUG5(LX(__func__));
                return (m_isRetrievable || m_isProactivelyReported);
            }
            void RangeControllerCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
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
            DirectiveHandlerConfiguration RangeControllerCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[{NAMESPACE, NAME_SETRANGEVALUE, m_endpointId, m_instance}] = neitherNonBlockingPolicy;
                configuration[{NAMESPACE, NAME_ADJUSTRANGEVALUE, m_endpointId, m_instance}] = neitherNonBlockingPolicy;
                return configuration;
            }
            string RangeControllerCapabilityAgent::buildRangeConfigurationJson() {
                JsonGenerator jsonGenerator;
                jsonGenerator.startObject("supportedRange");
                jsonGenerator.addMember("maximumValue", m_rangeControllerConfiguration.maximumValue);
                jsonGenerator.addMember("minimumValue", m_rangeControllerConfiguration.minimumValue);
                jsonGenerator.addMember("precision", m_rangeControllerConfiguration.precision);
                jsonGenerator.finishObject();
                if (m_rangeControllerAttributes.unitOfMeasure.hasValue()) {
                    jsonGenerator.addMember("unitOfMeasure", m_rangeControllerAttributes.unitOfMeasure.value());
                }
                if (m_rangeControllerAttributes.presets.size() > 0) {
                    jsonGenerator.startArray("presets");
                    for (const auto& preset : m_rangeControllerAttributes.presets) {
                        jsonGenerator.startArrayElement();
                        jsonGenerator.addMember("rangeValue", preset.first);
                        jsonGenerator.addRawJsonMember("presetResources", preset.second.toJson());
                        jsonGenerator.finishArrayElement();
                    }
                    jsonGenerator.finishArray();
                }
                ACSDK_DEBUG5(LX(__func__).sensitive("configuration", jsonGenerator.toString()));
                return jsonGenerator.toString();
            }
            CapabilityConfiguration RangeControllerCapabilityAgent::getCapabilityConfiguration() {
                ACSDK_DEBUG5(LX(__func__));
                auto additionalConfigurations = CapabilityConfiguration::AdditionalConfigurations();
                additionalConfigurations[CAPABILITY_RESOURCES_KEY] = m_rangeControllerAttributes.capabilityResources.toJson();
                additionalConfigurations[CAPABILITY_CONFIGURATION_KEY] = buildRangeConfigurationJson();
                CapabilityConfiguration configuration{CapabilityConfiguration::ALEXA_INTERFACE_TYPE, NAMESPACE, INTERFACE_VERSION,
                                                      Optional<string>(m_instance),Optional<CapabilityConfiguration::Properties>(
                                                      {m_isRetrievable, m_isProactivelyReported,{RANGEVALUE_PROPERTY_NAME},
                                                      Optional<bool>(m_isNonControllable)}), additionalConfigurations};
                return configuration;
            }
            void RangeControllerCapabilityAgent::onRangeChanged(const RangeState& rangeState, const AlexaStateChangeCauseType cause) {
                ACSDK_DEBUG5(LX(__func__));
                if (!m_isProactivelyReported) {
                    ACSDK_ERROR(LX("onRangeChangedFailed").d("reason", "invalidOnRangeChangedCall"));
                    return;
                }
                m_executor.submit([this, rangeState, cause] {
                    m_contextManager->reportStateChange(CapabilityTag(NAMESPACE, RANGEVALUE_PROPERTY_NAME, m_endpointId, m_instance),
                                            buildCapabilityState(rangeState), cause);
                });
            }
            void RangeControllerCapabilityAgent::doShutdown() {
                if (m_isProactivelyReported) m_rangeController->removeObserver(shared_from_this());
                m_executor.shutdown();
                m_rangeController.reset();
                m_responseSender.reset();
                if (m_isRetrievable) {
                    m_contextManager->removeStateProvider({NAMESPACE, RANGEVALUE_PROPERTY_NAME, m_endpointId, m_instance});
                }
                m_contextManager.reset();
            }
            void RangeControllerCapabilityAgent::removeDirective(shared_ptr<DirectiveInfo> info) {
                if (info && info->directive) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void RangeControllerCapabilityAgent::executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                if (info && info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void RangeControllerCapabilityAgent::executeSetRangeValueDirective(shared_ptr<DirectiveInfo> info, Document& payload) {
                ACSDK_DEBUG5(LX(__func__));
                double rangeValue = 0;
                if (!jsonUtils::retrieveValue(payload, RANGE_VALUE_KEY, &rangeValue)) {
                    string errorMessage = "rangeValueKeyNotFound";
                    ACSDK_ERROR(LX("executeSetRangeValueFailed").m(errorMessage));
                    sendExceptionEncounteredAndReportFailed(info, errorMessage);
                    return;
                }
                if (!validateRangeValue(rangeValue)) {
                    std::string errorMessage = "invalidRangeValueReceived";
                    ACSDK_ERROR(LX("executeSetRangeValueFailed").m(errorMessage));
                    sendExceptionEncounteredAndReportFailed(info, errorMessage);
                    return;
                }
                auto result = m_rangeController->setRangeValue(rangeValue, AlexaStateChangeCauseType::VOICE_INTERACTION);
                executeSendResponseEvent(info, result);
                executeSetHandlingCompleted(info);
            }
            void RangeControllerCapabilityAgent::executeAdjustRangeValueDirective(
                std::shared_ptr<DirectiveInfo> info,
                rapidjson::Document& payload) {
                ACSDK_DEBUG5(LX(__func__));
                double deltaRange = 0;
                if (!jsonUtils::retrieveValue(payload, RANGE_VALUE_DELTA_KEY, &deltaRange)) {
                    string errorMessage = "deltaRangeValueKeyInvalid";
                    ACSDK_ERROR(LX("executeAdjustRangeValueFailed").m(errorMessage));
                    sendExceptionEncounteredAndReportFailed(info, errorMessage);
                    return;
                }
                auto result = m_rangeController->adjustRangeValue(deltaRange, AlexaStateChangeCauseType::VOICE_INTERACTION);
                executeSendResponseEvent(info, result);
                executeSetHandlingCompleted(info);
            }
            void RangeControllerCapabilityAgent::executeUnknownDirective(shared_ptr<DirectiveInfo> info, ExceptionErrorType type) {
                ACSDK_ERROR(LX("executeUnknownDirectiveFailed").d("reason", "unknownDirective")
                    .d("namespace", info->directive->getNamespace()).d("name", info->directive->getName()));
                const string exceptionMessage = "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
                sendExceptionEncounteredAndReportFailed(info, exceptionMessage, type);
            }
            bool RangeControllerCapabilityAgent::validateRangeValue(double rangeValue) {
                if (rangeValue < m_rangeControllerConfiguration.minimumValue || rangeValue > m_rangeControllerConfiguration.maximumValue) {
                    return false;
                }
                return true;
            }
            void RangeControllerCapabilityAgent::executeProvideState(const CapabilityTag& stateProviderName, const ContextRequestToken contextRequestToken) {
                bool isError = false;
                if (stateProviderName.endpointId != m_endpointId) {
                    ACSDK_ERROR(LX("provideStateFailed").d("reason", "notExpectedEndpointId").sensitive("endpointId", stateProviderName.endpointId));
                    isError = true;
                }
                if (stateProviderName.name != RANGEVALUE_PROPERTY_NAME) {
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
                auto result = m_rangeController->getRangeState();
                if (AlexaResponseType::SUCCESS != result.first) {
                    ACSDK_WARN(LX("executeProvideState").m("failedToGetPropertyValue").sensitive("reason", result.first));
                    m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                } else {
                    if (!result.second.hasValue()) {
                        ACSDK_ERROR(LX("executeProvideStateFailed").m("emptyRangeState"));
                        m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, true);
                        return;
                    }
                    m_contextManager->provideStateResponse(stateProviderName, buildCapabilityState(result.second.value()), contextRequestToken);
                }
            }
            void RangeControllerCapabilityAgent::executeSendResponseEvent(const shared_ptr<CapabilityAgent::DirectiveInfo> info,
                                                                          const pair<AlexaResponseType, string> result) {
                if (AlexaResponseType::SUCCESS == result.first) {
                    m_responseSender->sendResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(), AVSMessageEndpoint(m_endpointId));
                } else {
                    m_responseSender->sendErrorResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(),
                                                     AVSMessageEndpoint(m_endpointId), m_responseSender->alexaResponseTypeToErrorType(result.first),
                                                              result.second);
                }
            }
            CapabilityState RangeControllerCapabilityAgent::buildCapabilityState(const RangeState& rangeState) {
                return CapabilityState("\"" + to_string(rangeState.value) + "\"", rangeState.timeOfSample, rangeState.valueUncertainty.count());
            }
        }
    }
}