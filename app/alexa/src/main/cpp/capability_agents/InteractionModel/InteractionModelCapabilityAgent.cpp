#include <avs/NamespaceAndName.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "InteractionModelCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace interactionModel {
            using namespace jsonUtils;
            static const string TAG{"InteractionModel"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE = "InteractionModel";
            static const NamespaceAndName NEW_DIALOG_REQUEST{NAMESPACE, "NewDialogRequest"};
            static const NamespaceAndName REQUEST_PROCESS_STARTED{NAMESPACE, "RequestProcessingStarted"};
            static const NamespaceAndName REQUEST_PROCESS_COMPLETED{NAMESPACE, "RequestProcessingCompleted"};
            static const string INTERACTION_MODEL_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string INTERACTION_MODEL_CAPABILITY_INTERFACE_NAME = "InteractionModel";
            static const string INTERACTION_MODEL_CAPABILITY_INTERFACE_VERSION = "1.2";
            static const string PAYLOAD_KEY_DIALOG_REQUEST_ID = "dialogRequestId";
            static shared_ptr<CapabilityConfiguration> getInteractionModelCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, INTERACTION_MODEL_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, INTERACTION_MODEL_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, INTERACTION_MODEL_CAPABILITY_INTERFACE_VERSION});
                return make_shared<CapabilityConfiguration>(configMap);
            }
            shared_ptr<InteractionModelCapabilityAgent> InteractionModelCapabilityAgent::create(shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                                                                shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) {
                if (!directiveSequencer) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullDirectiveSequencer"));
                    return nullptr;
                }
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncounteredSender"));
                    return nullptr;
                }
                return shared_ptr<InteractionModelCapabilityAgent>(
                    new InteractionModelCapabilityAgent(directiveSequencer, exceptionEncounteredSender));
            }
            void InteractionModelCapabilityAgent::addObserver(shared_ptr<InteractionModelRequestProcessingObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observerMutex};
                m_observers.insert(observer);
            }
            void InteractionModelCapabilityAgent::removeObserver(shared_ptr<InteractionModelRequestProcessingObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock{m_observerMutex};
                m_observers.erase(observer);
            }
            InteractionModelCapabilityAgent::InteractionModelCapabilityAgent(shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                                             shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                                                             CapabilityAgent{NAMESPACE, exceptionEncounteredSender},
                                                                             m_directiveSequencer{directiveSequencer} {
                ACSDK_DEBUG5(LX(__func__));
                m_capabilityConfigurations.insert(getInteractionModelCapabilityConfiguration());
            }
            InteractionModelCapabilityAgent::~InteractionModelCapabilityAgent() {
                ACSDK_DEBUG5(LX(__func__));
            }
            DirectiveHandlerConfiguration InteractionModelCapabilityAgent::getConfiguration() const {
                DirectiveHandlerConfiguration configuration;
                configuration[NEW_DIALOG_REQUEST] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[REQUEST_PROCESS_STARTED] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[REQUEST_PROCESS_COMPLETED] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                return configuration;
            }
            void InteractionModelCapabilityAgent::handleDirectiveImmediately(std::shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
            }
            void InteractionModelCapabilityAgent::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            bool InteractionModelCapabilityAgent::handleDirectiveHelper(shared_ptr<CapabilityAgent::DirectiveInfo> info, string* errMessage,
                                                                        ExceptionErrorType* type) {
                ACSDK_DEBUG5(LX(__func__));
                if (!errMessage) {
                    ACSDK_ERROR(LX("processDirectiveFailed").d("reason", "nullErrMessage"));
                    return false;
                }
                if (!type) {
                    ACSDK_ERROR(LX("processDirectiveFailed").d("reason", "nullType"));
                    return false;
                }
                if (!info) {
                    ACSDK_ERROR(LX("processDirectiveFailed").d("reason", "nullInfo"));
                    return false;
                }
                if (!info->directive) {
                    ACSDK_ERROR(LX("processDirectiveFailed").d("reason", "nullDirective"));
                    return false;
                }
                const string directiveName = info->directive->getName();
                Document payload(kObjectType);
                ParseResult result = payload.Parse(info->directive->getPayload().data());
                if (!result) {
                    ACSDK_ERROR(LX("processDirectiveFailed").d("reason", "directiveParseFailed"));
                    *errMessage = "Parse failure";
                    *type = ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED;
                    return false;
                }
                if (NEW_DIALOG_REQUEST.name == directiveName) {
                    Value::ConstMemberIterator it;
                    Value _payload{payload.GetString(), strlen(payload.GetString())};
                    string uuid;
                    if (findNode(_payload, PAYLOAD_KEY_DIALOG_REQUEST_ID, &it)) {
                        if (!retrieveValue(payload, PAYLOAD_KEY_DIALOG_REQUEST_ID, &uuid)) {
                            ACSDK_ERROR(LX("processDirectiveFailed").d("reason", "dialogRequestIDNotAccessible"));
                            *errMessage = "Dialog Request ID not accessible";
                            *type = ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED;
                            return false;
                        }
                        if (uuid.empty()) {
                            ACSDK_ERROR(LX("processDirectiveFailed").d("reason", "dialogRequestIDIsAnEmptyString"));
                            *errMessage = "Dialog Request ID is an Empty String";
                            *type = ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED;
                            return false;
                        }
                        m_directiveSequencer->setDialogRequestId(uuid);
                    } else {
                        *errMessage = "Dialog Request ID not specified";
                        *type = ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED;
                        return false;
                    }
                } else if (REQUEST_PROCESS_STARTED.name == directiveName) {
                    unique_lock<mutex> lock{m_observerMutex};
                    auto observers = m_observers;
                    lock.unlock();
                    for (auto observer : observers) {
                        if (observer) observer->onRequestProcessingStarted();
                    }
                    return true;
                } else if (REQUEST_PROCESS_COMPLETED.name == directiveName) {
                    unique_lock<mutex> lock{m_observerMutex};
                    auto observers = m_observers;
                    lock.unlock();
                    for (auto observer : observers) {
                        if (observer) observer->onRequestProcessingCompleted();
                    }
                    return true;
                } else {
                    *errMessage = directiveName + " not supported";
                    *type = ExceptionErrorType::UNSUPPORTED_OPERATION;
                    return false;
                }
                return true;
            }
            void InteractionModelCapabilityAgent::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullInfo"));
                    return;
                }
                if (!info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirective"));
                    return;
                }
                string errMessage;
                ExceptionErrorType errType;
                if (handleDirectiveHelper(info, &errMessage, &errType)) {
                    if (info->result) info->result->setCompleted();
                } else {
                    ACSDK_ERROR(LX("processDirectiveFailed").d("reason", errMessage));
                    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), errType, errMessage);
                    if (info->result) info->result->setFailed(errMessage);
                }
                removeDirective(info->directive->getMessageId());
            }
            void InteractionModelCapabilityAgent::cancelDirective(std::shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            unordered_set<shared_ptr<CapabilityConfiguration>> InteractionModelCapabilityAgent::
                getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
        }
    }
}