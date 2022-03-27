#include <algorithm>
#include <cctype>
#include <logger/Logger.h>
#include "SQLiteMiscStorage.h"
#include "SQLiteStatement.h"
#include "SQLiteUtils.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace configuration;
            using namespace logger;
            static const string TAG("SQLiteMiscStorage");
            static const string KEY_COLUMN_NAME = "key";
            static const string VALUE_COLUMN_NAME = "value";
            #define LX(event) LogEntry(TAG, event)
            static const string MISC_DATABASE_CONFIGURATION_ROOT_KEY = "miscDatabase";
            static const string MISC_DATABASE_DB_FILE_PATH_KEY = "databaseFilePath";
            static const string MISC_DATABASE_DB_COMPONENT_TABLE_NAMES_SEPARATOR = "_";
            static const string STRING_KEY_VALUE_TYPE = "STRING";
            static const string UNKNOWN_KEY_VALUE_TYPE = "UNKNOWN";
            static const string TEXT_DB_TYPE = "TEXT";
            static const bool CHECK_TABLE_EXISTS = true;
            static const bool CHECK_TABLE_NOT_EXISTS = false;
            static string basicDBChecks(SQLiteDatabase& db, const string& componentName, const string& tableName);
            static string basicDBChecks(SQLiteDatabase& db, const string& componentName, const string& tableName, bool tableShouldExist);
            static string getDBTableName(const string& componentName, const string& tableName);
            static string getKeyTypeString(SQLiteMiscStorage::KeyType keyType);
            static string getValueTypeString(SQLiteMiscStorage::ValueType valueType);
            static string getDBDataType(const string& keyValueType);
            unique_ptr<SQLiteMiscStorage> SQLiteMiscStorage::create(const ConfigurationNode& configurationRoot) {
                auto miscDatabaseConfigurationRoot = configurationRoot[MISC_DATABASE_CONFIGURATION_ROOT_KEY];
                if (!miscDatabaseConfigurationRoot) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config for misc database")
                                    .d("key", MISC_DATABASE_CONFIGURATION_ROOT_KEY));
                    return nullptr;
                }
                std::string miscDbFilePath;
                if (!miscDatabaseConfigurationRoot.getString(MISC_DATABASE_DB_FILE_PATH_KEY, &miscDbFilePath) ||
                    miscDbFilePath.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config value").d("key", MISC_DATABASE_DB_FILE_PATH_KEY));
                    return nullptr;
                }
                return unique_ptr<SQLiteMiscStorage>(new SQLiteMiscStorage(miscDbFilePath));
            }
            SQLiteMiscStorage::SQLiteMiscStorage(const string& dbFilePath) : m_db{dbFilePath} {}
            SQLiteMiscStorage::~SQLiteMiscStorage() {
                close();
            }
            bool SQLiteMiscStorage::open() {
                if (!m_db.open()) {
                    ACSDK_DEBUG0(LX("openDatabaseFailed"));
                    return false;
                }

                return true;
            }
            void SQLiteMiscStorage::close() {
                m_db.close();
            }
            bool SQLiteMiscStorage::createDatabase() {
                if (!m_db.initialize()) {
                    ACSDK_ERROR(LX("createDatabaseFailed"));
                    return false;
                }
                return true;
            }
            string getDBTableName(const string& componentName, const string& tableName) {
                if (componentName.empty() || tableName.empty()) {
                    string emptyParam;
                    if (componentName.empty() && tableName.empty()) emptyParam = "Component and table";
                    else {
                        if (componentName.empty()) emptyParam = "Component";
                        else emptyParam = "Table";
                    }
                    ACSDK_ERROR(LX("getDBTableNameError").d("reason", emptyParam + " name can't be empty."));
                    return "";
                }
                return (componentName + MISC_DATABASE_DB_COMPONENT_TABLE_NAMES_SEPARATOR + tableName);
            }
            string basicDBChecks(SQLiteDatabase& db, const string& componentName, const string& tableName) {
                if (!db.isDatabaseReady()) return "Database is not ready";
                if (componentName.empty()) return "Component name is empty";
                if (tableName.empty()) return "Table name is empty";
                return "";
            }
            string basicDBChecks(SQLiteDatabase& db, const string& componentName, const string& tableName, bool tableShouldExist) {
                const string errorReason = basicDBChecks(db, componentName, tableName);
                if (!errorReason.empty()) return errorReason;
                string dbTableName = getDBTableName(componentName, tableName);
                bool tableExists = db.tableExists(dbTableName);
                if (tableShouldExist && !tableExists) return "Table does not exist";
                if (!tableShouldExist && tableExists) return "Table already exists";
                return "";
            }
            string getKeyTypeString(SQLiteMiscStorage::KeyType keyType) {
                switch (keyType) {
                    case SQLiteMiscStorage::KeyType::STRING_KEY: return STRING_KEY_VALUE_TYPE;
                    case SQLiteMiscStorage::KeyType::UNKNOWN_KEY: return UNKNOWN_KEY_VALUE_TYPE;
                }
                return UNKNOWN_KEY_VALUE_TYPE;
            }
            string getValueTypeString(SQLiteMiscStorage::ValueType valueType) {
                switch (valueType) {
                    case SQLiteMiscStorage::ValueType::STRING_VALUE: return STRING_KEY_VALUE_TYPE;
                    case SQLiteMiscStorage::ValueType::UNKNOWN_VALUE: return UNKNOWN_KEY_VALUE_TYPE;
                }
                return UNKNOWN_KEY_VALUE_TYPE;
            }
            string getDBDataType(const std::string& keyValueType) {
                if (keyValueType == STRING_KEY_VALUE_TYPE) return TEXT_DB_TYPE;
                return "";
            }
            bool SQLiteMiscStorage::getKeyValueTypes(const string& componentName, const string& tableName, KeyType* keyType, ValueType* valueType) {
                const std::string errorEvent = "getKeyValueTypesFailed";
                const std::string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                if (!keyType || !valueType) {
                    ACSDK_ERROR(LX(errorEvent).m("Key/value pointers are null"));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                const string sqlString = "PRAGMA table_info(" + dbTableName + ");";
                auto sqlStatement = m_db.createStatement(sqlString);
                if ((!sqlStatement) || (!sqlStatement->step())) {
                    ACSDK_ERROR(LX(errorEvent).d("Could not get metadata of table", tableName));
                    return false;
                }
                const string tableInfoColumnName = "name";
                const string tableInfoColumnType = "type";
                string columnName, columnType;
                while (SQLITE_ROW == sqlStatement->getStepResult()) {
                    int numberColumns = sqlStatement->getColumnCount();
                    for (int i = 0; i < numberColumns; i++) {
                        string tableInfoColumn = sqlStatement->getColumnName(i);
                        if (tableInfoColumnName == tableInfoColumn) columnName = sqlStatement->getColumnText(i);
                        else if (tableInfoColumnType == tableInfoColumn) {
                            columnType = sqlStatement->getColumnText(i);
                            transform(columnType.begin(), columnType.end(), columnType.begin(), ::toupper);
                        }
                    }
                    if (!(columnName.empty()) && !(columnType.empty())) {
                        if (KEY_COLUMN_NAME == columnName) {
                            if (TEXT_DB_TYPE == columnType) *keyType = KeyType::STRING_KEY;
                            else *keyType = KeyType::UNKNOWN_KEY;
                        } else if (VALUE_COLUMN_NAME == columnName) {
                            if (TEXT_DB_TYPE == columnType) *valueType = ValueType::STRING_VALUE;
                            else *valueType = ValueType::UNKNOWN_VALUE;
                        }
                    }
                    columnName.clear();
                    columnType.clear();
                    sqlStatement->step();
                }
                return true;
            }
            string SQLiteMiscStorage::checkKeyType(const string& componentName, const string& tableName, KeyType keyType) {
                if (keyType == KeyType::UNKNOWN_KEY) return "Cannot check for unknown key column type";
                const std::string basicDBChecksError = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!basicDBChecksError.empty()) return basicDBChecksError;
                KeyType keyColumnType;
                ValueType valueColumnType;
                if (!getKeyValueTypes(componentName, tableName, &keyColumnType, &valueColumnType)) return "Unable to get key column type";
                if (keyColumnType == KeyType::UNKNOWN_KEY) return "Unknown key column type";
                if (keyColumnType != keyType) return "Unexpected key column type";
                return "";
            }
            string SQLiteMiscStorage::checkValueType(const string& componentName, const string& tableName, ValueType valueType) {
                if (valueType == ValueType::UNKNOWN_VALUE) return "Cannot check for unknown value column type";
                const string basicDBChecksError = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!basicDBChecksError.empty()) return basicDBChecksError;
                KeyType keyColumnType;
                ValueType valueColumnType;
                if (!getKeyValueTypes(componentName, tableName, &keyColumnType, &valueColumnType)) return "Unable to get value column type";
                if (valueColumnType == ValueType::UNKNOWN_VALUE) return "Unknown value column type";
                if (valueColumnType != valueType) return "Unexpected value column type";
                return "";
            }
            string SQLiteMiscStorage::checkKeyValueType(const string& componentName, const string& tableName, KeyType keyType, ValueType valueType) {
                if (keyType == KeyType::UNKNOWN_KEY) return "Cannot check for unknown key column type";
                if (valueType == ValueType::UNKNOWN_VALUE) return "Cannot check for unknown value column type";
                const string basicDBChecksError = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!basicDBChecksError.empty()) return basicDBChecksError;
                KeyType keyColumnType;
                ValueType valueColumnType;
                if (!getKeyValueTypes(componentName, tableName, &keyColumnType, &valueColumnType)) return "Unable to get key/value column types";
                if (keyColumnType == KeyType::UNKNOWN_KEY) return "Unknown key column type";
                if (valueColumnType == ValueType::UNKNOWN_VALUE) return "Unknown value column type";
                if (keyColumnType != keyType) return "Unexpected key column type";
                if (valueColumnType != valueType) return "Unexpected value column type";
                return "";
            }
            bool SQLiteMiscStorage::createTable(const string& componentName, const string& tableName, KeyType keyType, ValueType valueType) {
                const string errorEvent = "createTableFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_NOT_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                if (keyType == KeyType::UNKNOWN_KEY) {
                    ACSDK_ERROR(LX(errorEvent).m("Unknown key type"));
                    return false;
                }
                if (valueType == ValueType::UNKNOWN_VALUE) {
                    ACSDK_ERROR(LX(errorEvent).m("Unknown value type"));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                const string sqlString = "CREATE TABLE " + dbTableName + " (" + KEY_COLUMN_NAME + " " + getDBDataType(getKeyTypeString(keyType)) +
                                         " PRIMARY KEY NOT NULL," + VALUE_COLUMN_NAME + " " + getDBDataType(getValueTypeString(valueType)) + " NOT NULL);";
                if (!m_db.performQuery(sqlString)) {
                    ACSDK_ERROR(LX(errorEvent).d("Could not create table", tableName));
                    return false;
                }
                return true;
            }
            bool SQLiteMiscStorage::clearTable(const string& componentName, const string& tableName) {
                const string errorEvent = "clearTableFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                if (!m_db.clearTable(dbTableName)) {
                    ACSDK_ERROR(LX(errorEvent).d("Could not clear table", tableName));
                    return false;
                }
                return true;
            }
            bool SQLiteMiscStorage::deleteTable(const string& componentName, const string& tableName) {
                const string errorEvent = "deleteTableFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                int numOfTableEntries = 0;
                if (!getNumberTableRows(&m_db, dbTableName, &numOfTableEntries)) {
                    ACSDK_ERROR(LX(errorEvent).m("Failed to count rows in table"));
                    return false;
                }
                if (numOfTableEntries > 0) {
                    ACSDK_ERROR(LX(errorEvent).m("Unable to delete table that is not empty"));
                    return false;
                }
                const string sqlString = "DROP TABLE IF EXISTS " + dbTableName + ";";
                if (!m_db.performQuery(sqlString)) {
                    ACSDK_ERROR(LX(errorEvent).d("Could not delete table", tableName));
                    return false;
                }
                return true;
            }
            bool SQLiteMiscStorage::get(const string& componentName, const string& tableName, const string& key, string* value) {
                const string errorEvent = "getFromTableFailed";
                if (!value) {
                    ACSDK_ERROR(LX(errorEvent).m("Value is nullptr."));
                    return false;
                }
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                const string keyTypeError = checkKeyType(componentName, tableName, KeyType::STRING_KEY);
                if (!keyTypeError.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(keyTypeError));
                    return false;
                }
                const string sqlString = "SELECT value FROM " + dbTableName + " WHERE " + KEY_COLUMN_NAME + "=?;";
                const int keyIndex = 1;
                auto sqliteStatement = m_db.createStatement(sqlString);
                if (!sqliteStatement) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Create statement failed."));
                    return false;
                }
                if (!sqliteStatement->bindStringParameter(keyIndex, key)) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Bind parameter failed."));
                    return false;
                }
                if (!sqliteStatement->step()) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Step failed."));
                    return false;
                }
                if (SQLITE_ROW == sqliteStatement->getStepResult()) {
                    const int RESULT_COLUMN_POSITION = 0;
                    *value = sqliteStatement->getColumnText(RESULT_COLUMN_POSITION);
                }
                return true;
            }
            bool SQLiteMiscStorage::tableEntryExists(const string& componentName, const string& tableName, const string& key, bool* tableEntryExistsValue) {
                const string errorEvent = "tableEntryExistsFailed";
                if (!tableEntryExistsValue) {
                    ACSDK_ERROR(LX(errorEvent).m("tableEntryExistsValue is nullptr."));
                    return false;
                }
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                KeyType keyColumnType;
                ValueType valueColumnType;
                if (!getKeyValueTypes(componentName, tableName, &keyColumnType, &valueColumnType)) {
                    ACSDK_ERROR(LX(errorEvent).m("Unable to get key/value column types"));
                    return false;
                }
                if (keyColumnType != KeyType::STRING_KEY) {
                    ACSDK_ERROR(LX(errorEvent).m("Unexpected key column types"));
                    return false;
                }
                if (valueColumnType == ValueType::STRING_VALUE) {
                    string tableEntry;
                    if (!get(componentName, tableName, key, &tableEntry)) {
                        ACSDK_ERROR(LX(errorEvent).m("Unable to get table entry"));
                        return false;
                    }
                    *tableEntryExistsValue = !tableEntry.empty();
                    return true;
                }
                return false;
            }
            bool SQLiteMiscStorage::tableExists(const string& componentName, const string& tableName, bool* tableExistsValue) {
                const string errorEvent = "tableExistsFailed";
                if (!tableExistsValue) {
                    ACSDK_ERROR(LX(errorEvent).m("tableExistsValue is nullptr."));
                    return false;
                }
                const string errorReason = basicDBChecks(m_db, componentName, tableName);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                *tableExistsValue = m_db.tableExists(dbTableName);
                return true;
            }
            bool SQLiteMiscStorage::add(const string& componentName, const string& tableName, const string& key, const string& value) {
                const string errorEvent = "addToTableFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                const string keyValueTypeError = checkKeyValueType(componentName, tableName, KeyType::STRING_KEY, ValueType::STRING_VALUE);
                if (!keyValueTypeError.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(keyValueTypeError));
                    return false;
                }
                bool tableEntryExistsValue;
                if (!tableEntryExists(componentName, tableName, key, &tableEntryExistsValue)) {
                    ACSDK_ERROR(LX(errorEvent).d("Unable to get table entry information for " + key + " in table", tableName));
                    return false;
                }
                if (tableEntryExistsValue) {
                    ACSDK_ERROR(LX(errorEvent).d("An entry already exists for " + key + " in table", tableName));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                const string sqlString = "INSERT INTO " + dbTableName + " (" + KEY_COLUMN_NAME + ", " + VALUE_COLUMN_NAME + ") VALUES (?, ?);";
                const int keyIndex = 1;
                const int valueIndex = 2;
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Create statement failed."));
                    return false;
                }
                if (!statement->bindStringParameter(keyIndex, key) || !statement->bindStringParameter(valueIndex, value)) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Bind parameter failed."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Step failed."));
                    return false;
                }
                return true;
            }
            bool SQLiteMiscStorage::update(const string& componentName, const string& tableName, const string& key, const string& value) {
                const string errorEvent = "updateTableEntryFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                const string keyValueTypeError = checkKeyValueType(componentName, tableName, KeyType::STRING_KEY, ValueType::STRING_VALUE);
                if (!keyValueTypeError.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(keyValueTypeError));
                    return false;
                }
                bool tableEntryExistsValue;
                if (!tableEntryExists(componentName, tableName, key, &tableEntryExistsValue)) {
                    ACSDK_ERROR(LX(errorEvent).d("Unable to get table entry information for " + key + " in table", tableName));
                    return false;
                }
                if (!tableEntryExistsValue) {
                    ACSDK_ERROR(LX(errorEvent).d("An entry does not exist for " + key + " in table", tableName));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                const string sqlString = "UPDATE " + dbTableName + " SET " + VALUE_COLUMN_NAME + "=? WHERE " + KEY_COLUMN_NAME + "=?;";
                const int valueIndex = 1;
                const int keyIndex = 2;
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Create statement failed."));
                    return false;
                }
                if (!statement->bindStringParameter(keyIndex, key) || !statement->bindStringParameter(valueIndex, value)) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Bind parameter failed."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Step failed."));
                    return false;
                }
                return true;
            }
            bool SQLiteMiscStorage::put(const string& componentName, const string& tableName, const string& key, const string& value) {
                const string errorEvent = "putToTableFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                const string keyValueTypeError = checkKeyValueType(componentName, tableName, KeyType::STRING_KEY, ValueType::STRING_VALUE);
                if (!keyValueTypeError.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(keyValueTypeError));
                    return false;
                }
                bool tableEntryExistsValue;
                if (!tableEntryExists(componentName, tableName, key, &tableEntryExistsValue)) {
                    ACSDK_ERROR(LX(errorEvent).d("Unable to get table entry information for " + key + " in table", tableName));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                string sqlString;
                string errorValue;
                if (!tableEntryExistsValue) {
                    sqlString = "INSERT INTO " + dbTableName + " (" + VALUE_COLUMN_NAME + ", " + KEY_COLUMN_NAME + ") VALUES (?, ?);";
                    errorValue = "Could not add entry (" + key + ", " + value + ") to table";
                } else {
                    sqlString = "UPDATE " + dbTableName + " SET " + VALUE_COLUMN_NAME + "=? WHERE " + KEY_COLUMN_NAME + "=?;";
                    errorValue = "Could not update entry for " + key + " in table";
                }
                const int valueIndex = 1;
                const int keyIndex = 2;
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Create statement failed."));
                    return false;
                }
                if (!statement->bindStringParameter(keyIndex, key) || !statement->bindStringParameter(valueIndex, value)) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Bind parameter failed."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Step failed (" + errorValue + ")."));
                    return false;
                }
                return true;
            }
            bool SQLiteMiscStorage::remove(const string& componentName, const string& tableName, const string& key) {
                const string errorEvent = "removeTableEntryFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                const string keyTypeError = checkKeyType(componentName, tableName, KeyType::STRING_KEY);
                if (!keyTypeError.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(keyTypeError));
                    return false;
                }
                bool tableEntryExistsValue;
                if (!tableEntryExists(componentName, tableName, key, &tableEntryExistsValue)) {
                    ACSDK_ERROR(LX(errorEvent).d("Unable to get table entry information for " + key + " in table", tableName));
                    return false;
                }
                if (!tableEntryExistsValue) {
                    ACSDK_ERROR(LX(errorEvent).d("An entry does not exist for " + key + " in table", tableName));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                const string sqlString = "DELETE FROM " + dbTableName + " WHERE " + KEY_COLUMN_NAME + "=?;";
                const int keyIndex = 1;
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Create statement failed."));
                    return false;
                }
                if (!statement->bindStringParameter(keyIndex, key)) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Bind parameter failed."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX(errorEvent).d("reason", "Step failed."));
                    return false;
                }
                return true;
            }
            bool SQLiteMiscStorage::load(const string& componentName, const string& tableName, unordered_map<string, string>* valueContainer) {
                const string errorEvent = "loadFromTableFailed";
                const string errorReason = basicDBChecks(m_db, componentName, tableName, CHECK_TABLE_EXISTS);
                if (!errorReason.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(errorReason));
                    return false;
                }
                if (!valueContainer) {
                    ACSDK_ERROR(LX(errorEvent).m("Value container is nullptr."));
                    return false;
                }
                const string keyValueTypeError = checkKeyValueType(componentName, tableName, KeyType::STRING_KEY, ValueType::STRING_VALUE);
                if (!keyValueTypeError.empty()) {
                    ACSDK_ERROR(LX(errorEvent).m(keyValueTypeError));
                    return false;
                }
                string dbTableName = getDBTableName(componentName, tableName);
                const string sqlString = "SELECT * FROM " + dbTableName + ";";
                auto sqlStatement = m_db.createStatement(sqlString);
                if ((!sqlStatement) || (!sqlStatement->step())) {
                    ACSDK_ERROR(LX(errorEvent).d("Could not load entries from table", tableName));
                    return false;
                }
                string key, value;
                while (SQLITE_ROW == sqlStatement->getStepResult()) {
                    int numberColumns = sqlStatement->getColumnCount();
                    for (int i = 0; i < numberColumns; i++) {
                        string columnName = sqlStatement->getColumnName(i);
                        if (KEY_COLUMN_NAME == columnName) key = sqlStatement->getColumnText(i);
                        else if (VALUE_COLUMN_NAME == columnName) value = sqlStatement->getColumnText(i);
                    }
                    if (!(key.empty()) && !(value.empty())) valueContainer->insert(std::make_pair(key, value));
                    key.clear();
                    value.clear();
                    sqlStatement->step();
                }
                return true;
            }
            bool SQLiteMiscStorage::isOpened() {
                return m_db.isDatabaseReady();
            }
        }
    }
}
