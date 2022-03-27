#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_STORAGE_DEVICESETTINGSTORAGEINTERFACE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_STORAGE_DEVICESETTINGSTORAGEINTERFACE_H_

#include <string>
#include <tuple>
#include <vector>
#include <settings/SettingStatus.h>

namespace alexaClientSDK {
    namespace settings {
        namespace storage {
            class DeviceSettingStorageInterface {
            public:
                using SettingStatusAndValue = std::pair<SettingStatus, std::string>;
                virtual ~DeviceSettingStorageInterface() = default;
                virtual bool open();
                virtual void close();
                virtual bool storeSetting(const std::string& key, const std::string& value, SettingStatus status);
                virtual bool storeSettings(const std::vector<std::tuple<std::string, std::string, SettingStatus>>& data);
                virtual SettingStatusAndValue loadSetting(const std::string& key);
                virtual bool deleteSetting(const std::string& key);
                virtual bool updateSettingStatus(const std::string& key, SettingStatus status);
            };
        }
    }
}
#endif