#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_STORAGE_SQLITEDEVICESETTINGSTORAGE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_STORAGE_SQLITEDEVICESETTINGSTORAGE_H_

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <configuration/ConfigurationNode.h>
#include <storage/SQLiteDatabase.h>
#include "DeviceSettingStorageInterface.h"
#include "../SettingStatus.h"

namespace alexaClientSDK {
    namespace settings {
        namespace storage {
            class SQLiteDeviceSettingStorage : public DeviceSettingStorageInterface {
            public:
                static std::unique_ptr<SQLiteDeviceSettingStorage> create(const avsCommon::utils::configuration::ConfigurationNode& configurationRoot);
                ~SQLiteDeviceSettingStorage();
                bool open() override;
                void close() override;
                bool storeSetting(const std::string& key, const std::string& value, SettingStatus status) override;
                bool storeSettings(const std::vector<std::tuple<std::string, std::string, SettingStatus>>& data) override;
                SettingStatusAndValue loadSetting(const std::string& key) override;
                bool deleteSetting(const std::string& key) override;
                bool updateSettingStatus(const std::string& key, SettingStatus status) override;
            private:
                SQLiteDeviceSettingStorage(const std::string& dbFilePath);
                bool createSettingsTable();
                alexaClientSDK::storage::sqliteStorage::SQLiteDatabase m_db;
                std::mutex m_mutex;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_STORAGE_SQLITEDEVICESETTINGSTORAGE_H_
