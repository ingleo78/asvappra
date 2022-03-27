#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_DEVICESETTINGSMANAGERBUILDER_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_DEFAULTCLIENT_INCLUDE_DEFAULTCLIENT_DEVICESETTINGSMANAGERBUILDER_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <tuple>
#include <acl/AVSConnectionManager.h>
#include <sdkinterfaces/LocaleAssetsManagerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/SystemTimeZoneInterface.h>
#include <capability/do_not_disturb/DoNotDisturbCapabilityAgent.h>
#include <registration_manager/CustomerDataManager.h>
#include <settings/CloudControlledSettingProtocol.h>
#include <settings/DeviceControlledSettingProtocol.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Setting.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSender.h>
#include <settings/SettingsManagerBuilderBase.h>
#include <settings/SharedAVSSettingProtocol.h>
#include <settings/Storage/DeviceSettingStorageInterface.h>

namespace alexaClientSDK {
    namespace defaultClient {
        using namespace std;
        using namespace avsCommon;
        using namespace acl;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace capabilityAgents;
        using namespace doNotDisturb;
        using namespace settings;
        using namespace storage;
        using namespace registrationManager;
        class DeviceSettingsManagerBuilder : public SettingsManagerBuilderBase<settings::DeviceSettingsManager> {
        public:
            DeviceSettingsManagerBuilder(shared_ptr<DeviceSettingStorageInterface> settingStorage, shared_ptr<MessageSenderInterface> messageSender,
                                         shared_ptr<AVSConnectionManager> connectionManager, shared_ptr<CustomerDataManager> dataManager);
            DeviceSettingsManagerBuilder& withDoNotDisturbSetting(const shared_ptr<DoNotDisturbCapabilityAgent>& dndCA);
            DeviceSettingsManagerBuilder& withAlarmVolumeRampSetting();
            DeviceSettingsManagerBuilder& withWakeWordConfirmationSetting();
            DeviceSettingsManagerBuilder& withSpeechConfirmationSetting();
            DeviceSettingsManagerBuilder& withTimeZoneSetting(shared_ptr<SystemTimeZoneInterface> systemTimeZone = nullptr);
            DeviceSettingsManagerBuilder& withLocaleSetting(shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager);
            DeviceSettingsManagerBuilder& withLocaleAndWakeWordsSettings(shared_ptr<LocaleAssetsManagerInterface> localeAssetsManager);
            DeviceSettingsManagerBuilder& withNetworkInfoSetting();
            template <size_t index> shared_ptr<SettingType<index>> getSetting() const;
            template <size_t index> SettingConfiguration<SettingType<index>> getConfiguration() const;
            unique_ptr<DeviceSettingsManager> build() override;
        private:
            template <size_t index, class ProtocolT> DeviceSettingsManagerBuilder& withSynchronizedSetting(const SettingEventMetadata& metadata, const ValueType<index>& defaultValue,
                                                                                           function<bool(const ValueType<index>&)> applyFn = function<bool(const ValueType<index>&)>());
            shared_ptr<DeviceSettingStorageInterface> m_settingStorage;
            shared_ptr<MessageSenderInterface> m_messageSender;
            shared_ptr<AVSConnectionManager> m_connectionManager;
            shared_ptr<CustomerDataManager> m_dataManager;
            bool m_foundError;
        };
        template <size_t index> SettingConfiguration<DeviceSettingsManagerBuilder::SettingType<index>> DeviceSettingsManagerBuilder::getConfiguration() const {
            return get<index>(m_settingConfigs);
        }
        template <size_t index> shared_ptr<DeviceSettingsManagerBuilder::SettingType<index>> DeviceSettingsManagerBuilder::getSetting() const {
            return get<index>(m_settingConfigs).setting;
        }
    }
}
#endif