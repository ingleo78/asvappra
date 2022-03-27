#include <json/JSONUtils.h>
#include <settings/Setting.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSender.h>
#include <settings/SharedAVSSettingProtocol.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "TimeZoneHandler.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace json;
            using namespace jsonUtils;
            using namespace logger;
            using namespace rapidjson;
            static const string TAG("TimeZoneHandler");
            #define LX(event) LogEntry(TAG, event)
            static const string TIMEZONE_NAMESPACE = "System";
            static const string SET_TIMEZONE_DIRECTIVE = "SetTimeZone";
            static const string TIMEZONE_REPORT_EVENT = "TimeZoneReport";
            static const string TIMEZONE_CHANGED_EVENT = "TimeZoneChanged";
            static const string TIMEZONE_PAYLOAD_KEY = "timeZone";
            unique_ptr<TimeZoneHandler> TimeZoneHandler::create(shared_ptr<TimeZoneSetting> timeZoneSetting,
                                                                shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) {
                if (!timeZoneSetting) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullTimeZoneSetting"));
                    return nullptr;
                }
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncountered"));
                    return nullptr;
                }
                auto timezoneHandler = unique_ptr<TimeZoneHandler>(new TimeZoneHandler(timeZoneSetting, exceptionEncounteredSender));
                return timezoneHandler;
            }
            DirectiveHandlerConfiguration TimeZoneHandler::getConfiguration() const {
                return DirectiveHandlerConfiguration{{NamespaceAndName{TIMEZONE_NAMESPACE, SET_TIMEZONE_DIRECTIVE},
                                                      BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)}};
            }
            void TimeZoneHandler::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "directive is nullptr."));
                    return;
                }
                auto info = createDirectiveInfo(directive, nullptr);
                m_executor.submit([this, info]() { executeHandleDirectiveImmediately(info); });
            }
            void TimeZoneHandler::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            void TimeZoneHandler::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "info is nullptr."));
                    return;
                }
                m_executor.submit([this, info]() { executeHandleDirectiveImmediately(info); });
            }
            void TimeZoneHandler::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            void TimeZoneHandler::executeHandleDirectiveImmediately(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("executeHandleDirectiveImmediately"));
                auto& directive = info->directive;
                Document payload;
                payload.Parse(directive->getPayload().data());
                if (payload.HasParseError()) {
                    string errorMessage = "Unable to parse payload";
                    ACSDK_ERROR(LX("executeHandleDirectiveImmediatelyFailed").m(errorMessage));
                    sendProcessingDirectiveException(directive, errorMessage);
                    return;
                }
                auto directiveName = directive->getName();
                if (SET_TIMEZONE_DIRECTIVE == directiveName) handleSetTimeZone(directive, payload);
            }
            bool TimeZoneHandler::handleSetTimeZone(const shared_ptr<AVSDirective>& directive, const Document& payload) {
                string timeZoneValue;
                if (!retrieveValue(payload, TIMEZONE_PAYLOAD_KEY, &timeZoneValue)) {
                    string errorMessage = "timeZone not specified for SetTimeZone";
                    ACSDK_ERROR(LX("handleSetTimeZoneFailed").m(errorMessage));
                    sendProcessingDirectiveException(directive, errorMessage);
                    return false;
                }
                return m_timeZoneSetting->setAvsChange(timeZoneValue);
            }
            void TimeZoneHandler::sendProcessingDirectiveException(
                const shared_ptr<AVSDirective>& directive,
                const string& errorMessage) {
                auto unparsedDirective = directive->getUnparsedDirective();
                ACSDK_ERROR(LX("sendProcessingDirectiveException").d("errorMessage", errorMessage).d("unparsedDirective", unparsedDirective));
                m_exceptionEncounteredSender->sendExceptionEncountered(unparsedDirective, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, errorMessage);
            }
            SettingEventMetadata TimeZoneHandler::getTimeZoneMetadata() {
                return SettingEventMetadata{TIMEZONE_NAMESPACE, TIMEZONE_CHANGED_EVENT, TIMEZONE_REPORT_EVENT, TIMEZONE_PAYLOAD_KEY};
            }
            TimeZoneHandler::TimeZoneHandler(shared_ptr<TimeZoneSetting> timeZoneSetting, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                             CapabilityAgent(TIMEZONE_NAMESPACE, exceptionEncounteredSender), m_timeZoneSetting{timeZoneSetting} {}
        }
    }
}