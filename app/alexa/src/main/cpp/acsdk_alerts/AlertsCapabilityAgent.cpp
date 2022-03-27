#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <avs/CapabilityConfiguration.h>
#include <avs/MessageRequest.h>
#include <avs/SpeakerConstants/SpeakerConstants.h>
#include <util/file/FileUtils.h>
#include <json/JSONUtils.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include <timing/TimeUtils.h>
#include <settings/Setting.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSender.h>
#include <settings/SharedAVSSettingProtocol.h>
#include <settings/Storage/DeviceSettingStorageInterface.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "Alarm.h"
#include "AlertsCapabilityAgent.h"
#include "Reminder.h"
#include "Timer.h"


namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace chrono;
        using namespace acsdkAlertsInterfaces;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace configuration;
        using namespace file;
        using namespace json;
        using namespace jsonUtils;
        using namespace logger;
        using namespace storage;
        using namespace metrics;
        using namespace timing;
        using namespace sdkInterfaces;
        using namespace audio;
        using namespace certifiedSender;
        using namespace rapidjson;
        using namespace registrationManager;
        using namespace settings;
        using namespace types;
        static const string ALERTS_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
        static const string ALERTS_CAPABILITY_INTERFACE_NAME = "Alerts";
        static const string ALERTS_CAPABILITY_INTERFACE_VERSION = "1.4";
        static const string KEY_TYPE = "type";
        static const string DIRECTIVE_NAME_SET_ALERT = "SetAlert";
        static const string DIRECTIVE_NAME_DELETE_ALERT = "DeleteAlert";
        static const string DIRECTIVE_NAME_DELETE_ALERTS = "DeleteAlerts";
        static const string DIRECTIVE_NAME_SET_VOLUME = "SetVolume";
        static const string DIRECTIVE_NAME_ADJUST_VOLUME = "AdjustVolume";
        static const string DIRECTIVE_NAME_SET_ALARM_VOLUME_RAMP = "SetAlarmVolumeRamp";
        static const string SET_ALERT_SUCCEEDED_EVENT_NAME = "SetAlertSucceeded";
        static const string SET_ALERT_FAILED_EVENT_NAME = "SetAlertFailed";
        static const string DELETE_ALERT_SUCCEEDED_EVENT_NAME = "DeleteAlertSucceeded";
        static const string DELETE_ALERT_FAILED_EVENT_NAME = "DeleteAlertFailed";
        static const string ALERT_STARTED_EVENT_NAME = "AlertStarted";
        static const string ALERT_STOPPED_EVENT_NAME = "AlertStopped";
        static const string ALERT_ENTERED_FOREGROUND_EVENT_NAME = "AlertEnteredForeground";
        static const string ALERT_ENTERED_BACKGROUND_EVENT_NAME = "AlertEnteredBackground";
        static const string ALERT_VOLUME_CHANGED_EVENT_NAME = "VolumeChanged";
        static const string ALERT_DELETE_ALERTS_SUCCEEDED_EVENT_NAME = "DeleteAlertsSucceeded";
        static const string ALERT_DELETE_ALERTS_FAILED_EVENT_NAME = "DeleteAlertsFailed";
        static const string ALERT_ALARM_VOLUME_RAMP_CHANGED_EVENT_NAME = "AlarmVolumeRampChanged";
        static const string ALERT_REPORT_ALARM_VOLUME_RAMP_EVENT_NAME = "AlarmVolumeRampReport";
        static const string EVENT_PAYLOAD_TOKEN_KEY = "token";
        static const string EVENT_PAYLOAD_TOKENS_KEY = "tokens";
        static const string DIRECTIVE_PAYLOAD_TOKEN_KEY = "token";
        static const string DIRECTIVE_PAYLOAD_TOKENS_KEY = "tokens";
        static const string DIRECTIVE_PAYLOAD_VOLUME = "volume";
        static const string DIRECTIVE_PAYLOAD_ALARM_VOLUME_RAMP = "alarmVolumeRamp";
        static const string AVS_CONTEXT_HEADER_NAMESPACE_VALUE_KEY = "Alerts";
        static const string AVS_CONTEXT_HEADER_NAME_VALUE_KEY = "AlertsState";
        static const string AVS_CONTEXT_ALL_ALERTS_TOKEN_KEY = "allAlerts";
        static const string AVS_CONTEXT_ACTIVE_ALERTS_TOKEN_KEY = "activeAlerts";
        static const string AVS_CONTEXT_ALERT_TOKEN_KEY = "token";
        static const string AVS_CONTEXT_ALERT_TYPE_KEY = "type";
        static const string AVS_CONTEXT_ALERT_SCHEDULED_TIME_KEY = "scheduledTime";
        static const string AVS_PAYLOAD_VOLUME_KEY = "volume";
        static const string AVS_PAYLOAD_ALARM_VOLUME_RAMP_KEY = "alarmVolumeRamp";
        static const string AVS_PAYLOAD_ERROR_KEY = "error";
        static const string AVS_PAYLOAD_ERROR_TYPE_KEY = "type";
        static const string AVS_PAYLOAD_ERROR_MESSAGE_KEY = "message";
        static const string EMPTY_DIALOG_REQUEST_ID = "";
        static const string NAMESPACE = "Alerts";
        static const NamespaceAndName SET_ALERT{NAMESPACE, DIRECTIVE_NAME_SET_ALERT};
        static const NamespaceAndName DELETE_ALERT{NAMESPACE, DIRECTIVE_NAME_DELETE_ALERT};
        static const NamespaceAndName DELETE_ALERTS{NAMESPACE, DIRECTIVE_NAME_DELETE_ALERTS};
        static const NamespaceAndName SET_VOLUME{NAMESPACE, DIRECTIVE_NAME_SET_VOLUME};
        static const NamespaceAndName ADJUST_VOLUME{NAMESPACE, DIRECTIVE_NAME_ADJUST_VOLUME};
        static const NamespaceAndName SET_ALARM_VOLUME_RAMP{NAMESPACE, DIRECTIVE_NAME_SET_ALARM_VOLUME_RAMP};
        static const string TAG("AlertsCapabilityAgent");
        static const string ALERT_METRIC_SOURCE_PREFIX = "ALERT-";
        static const string FAILED_SNOOZE_ALERT = "failedToSnoozeAlert";
        static const string FAILED_SCHEDULE_ALERT = "failedToScheduleAlert";
        static const string INVALID_PAYLOAD_FOR_SET_ALARM_VOLUME = "invalidPayloadToSetAlarmRamping";
        static const string INVALID_PAYLOAD_FOR_CHANGE_ALARM_VOLUME = "invalidPayloadToChangeAlarmVolume";
        #define LX(event) LogEntry(TAG, event)
        static shared_ptr<CapabilityConfiguration> getAlertsCapabilityConfiguration();
        static Value buildAllAlertsContext(const vector<Alert::ContextInfo>& alertsInfo, Document::AllocatorType& allocator) {
            Value alertArray{rapidjson::kArrayType};
            for (const auto& info : alertsInfo) {
                rapidjson::Value alertJson;
                alertJson.SetObject();
                Value avs_context_alert_token_key{AVS_CONTEXT_ALERT_TOKEN_KEY.data(), strlen(AVS_CONTEXT_ALERT_TOKEN_KEY.data())};
                Value info_token{info.token.data(), strlen(info.token.data())};
                Value avs_context_alert_type_key{AVS_CONTEXT_ALERT_TYPE_KEY.data(), strlen(AVS_CONTEXT_ALERT_TYPE_KEY.data())};
                Value info_type{info.type.data(), strlen(info.type.data())};
                Value avs_context_alert_scheduled_time_key{AVS_CONTEXT_ALERT_SCHEDULED_TIME_KEY.data(), strlen(AVS_CONTEXT_ALERT_SCHEDULED_TIME_KEY.data())};
                Value info_scheduledTime_ISO_8601{info.scheduledTime_ISO_8601.data(), strlen(info.scheduledTime_ISO_8601.data())};
                alertJson.AddMember(avs_context_alert_token_key, info_token);
                alertJson.AddMember(avs_context_alert_type_key, info_type);
                alertJson.AddMember(avs_context_alert_scheduled_time_key, info_scheduledTime_ISO_8601);
                alertArray.PushBack(alertJson);
            }
            return alertArray;
        }
        static rapidjson::Value buildActiveAlertsContext(
            const std::vector<Alert::ContextInfo>& alertsInfo,
            Document::AllocatorType& allocator) {
            rapidjson::Value alertArray(rapidjson::kArrayType);
            if (!alertsInfo.empty()) {
                auto& info = alertsInfo[0];
                rapidjson::Value alertJson;
                alertJson.SetObject();
                Value avs_context_alert_token_key{AVS_CONTEXT_ALERT_TOKEN_KEY.data(), strlen(AVS_CONTEXT_ALERT_TOKEN_KEY.data())};
                Value info_token{info.token.data(), strlen(info.token.data())};
                Value avs_context_alert_type_key{AVS_CONTEXT_ALERT_TYPE_KEY.data(), strlen(AVS_CONTEXT_ALERT_TYPE_KEY.data())};
                Value info_type{info.type.data(), strlen(info.type.data())};
                Value avs_context_alert_scheduled_time_key{AVS_CONTEXT_ALERT_SCHEDULED_TIME_KEY.data(), strlen(AVS_CONTEXT_ALERT_SCHEDULED_TIME_KEY.data())};
                Value info_scheduledTime_ISO_8601{info.scheduledTime_ISO_8601.data(), strlen(info.scheduledTime_ISO_8601.data())};
                alertJson.AddMember(avs_context_alert_token_key, info_token);
                alertJson.AddMember(avs_context_alert_type_key, info_type);
                alertJson.AddMember(avs_context_alert_scheduled_time_key, info_scheduledTime_ISO_8601);
                alertArray.PushBack(alertJson);
            }
            return alertArray;
        }
        static void submitMetric(
            const shared_ptr<MetricRecorderInterface>& metricRecorder,
            const string& eventName,
            int count) {
            if (!metricRecorder) return;
            auto metricEvent = MetricEventBuilder{}.setActivityName(ALERT_METRIC_SOURCE_PREFIX + eventName)
                                   .addDataPoint(DataPointCounterBuilder{}.setName(eventName).increment(count).build()).build();
            if (metricEvent == nullptr) {
                ACSDK_ERROR(LX("Error creating metric."));
                return;
            }
            recordMetric(metricRecorder, metricEvent);
        }
        shared_ptr<AlertsCapabilityAgent> AlertsCapabilityAgent::create(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                                                        shared_ptr<certifiedSender::CertifiedSender> certifiedMessageSender,
                                                                        shared_ptr<FocusManagerInterface> focusManager, shared_ptr<SpeakerManagerInterface> speakerManager,
                                                                        shared_ptr<ContextManagerInterface> contextManager,
                                                                        shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                        shared_ptr<AlertStorageInterface> alertStorage, shared_ptr<AlertsAudioFactoryInterface> alertsAudioFactory,
                                                                        shared_ptr<RendererInterface> alertRenderer, shared_ptr<CustomerDataManager> dataManager,
                                                                        shared_ptr<AlarmVolumeRampSetting> alarmVolumeRampSetting,
                                                                        shared_ptr<DeviceSettingsManager> settingsManager, shared_ptr<MetricRecorderInterface> metricRecorder,
                                                                        bool startAlertSchedulingOnInitialization) {
            if (!alarmVolumeRampSetting) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullAlarmVolumeRampSetting"));
                return nullptr;
            }
            if (!settingsManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullSettingsManager"));
                return nullptr;
            }
            auto alertsCA = shared_ptr<AlertsCapabilityAgent>(new AlertsCapabilityAgent(messageSender, certifiedMessageSender, focusManager, speakerManager,
                                                              contextManager, exceptionEncounteredSender, alertStorage, alertsAudioFactory, alertRenderer,
                                                              dataManager, alarmVolumeRampSetting, settingsManager, metricRecorder));
            if (!alertsCA->initialize(startAlertSchedulingOnInitialization)) {
                ACSDK_ERROR(LX("createFailed").d("reason", "Initialization error."));
                return nullptr;
            }
            focusManager->addObserver(alertsCA);
            connectionManager->addConnectionStatusObserver(alertsCA);
            speakerManager->addSpeakerManagerObserver(alertsCA);
            return alertsCA;
        }
        DirectiveHandlerConfiguration AlertsCapabilityAgent::getConfiguration() const {
            auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
            auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
            avsCommon::avs::DirectiveHandlerConfiguration configuration;
            configuration[SET_ALERT] = neitherNonBlockingPolicy;
            configuration[DELETE_ALERT] = neitherNonBlockingPolicy;
            configuration[DELETE_ALERTS] = neitherNonBlockingPolicy;
            configuration[SET_VOLUME] = audioNonBlockingPolicy;
            configuration[ADJUST_VOLUME] = audioNonBlockingPolicy;
            configuration[SET_ALARM_VOLUME_RAMP] = audioNonBlockingPolicy;
            return configuration;
        }
        void AlertsCapabilityAgent::handleDirectiveImmediately(std::shared_ptr<avsCommon::avs::AVSDirective> directive) {
            if (!directive) { ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "directive is nullptr.")); }
            auto info = createDirectiveInfo(directive, nullptr);
            m_executor.submit([this, info]() { executeHandleDirectiveImmediately(info); });
        }
        void AlertsCapabilityAgent::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {}
        void AlertsCapabilityAgent::handleDirective(std::shared_ptr<DirectiveInfo> info) {
            if (!info) { ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "info is nullptr.")); }
            m_executor.submit([this, info]() { executeHandleDirectiveImmediately(info); });
        }
        void AlertsCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {}
        void AlertsCapabilityAgent::onDeregistered() {}
        void AlertsCapabilityAgent::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
            m_executor.submit([this, status, reason]() { executeOnConnectionStatusChanged(status, reason); });
        }
        void AlertsCapabilityAgent::onFocusChanged(FocusState focusState, MixingBehavior behavior) {
            ACSDK_DEBUG9(LX("onFocusChanged").d("focusState", focusState));
            m_executor.submit([this, focusState]() { executeOnFocusChanged(focusState); });
        }
        void AlertsCapabilityAgent::onFocusChanged(const string& channelName, FocusState newFocus) {
            m_executor.submit([this, channelName, newFocus]() { executeOnFocusManagerFocusChanged(channelName, newFocus); });
        }
        void AlertsCapabilityAgent::onAlertStateChange(const string& alertToken, const string& alertType, AlertObserverInterface::State state, const string& reason) {
            ACSDK_DEBUG9(LX("onAlertStateChange").d("alertToken", alertToken).d("alertType", alertType).d("state", state).d("reason", reason));
            m_executor.submit([this, alertToken, alertType, state, reason]() { executeOnAlertStateChange(alertToken, alertType, state, reason); });
        }
        void AlertsCapabilityAgent::addObserver(shared_ptr<AlertObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                return;
            }
            m_executor.submit([this, observer]() { executeAddObserver(observer); });
        }
        void AlertsCapabilityAgent::removeObserver(std::shared_ptr<AlertObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                return;
            }
            m_executor.submit([this, observer]() { executeRemoveObserver(observer); });
        }
        void AlertsCapabilityAgent::removeAllAlerts() {
            m_executor.submit([this]() { executeRemoveAllAlerts(); });
        }
        void AlertsCapabilityAgent::onLocalStop() {
            ACSDK_DEBUG9(LX("onLocalStop"));
            m_executor.submitToFront([this]() { executeOnLocalStop(); });
        }
        AlertsCapabilityAgent::AlertsCapabilityAgent(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<certifiedSender::CertifiedSender> certifiedMessageSender,
                                                     shared_ptr<FocusManagerInterface> focusManager, shared_ptr<SpeakerManagerInterface> speakerManager,
                                                     shared_ptr<ContextManagerInterface> contextManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                     shared_ptr<AlertStorageInterface> alertStorage, shared_ptr<AlertsAudioFactoryInterface> alertsAudioFactory,
                                                     shared_ptr<RendererInterface> alertRenderer, shared_ptr<CustomerDataManager> dataManager,
                                                     shared_ptr<AlarmVolumeRampSetting> alarmVolumeRampSetting, shared_ptr<DeviceSettingsManager> settingsManager,
                                                     shared_ptr<MetricRecorderInterface> metricRecorder) : CapabilityAgent("Alerts", exceptionEncounteredSender),
                                                     RequiresShutdown("AlertsCapabilityAgent"), CustomerDataHandler(dataManager), m_metricRecorder{metricRecorder},
                                                     m_messageSender{messageSender}, m_certifiedSender{certifiedMessageSender}, m_focusManager{focusManager},
                                                     m_speakerManager{speakerManager}, m_contextManager{contextManager}, m_isConnected{false},
                                                     m_alertScheduler{alertStorage, alertRenderer, ALERT_PAST_DUE_CUTOFF_MINUTES, metricRecorder},
                                                     m_alertsAudioFactory{alertsAudioFactory}, m_contentChannelIsActive{false}, m_commsChannelIsActive{false},
                                                     m_alertIsSounding{false}, m_alarmVolumeRampSetting{alarmVolumeRampSetting}, m_settingsManager{settingsManager} {
            m_capabilityConfigurations.insert(getAlertsCapabilityConfiguration());
        }
        shared_ptr<CapabilityConfiguration> getAlertsCapabilityConfiguration() {
            unordered_map<string, string> configMap;
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, ALERTS_CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, ALERTS_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, ALERTS_CAPABILITY_INTERFACE_VERSION});
            return make_shared<CapabilityConfiguration>(configMap);
        }
        void AlertsCapabilityAgent::doShutdown() {
            m_executor.shutdown();
            releaseChannel();
            m_messageSender.reset();
            m_certifiedSender.reset();
            m_focusManager.reset();
            m_contextManager.reset();
            m_observers.clear();
            m_alertScheduler.shutdown();
        }
        bool AlertsCapabilityAgent::initialize(bool startAlertSchedulingOnInitialization) {
            if (!initializeAlerts(startAlertSchedulingOnInitialization)) {
                ACSDK_ERROR(LX("initializeFailed").m("Could not initialize alerts."));
                return false;
            }
            if (!getAlertVolumeSettings(&m_lastReportedSpeakerSettings)) return false;
            updateContextManager();
            return true;
        }
        bool AlertsCapabilityAgent::initializeAlerts(bool startAlertSchedulingOnInitialization) {
            return m_alertScheduler.initialize(shared_from_this(), m_settingsManager, startAlertSchedulingOnInitialization);
        }
        SettingEventMetadata AlertsCapabilityAgent::getAlarmVolumeRampMetadata() {
            return SettingEventMetadata{NAMESPACE, ALERT_ALARM_VOLUME_RAMP_CHANGED_EVENT_NAME, ALERT_REPORT_ALARM_VOLUME_RAMP_EVENT_NAME,
                                        AVS_PAYLOAD_ALARM_VOLUME_RAMP_KEY};
        }
        bool AlertsCapabilityAgent::handleSetAlert(const shared_ptr<AVSDirective>& directive, const Document& payload, string* alertToken) {
            ACSDK_DEBUG9(LX("handleSetAlert"));
            string alertType;
            if (!retrieveValue(payload, KEY_TYPE.data(), &alertType)) {
                string errorMessage = "Alert type not specified for SetAlert";
                ACSDK_ERROR(LX("handleSetAlertFailed").m(errorMessage));
                sendProcessingDirectiveException(directive, errorMessage);
                return false;
            }
            shared_ptr<Alert> parsedAlert;
            if (Alarm::getTypeNameStatic() == alertType) {
                parsedAlert = make_shared<Alarm>(m_alertsAudioFactory->alarmDefault(), m_alertsAudioFactory->alarmShort(), m_settingsManager);
            } else if (Timer::getTypeNameStatic() == alertType) {
                parsedAlert = make_shared<Timer>(m_alertsAudioFactory->timerDefault(), m_alertsAudioFactory->timerShort(), m_settingsManager);
            } else if (Reminder::getTypeNameStatic() == alertType) {
                parsedAlert = make_shared<Reminder>(m_alertsAudioFactory->reminderDefault(), m_alertsAudioFactory->reminderShort(), m_settingsManager);
            }
            if (!parsedAlert) {
                ACSDK_ERROR(LX("handleSetAlertFailed").d("reason", "unknown alert type").d("type:", alertType));
                return false;
            }
            string errorMessage;
            auto parseStatus = parsedAlert->parseFromJson(payload, &errorMessage);
            if (Alert::ParseFromJsonStatus::MISSING_REQUIRED_PROPERTY == parseStatus) {
                sendProcessingDirectiveException(directive, "Missing required property.");
                return false;
            } else if (Alert::ParseFromJsonStatus::INVALID_VALUE == parseStatus) {
                sendProcessingDirectiveException(directive, "Invalid value.");
                return false;
            }
            *alertToken = parsedAlert->getToken();
            if (m_alertScheduler.isAlertActive(parsedAlert)) {
                if (!m_alertScheduler.snoozeAlert(parsedAlert->getToken(), parsedAlert->getScheduledTime_ISO_8601())) {
                    ACSDK_ERROR(LX("handleSetAlertFailed").d("reason", "failed to snooze alert"));
                    submitMetric(m_metricRecorder, FAILED_SNOOZE_ALERT, 1);
                    return false;
                }
                executeNotifyObservers(parsedAlert->getToken(), parsedAlert->getTypeName(),State::SCHEDULED_FOR_LATER, parsedAlert->getScheduledTime_ISO_8601());
                submitMetric(m_metricRecorder, FAILED_SNOOZE_ALERT, 0);
                submitMetric(m_metricRecorder, "alarmSnoozeCount", 1);
                return true;
            }
            if (!m_alertScheduler.scheduleAlert(parsedAlert)) {
                submitMetric(m_metricRecorder, FAILED_SCHEDULE_ALERT, 1);
                return false;
            }
            submitMetric(m_metricRecorder, FAILED_SCHEDULE_ALERT, 0);
            executeNotifyObservers(parsedAlert->getToken(), parsedAlert->getTypeName(),State::SCHEDULED_FOR_LATER, parsedAlert->getScheduledTime_ISO_8601());
            updateContextManager();
            return true;
        }
        bool AlertsCapabilityAgent::handleDeleteAlert(const shared_ptr<AVSDirective>& directive, const Document& payload, string* alertToken) {
            ACSDK_DEBUG5(LX(__func__));
            if (!retrieveValue(payload, DIRECTIVE_PAYLOAD_TOKEN_KEY, alertToken)) {
                ACSDK_ERROR(LX("handleDeleteAlertFailed").m("Could not find token in the payload."));
                return false;
            }
            if (!m_alertScheduler.deleteAlert(*alertToken)) {
                submitMetric(m_metricRecorder, "failedToDeleteAlert", 1);
                return false;
            }
            submitMetric(m_metricRecorder, "failedToDeleteAlert", 0);
            updateContextManager();
            return true;
        }
        bool AlertsCapabilityAgent::handleDeleteAlerts(const shared_ptr<AVSDirective>& directive, const Document& payload) {
            ACSDK_DEBUG5(LX(__func__));
            list<string> alertTokens;
            auto tokensPayload = payload.FindMember(DIRECTIVE_PAYLOAD_TOKENS_KEY.c_str());
            if (tokensPayload == payload.MemberEnd()) {
                ACSDK_ERROR(LX("handleDeleteAlertsFailed").d("reason", "Cannot find tokens in payload"));
                return false;
            }
            if (!tokensPayload->value.IsArray()) {
                ACSDK_ERROR(LX("handleDeleteAlertsFailed").d("reason", "value is expected to be an array").d("key", DIRECTIVE_PAYLOAD_TOKENS_KEY.c_str()));
                return false;
            }
            auto tokenArray = tokensPayload->value.GetArray();
            for (SizeType i = 0; i < tokenArray.Size(); i++) {
                string token;
                ValueMemoryPool data = tokenArray[i];
                if (!convertToValue(data, &token)) {
                    ACSDK_WARN(LX("handleDeleteAlertsFailed").d("reason", "invalid token in payload"));
                    continue;
                }
                alertTokens.push_back(token);
            }
            if (!m_alertScheduler.deleteAlerts(alertTokens)) {
                sendBulkEvent(ALERT_DELETE_ALERTS_FAILED_EVENT_NAME, alertTokens, true);
                return false;
            }
            sendBulkEvent(ALERT_DELETE_ALERTS_SUCCEEDED_EVENT_NAME, alertTokens, true);
            updateContextManager();
            return true;
        }
        bool AlertsCapabilityAgent::handleSetVolume(const shared_ptr<AVSDirective>& directive, const Document& payload) {
            ACSDK_DEBUG5(LX(__func__));
            int64_t volumeValue = 0;
            if (!retrieveValue(payload, DIRECTIVE_PAYLOAD_VOLUME, &volumeValue)) {
                ACSDK_ERROR(LX("handleSetVolumeFailed").m("Could not find volume in the payload."));
                submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_CHANGE_ALARM_VOLUME, 1);
                return false;
            }
            submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_CHANGE_ALARM_VOLUME, 0);
            setNextAlertVolume(volumeValue);
            return true;
        }
        bool AlertsCapabilityAgent::handleAdjustVolume(const shared_ptr<AVSDirective>& directive, const Document& payload) {
            ACSDK_DEBUG5(LX(__func__));
            int64_t adjustValue = 0;
            if (!retrieveValue(payload, DIRECTIVE_PAYLOAD_VOLUME, &adjustValue)) {
                ACSDK_ERROR(LX("handleAdjustVolumeFailed").m("Could not find volume in the payload."));
                submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_CHANGE_ALARM_VOLUME, 1);
                return false;
            }
            submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_CHANGE_ALARM_VOLUME, 0);
            SpeakerInterface::SpeakerSettings speakerSettings;
            if (!m_speakerManager->getSpeakerSettings(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, &speakerSettings).get()) {
                ACSDK_ERROR(LX("handleAdjustVolumeFailed").m("Could not retrieve speaker volume."));
                return false;
            }
            int64_t volume = adjustValue + speakerSettings.volume;
            setNextAlertVolume(volume);
            return true;
        }
        bool AlertsCapabilityAgent::handleSetAlarmVolumeRamp(const shared_ptr<AVSDirective>& directive, const Document& payload) {
            string jsonValue;
            if (!retrieveValue(payload, DIRECTIVE_PAYLOAD_ALARM_VOLUME_RAMP, &jsonValue)) {
                string errorMessage = DIRECTIVE_PAYLOAD_ALARM_VOLUME_RAMP + " not specified for " + DIRECTIVE_NAME_SET_ALARM_VOLUME_RAMP;
                ACSDK_ERROR(LX("handleSetAlarmVolumeRampFailed").m(errorMessage));
                sendProcessingDirectiveException(directive, errorMessage);
                submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_SET_ALARM_VOLUME, 1);
                return false;
            }
            submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_SET_ALARM_VOLUME, 0);
            auto value = getAlarmVolumeRampDefault();
            stringstream ss{jsonValue};
            ss >> value;
            if (ss.fail()) {
                ACSDK_ERROR(LX(__func__).d("error", "invalid").d("value", jsonValue));
                submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_CHANGE_ALARM_VOLUME, 1);
                return false;
            }
            submitMetric(m_metricRecorder, INVALID_PAYLOAD_FOR_CHANGE_ALARM_VOLUME, 0);
            return m_alarmVolumeRampSetting->setAvsChange(value);
        }
        void AlertsCapabilityAgent::sendEvent(const std::string& eventName, const std::string& alertToken, bool isCertified) {
            submitMetric(m_metricRecorder, eventName, 1);
            Document payload(kObjectType);
            Document::AllocatorType& alloc = payload.GetAllocator();
            Value event_payload_token_key{EVENT_PAYLOAD_TOKEN_KEY.data(), strlen(EVENT_PAYLOAD_TOKEN_KEY.data())};
            Value _alertToken{alertToken.data(), strlen(alertToken.data())};
            payload.AddMember(event_payload_token_key, _alertToken);
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX("sendEventFailed").m("Could not construct payload."));
                return;
            }
            auto jsonEventString = buildJsonEventString(eventName, EMPTY_DIALOG_REQUEST_ID, buffer.GetString()).second;
            if (isCertified) m_certifiedSender->sendJSONMessage(jsonEventString);
            else {
                if (!m_isConnected) { ACSDK_WARN(LX("sendEvent").m("Not connected to AVS.  Not sending Event.").d("event details", jsonEventString)); }
                else {
                    auto request = make_shared<MessageRequest>(jsonEventString);
                    m_messageSender->sendMessage(request);
                }
            }
        }
        void AlertsCapabilityAgent::sendBulkEvent(const string& eventName, const list<string>& tokenList, bool isCertified) {
            submitMetric(m_metricRecorder, eventName, 1);
            Document payload(kObjectType);
            Document::AllocatorType& alloc = payload.GetAllocator();
            Value jsonTokenList(kArrayType);
            for (auto& token : tokenList) {
                Value _token{token.data(), strlen(token.data())};
                jsonTokenList.PushBack(_token);
            }
            Value event_payload_tokens_key{EVENT_PAYLOAD_TOKENS_KEY.data(), strlen(EVENT_PAYLOAD_TOKENS_KEY.data())};
            payload.AddMember(event_payload_tokens_key, jsonTokenList);
            StringBuffer buffer;
            Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX("sendBulkEventFailed").m("Could not construct payload."));
                return;
            }
            auto jsonEventString = buildJsonEventString(eventName, EMPTY_DIALOG_REQUEST_ID, buffer.GetString()).second;
            if (isCertified) m_certifiedSender->sendJSONMessage(jsonEventString);
            else {
                if (!m_isConnected) { ACSDK_WARN(LX(__func__).m("Not connected to AVS.  Not sending Event.").d("event details", jsonEventString)); }
                else {
                    auto request = std::make_shared<MessageRequest>(jsonEventString);
                    m_messageSender->sendMessage(request);
                }
            }
        }
        void AlertsCapabilityAgent::updateAVSWithLocalVolumeChanges(int8_t volume, bool forceUpdate) {
            if (!forceUpdate && volume == m_lastReportedSpeakerSettings.volume) {
                ACSDK_DEBUG7(LX("updateAVSWithLocalVolumeChanges").d("Alerts volume already set to this value", volume));
                return;
            }
            m_lastReportedSpeakerSettings.volume = volume;
            Document payload(kObjectType);
            Document::AllocatorType& alloc = payload.GetAllocator();
            Value avs_payload_volume_key{AVS_PAYLOAD_VOLUME_KEY.data(), strlen(AVS_PAYLOAD_VOLUME_KEY.data())};
            Value _volume{volume};
            payload.AddMember(avs_payload_volume_key, _volume);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX("updateAVSWithLocalVolumeChangesFailed").m("Could not construct payload."));
                return;
            }
            auto jsonEventString = buildJsonEventString(ALERT_VOLUME_CHANGED_EVENT_NAME, EMPTY_DIALOG_REQUEST_ID, buffer.GetString()).second;
            m_certifiedSender->sendJSONMessage(jsonEventString);
        }
        void AlertsCapabilityAgent::sendProcessingDirectiveException(const shared_ptr<AVSDirective>& directive, const string& errorMessage) {
            auto unparsedDirective = directive->getUnparsedDirective();
            ACSDK_ERROR(LX("sendProcessingDirectiveException").m("Could not parse directive.").m(errorMessage).m(unparsedDirective));
            m_exceptionEncounteredSender->sendExceptionEncountered(unparsedDirective, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, errorMessage);
        }
        void AlertsCapabilityAgent::acquireChannel() {
            ACSDK_DEBUG9(LX("acquireChannel"));
            auto activity = FocusManagerInterface::Activity::create(NAMESPACE,shared_from_this(), milliseconds::zero(),ContentType::MIXABLE);
            m_focusManager->acquireChannel(FocusManagerInterface::ALERT_CHANNEL_NAME, activity);
        }
        void AlertsCapabilityAgent::releaseChannel() {
            ACSDK_DEBUG9(LX("releaseChannel"));
            if (m_alertScheduler.getFocusState() != FocusState::NONE) {
                m_focusManager->releaseChannel(FocusManagerInterface::ALERT_CHANNEL_NAME, shared_from_this());
            }
        }
        void AlertsCapabilityAgent::executeHandleDirectiveImmediately(std::shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG1(LX("executeHandleDirectiveImmediately"));
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
            string alertToken;
            if (DIRECTIVE_NAME_SET_ALERT == directiveName) {
                if (handleSetAlert(directive, payload, &alertToken)) sendEvent(SET_ALERT_SUCCEEDED_EVENT_NAME, alertToken, true);
                else sendEvent(SET_ALERT_FAILED_EVENT_NAME, alertToken, true);
            } else if (DIRECTIVE_NAME_DELETE_ALERT == directiveName) {
                if (handleDeleteAlert(directive, payload, &alertToken)) sendEvent(DELETE_ALERT_SUCCEEDED_EVENT_NAME, alertToken, true);
                else sendEvent(DELETE_ALERT_FAILED_EVENT_NAME, alertToken, true);
            } else if (DIRECTIVE_NAME_DELETE_ALERTS == directiveName) handleDeleteAlerts(directive, payload);
            else if (DIRECTIVE_NAME_SET_VOLUME == directiveName) handleSetVolume(directive, payload);
            else if (DIRECTIVE_NAME_ADJUST_VOLUME == directiveName) handleAdjustVolume(directive, payload);
            else if (DIRECTIVE_NAME_SET_ALARM_VOLUME_RAMP == directiveName) handleSetAlarmVolumeRamp(directive, payload);
        }
        void AlertsCapabilityAgent::executeOnConnectionStatusChanged(const Status status, const ChangedReason reason) {
            ACSDK_DEBUG1(LX("executeOnConnectionStatusChanged").d("status", status).d("reason", reason));
            m_isConnected = (Status::CONNECTED == status);
        }
        void AlertsCapabilityAgent::executeOnFocusChanged(FocusState focusState) {
            ACSDK_DEBUG1(LX("executeOnFocusChanged").d("focusState", focusState));
            m_alertScheduler.updateFocus(focusState);
        }
        void AlertsCapabilityAgent::executeOnFocusManagerFocusChanged(
            const string& channelName,
            FocusState focusState) {
            bool stateIsActive = focusState != FocusState::NONE;
            if (FocusManagerInterface::CONTENT_CHANNEL_NAME == channelName) m_contentChannelIsActive = stateIsActive;
            else if (FocusManagerInterface::COMMUNICATIONS_CHANNEL_NAME == channelName) m_commsChannelIsActive = stateIsActive;
            else return;
            if (m_alertIsSounding) {
                if (!m_commsChannelIsActive && !m_contentChannelIsActive) {
                    SpeakerInterface::SpeakerSettings speakerSettings;
                    if (!getAlertVolumeSettings(&speakerSettings)) {
                        ACSDK_ERROR(LX("executeOnFocusChangedFailed").d("reason", "Failed to get speaker settings."));
                        return;
                    }
                    if (speakerSettings.volume > m_lastReportedSpeakerSettings.volume) {
                        m_speakerManager->setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, m_lastReportedSpeakerSettings.volume,
                                          SpeakerManagerInterface::NotificationProperties(SpeakerManagerObserverInterface::Source::DIRECTIVE));
                    }
                }
            }
        }
        void AlertsCapabilityAgent::executeOnAlertStateChange(const string& alertToken, const string& alertType, AlertObserverInterface::State state,
                                                              const string& reason) {
            ACSDK_DEBUG1(LX("executeOnAlertStateChange").d("alertToken", alertToken).d("state", state).d("reason", reason));
            bool alertIsActive = false;
            switch(state) {
                case AlertObserverInterface::State::READY: acquireChannel(); break;
                case AlertObserverInterface::State::STARTED:
                    sendEvent(ALERT_STARTED_EVENT_NAME, alertToken, true);
                    updateContextManager();
                    alertIsActive = true;
                    break;
                case AlertObserverInterface::State::SNOOZED:
                    releaseChannel();
                    updateContextManager();
                    break;
                case AlertObserverInterface::State::STOPPED:
                    sendEvent(ALERT_STOPPED_EVENT_NAME, alertToken, true);
                    releaseChannel();
                    updateContextManager();
                    break;
                case AlertObserverInterface::State::COMPLETED:
                    sendEvent(ALERT_STOPPED_EVENT_NAME, alertToken, true);
                    releaseChannel();
                    updateContextManager();
                    break;
                case AlertObserverInterface::State::ERROR:
                    releaseChannel();
                    updateContextManager();
                    break;
                case AlertObserverInterface::State::PAST_DUE: sendEvent(ALERT_STOPPED_EVENT_NAME, alertToken, true); break;
                case AlertObserverInterface::State::FOCUS_ENTERED_FOREGROUND:
                    alertIsActive = true;
                    sendEvent(ALERT_ENTERED_FOREGROUND_EVENT_NAME, alertToken);
                    break;
                case AlertObserverInterface::State::FOCUS_ENTERED_BACKGROUND:
                    alertIsActive = true;
                    sendEvent(ALERT_ENTERED_BACKGROUND_EVENT_NAME, alertToken);
                    break;
                case AlertObserverInterface::State::SCHEDULED_FOR_LATER: case AlertObserverInterface::State::DELETED: break;
            }
            if (alertIsActive) {
                m_alertIsSounding = true;
                if (m_contentChannelIsActive || m_commsChannelIsActive) {
                    SpeakerInterface::SpeakerSettings contentSpeakerSettings;
                    if (getSpeakerVolumeSettings(&contentSpeakerSettings)) {
                        if (m_lastReportedSpeakerSettings.volume < contentSpeakerSettings.volume) {
                            m_speakerManager->setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, m_lastReportedSpeakerSettings.volume,
                                              SpeakerManagerInterface::NotificationProperties(SpeakerManagerObserverInterface::Source::DIRECTIVE));
                        }
                    }
                }
            } else {
                if (m_alertIsSounding) {
                    m_alertIsSounding = false;
                    m_speakerManager->setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, m_lastReportedSpeakerSettings.volume,
                                      SpeakerManagerInterface::NotificationProperties(SpeakerManagerObserverInterface::Source::LOCAL_API,
                                                false,false));
                }
            }
            m_executor.submit([this, alertToken, alertType, state, reason]() { executeNotifyObservers(alertToken, alertType, state, reason); });
        }
        void AlertsCapabilityAgent::executeAddObserver(std::shared_ptr<AlertObserverInterface> observer) {
            ACSDK_DEBUG1(LX("executeAddObserver").d("observer", observer.get()));
            m_observers.insert(observer);
        }
        void AlertsCapabilityAgent::executeRemoveObserver(std::shared_ptr<AlertObserverInterface> observer) {
            ACSDK_DEBUG1(LX("executeRemoveObserver").d("observer", observer.get()));
            m_observers.erase(observer);
        }
        void AlertsCapabilityAgent::executeNotifyObservers(const string& alertToken, const string& alertType, AlertObserverInterface::State state,
                                                           const string& reason) {
            ACSDK_DEBUG1(LX("executeNotifyObservers").d("alertToken", alertToken).d("alertType", alertType).d("state", state).d("reason", reason));
            for (auto observer : m_observers) observer->onAlertStateChange(alertToken, alertType, state, reason);
        }
        void AlertsCapabilityAgent::executeRemoveAllAlerts() {
            ACSDK_DEBUG1(LX("executeRemoveAllAlerts"));
            m_alertScheduler.clearData();
        }
        void AlertsCapabilityAgent::executeOnLocalStop() {
            ACSDK_DEBUG1(LX("executeOnLocalStop"));
            m_alertScheduler.onLocalStop();
        }
        void AlertsCapabilityAgent::updateContextManager() {
            string contextString = getContextString();
            NamespaceAndName namespaceAndName{AVS_CONTEXT_HEADER_NAMESPACE_VALUE_KEY, AVS_CONTEXT_HEADER_NAME_VALUE_KEY};
            auto setStateSuccess = m_contextManager->setState(namespaceAndName, contextString, StateRefreshPolicy::NEVER);
            if (setStateSuccess != SetStateResult::SUCCESS) {
                ACSDK_ERROR(LX("updateContextManagerFailed").m("Could not set the state on the contextManager").d("result", static_cast<int>(setStateSuccess)));
            }
        }
        string AlertsCapabilityAgent::getContextString() {
            rapidjson::Document state(kObjectType);
            rapidjson::Document::AllocatorType& alloc = state.GetAllocator();
            auto alertsContextInfo = m_alertScheduler.getContextInfo();
            auto allAlertsJsonValue = buildAllAlertsContext(alertsContextInfo.scheduledAlerts, alloc);
            auto activeAlertsJsonValue = buildActiveAlertsContext(alertsContextInfo.activeAlerts, alloc);
            Value avs_context_all_alerts_token_key{AVS_CONTEXT_ALL_ALERTS_TOKEN_KEY.data(), strlen(AVS_CONTEXT_ALL_ALERTS_TOKEN_KEY.data())};
            Value avs_context_active_alerts_token_key{AVS_CONTEXT_ACTIVE_ALERTS_TOKEN_KEY.data(), strlen(AVS_CONTEXT_ACTIVE_ALERTS_TOKEN_KEY.data())};
            state.AddMember(avs_context_all_alerts_token_key, allAlertsJsonValue, alloc);
            state.AddMember(avs_context_active_alerts_token_key, activeAlertsJsonValue, alloc);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!state.Accept(writer)) {
                ACSDK_ERROR(LX("getContextStringFailed").d("reason", "writerRefusedJsonObject"));
                return "";
            }
            return buffer.GetString();
        }
        void AlertsCapabilityAgent::clearData() {
            auto result = m_executor.submit([this]() { m_alertScheduler.clearData(Alert::StopReason::LOG_OUT); });
            result.wait();
        }
        unordered_set<shared_ptr<CapabilityConfiguration>> AlertsCapabilityAgent::getCapabilityConfigurations() {
            return m_capabilityConfigurations;
        }
        void AlertsCapabilityAgent::onSpeakerSettingsChanged(const SpeakerManagerObserverInterface::Source& source, const ChannelVolumeInterface::Type& type,
                                                             const SpeakerInterface::SpeakerSettings& settings) {
            m_executor.submit([this, settings, type]() { executeOnSpeakerSettingsChanged(type, settings); });
        }
        void AlertsCapabilityAgent::onSystemClockSynchronized() {
            m_alertScheduler.reloadAlertsFromDatabase(m_settingsManager, true);
        }
        bool AlertsCapabilityAgent::getAlertVolumeSettings(SpeakerInterface::SpeakerSettings* speakerSettings) {
            if (!m_speakerManager->getSpeakerSettings(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, speakerSettings).get()) {
                ACSDK_ERROR(LX("getAlertSpeakerSettingsFailed").d("reason", "Failed to get speaker settings"));
                return false;
            }
            return true;
        }
        bool AlertsCapabilityAgent::getSpeakerVolumeSettings(SpeakerInterface::SpeakerSettings* speakerSettings) {
            if (!m_speakerManager->getSpeakerSettings(ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME, speakerSettings).get()) {
                ACSDK_ERROR(LX("getContentSpeakerSettingsFailed").d("reason", "Failed to get speaker settings"));
                return false;
            }
            return true;
        }
        void AlertsCapabilityAgent::setNextAlertVolume(int64_t volume) {
            if (volume < speakerConstants::AVS_SET_VOLUME_MIN) {
                volume = speakerConstants::AVS_SET_VOLUME_MIN;
                ACSDK_DEBUG7(LX(__func__).m("Requested volume is lower than allowed minimum, using minimum instead."));
            } else if (volume > speakerConstants::AVS_SET_VOLUME_MAX) {
                volume = speakerConstants::AVS_SET_VOLUME_MAX;
                ACSDK_DEBUG7(LX(__func__).m("Requested volume is higher than allowed maximum, using maximum instead."));
            }
            ACSDK_DEBUG5(LX(__func__).d("New Alerts volume", volume));
            m_speakerManager->setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, static_cast<int8_t>(volume),
                              SpeakerManagerInterface::NotificationProperties(SpeakerManagerObserverInterface::Source::DIRECTIVE)).get();
            updateAVSWithLocalVolumeChanges(static_cast<int8_t>(volume), true);
        }
        void AlertsCapabilityAgent::executeOnSpeakerSettingsChanged(
            const ChannelVolumeInterface::Type& type,
            const SpeakerInterface::SpeakerSettings& speakerSettings) {
            if (ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME == type && !m_alertIsSounding) {
                updateAVSWithLocalVolumeChanges(speakerSettings.volume, true);
            }
        }
    }
}