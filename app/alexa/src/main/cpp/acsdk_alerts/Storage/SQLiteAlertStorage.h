#ifndef ACSDKALERTS_STORAGE_SQLITEALERTSTORAGE_H_
#define ACSDKALERTS_STORAGE_SQLITEALERTSTORAGE_H_

#include <set>
#include <sdkinterfaces/Audio/AlertsAudioFactoryInterface.h>
#include <configuration/ConfigurationNode.h>
#include <storage/SQLiteDatabase.h>
#include "AlertStorageInterface.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace storage {
            using namespace std;
            using namespace sdkInterfaces;
            using namespace audio;
            using namespace storage;
            using namespace utils;
            using namespace configuration;
            using namespace settings;
            using namespace alexaClientSDK::storage::sqliteStorage;
            class SQLiteAlertStorage : public AlertStorageInterface {
            public:
                static unique_ptr<SQLiteAlertStorage> create(const ConfigurationNode& configurationRoot, const shared_ptr<AlertsAudioFactoryInterface>& alertsAudioFactory);
                ~SQLiteAlertStorage();
                bool createDatabase() override;
                bool open() override;
                void close() override;
                bool store(shared_ptr<Alert> alert) override;
                bool load(vector<shared_ptr<Alert>>* alertContainer, shared_ptr<DeviceSettingsManager> settingsManager) override;
                bool modify(shared_ptr<Alert> alert) override;
                bool erase(shared_ptr<Alert> alert) override;
                bool bulkErase(const list<shared_ptr<Alert>>& alertList) override;
                bool clearDatabase() override;
                enum class StatLevel {
                    ONE_LINE,
                    ALERTS_SUMMARY,
                    EVERYTHING
                };
                void printStats(StatLevel level = StatLevel::ONE_LINE);
            private:
                SQLiteAlertStorage(const std::string& dbFilePath, const shared_ptr<AlertsAudioFactoryInterface>& alertsAudioFactory);
                bool migrateAlertsDbFromV1ToV2();
                bool loadHelper(int dbVersion, vector<shared_ptr<Alert>>* alertContainer, shared_ptr<DeviceSettingsManager> settingsManager);
                bool alertExists(const std::string& token);
                shared_ptr<AlertsAudioFactoryInterface> m_alertsAudioFactory;
                SQLiteDatabase m_db;
            };
        }
    }
}
#endif