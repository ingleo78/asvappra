#ifndef ACSDKBLUETOOTH_SQLITEBLUETOOTHSTORAGE_H_
#define ACSDKBLUETOOTH_SQLITEBLUETOOTHSTORAGE_H_

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <configuration/ConfigurationNode.h>
#include "BluetoothStorageInterface.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace configuration;
        using namespace storage;
        using namespace sqliteStorage;
        class SQLiteBluetoothStorage : public BluetoothStorageInterface {
        public:
            static unique_ptr<SQLiteBluetoothStorage> create(const ConfigurationNode& configurationRoot);
            bool createDatabase() override;
            bool open() override;
            void close() override;
            bool clear() override;
            bool getUuid(const string& mac, string* uuid) override;
            bool getCategory(const string& uuid, string* category) override;
            bool getMac(const string& uuid, string* mac) override;
            bool getMacToUuid(unordered_map<string, string>* macToUuid) override;
            bool getMacToCategory(unordered_map<string, string>* macToCategory) override;
            bool getUuidToMac(unordered_map<string, string>* uuidToMac) override;
            bool getUuidToCategory(unordered_map<string, string>* uuidToCategory) override;
            bool getOrderedMac(bool ascending, list<string>* macs) override;
            bool insertByMac(const string& mac, const string& uuid, bool overwrite = true) override;
            bool updateByCategory(const string& uuid, const string& category) override;
            bool remove(const string& mac) override;
        private:
            SQLiteBluetoothStorage(const string& filepath);
            bool getSingleRowLocked(unique_ptr<SQLiteStatement>& statement, unordered_map<string, string>* row);
            bool getMappingsLocked(const string& key, const string& value, unordered_map<string, string>* mappings);
            bool getAssociatedDataLocked(const string& constraintKey, const string& constraintVal, const string& resultKey, string* resultVal);
            bool updateValueLocked(const string& constraintKey, const string& constraintVal, const string& updateKey, const string& updateVal);
            bool insertEntryLocked(const string& operation, const string& uuid, const string& mac, const string& category);
            bool isDatabaseMigratedLocked();
            bool migrateDatabaseLocked();
            mutex m_databaseMutex;
            SQLiteDatabase m_db;
        };
    }
}
#endif