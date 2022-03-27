#include <avs/EventBuilder.h>
#include <avs/MessageRequest.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <uuid_generation/UUIDGeneration.h>
#include "AlexaInterfaceMessageSender.h"
#include "AlexaInterfaceConstants.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace alexa {
            using namespace logger;
            static const string TAG{"AlexaInterfaceMessageSender"};
            static const string EVENT_NAME_STATE_REPORT_STRING = "StateReport";
            static const string EVENT_NAME_RESPONSE_STRING = "Response";
            static const string EVENT_NAME_ERROR_RESPONSE_STRING = "ErrorResponse";
            static const string EVENT_NAME_DEFERRED_RESPONSE_STRING = "DeferredResponse";
            static const string EVENT_NAME_CHANGE_REPORT_STRING = "ChangeReport";
            static const string ESTIMATED_DEFERRAL_KEY_STRING = "estimatedDeferralInSeconds";
            static const string CAUSE_KEY_STRING = "cause";
            static const string CHANGE_KEY_STRING = "change";
            static const string MESSAGE_KEY_STRING = "message";
            static const string PROPERTIES_KEY_STRING = "properties";
            static const string TIME_OF_SAMPLE_KEY_STRING = "timeOfSample";
            static const string TYPE_KEY_STRING = "type";
            static const string UNCERTAINTY_IN_MILLISECONDS_KEY_STRING = "uncertaintyInMilliseconds";
            static const string VALUE_KEY_STRING = "value";
            static const map<AlexaInterfaceMessageSender::ErrorResponseType, string> errorTypeMap = {
                {AlexaInterfaceMessageSender::ErrorResponseType::ALREADY_IN_OPERATION, "ALREADY_IN_OPERATION"},
                {AlexaInterfaceMessageSender::ErrorResponseType::BRIDGE_UNREACHABLE, "BRIDGE_UNREACHABLE"},
                {AlexaInterfaceMessageSender::ErrorResponseType::ENDPOINT_BUSY, "ENDPOINT_BUSY"},
                {AlexaInterfaceMessageSender::ErrorResponseType::ENDPOINT_LOW_POWER, "ENDPOINT_LOW_POWER"},
                {AlexaInterfaceMessageSender::ErrorResponseType::ENDPOINT_UNREACHABLE, "ENDPOINT_UNREACHABLE"},
                {AlexaInterfaceMessageSender::ErrorResponseType::EXPIRED_AUTHORIZATION_CREDENTIAL, "EXPIRED_AUTHORIZATION_CREDENTIAL"},
                {AlexaInterfaceMessageSender::ErrorResponseType::FIRMWARE_OUT_OF_DATE, "FIRMWARE_OUT_OF_DATE"},
                {AlexaInterfaceMessageSender::ErrorResponseType::HARDWARE_MALFUNCTION, "HARDWARE_MALFUNCTION"},
                {AlexaInterfaceMessageSender::ErrorResponseType::INSUFFICIENT_PERMISSIONS, "INSUFFICIENT_PERMISSIONS"},
                {AlexaInterfaceMessageSender::ErrorResponseType::INTERNAL_ERROR, "INTERNAL_ERROR"},
                {AlexaInterfaceMessageSender::ErrorResponseType::INVALID_AUTHORIZATION_CREDENTIAL, "INVALID_AUTHORIZATION_CREDENTIAL"},
                {AlexaInterfaceMessageSender::ErrorResponseType::INVALID_DIRECTIVE, "INVALID_DIRECTIVE"},
                {AlexaInterfaceMessageSender::ErrorResponseType::INVALID_VALUE, "INVALID_VALUE"},
                {AlexaInterfaceMessageSender::ErrorResponseType::NO_SUCH_ENDPOINT, "NO_SUCH_ENDPOINT"},
                {AlexaInterfaceMessageSender::ErrorResponseType::NOT_CALIBRATED, "NOT_CALIBRATED"},
                {AlexaInterfaceMessageSender::ErrorResponseType::NOT_SUPPORTED_IN_CURRENT_MODE, "NOT_SUPPORTED_IN_CURRENT_MODE"},
                {AlexaInterfaceMessageSender::ErrorResponseType::NOT_IN_OPERATION, "NOT_IN_OPERATION"},
                {AlexaInterfaceMessageSender::ErrorResponseType::POWER_LEVEL_NOT_SUPPORTED, "POWER_LEVEL_NOT_SUPPORTED"},
                {AlexaInterfaceMessageSender::ErrorResponseType::RATE_LIMIT_EXCEEDED, "RATE_LIMIT_EXCEEDED"},
                {AlexaInterfaceMessageSender::ErrorResponseType::TEMPERATURE_VALUE_OUT_OF_RANGE, "TEMPERATURE_VALUE_OUT_OF_RANGE"},
                {AlexaInterfaceMessageSender::ErrorResponseType::VALUE_OUT_OF_RANGE, "VALUE_OUT_OF_RANGE"}
            };
            static const map<AlexaStateChangeCauseType, string> causeTypeMap = {
                {AlexaStateChangeCauseType::ALEXA_INTERACTION, "ALEXA_INTERACTION"},
                {AlexaStateChangeCauseType::APP_INTERACTION, "APP_INTERACTION"},
                {AlexaStateChangeCauseType::PHYSICAL_INTERACTION, "PHYSICAL_INTERACTION"},
                {AlexaStateChangeCauseType::PERIODIC_POLL, "PERIODIC_POLL"},
                {AlexaStateChangeCauseType::RULE_TRIGGER, "RULE_TRIGGER"},
                {AlexaStateChangeCauseType::VOICE_INTERACTION, "VOICE_INTERACTION"}
            };
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<AlexaInterfaceMessageSender> AlexaInterfaceMessageSender::create(shared_ptr<ContextManagerInterface> contextManager,
                                                                                        shared_ptr<MessageSenderInterface> messageSender) {
                if (!contextManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "contextManagerNull"));
                    return nullptr;
                }
                if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "messageSenderNull"));
                    return nullptr;
                }
                auto sender = shared_ptr<AlexaInterfaceMessageSender>(new AlexaInterfaceMessageSender(contextManager, messageSender));
                if (!sender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "outOfMemory"));
                    return nullptr;
                }
                if (!sender->initialize()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "initializationFailed"));
                    sender.reset();
                    return nullptr;
                }
                return sender;
            }
            AlexaInterfaceMessageSender::AlexaInterfaceMessageSender(shared_ptr<ContextManagerInterface> contextManager,
                                                                     shared_ptr<MessageSenderInterface> messageSender) :
                                                                     RequiresShutdown{"AlexaInterfaceMessageSender"},
                                                                     m_contextManager{contextManager}, m_messageSender{messageSender} {}
            AlexaInterfaceMessageSender::~AlexaInterfaceMessageSender() {
                m_pendingChangeReports.clear();
                m_pendingResponses.clear();
                m_messageSender.reset();
                m_contextManager.reset();
            }
            bool AlexaInterfaceMessageSender::initialize() {
                m_contextManager->addContextManagerObserver(shared_from_this());
                return true;
            }
            void AlexaInterfaceMessageSender::doShutdown() {
                m_contextManager->removeContextManagerObserver(shared_from_this());
                m_executor.shutdown();
            }
            bool AlexaInterfaceMessageSender::sendStateReportEvent(
                const string& instance,
                const string& correlationToken,
                const AVSMessageEndpoint& endpoint) {
                return sendCommonResponseEvent(EVENT_NAME_STATE_REPORT_STRING, instance, correlationToken, endpoint);
            }
            bool AlexaInterfaceMessageSender::sendResponseEvent(const string& instance, const string& correlationToken,
                                                                const AVSMessageEndpoint& endpoint, const string& jsonPayload) {
                return sendCommonResponseEvent(EVENT_NAME_RESPONSE_STRING, instance, correlationToken, endpoint, jsonPayload);
            }
            bool AlexaInterfaceMessageSender::sendErrorResponseEvent(const string& instance, const string& correlationToken,
                                                                     const AVSMessageEndpoint& endpoint, const AlexaInterfaceMessageSender::ErrorResponseType errorType,
                                                                     const string& errorMessage) {
                if (errorTypeMap.find(errorType) == errorTypeMap.end()) {
                    ACSDK_ERROR(LX("sendErrorResponseEventFailed").d("reason", "invalidErrorType"));
                    return false;
                }
                auto errorTypeString = errorTypeMap.at(errorType);
                AVSMessageHeader eventHeader = AVSMessageHeader::createAVSEventHeader(ALEXA_INTERFACE_NAME, EVENT_NAME_ERROR_RESPONSE_STRING,
                                                                      "", correlationToken, ALEXA_INTERFACE_VERSION, instance);
                json::JsonGenerator jsonGenerator;
                jsonGenerator.addMember(TYPE_KEY_STRING, errorTypeString);
                jsonGenerator.addMember(MESSAGE_KEY_STRING, errorMessage);
                jsonGenerator.finishObject();
                auto jsonString = buildJsonEventString(eventHeader, Optional<AVSMessageEndpoint>(endpoint), jsonGenerator.toString());
                sendEvent(jsonString);
                return true;
            }
            bool AlexaInterfaceMessageSender::sendDeferredResponseEvent(const string& instance, const string& correlationToken,
                                                                        const int estimatedDeferralInSeconds) {
                AVSMessageHeader eventHeader = AVSMessageHeader::createAVSEventHeader(ALEXA_INTERFACE_NAME, EVENT_NAME_DEFERRED_RESPONSE_STRING,
                                                                      "", correlationToken, ALEXA_INTERFACE_VERSION, instance);
                json::JsonGenerator jsonGenerator;
                jsonGenerator.addMember(ESTIMATED_DEFERRAL_KEY_STRING, estimatedDeferralInSeconds);
                jsonGenerator.finishObject();
                auto jsonString = buildJsonEventString(eventHeader, Optional<AVSMessageEndpoint>(), jsonGenerator.toString());
                sendEvent(jsonString);
                return true;
            }
            AlexaInterfaceMessageSender::ResponseData::ResponseData(const string& typeIn, const string& instanceIn, const string& correlationTokenIn,
                                                                    const AVSMessageEndpoint& endpointIn, const string& jsonPayloadIn) :
                                                                    type(typeIn), instance(instanceIn), correlationToken(correlationTokenIn),
                                                                    endpoint(endpointIn), jsonPayload(jsonPayloadIn) {}
            AlexaInterfaceMessageSender::ChangeReportData::ChangeReportData(const CapabilityTag& tagIn, const CapabilityState& stateIn,
                                                                            const AlexaStateChangeCauseType& causeIn) : tag(tagIn),
                                                                            state(stateIn), cause(causeIn) {}
            bool AlexaInterfaceMessageSender::sendCommonResponseEvent(const string& type, const string& instance, const string& correlationToken,
                                                                      const AVSMessageEndpoint& endpoint, const string& jsonPayload) {
                auto event = make_shared<ResponseData>(type, instance, correlationToken, endpoint, jsonPayload);
                m_executor.submit([this, event]() {
                    auto token = m_contextManager->getContext(shared_from_this(), event->endpoint.endpointId);
                    m_pendingResponses[token] = event;
                });
                return true;
            }
            void AlexaInterfaceMessageSender::completeResponseEvent(const shared_ptr<ResponseData>& event, const Optional<AVSContext>& context) {
                AVSMessageHeader eventHeader = AVSMessageHeader::createAVSEventHeader(ALEXA_INTERFACE_NAME, event->type, "",
                                                                                      event->correlationToken, ALEXA_INTERFACE_VERSION,
                                                                                      event->instance);
                auto jsonString = buildJsonEventString(eventHeader, Optional<AVSMessageEndpoint>(event->endpoint), event->jsonPayload,
                                                       Optional<AVSContext>(context));
                sendEvent(jsonString);
            }
            void AlexaInterfaceMessageSender::completeChangeReportEvent(const shared_ptr<ChangeReportData>& event, const AVSContext& context) {
                if (causeTypeMap.find(event->cause) == causeTypeMap.end()) {
                    ACSDK_ERROR(LX("completeChangeReportEventtFailed").d("reason", "invalidCauseType"));
                    return;
                }
                auto causeTypeString = causeTypeMap.at(event->cause);
                string instance;
                if (event->tag.instance.hasValue()) instance = event->tag.instance.value();
                AVSMessageHeader eventHeader = AVSMessageHeader::createAVSEventHeader(ALEXA_INTERFACE_NAME, EVENT_NAME_CHANGE_REPORT_STRING,
                                                                       "", "", ALEXA_INTERFACE_VERSION, instance);
                auto prunedContext = context;
                prunedContext.removeState(event->tag);
                json::JsonGenerator jsonGenerator;
                jsonGenerator.startObject(CHANGE_KEY_STRING);
                {
                    jsonGenerator.startObject(CAUSE_KEY_STRING);
                    jsonGenerator.addMember(TYPE_KEY_STRING, causeTypeString);
                    jsonGenerator.finishObject();
                    jsonGenerator.startArray(PROPERTIES_KEY_STRING);
                    jsonGenerator.startArrayElement();
                    jsonGenerator.addMember(constants::NAMESPACE_KEY_STRING, event->tag.nameSpace);
                    jsonGenerator.addMember(constants::NAME_KEY_STRING, event->tag.name);
                    if (event->tag.instance.hasValue()) jsonGenerator.addMember("instance", event->tag.instance.value());
                    jsonGenerator.addRawJsonMember(VALUE_KEY_STRING, event->state.valuePayload);
                    jsonGenerator.addMember(TIME_OF_SAMPLE_KEY_STRING, event->state.timeOfSample.getTime_ISO_8601());
                    jsonGenerator.addMember(UNCERTAINTY_IN_MILLISECONDS_KEY_STRING, event->state.uncertaintyInMilliseconds);
                    jsonGenerator.finishArrayElement();
                    jsonGenerator.finishArray();
                }
                jsonGenerator.finishObject();
                auto jsonString = buildJsonEventString(eventHeader, Optional<AVSMessageEndpoint>(event->tag.endpointId),jsonGenerator.toString(),
                                                       Optional<AVSContext>(prunedContext));
                sendEvent(jsonString);
            }
            void AlexaInterfaceMessageSender::sendEvent(const string& eventJson) {
                auto request = make_shared<MessageRequest>(eventJson);
                request->addObserver(shared_from_this());
                m_messageSender->sendMessage(request);
            }
            void AlexaInterfaceMessageSender::onStateChanged(const CapabilityTag& identifier, const CapabilityState& state,
                                                             const AlexaStateChangeCauseType cause) {
                auto event = make_shared<ChangeReportData>(identifier, state, cause);
                m_executor.submit([this, event]() {
                    auto token = m_contextManager->getContext(shared_from_this(), event->tag.endpointId);
                    m_pendingChangeReports[token] = event;
                });
            }
            void AlexaInterfaceMessageSender::onContextAvailable(const string& endpointId, const AVSContext& endpointContext,
                                                                 ContextRequestToken token) {
                m_executor.submit([this, endpointId, endpointContext, token]() {
                    ACSDK_DEBUG(LX("onContextAvailable").sensitive("endpointId", endpointId));
                    if (m_pendingResponses.find(token) != m_pendingResponses.end()) {
                        completeResponseEvent(m_pendingResponses[token], Optional<AVSContext>(endpointContext));
                        m_pendingResponses.erase(token);
                    } else if (m_pendingChangeReports.find(token) != m_pendingChangeReports.end()) {
                        completeChangeReportEvent(m_pendingChangeReports[token], endpointContext);
                        m_pendingChangeReports.erase(token);
                    } else { ACSDK_ERROR(LX("onContextAvailable").d("reason", "unknownEvent").d("token", token)); }
                });
            }
            void AlexaInterfaceMessageSender::onContextFailure(const ContextRequestError error, ContextRequestToken token) {
                m_executor.submit([this, error, token]() {
                    ACSDK_ERROR(LX("executeOnContextFailure").d("error", error));
                    if (m_pendingResponses.find(token) != m_pendingResponses.end()) {
                        completeResponseEvent(m_pendingResponses[token]);
                        m_pendingResponses.erase(token);
                        return;
                    } else if (m_pendingChangeReports.find(token) != m_pendingChangeReports.end()) {
                        ACSDK_ERROR(LX("executeOnContextFailure").d("reason", "cannotSendWithoutContext").d("token", token));
                        m_pendingChangeReports.erase(token);
                    } else { ACSDK_ERROR(LX("executeOnContextFailure").d("reason", "unknownEvent").d("token", token)); }
                });
            }
            void AlexaInterfaceMessageSender::onSendCompleted(MessageRequestObserverInterface::Status status) {
                if (status == MessageRequestObserverInterface::Status::SUCCESS ||
                    status == MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED ||
                    status == MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT) {
                    ACSDK_DEBUG(LX("onSendCompleted").d("status", status));
                    return;
                }
                ACSDK_ERROR(LX("onSendCompleted").m("sendFailed").d("status", status));
            }
            void AlexaInterfaceMessageSender::onExceptionReceived(const string& exceptionMessage) {
                ACSDK_ERROR(LX("onExceptionReceived").d("exception", exceptionMessage));
            }
        }
    }
}