#include <logger/Logger.h>
#include "SQLiteCapabilitiesDelegateStorage.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace storage {
            using namespace configuration;
            using namespace logger;
            static const string TAG("SQLiteCapabilitiesDelegateStorage");
            #define LX(event) LogEntry(TAG, event)
            static const string CAPABILITIES_DELEGATE_CONFIGURATION_ROOT_KEY = "capabilitiesDelegate";
            static const string DB_FILE_PATH = "databaseFilePath";
            static const string ENDPOINT_CONFIG_TABLE_NAME = "endpointConfigTable";
            static const string DATABASE_COLUMN_ENDPOINT_ID_NAME = "endpointId";
            static const string DATABASE_COLUMN_ENDPOINT_CONFIG_NAME = "endpointConfig";
            static const string CREATE_ENDPOINT_CONFIG_TABLE_SQL_STRING = string("CREATE TABLE ") + ENDPOINT_CONFIG_TABLE_NAME + " (" + DATABASE_COLUMN_ENDPOINT_ID_NAME +
                                                                          " TEXT NOT NULL UNIQUE," + DATABASE_COLUMN_ENDPOINT_CONFIG_NAME + " TEXT NOT NULL);";
            unique_ptr<SQLiteCapabilitiesDelegateStorage> SQLiteCapabilitiesDelegateStorage::create(
                const ConfigurationNode& configurationRoot) {
                ACSDK_DEBUG5(LX(__func__));
                auto capabilitiesStorageRoot = configurationRoot[CAPABILITIES_DELEGATE_CONFIGURATION_ROOT_KEY];
                if (!capabilitiesStorageRoot) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load capabilitiesDelegate root"));
                    return nullptr;
                }
                string dbFilePath;
                if (!capabilitiesStorageRoot.getString(DB_FILE_PATH, &dbFilePath) || dbFilePath.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load database file path"));
                    return nullptr;
                }
                return unique_ptr<SQLiteCapabilitiesDelegateStorage>(new SQLiteCapabilitiesDelegateStorage(dbFilePath));
            }
            SQLiteCapabilitiesDelegateStorage::SQLiteCapabilitiesDelegateStorage(const string& dbFilePath) : m_database{dbFilePath} {}
            SQLiteCapabilitiesDelegateStorage::~SQLiteCapabilitiesDelegateStorage() {
                close();
            }
            bool SQLiteCapabilitiesDelegateStorage::createDatabase() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_mutex);
                if (!m_database.initialize()) {
                    ACSDK_ERROR(LX("createDatabaseFailed").d("reason", "unable to initialize Database"));
                    return false;
                }
                if (!m_database.performQuery(CREATE_ENDPOINT_CONFIG_TABLE_SQL_STRING)) {
                    ACSDK_ERROR(LX("createDatabaseFailed").d("reason", "unable to create Endpoint Config table."));
                    close();
                    return false;
                }
                return true;
            }
            bool SQLiteCapabilitiesDelegateStorage::open() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                return m_database.open();
            }
            void SQLiteCapabilitiesDelegateStorage::close() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                m_database.close();
            }
            bool SQLiteCapabilitiesDelegateStorage::storeLocked(const string& endpointId, const string& endpointConfig) {
                ACSDK_DEBUG5(LX(__func__));
                const string sqlString = "REPLACE INTO " + ENDPOINT_CONFIG_TABLE_NAME + " (" + DATABASE_COLUMN_ENDPOINT_ID_NAME + ", " +
                                         DATABASE_COLUMN_ENDPOINT_CONFIG_NAME + ") VALUES (?, ?);";
                auto statement = m_database.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not create statement"));
                    return false;
                }
                int ENDPOINT_ID_INDEX = 1;
                int ENDPOINT_CONFIG_INDEX = 2;
                if (!statement->bindStringParameter(ENDPOINT_ID_INDEX, endpointId) ||
                    !statement->bindStringParameter(ENDPOINT_CONFIG_INDEX, endpointConfig)) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not bind parameter"));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
            bool SQLiteCapabilitiesDelegateStorage::store(const string& endpointId, const string& endpointConfig) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                return storeLocked(endpointId, endpointConfig);
            }
            bool SQLiteCapabilitiesDelegateStorage::store(const unordered_map<string, string>& endpointIdToConfigMap) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                for (auto it = endpointIdToConfigMap.begin(); it != endpointIdToConfigMap.end(); ++it) {
                    if (!storeLocked(it->first, it->second)) {
                        ACSDK_ERROR(LX("storeFailed").m("Could not store endpointConfigMap"));
                        return false;
                    }
                }
                return true;
            }
            bool SQLiteCapabilitiesDelegateStorage::load(std::unordered_map<std::string, std::string>* endpointConfigMap) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                if (!endpointConfigMap || !endpointConfigMap->empty()) {
                    ACSDK_ERROR(LX("loadFailed").d("reason", "Invalid endpointConfigMap"));
                    return false;
                }
                const string sqlString = "SELECT * FROM " + ENDPOINT_CONFIG_TABLE_NAME + ";";
                auto statement = m_database.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("loadFailed").m("Could not create statement."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("loadFailed").m("Could not perform step."));
                    return false;
                }
                string endpointId;
                string endpointConfig;
                while(SQLITE_ROW == statement->getStepResult()) {
                    int numberColumns = statement->getColumnCount();
                    for (int i = 0; i < numberColumns; ++i) {
                        string columnName = statement->getColumnName(i);
                        if (DATABASE_COLUMN_ENDPOINT_ID_NAME == columnName) endpointId = statement->getColumnText(i);
                        else if (DATABASE_COLUMN_ENDPOINT_CONFIG_NAME == columnName) endpointConfig = statement->getColumnText(i);
                    }
                    endpointConfigMap->insert({endpointId, endpointConfig});
                    statement->step();
                }
                return true;
            }
            bool SQLiteCapabilitiesDelegateStorage::load(const string& endpointId, string* endpointConfig) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                if (!endpointConfig) {
                    ACSDK_ERROR(LX("loadFailed").d("reason", "Invalid endpointConfig"));
                    return false;
                }
                const string sqlString = "SELECT * FROM " + ENDPOINT_CONFIG_TABLE_NAME + " WHERE " + DATABASE_COLUMN_ENDPOINT_ID_NAME + "=?;";
                auto statement = m_database.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("loadFailed").m("Could not create statement."));
                    return false;
                }
                int ENDPOINT_ID_INDEX = 1;
                if (!statement->bindStringParameter(ENDPOINT_ID_INDEX, endpointId)) {
                    ACSDK_ERROR(LX("loadFailed").m("Could not bind endpointId"));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("loadFailed").m("Could not perform step."));
                    return false;
                }
                if (SQLITE_ROW == statement->getStepResult()) *endpointConfig = statement->getColumnText(1);
                return true;
            }
            bool SQLiteCapabilitiesDelegateStorage::eraseLocked(const std::string& endpointId) {
                ACSDK_DEBUG5(LX(__func__));
                const string sqlString = "DELETE FROM " + ENDPOINT_CONFIG_TABLE_NAME + " WHERE " + DATABASE_COLUMN_ENDPOINT_ID_NAME + "=?;";
                auto statement = m_database.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("eraseFailed").m("Could not create statement."));
                    return false;
                }
                int ENDPOINT_ID_INDEX = 1;
                if (!statement->bindStringParameter(ENDPOINT_ID_INDEX, endpointId)) {
                    ACSDK_DEBUG5(LX("eraseFailed").m("Could not bind endpointId."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("eraseFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
            bool SQLiteCapabilitiesDelegateStorage::erase(const string& endpointId) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<std::mutex> lock{m_mutex};
                return eraseLocked(endpointId);
            }
            bool SQLiteCapabilitiesDelegateStorage::erase(
                const unordered_map<string, string>& endpointIdToConfigMap) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                for (auto endpointIdToConfig : endpointIdToConfigMap) {
                    if (!eraseLocked(endpointIdToConfig.first)) return false;
                }
                return true;
            }
            bool SQLiteCapabilitiesDelegateStorage::clearDatabase() {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock{m_mutex};
                return m_database.clearTable(ENDPOINT_CONFIG_TABLE_NAME);
            }
        }
    }
}