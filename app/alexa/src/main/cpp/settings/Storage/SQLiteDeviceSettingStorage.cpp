#include <logger/Logger.h>
#include "SQLiteDeviceSettingStorage.h"

namespace alexaClientSDK {
    namespace settings {
        namespace storage {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace configuration;
            using namespace logger;
            static const string TAG("SQLiteDeviceSettingStorage");
            #define LX(event) LogEntry(TAG, event)
            static const string DEVICE_SETTING_DATABASE_CONFIGURATION_ROOT_KEY = "deviceSettings";
            static const string DEVICE_SETTING_DATABASE_DB_FILE_PATH_KEY = "databaseFilePath";
            static const string DEVICE_SETTING_DATABASE_DB_COMPONENT_TABLE_NAMES_SEPARATOR = "_";
            static const string DEVICE_SETTINGS_TABLE_NAME = "deviceSettings";
            static const string DEVICE_SETTINGS_KEY_COLUMN_NAME = "key";
            static const string DEVICE_SETTINGS_VALUE_COLUMN_NAME = "value";
            static const string DEVICE_SETTINGS_STATUS_COLUMN_NAME = "status";
            static const int KEY_INDEX = 0;
            static const int VALUE_INDEX = 1;
            static const int STATUS_INDEX = 2;
             static const string CREATE_DEVICE_SETTINGS_TABLE_SQL_STRING = std::string("CREATE TABLE ") + DEVICE_SETTINGS_TABLE_NAME + " (" +
                     DEVICE_SETTINGS_KEY_COLUMN_NAME  + " TEXT PRIMARY KEY NOT NULL," + DEVICE_SETTINGS_VALUE_COLUMN_NAME+ " TEXT NOT NULL," +
                     DEVICE_SETTINGS_STATUS_COLUMN_NAME + " TEXT NOT NULL);";
            unique_ptr<SQLiteDeviceSettingStorage> SQLiteDeviceSettingStorage::create(const ConfigurationNode& configurationRoot) {
                ACSDK_DEBUG5(LX(__func__));
                auto deviceSettingDatabaseConfigurationRoot = configurationRoot[DEVICE_SETTING_DATABASE_CONFIGURATION_ROOT_KEY];
                if (!deviceSettingDatabaseConfigurationRoot) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config for deviceSetting database")
                                    .d("key", DEVICE_SETTING_DATABASE_CONFIGURATION_ROOT_KEY));
                    return nullptr;
                }
                string deviceSettingDbFilePath;
                if (!deviceSettingDatabaseConfigurationRoot.getString(DEVICE_SETTING_DATABASE_DB_FILE_PATH_KEY, &deviceSettingDbFilePath) ||
                    deviceSettingDbFilePath.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config value")
                                    .d("key", DEVICE_SETTING_DATABASE_DB_FILE_PATH_KEY));
                    return nullptr;
                }
                return unique_ptr<SQLiteDeviceSettingStorage>(new SQLiteDeviceSettingStorage(deviceSettingDbFilePath));
            }
            SQLiteDeviceSettingStorage::SQLiteDeviceSettingStorage(const std::string& dbFilePath) : m_db{dbFilePath} {}
            SQLiteDeviceSettingStorage::~SQLiteDeviceSettingStorage() {
                close();
            }
            bool SQLiteDeviceSettingStorage::open() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_mutex);
                if (m_db.isDatabaseReady()) return true;
                if (!m_db.open()) {
                    if (!m_db.initialize()) {
                        ACSDK_ERROR(LX("openFailed"));
                        return false;
                    }
                }
                if (!m_db.tableExists(DEVICE_SETTINGS_TABLE_NAME) && !createSettingsTable()) {
                    ACSDK_ERROR(LX("openFailed").m("Cannot create " + DEVICE_SETTINGS_TABLE_NAME + " table"));
                    m_db.close();
                    return false;
                }
                return true;
            }
            void SQLiteDeviceSettingStorage::close() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_mutex);
                m_db.close();
            }
            bool SQLiteDeviceSettingStorage::storeSetting(const string& key, const string& value, SettingStatus status) {
                ACSDK_DEBUG5(LX(__func__).d("key", key).d("status", settingStatusToString(status)).sensitive("value", value));
                lock_guard<std::mutex> lock(m_mutex);
                 const std::string sqlString = "REPLACE INTO " + DEVICE_SETTINGS_TABLE_NAME + " (" + DEVICE_SETTINGS_KEY_COLUMN_NAME + ", "
                                        + DEVICE_SETTINGS_VALUE_COLUMN_NAME + ", " + DEVICE_SETTINGS_STATUS_COLUMN_NAME + ") VALUES (?, ?, ?);";
                const int keyIndex = 1;
                const int valueIndex = 2;
                const int statusIndex = 3;
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("storeSettingFailed").d("reason", "createStatementFailed"));
                    return false;
                }
                if (!statement->bindStringParameter(keyIndex, key) || !statement->bindStringParameter(valueIndex, value) ||
                    !statement->bindStringParameter(statusIndex, settingStatusToString(status))) {
                    ACSDK_ERROR(LX("storeSettingFailed").d("reason", "bindParameterFailed"));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("storeSettingFailed").d("reason", "stepFailed"));
                    return false;
                }
                return true;
            }
            bool SQLiteDeviceSettingStorage::storeSettings(const vector<tuple<string, string, SettingStatus>>& data) {
                if (data.empty()) return true;
                lock_guard<mutex> lock(m_mutex);
                string sqlString = "REPLACE INTO " + DEVICE_SETTINGS_TABLE_NAME + " (" + DEVICE_SETTINGS_KEY_COLUMN_NAME + ", " +
                                   DEVICE_SETTINGS_VALUE_COLUMN_NAME + ", " + DEVICE_SETTINGS_STATUS_COLUMN_NAME + ") VALUES ";
                ACSDK_DEBUG5(LX(__func__).sensitive("query", sqlString));
                std::string valueString;
                std::string separator = "";
                for (size_t counter = 0; counter < data.size(); ++counter) {
                    valueString += separator;
                    valueString += "(?, ?, ?) ";
                    separator = ", ";
                }
                sqlString += valueString + ";";
                ACSDK_DEBUG5(LX(__func__).sensitive("query", sqlString));
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("storeSettingsFailed").d("reason", "createStatementFailed"));
                    return false;
                }
                int paramIndex = 1;
                for (auto& row : data) {
                    if (!statement->bindStringParameter(paramIndex++, std::get<KEY_INDEX>(row)) ||
                        !statement->bindStringParameter(paramIndex++, std::get<VALUE_INDEX>(row)) ||
                        !statement->bindStringParameter(paramIndex++, settingStatusToString(std::get<STATUS_INDEX>(row)))) {
                        ACSDK_ERROR(LX("storeSettingsFailed").d("reason", "bindParameterFailed"));
                        return false;
                    }
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("storeSettingFailed").d("reason", "stepFailed"));
                    return false;
                }
                return true;
            }
            DeviceSettingStorageInterface::SettingStatusAndValue SQLiteDeviceSettingStorage::loadSetting(const string& key) {
                ACSDK_DEBUG5(LX(__func__).d("key", key));
                lock_guard<mutex> lock(m_mutex);
                const int VALUE_COLUMN_INDEX = 0;
                const int STATUS_COLUMN_INDEX = 1;
                if (!m_db.isDatabaseReady()) {
                    const string error = "Database not ready";
                    ACSDK_ERROR(LX("loadSettingFailed").d("reason", error));
                    return make_pair(SettingStatus::NOT_AVAILABLE, error);
                }
                 const string sqlString = "SELECT " + DEVICE_SETTINGS_VALUE_COLUMN_NAME + "," + DEVICE_SETTINGS_STATUS_COLUMN_NAME
                                + " FROM " + DEVICE_SETTINGS_TABLE_NAME + " WHERE " + DEVICE_SETTINGS_KEY_COLUMN_NAME + "=?;";
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    const string error = "Can not create SQL Statement.";
                    ACSDK_ERROR(LX("loadSettingFailed").d("reason", error).d("sql", sqlString));
                    return make_pair(SettingStatus::NOT_AVAILABLE, error);
                }
                int boundParam = 1;
                if (!statement->bindStringParameter(boundParam, key)) {
                    const string error = "Binding key to SQL statement failed.";
                    ACSDK_ERROR(LX("loadSettingFailed").d("reason", error));
                    return make_pair(SettingStatus::NOT_AVAILABLE, error);
                }
                if (!statement->step()) {
                    const string error = "SQL statement execution failed.";
                    ACSDK_ERROR(LX("loadSettingFailed").d("reason", error).d("sql", sqlString));
                    return make_pair(SettingStatus::NOT_AVAILABLE, error);
                }
                if (statement->getStepResult() != SQLITE_ROW) {
                    const string error = "Retrieving row from database failed.";
                    ACSDK_ERROR(LX("loadSettingFailed").d("reason", error).d("sql", sqlString));
                    return make_pair(SettingStatus::NOT_AVAILABLE, error);
                }
                string value = statement->getColumnText(VALUE_COLUMN_INDEX);
                SettingStatus status = stringToSettingStatus(statement->getColumnText(STATUS_COLUMN_INDEX));
                ACSDK_DEBUG5(LX("loadSetting").sensitive("value", value));
                return std::make_pair(status, value);
            }
            bool SQLiteDeviceSettingStorage::deleteSetting(const string& key) {
                ACSDK_DEBUG5(LX(__func__).d("key", key));
                lock_guard<std::mutex> lock(m_mutex);
                if (!m_db.isDatabaseReady()) {
                    ACSDK_ERROR(LX("deleteSettingFailed").d("reason", "Database not ready"));
                    return false;
                }
                const string sqlString = "DELETE FROM " + DEVICE_SETTINGS_TABLE_NAME + " WHERE " + DEVICE_SETTINGS_KEY_COLUMN_NAME + "=?;";
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("deleteSettingFailed").d("reason", "createStatementFailed"));
                    return false;
                }
                int boundParam = 1;
                if (!statement->bindStringParameter(boundParam, key)) {
                    ACSDK_ERROR(LX("deleteSettingFailed").d("reason", "bindIntParameterFailed"));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("deleteSettingFailed").d("reason", "stepFailed"));
                    return false;
                }
                return true;
            }
            bool SQLiteDeviceSettingStorage::createSettingsTable() {
                ACSDK_DEBUG5(LX(__func__));
                if (!m_db.performQuery(CREATE_DEVICE_SETTINGS_TABLE_SQL_STRING)) {
                    ACSDK_ERROR(LX("createSettingsTableFailed").m("Table could not be created."));
                    return false;
                }
                return true;
            }
            bool SQLiteDeviceSettingStorage::updateSettingStatus(const string& key, SettingStatus status) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_mutex);
                if (!m_db.isDatabaseReady()) {
                    ACSDK_ERROR(LX("deleteSettingFailed").d("reason", "Database not ready"));
                    return false;
                }
                const string sqlString = "UPDATE " + DEVICE_SETTINGS_TABLE_NAME + " SET " + DEVICE_SETTINGS_STATUS_COLUMN_NAME + "=?"
                                         " WHERE " + DEVICE_SETTINGS_KEY_COLUMN_NAME + "=?;";
                const int statusIndex = 1;
                const int keyIndex = 2;
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("updateSettingStatusFailed").d("reason", "createStatementFailed"));
                    return false;
                }
                if (!statement->bindStringParameter(statusIndex, settingStatusToString(status)) ||
                    !statement->bindStringParameter(keyIndex, key)) {
                    ACSDK_ERROR(LX("updateSettingStatusFailed").d("reason", "bindParameterFailed"));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("updateSettingStatusFailed").d("reason", "stepFailed").d("sql", sqlString));
                    return false;
                }
                return true;
            }
        }
    }
}
