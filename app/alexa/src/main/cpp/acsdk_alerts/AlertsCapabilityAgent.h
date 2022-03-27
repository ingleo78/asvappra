#ifndef ACSDKALERTS_ALERTSCAPABILITYAGENT_H_
#define ACSDKALERTS_ALERTSCAPABILITYAGENT_H_

#include <chrono>
#include <set>
#include <string>
#include <unordered_set>
#include <avs/CapabilityAgent.h>
#include <avs/MessageRequest.h>
#include <avs/FocusState.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/Audio/AlertsAudioFactoryInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/SpeakerManagerInterface.h>
#include <avs/CapabilityConfiguration.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <sdkinterfaces/SystemClockMonitorObserverInterface.h>
#include <timing/Timer.h>
#include <certified_sender/CertifiedSender.h>
#include <registration_manager/CustomerDataManager.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Setting.h>
#include <settings/SettingEventMetadata.h>
#include <acsdk_alerts_interfaces/AlertObserverInterface.h>
#include <acsdk_alerts/Alert.h>
#include <acsdk_alerts/AlertScheduler.h>
#include <registration_manager/CustomerDataHandler.h>

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace acsdkAlerts;
        using namespace acsdkAlertsInterfaces;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace audio;
        using namespace utils;
        using namespace certifiedSender;
        using namespace metrics;
        using namespace renderer;
        using namespace registrationManager;
        using namespace settings;
        using namespace threading;
        using namespace acsdkAlerts::storage;
        using namespace rapidjson;
        static const minutes ALERT_PAST_DUE_CUTOFF_MINUTES = minutes(30);
        class AlertsCapabilityAgent : public CapabilityAgent, public ConnectionStatusObserverInterface, public CapabilityConfigurationInterface,
                                      public SpeakerManagerObserverInterface, public FocusManagerObserverInterface , public SystemClockMonitorObserverInterface,
                                      public AlertObserverInterface , public RequiresShutdown , public CustomerDataHandler,
                                      public enable_shared_from_this<AlertsCapabilityAgent> {
        public:
            static shared_ptr<AlertsCapabilityAgent> create(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                                            shared_ptr<CertifiedSender> certifiedMessageSender, shared_ptr<FocusManagerInterface> focusManager,
                                                            shared_ptr<SpeakerManagerInterface> speakerManager, shared_ptr<ContextManagerInterface> contextManager,
                                                            shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                            shared_ptr<AlertStorageInterface> alertStorage, shared_ptr<AlertsAudioFactoryInterface> alertsAudioFactory,
                                                            shared_ptr<RendererInterface> alertRenderer, shared_ptr<CustomerDataManager> dataManager,
                                                            shared_ptr<AlarmVolumeRampSetting> alarmVolumeRampSetting, shared_ptr<DeviceSettingsManager> settingsManager,
                                                            shared_ptr<MetricRecorderInterface> metricRecorder = nullptr, bool startAlertSchedulingOnInitialization = true);
            DirectiveHandlerConfiguration getConfiguration() const override;
            void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
            void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
            void handleDirective(shared_ptr<DirectiveInfo> info) override;
            void cancelDirective(shared_ptr<DirectiveInfo> info) override;
            void onDeregistered() override;
            void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;
            void onFocusChanged(FocusState focusState, MixingBehavior behavior) override;
            void onAlertStateChange(const string& token, const string& alertType, AlertObserverInterface::State state, const string& reason);
            void onFocusChanged(const string& channelName, FocusState newFocus) override;
            unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
            void onSpeakerSettingsChanged(const Source& source, const ChannelVolumeInterface::Type& type, const SpeakerInterface::SpeakerSettings& settings) override;
            void onSystemClockSynchronized() override;
            void addObserver(shared_ptr<AlertObserverInterface> observer);
            void removeObserver(shared_ptr<AlertObserverInterface> observer);
            void removeAllAlerts();
            void onLocalStop();
            void clearData() override;
            static SettingEventMetadata getAlarmVolumeRampMetadata();
        private:
            AlertsCapabilityAgent(shared_ptr<MessageSenderInterface> messageSender, shared_ptr<CertifiedSender> certifiedMessageSender,
                                  shared_ptr<FocusManagerInterface> focusManager, shared_ptr<SpeakerManagerInterface> speakerManager,
                                  shared_ptr<ContextManagerInterface> contextManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                  shared_ptr<AlertStorageInterface> alertStorage, shared_ptr<AlertsAudioFactoryInterface> alertsAudioFactory,
                                  shared_ptr<RendererInterface> alertRenderer, shared_ptr<CustomerDataManager> dataManager, shared_ptr<AlarmVolumeRampSetting> alarmVolumeRampSetting,
                                  shared_ptr<DeviceSettingsManager> settingsManager, shared_ptr<MetricRecorderInterface> metricRecorder);
            void doShutdown() override;
            bool initialize(bool startAlertSchedulingOnInitialization = true);
            bool initializeAlerts(bool startAlertSchedulingOnInitialization = true);
            void executeHandleDirectiveImmediately(shared_ptr<DirectiveInfo> info);
            void executeOnConnectionStatusChanged(const Status status, const ChangedReason reason);
            void executeOnFocusChanged(FocusState focusState);
            void executeOnFocusManagerFocusChanged(const string& channelName, FocusState focusState);
            void executeOnSpeakerSettingsChanged(const ChannelVolumeInterface::Type& type, const SpeakerInterface::SpeakerSettings& settings);
            void executeOnAlertStateChange(const string& alertToken, const string& alertType, AlertObserverInterface::State state, const string& reason);
            void executeAddObserver(shared_ptr<AlertObserverInterface> observer);
            void executeRemoveObserver(shared_ptr<AlertObserverInterface> observer);
            void executeNotifyObservers(const string& alertToken, const string& alertType, AlertObserverInterface::State state, const string& reason = "");
            void executeRemoveAllAlerts();
            void executeOnLocalStop();
            bool handleSetAlert(const shared_ptr<AVSDirective>& directive, const Document& payload, string* alertToken);
            bool handleDeleteAlert(const shared_ptr<AVSDirective>& directive, const Document& payload, string* alertToken);
            bool handleDeleteAlerts(const shared_ptr<AVSDirective>& directive, const Document& payload);
            bool handleSetVolume(const shared_ptr<AVSDirective>& directive, const Document& payload);
            bool handleAdjustVolume(const shared_ptr<AVSDirective>& directive, const Document& payload);
            bool handleSetAlarmVolumeRamp(const shared_ptr<AVSDirective>& directive, const Document& payload);
            void sendEvent(const string& eventName, const string& alertToken, bool isCertified = false);
            void sendBulkEvent(const string& eventName, const list<std::string>& tokenList, bool isCertified = false);
            void updateAVSWithLocalVolumeChanges(int8_t volume, bool forceUpdate);
            void sendProcessingDirectiveException(const shared_ptr<AVSDirective>& directive, const string& errorMessage);
            void acquireChannel();
            void releaseChannel();
            void updateContextManager();
            string getContextString();
            bool getAlertVolumeSettings(SpeakerInterface::SpeakerSettings* speakerSettings);
            bool getSpeakerVolumeSettings(SpeakerInterface::SpeakerSettings* speakerSettings);
            void setNextAlertVolume(int64_t volume);
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            shared_ptr<MessageSenderInterface> m_messageSender;
            shared_ptr<CertifiedSender> m_certifiedSender;
            shared_ptr<FocusManagerInterface> m_focusManager;
            shared_ptr<SpeakerManagerInterface> m_speakerManager;
            shared_ptr<ContextManagerInterface> m_contextManager;
            unordered_set<shared_ptr<acsdkAlertsInterfaces::AlertObserverInterface>> m_observers;
            bool m_isConnected;
            AlertScheduler m_alertScheduler;
            shared_ptr<AlertsAudioFactoryInterface> m_alertsAudioFactory;
            unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
            SpeakerInterface::SpeakerSettings m_lastReportedSpeakerSettings;
            bool m_contentChannelIsActive;
            bool m_commsChannelIsActive;
            bool m_alertIsSounding;
            Executor m_executor;
            shared_ptr<AlarmVolumeRampSetting> m_alarmVolumeRampSetting;
            shared_ptr<DeviceSettingsManager> m_settingsManager;
        };
    }
}
#endif