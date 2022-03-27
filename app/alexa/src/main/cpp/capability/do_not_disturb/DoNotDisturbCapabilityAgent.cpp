#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/CapabilityConfiguration.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <settings/SettingEventMetadata.h>
#include "DNDMessageRequest.h"
#include "DNDSettingProtocol.h"
#include "DoNotDisturbCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace doNotDisturb {
            using namespace jsonUtils;
            static const string TAG{"DoNotDisturbCapabilityAgent"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE = "Alexa.DoNotDisturb";
            static const NamespaceAndName DIRECTIVE_SETDONOTDISTURB{NAMESPACE, "SetDoNotDisturb"};
            static const NamespaceAndName EVENT_DONOTDISTURBCHANGED{NAMESPACE, "DoNotDisturbChanged"};
            static const NamespaceAndName EVENT_REPORTDONOTDISTURB{NAMESPACE, "ReportDoNotDisturb"};
            static const string DND_JSON_INTERFACE_TYPE = "AlexaInterface";
            static const string DND_JSON_INTERFACE_NAME = "Alexa.DoNotDisturb";
            static const string DND_JSON_INTERFACE_VERSION = "1.0";
            static constexpr char JSON_KEY_ENABLED[] = "enabled";
            shared_ptr<DoNotDisturbCapabilityAgent> DoNotDisturbCapabilityAgent::create(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                                        shared_ptr<MessageSenderInterface> messageSender,
                                                                                        shared_ptr<DeviceSettingStorageInterface> settingsStorage) {
                if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "messageSenderNull"));
                    return nullptr;
                }
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "exceptionEncounteredSenderNull"));
                    return nullptr;
                }
                if (!settingsStorage) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "settingsStorageNull"));
                    return nullptr;
                }
                auto dndCA = shared_ptr<DoNotDisturbCapabilityAgent>(new DoNotDisturbCapabilityAgent(exceptionEncounteredSender, messageSender));
                if (!dndCA->initialize(settingsStorage)) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Initialization failed."));
                    return nullptr;
                }
                return dndCA;
            }
            DoNotDisturbCapabilityAgent::DoNotDisturbCapabilityAgent(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                     shared_ptr<MessageSenderInterface> messageSender) : CapabilityAgent{NAMESPACE, exceptionEncounteredSender},
                                                                     RequiresShutdown{"acsdkDoNotDisturb"}, m_messageSender{messageSender}, m_isConnected{false},
                                                                     m_hasOfflineChanges{false} {
                generateCapabilityConfiguration();
            }
            shared_ptr<settings::DoNotDisturbSetting> DoNotDisturbCapabilityAgent::getDoNotDisturbSetting() const {
                return m_dndModeSetting;
            }
            SettingEventMetadata DoNotDisturbCapabilityAgent::getDoNotDisturbEventsMetadata() {
                return SettingEventMetadata{NAMESPACE, EVENT_DONOTDISTURBCHANGED.name, EVENT_REPORTDONOTDISTURB.name, JSON_KEY_ENABLED};
            }
            bool DoNotDisturbCapabilityAgent::initialize(
                shared_ptr<DeviceSettingStorageInterface> settingsStorage) {
                auto metadata = getDoNotDisturbEventsMetadata();
                auto protocol = DNDSettingProtocol::create(metadata, shared_from_this(), settingsStorage);
                m_dndModeSetting = Setting<bool>::create(false, move(protocol));
                return m_dndModeSetting != nullptr;
            }
            void DoNotDisturbCapabilityAgent::generateCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, DND_JSON_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, DND_JSON_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, DND_JSON_INTERFACE_VERSION});
                m_capabilityConfigurations.insert(std::make_shared<CapabilityConfiguration>(configMap));
            }
            DirectiveHandlerConfiguration DoNotDisturbCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                configuration[DIRECTIVE_SETDONOTDISTURB] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                return configuration;
            }
            void DoNotDisturbCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void DoNotDisturbCapabilityAgent::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
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
            void DoNotDisturbCapabilityAgent::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                m_executor.submit([this, info] {
                    const string directiveName = info->directive->getName();
                    Document payload(kObjectType);
                    if (!parseDirectivePayload(info->directive->getPayload(), &payload)) {
                        sendExceptionEncounteredAndReportFailed(info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    if (directiveName == DIRECTIVE_SETDONOTDISTURB.name) {
                        if (!handleSetDoNotDisturbDirective(info, payload)) return;
                    } else {
                        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "Unknown directive."));
                        sendExceptionEncounteredAndReportFailed(info, "Unexpected Directive", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    if (info->result) info->result->setCompleted();
                    removeDirective(info->directive->getMessageId());
                });
            }
            void DoNotDisturbCapabilityAgent::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (info && info->directive) removeDirective(info->directive->getMessageId());
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> DoNotDisturbCapabilityAgent::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            void DoNotDisturbCapabilityAgent::doShutdown() {
                m_dndModeSetting.reset();
            }
            bool DoNotDisturbCapabilityAgent::handleSetDoNotDisturbDirective(
                shared_ptr<CapabilityAgent::DirectiveInfo> info,
                Document& document) {
                bool state = false;
                if (!retrieveValue(document, JSON_KEY_ENABLED, &state)) {
                    string errorMessage = "'enabled' value not specified for SetDoNotDisturb";
                    ACSDK_ERROR(LX("handleSetDoNotDisturbDirectiveFailed").m(errorMessage));
                    sendExceptionEncounteredAndReportFailed(info, errorMessage);
                    return false;
                }
                m_dndModeSetting->setAvsChange(state);
                return true;
            }
            shared_future<MessageRequestObserverInterface::Status> DoNotDisturbCapabilityAgent::sendDNDEvent(const string& eventName, const string& value) {
                const string EMPTY_DIALOG_REQUEST_ID;
                JsonGenerator payload;
                if (!payload.addRawJsonMember(JSON_KEY_ENABLED, value)) {
                    ACSDK_ERROR(LX("sendEventFailed").d("reason", "failedToAddValueToPayload"));
                    promise<MessageRequestObserverInterface::Status> promise;
                    promise.set_value(MessageRequestObserverInterface::Status::INTERNAL_ERROR);
                    return promise.get_future();
                }
                auto jsonEventString = buildJsonEventString(eventName, EMPTY_DIALOG_REQUEST_ID, payload.toString()).second;
                auto request = make_shared<DNDMessageRequest>(jsonEventString);
                m_messageSender->sendMessage(request);
                return request->getCompletionFuture();
            }
            shared_future<bool> DoNotDisturbCapabilityAgent::sendChangedEvent(const string& value) {
                promise<bool> promise;
                {
                    lock_guard<mutex> guard(m_connectedStateMutex);
                    if (!m_isConnected) {
                        m_hasOfflineChanges = true;
                        promise.set_value(false);
                        return promise.get_future();
                    }
                    m_hasOfflineChanges = false;
                }
                m_executor.submit([this, value]() {
                    MessageRequestObserverInterface::Status status = sendDNDEvent(EVENT_DONOTDISTURBCHANGED.name, value).get();
                    bool isSucceeded = MessageRequestObserverInterface::Status::SUCCESS == status || MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT == status;
                    if (!isSucceeded) sendDNDEvent(EVENT_REPORTDONOTDISTURB.name, m_dndModeSetting->get() ? "true" : "false");
                });
                promise.set_value(true);
                return promise.get_future();
            }
            shared_future<bool> DoNotDisturbCapabilityAgent::sendReportEvent(const string& value) {
                promise<bool> promise;
                if (!m_isConnected) {
                    promise.set_value(false);
                    return promise.get_future();
                }
                m_executor.submit([this, value]() { sendDNDEvent(EVENT_REPORTDONOTDISTURB.name, value); });
                promise.set_value(true);
                return promise.get_future();
            }
            shared_future<bool> DoNotDisturbCapabilityAgent::sendStateReportEvent(const string& payload) {
                promise<bool> promise;
                promise.set_value(false);
                return promise.get_future();
            }
            void DoNotDisturbCapabilityAgent::cancel() {}
            void DoNotDisturbCapabilityAgent::onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status,
                                                                        const ConnectionStatusObserverInterface::ChangedReason reason) {
                lock_guard<mutex> guard(m_connectedStateMutex);
                m_isConnected = status == ConnectionStatusObserverInterface::Status::CONNECTED;
                if (m_isConnected) {
                    string modeString = m_dndModeSetting->get() ? "true" : "false";
                    if (m_hasOfflineChanges) m_dndModeSetting->setLocalChange(m_dndModeSetting->get());
                    else sendReportEvent(modeString);
                }
            }
        }
    }
}