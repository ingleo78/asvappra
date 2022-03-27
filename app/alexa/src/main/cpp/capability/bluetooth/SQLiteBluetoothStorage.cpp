#include <util/bluetooth/DeviceCategory.h>
#include <logger/Logger.h>
#include "BluetoothStorageInterface.h"
#include "SQLiteBluetoothStorage.h"

static const std::string TAG{"SQLiteBluetoothStorage"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace acsdkBluetooth {
        static const string BLUETOOTH_CONFIGURATION_ROOT_KEY = "bluetooth";
        static const string BLUETOOTH_DB_FILE_PATH_KEY = "databaseFilePath";
        static const string UUID_TABLE_NAME = "uuidMapping";
        static const string COLUMN_UUID = "uuid";
        static const string COLUMN_MAC = "mac";
        static const string COLUMN_CATEGORY = "category";
        std::unique_ptr<SQLiteBluetoothStorage> SQLiteBluetoothStorage::create(
            const ConfigurationNode& configurationRoot) {
            ACSDK_DEBUG5(LX(__func__));
            auto bluetoothConfigurationRoot = configurationRoot[BLUETOOTH_CONFIGURATION_ROOT_KEY];
            if (!bluetoothConfigurationRoot) {
                ACSDK_ERROR(LX(__func__).d("reason", "loadConfigFailed").d("key", BLUETOOTH_CONFIGURATION_ROOT_KEY));
                return nullptr;
            }
            string filePath;
            if (!bluetoothConfigurationRoot.getString(BLUETOOTH_DB_FILE_PATH_KEY, &filePath) || filePath.empty()) {
                ACSDK_ERROR(LX(__func__).d("reason", "retrieveValueFailed").d("key", BLUETOOTH_DB_FILE_PATH_KEY));
                return nullptr;
            }
            return unique_ptr<SQLiteBluetoothStorage>(new SQLiteBluetoothStorage(filePath));
        }
        bool SQLiteBluetoothStorage::createDatabase() {
            ACSDK_DEBUG5(LX(__func__));
            string defaultCategory = deviceCategoryToString(DeviceCategory::UNKNOWN).data();
            const string sqlString = "CREATE TABLE " + UUID_TABLE_NAME + "(" + COLUMN_UUID + " text not null unique, " + COLUMN_MAC + " text not null unique, " +
                                     COLUMN_CATEGORY + " text not null default "+ defaultCategory +");";
            lock_guard<mutex> lock(m_databaseMutex);
            if (!m_db.initialize()) {
                ACSDK_ERROR(LX(__func__).d("reason", "initializeDBFailed"));
                return false;
            }
            if (!m_db.performQuery(sqlString)) {
                ACSDK_ERROR(LX(__func__).d("reason", "createTableFailed"));
                close();
                return false;
            }
            return true;
        }
        bool SQLiteBluetoothStorage::open() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_databaseMutex);
            bool ret = m_db.open();
            if (ret && !isDatabaseMigratedLocked()) {
                ACSDK_INFO(LX(__func__).d("reason", "Legacy Database, migrating database"));
                migrateDatabaseLocked();
            }
            return ret;
        }
        void SQLiteBluetoothStorage::close() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_databaseMutex);
            return m_db.close();
        }
        bool SQLiteBluetoothStorage::clear() {
            ACSDK_DEBUG5(LX(__func__));
            const string sqlString = "DELETE FROM " + UUID_TABLE_NAME + ";";
            lock_guard<mutex> lock(m_databaseMutex);
            auto statement = m_db.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "createStatementFailed"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX(__func__).d("reason", "stepFailed"));
                return false;
            }
            return true;
        }
        bool SQLiteBluetoothStorage::getSingleRowLocked(unique_ptr<SQLiteStatement>& statement, unordered_map<string, string>* row) {
            ACSDK_DEBUG9(LX(__func__));
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullStatement"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX(__func__).d("reason", "stepFailed"));
                return false;
            }
            if (SQLITE_ROW != statement->getStepResult()) {
                ACSDK_INFO(LX(__func__).d("reason", "getStepResultFailed"));
                return false;
            }
            for (int i = 0; i < statement->getColumnCount(); i++) {
                row->insert({statement->getColumnName(i), statement->getColumnText(i)});
            }
            return true;
        }
        bool SQLiteBluetoothStorage::getUuid(const string& mac, string* uuid) {
            ACSDK_DEBUG5(LX(__func__));
            if (!uuid) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullUuid"));
                return false;
            }
            lock_guard<mutex> lock(m_databaseMutex);
            return getAssociatedDataLocked(COLUMN_MAC, mac, COLUMN_UUID, uuid);
        }
        bool SQLiteBluetoothStorage::getCategory(const string& uuid, string* category) {
            ACSDK_DEBUG5(LX(__func__));
            if (!category) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullCategory"));
                return false;
            }
            lock_guard<mutex> lock(m_databaseMutex);
            return getAssociatedDataLocked(COLUMN_UUID, uuid, COLUMN_CATEGORY, category);
        }
        bool SQLiteBluetoothStorage::getMac(const string& uuid, string* mac) {
            ACSDK_DEBUG5(LX(__func__));
            if (!mac) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullMac"));
                return false;
            }
            lock_guard<mutex> lock(m_databaseMutex);
            return getAssociatedDataLocked(COLUMN_UUID, uuid, COLUMN_MAC, mac);
        }

        bool SQLiteBluetoothStorage::getAssociatedDataLocked(const string& constraintKey, const string& constraintVal, const string& resultKey, string* resultVal) {
            ACSDK_DEBUG5(LX(__func__));
            if (!resultVal) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullResult"));
                return false;
            }
            const string sqlString = "SELECT " + resultKey + " FROM " + UUID_TABLE_NAME + " WHERE " + constraintKey + " IS ?;";
            auto statement = m_db.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "createStatementFailed"));
                return false;
            }
            const int VALUE_INDEX = 1;
            if (!statement->bindStringParameter(VALUE_INDEX, constraintVal)) {
                ACSDK_ERROR(LX(__func__).d("reason", "bindFailed"));
                return false;
            }
            unordered_map<string, string> row;
            if (getSingleRowLocked(statement, &row) && row.count(resultKey) > 0) {
                *resultVal = row.at(resultKey);
                return true;
            }
            return false;
        }
        bool SQLiteBluetoothStorage::getMappingsLocked(const string& key, const string& value, unordered_map<string, string>* mappings) {
            ACSDK_DEBUG5(LX(__func__).d("key", key).d("value", value));
            if (!mappings) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullMappings"));
                return false;
            }
            if (COLUMN_UUID != key && COLUMN_MAC != key && COLUMN_CATEGORY != key) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidKey").d("key", key));
                return false;
            }
            if (COLUMN_UUID != value && COLUMN_MAC != value && COLUMN_CATEGORY != value) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidValue").d("value", value));
                return false;
            }
            const std::string sqlString = "SELECT * FROM " + UUID_TABLE_NAME + ";";
            auto statement = m_db.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "createStatementFailed"));
                return false;
            }
            unordered_map<string, string> row;
            while(getSingleRowLocked(statement, &row)) {
                if (0 == row.count(key) || 0 == row.count(value)) {
                    ACSDK_ERROR(LX(__func__).d("reason", "missingData").d("keyPresent", row.count(key)).d("valuePresent", row.count(value)));
                    continue;
                }
                mappings->insert({row.at(key), row.at(value)});
                row.clear();
            }
            return true;
        }
        bool SQLiteBluetoothStorage::updateValueLocked(const string& constraintKey, const string& constraintVal, const string& updateKey, const string& updateVal) {
            if (COLUMN_UUID != constraintKey && COLUMN_MAC != constraintKey && COLUMN_CATEGORY != constraintKey) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidConstraintKey").d("constraintKey", constraintKey));
                return false;
            }
            if (COLUMN_UUID != updateKey && COLUMN_MAC != updateKey && COLUMN_CATEGORY != updateKey) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidUpdateKey").d("updateKey", updateKey));
                return false;
            }
            const string sqlString = "UPDATE " + UUID_TABLE_NAME + " SET " + updateKey + "=? WHERE " + constraintKey + "=?;";
            auto statement = m_db.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "createStatementFailed"));
                return false;
            }
            const int UPDATE_VAL_INDEX = 1;
            const int CONSTRAINT_VAL_INDEX = 2;
            if (!statement->bindStringParameter(UPDATE_VAL_INDEX, updateVal) || !statement->bindStringParameter(CONSTRAINT_VAL_INDEX, constraintVal)) {
                ACSDK_ERROR(LX(__func__).d("reason", "bindParameterFailed"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX(__func__).d("reason", "stepFailed"));
                return false;
            }
            return true;
        }
        bool SQLiteBluetoothStorage::insertEntryLocked(const string& operation, const string& uuid, const string& mac, const string& category) {
            if (operation != "REPLACE" && operation != "INSERT") {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidOperation").d("operation", operation));
                return false;
            }
            const string sqlString = operation + " INTO " + UUID_TABLE_NAME + " (" + COLUMN_UUID + "," + COLUMN_MAC + "," + COLUMN_CATEGORY + ") VALUES (?,?,?);";
            auto statement = m_db.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "createStatementFailed"));
                return false;
            }
            const int UUID_INDEX = 1;
            const int MAC_INDEX = 2;
            const int CATEGORY_INDEX = 3;
            if (!statement->bindStringParameter(UUID_INDEX, uuid) || !statement->bindStringParameter(MAC_INDEX, mac) ||
                !statement->bindStringParameter(CATEGORY_INDEX, category)) {
                ACSDK_ERROR(LX(__func__).d("reason", "bindParameterFailed"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX(__func__).d("reason", "stepFailed"));
                return false;
            }
            return true;
        }
        bool SQLiteBluetoothStorage::isDatabaseMigratedLocked() {
            auto sqlStatement = m_db.createStatement("PRAGMA table_info(" + UUID_TABLE_NAME + ");");
            if ((!sqlStatement) || (!sqlStatement->step())) {
                ACSDK_ERROR(LX(__func__).d("reason", "failedSQLMigrationQuery"));
                return false;
            }
            const string tableInfoColumnName = "name";
            string columnName;
            while(SQLITE_ROW == sqlStatement->getStepResult()) {
                int columnCount = sqlStatement->getColumnCount();
                for (int i = 0; i < columnCount; i++) {
                    string tableColumnName = sqlStatement->getColumnName(i);
                    if (tableInfoColumnName == tableColumnName) {
                        columnName = sqlStatement->getColumnText(i);
                        if (columnName == COLUMN_CATEGORY) return true;
                    }
                }
                if (!sqlStatement->step()) {
                    ACSDK_ERROR(LX(__func__).d("reason", "stepFailed"));
                    return false;
                }
            }
            return false;
        }
        bool SQLiteBluetoothStorage::migrateDatabaseLocked() {
            const string defaultCategory = deviceCategoryToString(DeviceCategory::UNKNOWN).data();
            if (!m_db.performQuery("ALTER TABLE " + UUID_TABLE_NAME + " ADD COLUMN " + COLUMN_CATEGORY + " text not null default " + defaultCategory + ";")) {
                ACSDK_ERROR(LX(__func__).d("reason", "addingCategoryColumnFailed"));
                return false;
            }
            auto statement = m_db.createStatement("UPDATE " + UUID_TABLE_NAME + " SET " + COLUMN_CATEGORY + "=?;");
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "createStatementFailed"));
                return false;
            }
            const int UPDATE_CATEGORY_VAL_INDEX = 1;
            const string otherCategory = deviceCategoryToString(DeviceCategory::OTHER).data();
            if (!statement->bindStringParameter(UPDATE_CATEGORY_VAL_INDEX, otherCategory)) {
                ACSDK_ERROR(LX(__func__).d("reason", "bindParameterFailed"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX(__func__).d("reason", "stepFailed"));
                return false;
            }
            return true;
        }
        bool SQLiteBluetoothStorage::getMacToUuid(unordered_map<string, string>* macToUuid) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<std::mutex> lock(m_databaseMutex);
            return getMappingsLocked(COLUMN_MAC, COLUMN_UUID, macToUuid);
        }
        bool SQLiteBluetoothStorage::getMacToCategory(unordered_map<string, string>* macToCategory) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_databaseMutex);
            return getMappingsLocked(COLUMN_MAC, COLUMN_CATEGORY, macToCategory);
        }
        bool SQLiteBluetoothStorage::getUuidToMac(unordered_map<string, string>* uuidToMac) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_databaseMutex);
            return getMappingsLocked(COLUMN_UUID, COLUMN_MAC, uuidToMac);
        }
        bool SQLiteBluetoothStorage::getUuidToCategory(unordered_map<string, string>* uuidToCategory) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_databaseMutex);
            return getMappingsLocked(COLUMN_UUID, COLUMN_CATEGORY, uuidToCategory);
        }
        bool SQLiteBluetoothStorage::getOrderedMac(bool ascending, list<string>* macs) {
            ACSDK_DEBUG5(LX(__func__));
            if (!macs) {
                ACSDK_ERROR(LX(__func__).d("reason", "nullMacs"));
                return false;
            }
            const string order = ascending ? "ASC" : "DESC";
            const string sqlString = "SELECT * FROM " + UUID_TABLE_NAME + " ORDER BY rowid " + order + ";";
            lock_guard<mutex> lock(m_databaseMutex);
            auto statement = m_db.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX(__func__).d("reason", "createStatementFailed"));
                return false;
            }
            unordered_map<string, string> row;
            while(getSingleRowLocked(statement, &row)) {
                if (row.count(COLUMN_MAC) > 0 && !row.at(COLUMN_MAC).empty()) macs->push_back(row.at(COLUMN_MAC));
                row.clear();
            }
            return true;
        };
        bool SQLiteBluetoothStorage::insertByMac(const string& mac, const string& uuid, bool overwrite) {
            ACSDK_DEBUG5(LX(__func__));
            const string operation = overwrite ? "REPLACE" : "INSERT";
            string category = deviceCategoryToString(DeviceCategory::UNKNOWN).data();
            lock_guard<mutex> lock(m_databaseMutex);
            getAssociatedDataLocked(COLUMN_UUID, uuid, COLUMN_CATEGORY, &category);
            return insertEntryLocked(operation, uuid, mac, category);
        }
        bool SQLiteBluetoothStorage::updateByCategory(const string& uuid, const string& category) {
            ACSDK_DEBUG5(LX(__func__));
            string mac = deviceCategoryToString(DeviceCategory::UNKNOWN).data();
            lock_guard<mutex> lock(m_databaseMutex);
            if (getAssociatedDataLocked(COLUMN_UUID, uuid, COLUMN_MAC, &mac)) {
                return updateValueLocked(COLUMN_UUID, uuid, COLUMN_CATEGORY, category);
            }
            ACSDK_ERROR(LX("updateByCategoryFailed").d("reason", "UUID not found in database."));
            return false;
        }
        bool SQLiteBluetoothStorage::remove(const string& mac) {
            ACSDK_DEBUG5(LX(__func__));
            const string sqlString = "DELETE FROM " + UUID_TABLE_NAME + " WHERE " + COLUMN_MAC + "=?;";
            lock_guard<mutex> lock(m_databaseMutex);
            auto statement = m_db.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("removeFailed").d("reason", "createStatementFailed"));
                return false;
            }
            const int MAC_INDEX = 1;
            if (!statement->bindStringParameter(MAC_INDEX, mac)) {
                ACSDK_ERROR(LX("removeFailed").d("reason", "bindFailed"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("removeFailed").d("reason", "stepFailed"));
                return false;
            }
            return true;
        }
        SQLiteBluetoothStorage::SQLiteBluetoothStorage(const string& filePath) : m_db{filePath} {}
    }
}