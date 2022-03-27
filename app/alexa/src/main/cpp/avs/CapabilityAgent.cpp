#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/DirectiveHandlerResultInterface.h>
#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <logger/Logger.h>
#include <uuid_generation/UUIDGeneration.h>
#include "CapabilityAgent.h"
#include "EventBuilder.h"
#include "AVSDirective.h"
#include "ExceptionErrorType.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace rapidjson;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace std;
            static const string TAG("CapabilityAgent");
            static const int CAPABILITY_QUEUE_WARN_SIZE = 10;
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<CapabilityAgent::DirectiveInfo> CapabilityAgent::createDirectiveInfo(shared_ptr<AVSDirective> directive,
                unique_ptr<sdkInterfaces::DirectiveHandlerResultInterface> result) {
                return make_shared<DirectiveInfo>(directive, move(result));
            }
            CapabilityAgent::CapabilityAgent(const string& nameSpace, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                             m_namespace{nameSpace}, m_exceptionEncounteredSender{exceptionEncounteredSender} {}
            CapabilityAgent::DirectiveInfo::DirectiveInfo(shared_ptr<AVSDirective> directiveIn, unique_ptr<DirectiveHandlerResultInterface> resultIn) :
                                                          directive{directiveIn}, result{move(resultIn)},isCancelled{false} {}
            void CapabilityAgent::preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result) {
                string messageId = directive->getMessageId();
                auto info = getDirectiveInfo(messageId);
                if (info) {
                    static const string error{"messageIdIsAlreadyInUse"};
                    ACSDK_ERROR(LX("preHandleDirectiveFailed").d("reason", error).d("messageId", messageId));
                    result->setFailed(error);
                    if (m_exceptionEncounteredSender) {
                        m_exceptionEncounteredSender->sendExceptionEncountered(directive->getUnparsedDirective(), ExceptionErrorType::INTERNAL_ERROR, error);
                    }
                    return;
                }
                ACSDK_DEBUG(LX("addingMessageIdToMap").d("messageId", messageId));
                info = createDirectiveInfo(directive, std::move(result));
                {
                    lock_guard<std::mutex> lock(m_mutex);
                    m_directiveInfoMap[messageId] = info;
                    if (m_directiveInfoMap.size() > CAPABILITY_QUEUE_WARN_SIZE) {
                        ACSDK_WARN(LX("Expected directiveInfoMap size exceeded").d("namespace", directive->getNamespace()).d("name", directive->getName())
                            .d("size", m_directiveInfoMap.size()).d("limit", CAPABILITY_QUEUE_WARN_SIZE));
                    }
                }
                preHandleDirective(info);
            }
            bool CapabilityAgent::handleDirective(const string& messageId) {
                auto info = getDirectiveInfo(messageId);
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "messageIdNotFound").d("messageId", messageId));
                    return false;
                }
                handleDirective(info);
                return true;
            }
            void CapabilityAgent::cancelDirective(const string& messageId) {
                auto info = getDirectiveInfo(messageId);
                if (!info) {
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "messageIdNotFound").d("messageId", messageId));
                    return;
                }
                info->isCancelled = true;
                cancelDirective(info);
            }
            void CapabilityAgent::sendExceptionEncounteredAndReportFailed(shared_ptr<DirectiveInfo> info, const string& message, ExceptionErrorType type) {
                if (info) {
                    if (info->directive) {
                        m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);
                        removeDirective(info->directive->getMessageId());
                    } else { ACSDK_ERROR(LX("sendExceptionEncounteredAndReportFailed").d("reason", "infoHasNoDirective")); }
                    if (info->result) info->result->setFailed(message);
                    else { ACSDK_ERROR(LX("sendExceptionEncounteredAndReportFailed").d("reason", "infoHasNoResult")); }
                } else { ACSDK_ERROR(LX("sendExceptionEncounteredAndReportFailed").d("reason", "infoNotFound")); }
            }
            void CapabilityAgent::onDeregistered() {}
            void CapabilityAgent::removeDirective(const string& messageId) {
                lock_guard<std::mutex> lock(m_mutex);
                ACSDK_DEBUG(LX("removingMessageIdFromMap").d("messageId", messageId));
                m_directiveInfoMap.erase(messageId);
            }
            void CapabilityAgent::onFocusChanged(FocusState, MixingBehavior) {}
            const pair<string, string> CapabilityAgent::buildJsonEventString(
                const string& eventName,
                const string& dialogRequestIdString,
                const string& payload,
                const string& context) const {
                return avs::buildJsonEventString(m_namespace, eventName, dialogRequestIdString, payload, context);
            }
            shared_ptr<CapabilityAgent::DirectiveInfo> CapabilityAgent::getDirectiveInfo(const string& messageId) {
                lock_guard<std::mutex> lock(m_mutex);
                auto it = m_directiveInfoMap.find(messageId);
                if (it != m_directiveInfoMap.end()) return it->second;
                return nullptr;
            }
        }
    }
}