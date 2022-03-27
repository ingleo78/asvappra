#include <json/JSONUtils.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Setting.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSender.h>
#include <settings/SharedAVSSettingProtocol.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "LocaleHandler.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace json;
            using namespace jsonUtils;
            using namespace logger;
            static const string TAG("LocaleHandler");
            #define LX(event) LogEntry(TAG, event)
            static const string LOCALE_NAMESPACE = "System";
            static const string SET_LOCALE_DIRECTIVE = "SetLocales";
            static const string LOCALE_REPORT_EVENT = "LocalesReport";
            static const string LOCALE_CHANGED_EVENT = "LocalesChanged";
            static const string LOCALE_PAYLOAD_KEY = "locales";
            unique_ptr<LocaleHandler> LocaleHandler::create(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                            shared_ptr<LocalesSetting> localeSetting) {
                if (!exceptionSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
                    return nullptr;
                }
                if (!localeSetting) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullLocaleSetting"));
                    return nullptr;
                }
                return unique_ptr<LocaleHandler>(new LocaleHandler(exceptionSender, localeSetting));
            }
            LocaleHandler::LocaleHandler(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                         shared_ptr<LocalesSetting> localeSetting) : CapabilityAgent(LOCALE_NAMESPACE, exceptionSender),
                                         m_localeSetting{localeSetting} {}
            SettingEventMetadata LocaleHandler::getLocaleEventsMetadata() {
                return SettingEventMetadata{LOCALE_NAMESPACE, LOCALE_CHANGED_EVENT, LOCALE_REPORT_EVENT, LOCALE_PAYLOAD_KEY};
            }
            DirectiveHandlerConfiguration LocaleHandler::getConfiguration() const {
                return DirectiveHandlerConfiguration{{NamespaceAndName{LOCALE_NAMESPACE, SET_LOCALE_DIRECTIVE},
                                                      BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)}};
            }
            void LocaleHandler::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "nullDirective"));
                    return;
                }
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void LocaleHandler::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            void LocaleHandler::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullInfo"));
                    return;
                }
                if (!info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirective"));
                    return;
                }
                m_executor.submit([this, info]() { executeHandleDirective(info); });
            }
            void LocaleHandler::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            void LocaleHandler::executeHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("executeHandleDirectiveImmediately"));
                auto directiveName = info->directive->getName();
                if (SET_LOCALE_DIRECTIVE == directiveName) handleSetLocale(info);
                else {
                    ACSDK_ERROR(LX("executeHandleDirectiveFailed").d("reason", "unknownDirective").d("namespace", info->directive->getNamespace())
                        .d("name", info->directive->getName()));
                    const string errorMessage = "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();
                    sendProcessingDirectiveException(info, errorMessage);
                }
            }
            void LocaleHandler::handleSetLocale(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                auto locales = retrieveStringArray<DeviceLocales>(info->directive->getPayload(), LOCALE_PAYLOAD_KEY);
                if (locales.empty()) {
                    string errorMessage = "locale not specified for SetLocale";
                    ACSDK_ERROR(LX("handleSetLocaleFailed").d("reason", "localeMissing"));
                    sendProcessingDirectiveException(info, errorMessage);
                    return;
                }
                if (!m_localeSetting->setAvsChange(locales)) {
                    ACSDK_ERROR(LX("handleSetLocaleFailed").d("reason", "setRequestFailed"));
                    sendProcessingDirectiveException(info, "cannot apply locale change");
                    return;
                }
                if (info->result) info->result->setCompleted();
                if (info->directive) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
            void LocaleHandler::sendProcessingDirectiveException(
                shared_ptr<CapabilityAgent::DirectiveInfo> info,
                const string& errorMessage) {
                auto unparsedDirective = info->directive->getUnparsedDirective();
                m_exceptionEncounteredSender->sendExceptionEncountered(unparsedDirective, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                                       errorMessage);
                if (info->result) info->result->setFailed(errorMessage);
                if (info->directive && info->result) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
        }
    }
}