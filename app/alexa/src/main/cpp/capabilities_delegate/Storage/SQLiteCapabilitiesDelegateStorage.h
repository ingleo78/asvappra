#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_STORAGE_SQLITECAPABILITIESDELEGATESTORAGE_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_STORAGE_SQLITECAPABILITIESDELEGATESTORAGE_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <configuration/ConfigurationNode.h>
#include <storage/SQLiteDatabase.h>
#include "CapabilitiesDelegateStorageInterface.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace storage {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace configuration;
            using namespace alexaClientSDK::storage;
            using namespace sqliteStorage;
            class SQLiteCapabilitiesDelegateStorage : public CapabilitiesDelegateStorageInterface {
            public:
                static unique_ptr<SQLiteCapabilitiesDelegateStorage> create(const ConfigurationNode& configurationRoot);
                bool createDatabase() override;
                bool open() override;
                void close() override;
                bool store(const string& endpointId, const string& endpointConfig) override;
                bool store(const unordered_map<string, string>& endpointIdToConfigMap) override;
                bool load(unordered_map<string, string>* endpointConfigMap) override;
                bool load(const string& endpointId, string* endpointConfig) override;
                bool erase(const string& endpointId) override;
                bool erase(const unordered_map<string, string>& endpointIdToConfigMap) override;
                bool clearDatabase() override;
                ~SQLiteCapabilitiesDelegateStorage();
            private:
                SQLiteCapabilitiesDelegateStorage(const string& dbFilePath);
                bool storeLocked(const string& endpointId, const string& endpointConfig);
                bool eraseLocked(const string& endpointId);
                mutex m_mutex;
                SQLiteDatabase m_database;
            };
        }
    }
}
#endif