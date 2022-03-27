#include <fstream>
#include <set>
#include <acsdk_alerts/Alarm.h>
#include <acsdk_alerts/Alert.h>
#include <acsdk_alerts/Reminder.h>
#include <acsdk_alerts/Timer.h>
#include <sdkinterfaces/Audio/AudioFactoryInterface.h>
#include <storage/SQLiteStatement.h>
#include <storage/SQLiteUtils.h>
#include <util/file/FileUtils.h>
#include <logger/Logger.h>
#include <util/string/StringUtils.h>
#include "SQLiteAlertStorage.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace storage {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace audio;
            using namespace storage;
            using namespace configuration;
            using namespace file;
            using namespace logger;
            using namespace settings;
            using namespace utils::string;
            using namespace alexaClientSDK::storage::sqliteStorage;
            static const std::string TAG("SQLiteAlertStorage");
            #define LX(event) LogEntry(TAG, event)
            static const std::string ALERTS_CAPABILITY_AGENT_CONFIGURATION_ROOT_KEY = "alertsCapabilityAgent";
            static const std::string ALERTS_CAPABILITY_AGENT_DB_FILE_PATH_KEY = "databaseFilePath";
            static const int ALERT_EVENT_TYPE_ALARM = 1;
            static const int ALERT_EVENT_TYPE_TIMER = 2;
            static const int ALERT_EVENT_TYPE_REMINDER = 3;
            static const std::string ALERT_EVENT_TYPE_ALARM_STRING = "ALARM";
            static const std::string ALERT_EVENT_TYPE_TIMER_STRING = "TIMER";
            static const std::string ALERT_EVENT_TYPE_REMINDER_STRING = "REMINDER";
            static const int ALERT_STATE_UNSET = 1;
            static const int ALERT_STATE_SET = 2;
            static const int ALERT_STATE_ACTIVATING = 3;
            static const int ALERT_STATE_ACTIVE = 4;
            static const int ALERT_STATE_SNOOZING = 5;
            static const int ALERT_STATE_SNOOZED = 6;
            static const int ALERT_STATE_STOPPING = 7;
            static const int ALERT_STATE_STOPPED = 8;
            static const int ALERT_STATE_COMPLETED = 9;
            static const int ALERT_STATE_READY = 10;
            static const std::string DATABASE_COLUMN_ID_NAME = "id";
            static const int ALERTS_DATABASE_VERSION_ONE = 1;
            static const int ALERTS_DATABASE_VERSION_TWO = 2;
            static const std::string ALERTS_TABLE_NAME = "alerts";
            static const std::string ALERTS_V2_TABLE_NAME = "alerts_v2";
            static const std::string CREATE_ALERTS_TABLE_SQL_STRING = std::string("CREATE TABLE ") + ALERTS_V2_TABLE_NAME + " (" + DATABASE_COLUMN_ID_NAME +
                                                                                  " INT PRIMARY KEY NOT NULL,token TEXT NOT NULL,type INT NOT NULL," +
                                                                                  "state INT NOT NULL," + "scheduled_time_unix INT NOT NULL, scheduled_time_iso" +
                                                                                  "_8601 TEXT NOT NULL,asset_loop_count INT NOT NULL," + "asset_loop_pause_milli" +
                                                                                  "seconds INT NOT NULL, background_asset TEXT NOT NULL);";
            static const std::string ALERT_ASSETS_TABLE_NAME = "alertAssets";
            static const std::string CREATE_ALERT_ASSETS_TABLE_SQL_STRING = std::string("CREATE TABLE ") + ALERT_ASSETS_TABLE_NAME + " (" + "id INT PRIMARY " +
                                                                                        "KEY NOT NULL," + "alert_id INT NOT NULL, avs_id TEXT NOT NULL," +
                                                                                        "url TEXT NOT NULL);";
            static const std::string ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME = "alertAssetPlayOrderItems";
            static const std::string CREATE_ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_SQL_STRING = std::string("CREATE TABLE ") + ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME +
                                                                                                        " (id INT PRIMARY KEY NOT NULL, alert_id INT NOT NULL," +
                                                                                                        "asset_play_order_position INT NOT NULL," +
                                                                                                        "asset_play_order_token TEXT NOT NULL);";
            struct AssetOrderItem {
                int index;
                std::string name;
            };
            struct AssetOrderItemCompare {
                bool operator()(const AssetOrderItem& lhs, const AssetOrderItem& rhs) const {
                    return lhs.index < rhs.index;
                }
            };
            static bool alertTypeToDbField(const std::string& alertType, int* dbType) {
                if (ALERT_EVENT_TYPE_ALARM_STRING == alertType) *dbType = ALERT_EVENT_TYPE_ALARM;
                else if (ALERT_EVENT_TYPE_TIMER_STRING == alertType) *dbType = ALERT_EVENT_TYPE_TIMER;
                else if (ALERT_EVENT_TYPE_REMINDER_STRING == alertType) *dbType = ALERT_EVENT_TYPE_REMINDER;
                else {
                    ACSDK_ERROR(LX("alertTypeToDbFieldFailed").m("Could not determine alert type.").d("alert type string", alertType));
                    return false;
                }
                return true;
            }
            static bool alertStateToDbField(Alert::State state, int* dbState) {
                switch(state) {
                    case Alert::State::UNSET:
                        *dbState = ALERT_STATE_UNSET;
                        return true;
                    case Alert::State::SET:
                        *dbState = ALERT_STATE_SET;
                        return true;
                    case Alert::State::READY:
                        *dbState = ALERT_STATE_READY;
                        return true;
                    case Alert::State::ACTIVATING:
                        *dbState = ALERT_STATE_ACTIVATING;
                        return true;
                    case Alert::State::ACTIVE:
                        *dbState = ALERT_STATE_ACTIVE;
                        return true;
                    case Alert::State::SNOOZING:
                        *dbState = ALERT_STATE_SNOOZING;
                        return true;
                    case Alert::State::SNOOZED:
                        *dbState = ALERT_STATE_SNOOZED;
                        return true;
                    case Alert::State::STOPPING:
                        *dbState = ALERT_STATE_STOPPING;
                        return true;
                    case Alert::State::STOPPED:
                        *dbState = ALERT_STATE_STOPPED;
                        return true;
                    case Alert::State::COMPLETED:
                        *dbState = ALERT_STATE_COMPLETED;
                        return true;
                }
                ACSDK_ERROR(LX("alertStateToDbFieldFailed").m("Could not convert alert state.").d("alert object state", static_cast<int>(state)));
                return false;
            }
            static bool dbFieldToAlertState(int dbState, Alert::State* state) {
                switch(dbState) {
                    case ALERT_STATE_UNSET:
                        *state = Alert::State::UNSET;
                        return true;
                    case ALERT_STATE_SET:
                        *state = Alert::State::SET;
                        return true;
                    case ALERT_STATE_READY:
                        *state = Alert::State::READY;
                        return true;
                    case ALERT_STATE_ACTIVATING:
                        *state = Alert::State::ACTIVATING;
                        return true;
                    case ALERT_STATE_ACTIVE:
                        *state = Alert::State::ACTIVE;
                        return true;
                    case ALERT_STATE_SNOOZING:
                        *state = Alert::State::SNOOZING;
                        return true;
                    case ALERT_STATE_SNOOZED:
                        *state = Alert::State::SNOOZED;
                        return true;
                    case ALERT_STATE_STOPPING:
                        *state = Alert::State::STOPPING;
                        return true;
                    case ALERT_STATE_STOPPED:
                        *state = Alert::State::STOPPED;
                        return true;
                    case ALERT_STATE_COMPLETED:
                        *state = Alert::State::COMPLETED;
                        return true;
                }
                ACSDK_ERROR(LX("dbFieldToAlertStateFailed").m("Could not convert db value.").d("db value", dbState));
                return false;
            }
            unique_ptr<SQLiteAlertStorage> SQLiteAlertStorage::create(const ConfigurationNode& configurationRoot,
                                                                      const shared_ptr<AlertsAudioFactoryInterface>& alertsAudioFactory) {
                auto alertsConfigurationRoot = configurationRoot[ALERTS_CAPABILITY_AGENT_CONFIGURATION_ROOT_KEY];
                if (!alertsConfigurationRoot) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config for the Alerts capability agent")
                        .d("key", ALERTS_CAPABILITY_AGENT_CONFIGURATION_ROOT_KEY));
                    return nullptr;
                }
                std::string alertDbFilePath;
                if (!alertsConfigurationRoot.getString(ALERTS_CAPABILITY_AGENT_DB_FILE_PATH_KEY, &alertDbFilePath) ||
                    alertDbFilePath.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config value").d("key", ALERTS_CAPABILITY_AGENT_DB_FILE_PATH_KEY));
                    return nullptr;
                }
                return unique_ptr<SQLiteAlertStorage>(new SQLiteAlertStorage(alertDbFilePath, alertsAudioFactory));
            }
            SQLiteAlertStorage::SQLiteAlertStorage(const std::string& dbFilePath, const shared_ptr<AlertsAudioFactoryInterface>& alertsAudioFactory) :
                                                   m_alertsAudioFactory{alertsAudioFactory}, m_db{dbFilePath} {}
            SQLiteAlertStorage::~SQLiteAlertStorage() {
                close();
            }
            static bool createAlertsTable(SQLiteDatabase* db) {
                if (!db) {
                    ACSDK_ERROR(LX("createAlertsTableFailed").m("null db"));
                    return false;
                }
                if (!db->performQuery(CREATE_ALERTS_TABLE_SQL_STRING)) {
                    ACSDK_ERROR(LX("createAlertsTableFailed").m("Table could not be created."));
                    return false;
                }
                return true;
            }
            static bool createAlertAssetsTable(SQLiteDatabase* db) {
                if (!db) {
                    ACSDK_ERROR(LX("createAlertAssetsTableFailed").m("null db"));
                    return false;
                }
                if (!db->performQuery(CREATE_ALERT_ASSETS_TABLE_SQL_STRING)) {
                    ACSDK_ERROR(LX("createAlertAssetsTableFailed").m("Table could not be created."));
                    return false;
                }
                return true;
            }
            static bool createAlertAssetPlayOrderItemsTable(SQLiteDatabase* db) {
                if (!db) {
                    ACSDK_ERROR(LX("createAlertAssetPlayOrderItemsTableFailed").m("null db"));
                    return false;
                }
                if (!db->performQuery(CREATE_ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_SQL_STRING)) {
                    ACSDK_ERROR(LX("createAlertAssetPlayOrderItemsTableFailed").m("Table could not be created."));
                    return false;
                }
                return true;
            }
            bool SQLiteAlertStorage::createDatabase() {
                if (!m_db.initialize()) {
                    ACSDK_ERROR(LX("createDatabaseFailed"));
                    return false;
                }
                if (!createAlertsTable(&m_db)) {
                    ACSDK_ERROR(LX("createDatabaseFailed").m("Alerts table could not be created."));
                    close();
                    return false;
                }
                if (!createAlertAssetsTable(&m_db)) {
                    ACSDK_ERROR(LX("createDatabaseFailed").m("AlertAssets table could not be created."));
                    close();
                    return false;
                }
                if (!createAlertAssetPlayOrderItemsTable(&m_db)) {
                    ACSDK_ERROR(LX("createDatabaseFailed").m("AlertAssetPlayOrderItems table could not be created."));
                    close();
                    return false;
                }
                return true;
            }
            bool SQLiteAlertStorage::migrateAlertsDbFromV1ToV2() {
                if (m_db.tableExists(ALERTS_V2_TABLE_NAME)) return true;
                if (!createAlertsTable(&m_db)) {
                    ACSDK_ERROR(LX("migrateAlertsDbFromV1ToV2Failed").m("Alert table could not be created."));
                    return false;
                }
                if (!m_db.tableExists(ALERT_ASSETS_TABLE_NAME)) {
                    if (!createAlertAssetsTable(&m_db)) {
                        ACSDK_ERROR(LX("migrateAlertsDbFromV1ToV2Failed").m("AlertAssets table could not be created."));
                        return false;
                    }
                }
                if (!m_db.tableExists(ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME)) {
                    if (!createAlertAssetPlayOrderItemsTable(&m_db)) {
                        ACSDK_ERROR(LX("migrateAlertsDbFromV1ToV2Failed").m("AlertAssetPlayOrderItems table could not be created."));
                        return false;
                    }
                }
                if (m_db.tableExists(ALERTS_TABLE_NAME)) {
                    vector<shared_ptr<Alert>> alertContainer;
                    if (!loadHelper(ALERTS_DATABASE_VERSION_ONE, &alertContainer, nullptr)) {
                        ACSDK_ERROR(LX("migrateAlertsDbFromV1ToV2Failed").m("Could not load V1 alert records."));
                        return false;
                    }
                    for (auto& alert : alertContainer) {
                        if (!store(alert)) {
                            ACSDK_ERROR(LX("migrateAlertsDbFromV1ToV2Failed").m("Could not migrate alert to V2 database."));
                            alert->printDiagnostic();
                            return false;
                        }
                    }
                    const std::string sqlString = "DROP TABLE IF EXISTS " + ALERTS_TABLE_NAME + ";";
                    if (!m_db.performQuery(sqlString)) {
                        ACSDK_ERROR(LX("migrateAlertsDbFromV1ToV2Failed").m("Alerts table could not be dropped."));
                        return false;
                    }
                }
                return true;
            }
            bool SQLiteAlertStorage::open() {
                return m_db.open();
            }
            void SQLiteAlertStorage::close() {
                m_db.close();
            }
            bool SQLiteAlertStorage::alertExists(const std::string& token) {
                const std::string sqlString = "SELECT COUNT(*) FROM " + ALERTS_V2_TABLE_NAME + " WHERE token=?;";
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("alertExistsFailed").m("Could not create statement."));
                    return false;
                }
                int boundParam = 1;
                if (!statement->bindStringParameter(boundParam, token)) {
                    ACSDK_ERROR(LX("alertExistsFailed").m("Could not bind a parameter."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("alertExistsFailed").m("Could not step to next row."));
                    return false;
                }
                const int RESULT_COLUMN_POSITION = 0;
                std::string rowValue = statement->getColumnText(RESULT_COLUMN_POSITION);
                int countValue = 0;
                if (!stringToInt(rowValue.c_str(), &countValue)) {
                    ACSDK_ERROR(LX("alertExistsFailed").d("Could not convert string to integer", rowValue));
                    return false;
                }
                return countValue > 0;
            }
            static bool storeAlertAssets(
                SQLiteDatabase* db,
                int alertId,
                const unordered_map<std::string, Alert::Asset>& assets) {
                if (assets.empty()) return true;
                const std::string sqlString = "INSERT INTO " + ALERT_ASSETS_TABLE_NAME + " (id, alert_id, avs_id, url) VALUES (?, ?, ?, ?);";
                int id = 0;
                if (!getTableMaxIntValue(db, ALERT_ASSETS_TABLE_NAME, DATABASE_COLUMN_ID_NAME, &id)) {
                    ACSDK_ERROR(LX("storeAlertAssetsFailed").m("Cannot generate asset id."));
                    return false;
                }
                id++;
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("storeAlertAssetsFailed").m("Could not create statement."));
                    return false;
                }
                for (auto& assetIter : assets) {
                    auto& asset = assetIter.second;
                    int boundParam = 1;
                    if (!statement->bindIntParameter(boundParam++, id) || !statement->bindIntParameter(boundParam++, alertId) ||
                        !statement->bindStringParameter(boundParam++, asset.id) ||
                        !statement->bindStringParameter(boundParam, asset.url)) {
                        ACSDK_ERROR(LX("storeAlertAssetsFailed").m("Could not bind a parameter."));
                        return false;
                    }
                    if (!statement->step()) {
                        ACSDK_ERROR(LX("storeAlertAssetsFailed").m("Could not step to next row."));
                        return false;
                    }
                    if (!statement->reset()) {
                        ACSDK_ERROR(LX("storeAlertAssetsFailed").m("Could not reset the statement."));
                        return false;
                    }
                    id++;
                }
                return true;
            }
            static bool storeAlertAssetPlayOrderItems(
                SQLiteDatabase* db,
                int alertId,
                const std::vector<std::string>& assetPlayOrderItems) {
                if (assetPlayOrderItems.empty()) return true;
                const std::string sqlString = "INSERT INTO " + ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME + " (id, alert_id, asset_play_order_position, " +
                                              "asset_play_order_token) VALUES (?, ?, ?, ?);";
                int id = 0;
                if (!getTableMaxIntValue(db, ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME, DATABASE_COLUMN_ID_NAME, &id)) {
                    ACSDK_ERROR(LX("storeAlertAssetPlayOrderItemsFailed").m("Cannot generate asset id."));
                    return false;
                }
                id++;
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("storeAlertAssetPlayOrderItemsFailed").m("Could not create statement."));
                    return false;
                }
                int itemIndex = 1;
                for (auto& assetId : assetPlayOrderItems) {
                    int boundParam = 1;
                    if (!statement->bindIntParameter(boundParam++, id) || !statement->bindIntParameter(boundParam++, alertId) ||
                        !statement->bindIntParameter(boundParam++, itemIndex) ||
                        !statement->bindStringParameter(boundParam, assetId)) {
                        ACSDK_ERROR(LX("storeAlertAssetPlayOrderItemsFailed").m("Could not bind a parameter."));
                        return false;
                    }
                    if (!statement->step()) {
                        ACSDK_ERROR(LX("storeAlertAssetPlayOrderItemsFailed").m("Could not step to next row."));
                        return false;
                    }
                    if (!statement->reset()) {
                        ACSDK_ERROR(LX("storeAlertAssetPlayOrderItemsFailed").m("Could not reset the statement."));
                        return false;
                    }
                    id++;
                    itemIndex++;
                }
                return true;
            }
            bool SQLiteAlertStorage::store(std::shared_ptr<Alert> alert) {
                if (!alert) {
                    ACSDK_ERROR(LX("storeFailed").m("Alert parameter is nullptr"));
                    return false;
                }
                if (alertExists(alert->getToken())) {
                    ACSDK_ERROR(LX("storeFailed").m("Alert already exists.").d("token", alert->getToken()));
                    return false;
                }
                const std::string sqlString = "INSERT INTO " + ALERTS_V2_TABLE_NAME + " (id, token, type, state, scheduled_time_unix, scheduled_time_iso_8601, " +
                                              "asset_loop_count, asset_loop_pause_milliseconds, background_asset) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
                int id = 0;
                if (!getTableMaxIntValue(&m_db, ALERTS_V2_TABLE_NAME, DATABASE_COLUMN_ID_NAME, &id)) {
                    ACSDK_ERROR(LX("storeFailed").m("Cannot generate alert id."));
                    return false;
                }
                id++;
                int alertType = ALERT_EVENT_TYPE_ALARM;
                if (!alertTypeToDbField(alert->getTypeName(), &alertType)) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not convert type name to db field."));
                    return false;
                }
                int alertState = ALERT_STATE_SET;
                if (!alertStateToDbField(alert->getState(), &alertState)) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not convert alert state to db field."));
                    return false;
                }
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not create statement."));
                    return false;
                }
                int boundParam = 1;
                auto token = alert->getToken();
                auto iso8601 = alert->getScheduledTime_ISO_8601();
                auto assetId = alert->getBackgroundAssetId();
                if (!statement->bindIntParameter(boundParam++, id) || !statement->bindStringParameter(boundParam++, token) ||
                    !statement->bindIntParameter(boundParam++, alertType) || !statement->bindIntParameter(boundParam++, alertState) ||
                    !statement->bindInt64Parameter(boundParam++, alert->getScheduledTime_Unix()) ||
                    !statement->bindStringParameter(boundParam++, iso8601) || !statement->bindIntParameter(boundParam++, alert->getLoopCount()) ||
                    !statement->bindIntParameter(boundParam++, alert->getLoopPause().count()) || !statement->bindStringParameter(boundParam, assetId)) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not bind parameter."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not perform step."));
                    return false;
                }
                Alert::StaticData staticData;
                alert->getAlertData(&staticData, nullptr);
                staticData.dbId = id;
                if (!alert->setAlertData(&staticData, nullptr)) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not set alert data."));
                    return false;
                }
                if (!storeAlertAssets(&m_db, id, alert->getAssetConfiguration().assets)) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not store alertAssets."));
                    return false;
                }
                if (!storeAlertAssetPlayOrderItems(&m_db, id, alert->getAssetConfiguration().assetPlayOrderItems)) {
                    ACSDK_ERROR(LX("storeFailed").m("Could not store alertAssetPlayOrderItems."));
                    return false;
                }
                return true;
            }
            static bool loadAlertAssets(SQLiteDatabase* db, std::map<int, vector<Alert::Asset>>* alertAssetsMap) {
                const std::string sqlString = "SELECT * FROM " + ALERT_ASSETS_TABLE_NAME + ";";
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("loadAlertAssetsFailed").m("Could not create statement."));
                    return false;
                }
                int alertId = 0;
                std::string avsId;
                std::string url;
                if (!statement->step()) {
                    ACSDK_ERROR(LX("loadAlertAssetsFailed").m("Could not perform step."));
                    return false;
                }
                while(SQLITE_ROW == statement->getStepResult()) {
                    int numberColumns = statement->getColumnCount();
                    for (int i = 0; i < numberColumns; i++) {
                        std::string columnName = statement->getColumnName(i);
                        if ("alert_id" == columnName) alertId = statement->getColumnInt(i);
                        else if ("avs_id" == columnName) avsId = statement->getColumnText(i);
                        else if ("url" == columnName) url = statement->getColumnText(i);
                    }
                    (*alertAssetsMap)[alertId].push_back(Alert::Asset(avsId, url));
                    statement->step();
                }
                return true;
            }
            static bool loadAlertAssetPlayOrderItems(
                SQLiteDatabase* db,
                map<int, set<AssetOrderItem, AssetOrderItemCompare>>* alertAssetOrderItemsMap) {
                const std::string sqlString = "SELECT * FROM " + ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME + ";";
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("loadAlertAssetPlayOrderItemsFailed").m("Could not create statement."));
                    return false;
                }
                int alertId = 0;
                int playOrderPosition = 0;
                std::string playOrderToken;
                if (!statement->step()) {
                    ACSDK_ERROR(LX("loadAlertAssetPlayOrderItemsFailed").m("Could not perform step."));
                    return false;
                }
                while(SQLITE_ROW == statement->getStepResult()) {
                    int numberColumns = statement->getColumnCount();
                    for (int i = 0; i < numberColumns; i++) {
                        std::string columnName = statement->getColumnName(i);
                        if ("alert_id" == columnName) alertId = statement->getColumnInt(i);
                        else if ("asset_play_order_position" == columnName) playOrderPosition = statement->getColumnInt(i);
                        else if ("asset_play_order_token" == columnName) playOrderToken = statement->getColumnText(i);
                    }
                    (*alertAssetOrderItemsMap)[alertId].insert(AssetOrderItem{playOrderPosition, playOrderToken});
                    statement->step();
                }
                return true;
            }
            bool SQLiteAlertStorage::loadHelper(int dbVersion, vector<shared_ptr<Alert>>* alertContainer, shared_ptr<DeviceSettingsManager> settingsManager) {
                if (!alertContainer) {
                    ACSDK_ERROR(LX("loadHelperFailed").m("Alert container parameter is nullptr."));
                    return false;
                }
                if (dbVersion != ALERTS_DATABASE_VERSION_ONE && dbVersion != ALERTS_DATABASE_VERSION_TWO) {
                    ACSDK_ERROR(LX("loadHelperFailed").d("Invalid version", dbVersion));
                    return false;
                }
                map<int, vector<Alert::Asset>> alertAssetsMap;
                if (!loadAlertAssets(&m_db, &alertAssetsMap)) {
                    ACSDK_ERROR(LX("loadHelperFailed").m("Could not load alert assets."));
                    return false;
                }
                map<int, set<AssetOrderItem, AssetOrderItemCompare>> alertAssetOrderItemsMap;
                if (!loadAlertAssetPlayOrderItems(&m_db, &alertAssetOrderItemsMap)) {
                    ACSDK_ERROR(LX("loadHelperFailed").m("Could not load alert asset play order items."));
                    return false;
                }
                std::string alertsTableName = ALERTS_TABLE_NAME;
                if (ALERTS_DATABASE_VERSION_TWO == dbVersion) alertsTableName = ALERTS_V2_TABLE_NAME;
                const std::string sqlString = "SELECT * FROM " + alertsTableName + ";";
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("loadHelperFailed").m("Could not create statement."));
                    return false;
                }
                int id = 0;
                std::string token;
                int type = 0;
                int state = 0;
                std::string scheduledTime_ISO_8601;
                int loopCount = 0;
                int loopPauseInMilliseconds = 0;
                std::string backgroundAssetId;
                if (!statement->step()) {
                    ACSDK_ERROR(LX("loadHelperFailed").m("Could not perform step."));
                    return false;
                }
                while(SQLITE_ROW == statement->getStepResult()) {
                    int numberColumns = statement->getColumnCount();
                    for (int i = 0; i < numberColumns; i++) {
                        std::string columnName = statement->getColumnName(i);
                        if ("id" == columnName) id = statement->getColumnInt(i);
                        else if ("token" == columnName) token = statement->getColumnText(i);
                        else if ("type" == columnName) type = statement->getColumnInt(i);
                        else if ("state" == columnName) state = statement->getColumnInt(i);
                        else if ("scheduled_time_iso_8601" == columnName) scheduledTime_ISO_8601 = statement->getColumnText(i);
                        else if ("asset_loop_count" == columnName) loopCount = statement->getColumnInt(i);
                        else if ("asset_loop_pause_milliseconds" == columnName) loopPauseInMilliseconds = statement->getColumnInt(i);
                        else if ("background_asset" == columnName) backgroundAssetId = statement->getColumnText(i);
                    }
                    std::shared_ptr<Alert> alert;
                    if (ALERT_EVENT_TYPE_ALARM == type) {
                        alert = make_shared<Alarm>(m_alertsAudioFactory->alarmDefault(), m_alertsAudioFactory->alarmShort(), settingsManager);
                    } else if (ALERT_EVENT_TYPE_TIMER == type) {
                        alert = make_shared<Timer>(m_alertsAudioFactory->timerDefault(), m_alertsAudioFactory->timerShort(), settingsManager);
                    } else if (ALERT_EVENT_TYPE_REMINDER == type) {
                        alert = make_shared<Reminder>(m_alertsAudioFactory->reminderDefault(), m_alertsAudioFactory->reminderShort(), settingsManager);
                    } else {
                        ACSDK_ERROR(LX("loadHelperFailed").m("Could not instantiate an alert object.").d("type read from database", type));
                        return false;
                    }
                    Alert::DynamicData dynamicData;
                    Alert::StaticData staticData;
                    alert->getAlertData(&staticData, &dynamicData);
                    staticData.dbId = id;
                    staticData.token = token;
                    dynamicData.timePoint.setTime_ISO_8601(scheduledTime_ISO_8601);
                    dynamicData.loopCount = loopCount;
                    dynamicData.assetConfiguration.loopPause = std::chrono::milliseconds{loopPauseInMilliseconds};
                    dynamicData.assetConfiguration.backgroundAssetId = backgroundAssetId;
                    if (alertAssetsMap.find(id) != alertAssetsMap.end()) {
                        for (auto& mapEntry : alertAssetsMap[id]) dynamicData.assetConfiguration.assets[mapEntry.id] = mapEntry;
                    }
                    if (alertAssetOrderItemsMap.find(id) != alertAssetOrderItemsMap.end()) {
                        for (auto& mapEntry : alertAssetOrderItemsMap[id]) dynamicData.assetConfiguration.assetPlayOrderItems.push_back(mapEntry.name);
                    }
                    if (!dbFieldToAlertState(state, &dynamicData.state)) {
                        ACSDK_ERROR(LX("loadHelperFailed").m("Could not convert alert state."));
                        return false;
                    }
                    if (!alert->setAlertData(&staticData, &dynamicData)) {
                        ACSDK_ERROR(LX("loadHelperFailed").m("Could not set alert data."));
                        return false;
                    }
                    alertContainer->push_back(alert);
                    statement->step();
                }
                statement->finalize();
                return true;
            }
            bool SQLiteAlertStorage::load(
                std::vector<std::shared_ptr<Alert>>* alertContainer,
                std::shared_ptr<settings::DeviceSettingsManager> settingsManager) {
                return loadHelper(ALERTS_DATABASE_VERSION_TWO, alertContainer, settingsManager);
            }

            bool SQLiteAlertStorage::modify(std::shared_ptr<Alert> alert) {
                if (!alert) {
                    ACSDK_ERROR(LX("modifyFailed").m("Alert parameter is nullptr."));
                    return false;
                }
                if (!alertExists(alert->getToken())) {
                    ACSDK_ERROR(LX("modifyFailed").m("Cannot modify alert.").d("token", alert->getToken()));
                    return false;
                }
                const std::string sqlString = "UPDATE " + ALERTS_V2_TABLE_NAME + " SET state=?, scheduled_time_unix=?, scheduled_time_iso_8601=? " + "WHERE id=?;";
                int alertState = ALERT_STATE_SET;
                if (!alertStateToDbField(alert->getState(), &alertState)) {
                    ACSDK_ERROR(LX("modifyFailed").m("Cannot convert state."));
                    return false;
                }
                auto statement = m_db.createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("modifyFailed").m("Could not create statement."));
                    return false;
                }
                int boundParam = 1;
                auto iso8601 = alert->getScheduledTime_ISO_8601();
                if (!statement->bindIntParameter(boundParam++, alertState) || !statement->bindInt64Parameter(boundParam++, alert->getScheduledTime_Unix()) ||
                    !statement->bindStringParameter(boundParam++, iso8601) || !statement->bindIntParameter(boundParam++, alert->getId())) {
                    ACSDK_ERROR(LX("modifyFailed").m("Could not bind a parameter."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("modifyFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
            static bool eraseAlert(SQLiteDatabase* db, int alertId) {
                const std::string sqlString = "DELETE FROM " + ALERTS_V2_TABLE_NAME + " WHERE id=?;";
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("eraseAlertByAlertIdFailed").m("Could not create statement."));
                    return false;
                }
                int boundParam = 1;
                if (!statement->bindIntParameter(boundParam, alertId)) {
                    ACSDK_ERROR(LX("eraseAlertByAlertIdFailed").m("Could not bind a parameter."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("eraseAlertByAlertIdFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
            static bool eraseAlertAssets(SQLiteDatabase* db, int alertId) {
                const std::string sqlString = "DELETE FROM " + ALERT_ASSETS_TABLE_NAME + " WHERE alert_id=?;";
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("eraseAlertAssetsFailed").m("Could not create statement."));
                    return false;
                }
                int boundParam = 1;
                if (!statement->bindIntParameter(boundParam, alertId)) {
                    ACSDK_ERROR(LX("eraseAlertAssetsFailed").m("Could not bind a parameter."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("eraseAlertAssetsFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
            static bool eraseAlertAssetPlayOrderItems(SQLiteDatabase* db, int alertId) {
                const std::string sqlString = "DELETE FROM " + ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME + " WHERE alert_id=?;";
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("eraseAlertAssetPlayOrderItemsFailed").m("Could not create statement."));
                    return false;
                }
                int boundParam = 1;
                if (!statement->bindIntParameter(boundParam, alertId)) {
                    ACSDK_ERROR(LX("eraseAlertAssetPlayOrderItemsFailed").m("Could not bind a parameter."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("eraseAlertAssetPlayOrderItemsFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
            static bool eraseAlertByAlertId(SQLiteDatabase* db, int alertId) {
                if (!db) {
                    ACSDK_ERROR(LX("eraseAlertByAlertIdFailed").m("db is nullptr."));
                    return false;
                }
                if (!eraseAlert(db, alertId)) {
                    ACSDK_ERROR(LX("eraseAlertByAlertIdFailed").m("Could not erase alert table items."));
                    return false;
                }
                if (!eraseAlertAssets(db, alertId)) {
                    ACSDK_ERROR(LX("eraseAlertByAlertIdFailed").m("Could not erase alertAsset table items."));
                    return false;
                }
                if (!eraseAlertAssetPlayOrderItems(db, alertId)) {
                    ACSDK_ERROR(LX("eraseAlertByAlertIdFailed").m("Could not erase alertAssetPlayOrderItems table items."));
                    return false;
                }
                return true;
            }
            bool SQLiteAlertStorage::erase(std::shared_ptr<Alert> alert) {
                if (!alert) {
                    ACSDK_ERROR(LX("eraseFailed").m("Alert parameter is nullptr."));
                    return false;
                }
                if (!alertExists(alert->getToken())) {
                    ACSDK_ERROR(LX("eraseFailed").m("Cannot delete alert - not in database.").d("token", alert->getToken()));
                    return false;
                }
                return eraseAlertByAlertId(&m_db, alert->getId());
            }
            bool SQLiteAlertStorage::bulkErase(const std::list<std::shared_ptr<Alert>>& alertList) {
                if (alertList.empty()) return true;
                auto transaction = m_db.beginTransaction();
                if (!transaction) {
                    ACSDK_ERROR(LX("bulkEraseFailed").d("reason", "Failed to begin transaction."));
                    return false;
                }
                for (auto& alert : alertList) {
                    if (!erase(alert)) {
                        ACSDK_ERROR(LX("bulkEraseFailed").d("reason", "Failed to erase alert"));
                        if (!transaction->rollback()) { ACSDK_ERROR(LX("bulkEraseFailed").d("reason", "Failed to rollback alerts storage changes")); }
                        return false;
                    }
                }
                if (!transaction->commit()) {
                    ACSDK_ERROR(LX("bulkEraseFailed").d("reason", "Failed to commit alerts storage changes"));
                    return false;
                }
                return true;
            }
            bool SQLiteAlertStorage::clearDatabase() {
                const vector<std::string> tablesToClear = { ALERTS_V2_TABLE_NAME, ALERT_ASSETS_TABLE_NAME, ALERT_ASSET_PLAY_ORDER_ITEMS_TABLE_NAME };
                for (auto& tableName : tablesToClear) {
                    if (!m_db.clearTable(tableName)) {
                        ACSDK_ERROR(LX("clearDatabaseFailed").d("could not clear table", tableName));
                        return false;
                    }
                }
                return true;
            }
            static void printOneLineSummary(SQLiteDatabase* db) {
                int numberAlerts = 0;
                if (!getNumberTableRows(db, ALERTS_V2_TABLE_NAME, &numberAlerts)) {
                    ACSDK_ERROR(LX("printOneLineSummaryFailed").m("could not read number of alerts."));
                    return;
                }
                ACSDK_INFO(LX("ONE-LINE-STAT: Number of alerts:" + std::to_string(numberAlerts)));
            }
            static void printAlertsSummary(
                SQLiteDatabase* db,
                const std::vector<std::shared_ptr<Alert>>& alerts,
                bool shouldPrintEverything = false) {
                printOneLineSummary(db);
                for (auto& alert : alerts) alert->printDiagnostic();
            }
            void SQLiteAlertStorage::printStats(StatLevel level) {
                vector<shared_ptr<Alert>> alerts;
                load(&alerts, nullptr);
                switch(level) {
                    case StatLevel::ONE_LINE: printOneLineSummary(&m_db); break;
                    case StatLevel::ALERTS_SUMMARY: printAlertsSummary(&m_db, alerts, false); break;
                    case StatLevel::EVERYTHING: printAlertsSummary(&m_db, alerts, true); break;
                }
            }
        }
    }
}