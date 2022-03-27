#include "AlexaInterfaceCapabilityAgent.h"
#include "AlexaInterfaceConstants.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace alexa {
            static const string TAG{"AlexaInterfaceCapabilityAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE = ALEXA_INTERFACE_NAME;
            static const string EVENT_PROCESSED_DIRECTIVE_NAME = "EventProcessed";
            static const string REPORT_STATE_DIRECTIVE_NAME = "ReportState";
            shared_ptr<AlexaInterfaceCapabilityAgent> AlexaInterfaceCapabilityAgent::create(
                const DeviceInfo& deviceInfo,
                const EndpointIdentifier& endpointId,
                shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender) {
                ACSDK_DEBUG5(LX(__func__));
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
                } else if (!alexaMessageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullAlexaMessageSender"));
                } else {
                    auto instance = shared_ptr<AlexaInterfaceCapabilityAgent>(new AlexaInterfaceCapabilityAgent(deviceInfo, endpointId,
                                                                              exceptionEncounteredSender, alexaMessageSender));
                    return instance;
                }
                return nullptr;
            }
            AlexaInterfaceCapabilityAgent::AlexaInterfaceCapabilityAgent(
                const DeviceInfo& deviceInfo,
                const EndpointIdentifier& endpointId,
                shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender) : CapabilityAgent{NAMESPACE, exceptionEncounteredSender},
                                                                                               m_deviceInfo{deviceInfo}, m_endpointId{endpointId},
                                                                                               m_alexaMessageSender{alexaMessageSender} {}
            DirectiveHandlerConfiguration AlexaInterfaceCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                if (m_endpointId == m_deviceInfo.getDefaultEndpointId()) {
                    ACSDK_DEBUG5(LX("registeringEventProcessedDirective").d("reason", "defaultEndpoint"));
                    configuration[NamespaceAndName{NAMESPACE, EVENT_PROCESSED_DIRECTIVE_NAME}] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                }
                configuration[{NAMESPACE, REPORT_STATE_DIRECTIVE_NAME, m_endpointId}] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                return configuration;
            }
            CapabilityConfiguration AlexaInterfaceCapabilityAgent::getCapabilityConfiguration() {
                return CapabilityConfiguration(ALEXA_INTERFACE_TYPE, ALEXA_INTERFACE_NAME, ALEXA_INTERFACE_VERSION);
            }
            void AlexaInterfaceCapabilityAgent::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
            }
            void AlexaInterfaceCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
            }
            void AlexaInterfaceCapabilityAgent::handleDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                m_executor.submit([this, info] {
                    if (!info || !info->directive) {
                        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirective"));
                        return;
                    }
                    if (EVENT_PROCESSED_DIRECTIVE_NAME == info->directive->getName()) {
                        if (!executeHandleEventProcessed(info->directive)) {
                            executeSendExceptionEncounteredAndReportFailed(info, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                               "empty event correlation token");
                            return;
                        }
                    } else if (REPORT_STATE_DIRECTIVE_NAME == info->directive->getName()) {
                        if (!info->directive->getEndpoint().hasValue() ||
                            info->directive->getEndpoint().value().endpointId.empty()) {
                            executeSendErrorResponse(info,AlexaInterfaceMessageSenderInternalInterface::ErrorResponseType::INVALID_DIRECTIVE,
                                         "missing endpoint");
                        } else if (!m_alexaMessageSender->sendStateReportEvent(info->directive->getInstance(), info->directive->getCorrelationToken(),
                                                                               info->directive->getEndpoint().value().endpointId)) {
                            executeSendErrorResponse(info,AlexaInterfaceMessageSenderInternalInterface::ErrorResponseType::INTERNAL_ERROR,
                                         "failed to handle report state");
                        }
                    } else {
                        executeSendExceptionEncounteredAndReportFailed(info, ExceptionErrorType::UNSUPPORTED_OPERATION, "unknown directive");
                        return;
                    }
                    if (info->result) info->result->setCompleted();
                    removeDirective(info);
                });
            }
            void AlexaInterfaceCapabilityAgent::cancelDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                removeDirective(info);
            }
            void AlexaInterfaceCapabilityAgent::removeDirective(const shared_ptr<DirectiveInfo>& info) {
                ACSDK_DEBUG5(LX(__func__));
                if (info && info->directive) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void AlexaInterfaceCapabilityAgent::executeSendExceptionEncounteredAndReportFailed(const shared_ptr<DirectiveInfo>& info, ExceptionErrorType errorType,
                                                                                               const string& errorMessage) {
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", errorMessage).d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));
                const string exceptionMessage = errorMessage + " " + info->directive->getNamespace() + ":" + info->directive->getName();
                sendExceptionEncounteredAndReportFailed(info, exceptionMessage, errorType);
            }
            void AlexaInterfaceCapabilityAgent::executeSendErrorResponse(const shared_ptr<DirectiveInfo>& info, AlexaInterfaceMessageSenderInterface::ErrorResponseType errorType,
                                                                         const string& errorMessage) {
                if (!m_alexaMessageSender->sendErrorResponseEvent(info->directive->getInstance(), info->directive->getCorrelationToken(),
                    info->directive->getEndpoint().value().endpointId, errorType, errorMessage)) {
                    ACSDK_ERROR(LX("executeSendErrorResponseFailed").d("reason", "failedToSendEvent"));
                }
            }
            bool AlexaInterfaceCapabilityAgent::executeHandleEventProcessed(const shared_ptr<AVSDirective>& directive) {
                ACSDK_DEBUG5(LX(__func__));
                if (!directive) {
                    ACSDK_ERROR(LX("executeHandleEventProcessedFailed").d("reason", "nullDirective"));
                    return false;
                }
                string eventCorrelationToken = directive->getEventCorrelationToken();
                if (eventCorrelationToken.empty()) {
                    ACSDK_ERROR(LX("executeHandleEventProcessedFailed").d("reason", "emptyEventCorrelationToken"));
                    return false;
                }
                lock_guard<mutex> lock{m_observerMutex};
                for (const auto& observer : m_observers) observer->onAlexaEventProcessedReceived(eventCorrelationToken);
                return true;
            }
            void AlexaInterfaceCapabilityAgent::addEventProcessedObserver(const shared_ptr<AlexaEventProcessedObserverInterface>& observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addEventProcessedObserver").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observerMutex};
                m_observers.insert(observer);
            }

            void AlexaInterfaceCapabilityAgent::removeEventProcessedObserver(const shared_ptr<AlexaEventProcessedObserverInterface>& observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeEventProcessedObserver").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observerMutex};
                m_observers.erase(observer);
            }
        }
    }
}