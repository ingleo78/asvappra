#ifndef ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITEMISCSTORAGE_H_
#define ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITEMISCSTORAGE_H_

#include <sdkinterfaces/Storage/MiscStorageInterface.h>
#include <configuration/ConfigurationNode.h>
#include "SQLiteDatabase.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace sdkInterfaces::storage;
            using namespace configuration;
            class SQLiteMiscStorage : public MiscStorageInterface {
            public:
                static unique_ptr<SQLiteMiscStorage> create(const ConfigurationNode& configurationRoot);
                ~SQLiteMiscStorage();
                bool createDatabase() override;
                bool open() override;
                bool isOpened() override;
                void close() override;
                bool createTable(const string& componentName, const string& tableName, KeyType keyType, ValueType valueType) override;
                bool clearTable(const string& componentName, const string& tableName) override;
                bool deleteTable(const string& componentName, const string& tableName) override;
                bool get(const string& componentName, const string& tableName, const string& key, string* value) override;
                bool add(const string& componentName, const string& tableName, const string& key, const string& value) override;
                bool update(const string& componentName, const string& tableName, const string& key, const string& value) override;
                bool put(const string& componentName, const string& tableName, const string& key, const string& value) override;
                bool remove(const string& componentName, const string& tableName, const string& key) override;
                bool tableEntryExists(const string& componentName, const string& tableName, const string& key, bool* tableEntryExistsValue) override;
                bool tableExists(const string& componentName, const string& tableName, bool* tableExistsValue) override;
                bool load(const string& componentName, const string& tableName, unordered_map<string, string>* valueContainer) override;
            private:
                SQLiteMiscStorage(const string& dbFilePath);
                bool getKeyValueTypes(const string& componentName, const string& tableName, KeyType* keyType, ValueType* valueType);
                string checkKeyType(const string& componentName, const string& tableName, KeyType keyType);
                string checkValueType(const string& componentName, const string& tableName, ValueType valueType);
                string checkKeyValueType(const string& componentName, const string& tableName, KeyType keyType, ValueType valueType);
                SQLiteDatabase m_db;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITEMISCSTORAGE_H_
