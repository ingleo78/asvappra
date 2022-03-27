#include <string>
#include <capability_agents/AIP/AudioInputProcessor.h>
#include <acsdk_alerts/AlertsCapabilityAgent.h>
#include <configuration/ConfigurationNode.h>
#include <logger/Logger.h>
#include <settings/Types/LocaleWakeWordsSetting.h>
#include <capability_agents/System/TimeZoneHandler.h>
#include <capability_agents/System/LocaleHandler.h>
#include "DeviceSettingsManagerBuilder.h"

using namespace std;
using namespace chrono;
using namespace alexaClientSDK::avsCommon::utils::logger;

static const string TAG("SettingsManagerBuilder");
#define LX(event) LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace defaultClient {
        using namespace avsCommon;
        using namespace acsdkAlerts;
        using namespace acsdkAlertsInterfaces;
        using namespace acl;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace capabilityAgents;
        using namespace doNotDisturb;
        using namespace system;
        using namespace aip;
        using namespace configuration;
        using namespace registrationManager;
        using namespace settings;
        using namespace settings::storage;
        using namespace types;
        static const string DEFAULT_TIMEZONE = "Etc/GMT";
        static const string SETTINGS_CONFIGURATION_ROOT_KEY = "deviceSettings";
        static const string DEFAULT_TIMEZONE_CONFIGURATION_KEY = "defaultTimezone";
        static const SettingEventMetadata NETWORK_INFO_METADATA = {"System","NetworkInfoChanged","NetworkInfoReport",
                                                                   "networkInfo" };
        template <typename PointerT> static inline bool checkPointer(const shared_ptr<PointerT>& pointer, const string& variableName) {
            if (!pointer) {
                ACSDK_ERROR(LX("checkPointerFailed").d("variable", variableName));
                return false;
            }
            return true;
        }
        DeviceSettingsManagerBuilder::DeviceSettingsManagerBuilder(shared_ptr<DeviceSettingStorageInterface> settingStorage,
                                                                   shared_ptr<MessageSenderInterface> messageSender, shared_ptr<AVSConnectionManager> connectionManager,
                                                                   shared_ptr<CustomerDataManager> dataManager) : m_settingStorage{settingStorage}, m_messageSender{messageSender},
                                                                   m_connectionManager{connectionManager}, m_dataManager{dataManager}, m_foundError{false} {
            m_foundError = !(checkPointer(settingStorage, "settingStorage") && checkPointer(messageSender, "messageSender") &&
                             checkPointer(connectionManager, "connectionManager"));
        }
        template <size_t index> bool addSetting(DeviceSettingsManagerBuilder builder, DeviceSettingsManager& manager) {
            auto setting = builder.getSetting<index>();
            return addSetting<index - 1>(builder, manager) && (!setting || manager.addSetting<index>(setting));
        }
        template <> bool addSetting<0>(DeviceSettingsManagerBuilder builder, DeviceSettingsManager& manager) {
            auto setting = builder.getSetting<0>();
            return !setting || manager.addSetting<0>(setting);
        }
        unique_ptr<DeviceSettingsManager> DeviceSettingsManagerBuilder::build() {
            if (m_foundError) {
                ACSDK_ERROR(LX("buildFailed").d("reason", "settingConstructionFailed"));
                return nullptr;
            }
            unique_ptr<DeviceSettingsManager> manager{new DeviceSettingsManager(m_dataManager)};
            if (!addSetting<NUMBER_OF_SETTINGS - 1>(*this, *manager)) {
                ACSDK_ERROR(LX("buildFailed").d("reason", "addSettingFailed"));
                return nullptr;
            }
            return manager;
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withAlarmVolumeRampSetting() {
            return withSynchronizedSetting<DeviceSettingsIndex::ALARM_VOLUME_RAMP, SharedAVSSettingProtocol>(
                AlertsCapabilityAgent::getAlarmVolumeRampMetadata(), types::getAlarmVolumeRampDefault());
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withWakeWordConfirmationSetting() {
            return withSynchronizedSetting<DeviceSettingsIndex::WAKEWORD_CONFIRMATION, SharedAVSSettingProtocol>(AudioInputProcessor::getWakeWordConfirmationMetadata(),
                                                                                                                 getWakeWordConfirmationDefault());
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withSpeechConfirmationSetting() {
            return withSynchronizedSetting<DeviceSettingsIndex::SPEECH_CONFIRMATION, SharedAVSSettingProtocol>(AudioInputProcessor::getSpeechConfirmationMetadata(),
                                                                                                               getSpeechConfirmationDefault());
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withTimeZoneSetting(
            shared_ptr<SystemTimeZoneInterface> systemTimeZone) {
            function<bool(const TimeZoneSetting::ValueType&)> applyFunction;
            if (systemTimeZone) {
                applyFunction = [systemTimeZone](const TimeZoneSetting::ValueType& value) {
                    return systemTimeZone->setTimezone(value);
                };
            }
            string defaultTimezone = DEFAULT_TIMEZONE;
            auto settingsConfig = ConfigurationNode::getRoot()[SETTINGS_CONFIGURATION_ROOT_KEY];
            if (settingsConfig) {
                settingsConfig.getString(DEFAULT_TIMEZONE_CONFIGURATION_KEY, &defaultTimezone, DEFAULT_TIMEZONE);
            }
            return withSynchronizedSetting<DeviceSettingsIndex::TIMEZONE, SharedAVSSettingProtocol>(TimeZoneHandler::getTimeZoneMetadata(), defaultTimezone,
                                                                                                    applyFunction);
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withLocaleSetting(
            shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager) {
            auto dummySender = SettingEventSender::create(SettingEventMetadata(), m_messageSender);
            auto localeMetadata = LocaleHandler::getLocaleEventsMetadata();
            auto localeEventSender = SettingEventSender::create(localeMetadata, m_messageSender);
            auto setting = LocaleWakeWordsSetting::create(move(localeEventSender),move(dummySender),m_settingStorage,
                                                          localeAssetsManager);
            if (!setting) {
                ACSDK_ERROR(LX("createLocaleWakeWordsSettingFailed").d("reason", "cannotAddSetting"));
                m_foundError = true;
                return *this;
            } else get<DeviceSettingsIndex::LOCALE>(m_settingConfigs) = SettingConfiguration<LocalesSetting>{setting, localeMetadata};
            m_connectionManager->addConnectionStatusObserver(setting);
            return *this;
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withLocaleAndWakeWordsSettings(shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager) {
            auto localeMetadata = LocaleHandler::getLocaleEventsMetadata();
            auto wakeWordsMetadata = AudioInputProcessor::getWakeWordsEventsMetadata();
            auto localeEventSender = SettingEventSender::create(localeMetadata, m_messageSender);
            auto wakeWordsEventSender = SettingEventSender::create(wakeWordsMetadata, m_messageSender);
            auto setting = LocaleWakeWordsSetting::create(move(localeEventSender),move(wakeWordsEventSender),m_settingStorage,
                                                          localeAssetsManager);
            if (!setting) {
                ACSDK_ERROR(LX("createLocaleWakeWordsSettingFailed").d("reason", "cannotAddSetting"));
                m_foundError = true;
                return *this;
            } else {
                get<DeviceSettingsIndex::LOCALE>(m_settingConfigs) = SettingConfiguration<LocalesSetting>{setting, localeMetadata};
                get<DeviceSettingsIndex::WAKE_WORDS>(m_settingConfigs) = SettingConfiguration<WakeWordsSetting>{setting, wakeWordsMetadata};
            }
            m_connectionManager->addConnectionStatusObserver(setting);
            return *this;
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withNetworkInfoSetting() {
            return withSynchronizedSetting<DeviceSettingsIndex::NETWORK_INFO, DeviceControlledSettingProtocol>(NETWORK_INFO_METADATA, types::NetworkInfo());
        }
        DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withDoNotDisturbSetting(const shared_ptr<DoNotDisturbCapabilityAgent>& dndCA) {
            if (!dndCA) {
                ACSDK_ERROR(LX("withDNDSettingFailed").d("reason", "nullCA"));
                m_foundError = true;
                return *this;
            }
            get<DeviceSettingsIndex::DO_NOT_DISTURB>(m_settingConfigs) = SettingConfiguration<DoNotDisturbSetting>{dndCA->getDoNotDisturbSetting(),
                                                                                                                       dndCA->getDoNotDisturbEventsMetadata()};
            return *this;
        }
        template <size_t index, class ProtocolT> DeviceSettingsManagerBuilder& DeviceSettingsManagerBuilder::withSynchronizedSetting(const SettingEventMetadata& metadata,
                                                                                                                     const ValueType<index>& defaultValue,
                                                                                                                     function<bool(const ValueType<index>&)> applyFn) {
            auto eventSender = SettingEventSender::create(metadata, m_connectionManager);
            auto protocol = ProtocolT::create(metadata, move(eventSender), m_settingStorage, m_connectionManager);
            auto setting = Setting<ValueType<index>>::create(defaultValue, move(protocol), applyFn);
            if (!setting) {
                m_foundError = true;
                return *this;
            }
            get<index>(m_settingConfigs) = SettingConfiguration<SettingType<index>>{setting, metadata};
            return *this;
        }
    }
}