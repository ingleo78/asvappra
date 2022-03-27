#include <storage/SQLiteUtils.h>
#include <storage/SQLiteStatement.h>
#include <util/file/FileUtils.h>
#include <logger/Logger.h>
#include "SQLiteNotificationsStorage.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        using namespace avsCommon::utils::file;
        using namespace avsCommon::utils::logger;
        using namespace alexaClientSDK::storage::sqliteStorage;
        static const string TAG("SQLiteNotificationsStorage");
        #define LX(event) LogEntry(TAG, event)
        static const string NOTIFICATIONS_CONFIGURATION_ROOT_KEY = "notifications";
        static const string NOTIFICATIONS_DB_FILE_PATH_KEY = "databaseFilePath";
        static const string NOTIFICATION_INDICATOR_TABLE_NAME = "notificationIndicators";
        static const string DATABASE_COLUMN_PERSIST_VISUAL_INDICATOR_NAME = "persistVisualIndicator";
        static const string DATABASE_COLUMN_PLAY_AUDIO_INDICATOR_NAME = "playAudioIndicator";
        static const string DATABASE_COLUMN_ASSET_ID_NAME = "assetId";
        static const string DATABASE_COLUMN_ASSET_URL_NAME = "assetUrl";
        static const string CREATE_NOTIFICATION_INDICATOR_TABLE_SQL_STRING = string("CREATE TABLE ") + NOTIFICATION_INDICATOR_TABLE_NAME +
                                                                             " (" + DATABASE_COLUMN_PERSIST_VISUAL_INDICATOR_NAME +
                                                                             " INT NOT NULL," + DATABASE_COLUMN_PLAY_AUDIO_INDICATOR_NAME +
                                                                             " INT NOT NULL," + DATABASE_COLUMN_ASSET_ID_NAME +
                                                                             " TEXT NOT NULL," + DATABASE_COLUMN_ASSET_URL_NAME +
                                                                             " TEXT NOT NULL);";
        static const string INDICATOR_STATE_NAME = "indicatorState";
        static const string CREATE_INDICATOR_STATE_TABLE_SQL_STRING = string("CREATE TABLE ") + INDICATOR_STATE_NAME + " (" +
                                                                      INDICATOR_STATE_NAME + " INT NOT NULL);";
        static const NotificationsStorageInterface::IndicatorState DEFAULT_INDICATOR_STATE = NotificationsStorageInterface::IndicatorState::OFF;
        unique_ptr<SQLiteNotificationsStorage> SQLiteNotificationsStorage::create(
            const avsCommon::utils::configuration::ConfigurationNode& configurationRoot) {
            auto notificationConfigurationRoot = configurationRoot[NOTIFICATIONS_CONFIGURATION_ROOT_KEY];
            if (!notificationConfigurationRoot) {
                ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config for the Notification Storage database")
                    .d("key", NOTIFICATIONS_CONFIGURATION_ROOT_KEY));
                return nullptr;
            }
            std::string notificationDatabaseFilePath;
            if (!notificationConfigurationRoot.getString(NOTIFICATIONS_DB_FILE_PATH_KEY, &notificationDatabaseFilePath) ||
                notificationDatabaseFilePath.empty()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config value").d("key", NOTIFICATIONS_DB_FILE_PATH_KEY));
                return nullptr;
            }
            return std::unique_ptr<SQLiteNotificationsStorage>(new SQLiteNotificationsStorage(notificationDatabaseFilePath));
        }
        SQLiteNotificationsStorage::SQLiteNotificationsStorage(const string& databaseFilePath) : m_database{databaseFilePath} {}
        bool SQLiteNotificationsStorage::createDatabase() {
            if (!m_database.initialize()) {
                ACSDK_ERROR(LX("createDatabaseFailed").d("reason", "SQLiteCreateDatabaseFailed"));
                return false;
            }
            if (!m_database.performQuery(CREATE_NOTIFICATION_INDICATOR_TABLE_SQL_STRING)) {
                ACSDK_ERROR(LX("createDatabaseFailed").d("reason", "failed to create notification indicator table"));
                close();
                return false;
            }
            if (!m_database.performQuery(CREATE_INDICATOR_STATE_TABLE_SQL_STRING)) {
                ACSDK_ERROR(LX("createDatabaseFailed").d("reason", "failed to create indicator state table"));
                close();
                return false;
            }
            if (!setIndicatorState(IndicatorState::OFF)) {
                ACSDK_ERROR(LX("createDatabaseFailed").d("reason", "failed to set default indicator state"));
                close();
                return false;
            }
            return true;
        }
        bool SQLiteNotificationsStorage::open() {
            if (!m_database.open()) {
                ACSDK_ERROR(LX("openFailed").d("reason", "openSQLiteDatabaseFailed"));
                return false;
            }
            if (!m_database.tableExists(NOTIFICATION_INDICATOR_TABLE_NAME)) {
                ACSDK_ERROR(LX("openFailed").d("reason", "table doesn't exist").d("TableName", NOTIFICATION_INDICATOR_TABLE_NAME));
                return false;
            }
            if (!m_database.tableExists(INDICATOR_STATE_NAME)) {
                ACSDK_ERROR(LX("openFailed").d("reason", "table doesn't exist").d("TableName", INDICATOR_STATE_NAME));
                return false;
            }
            return true;
        }
        void SQLiteNotificationsStorage::close() {
            m_database.close();
        }
        bool SQLiteNotificationsStorage::enqueue(const NotificationIndicator& notificationIndicator) {
            const string sqlString = "INSERT INTO " + NOTIFICATION_INDICATOR_TABLE_NAME + " (" + DATABASE_COLUMN_PERSIST_VISUAL_INDICATOR_NAME +
                                     "," + DATABASE_COLUMN_PLAY_AUDIO_INDICATOR_NAME + "," + DATABASE_COLUMN_ASSET_ID_NAME + "," +
                                     DATABASE_COLUMN_ASSET_URL_NAME + ") VALUES (" + "?, ?, ?, ?);";
            lock_guard<mutex> lock{m_databaseMutex};
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("enqueueFailed").m("Could not create statement"));
                return false;
            }
            int boundParam = 1;
            if (!statement->bindIntParameter(boundParam++, notificationIndicator.persistVisualIndicator) ||
                !statement->bindIntParameter(boundParam++, notificationIndicator.playAudioIndicator) ||
                !statement->bindStringParameter(boundParam++, notificationIndicator.asset.assetId) ||
                !statement->bindStringParameter(boundParam++, notificationIndicator.asset.url)) {
                ACSDK_ERROR(LX("enqueueFailed").m("Could not bind parameter"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("enqueueFailed").m("Could not perform step"));
                return false;
            }
            statement->finalize();
            return true;
        }
        static bool popNotificationIndicatorLocked(SQLiteDatabase* database) {
            if (!database) {
                ACSDK_ERROR(LX("popNotificationIndicatorLockedFailed").m("null database"));
                return false;
            }
            const string minTableId = "(SELECT ROWID FROM " + NOTIFICATION_INDICATOR_TABLE_NAME + " order by ROWID limit 1)";
            const string sqlString = "DELETE FROM " + NOTIFICATION_INDICATOR_TABLE_NAME + " WHERE ROWID=" + minTableId + ";";
            auto statement = database->createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("popNotificationIndicatorLockedFailed").m("Could not create statement."));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("popNotificationIndicatorLockedFailed").m("Could not perform step."));
                return false;
            }
            return true;
        }
        bool SQLiteNotificationsStorage::dequeue() {
            lock_guard<mutex> lock{m_databaseMutex};
            NotificationIndicator dummyIndicator;
            if (!getNextNotificationIndicatorLocked(&dummyIndicator)) {
                ACSDK_ERROR(LX("dequeueFailed").m("No records left in the database!"));
                return false;
            }
            if (!popNotificationIndicatorLocked(&m_database)) {
                ACSDK_ERROR(LX("dequeueFailed").m("Could not pop notificationIndicator from table"));
                return false;
            }
            return true;
        }
        bool SQLiteNotificationsStorage::peek(NotificationIndicator* notificationIndicator) {
            lock_guard<std::mutex> lock{m_databaseMutex};
            if (!getNextNotificationIndicatorLocked(notificationIndicator)) {
                ACSDK_ERROR(LX("peekFailed").m("Could not retrieve the next notificationIndicator from the database"));
                return false;
            }
            return true;
        }
        bool SQLiteNotificationsStorage::setIndicatorState(IndicatorState state) {
            lock_guard<mutex> lock{m_databaseMutex};
            {
                auto transaction = m_database.beginTransaction();
                string sqlString = "DELETE FROM " + INDICATOR_STATE_NAME + " WHERE ROWID IN (SELECT ROWID FROM " +
                                        INDICATOR_STATE_NAME + " limit 1);";
                auto deleteStatement = m_database.createStatement(sqlString);
                if (!deleteStatement) {
                    ACSDK_ERROR(LX("setIndicatorStateFailed").m("Could not create deleteStatement"));
                    return false;
                }
                if (!deleteStatement->step()) {
                    ACSDK_ERROR(LX("setIndicatorStateFailed").m("Could not perform step"));
                    return false;
                }
                deleteStatement->finalize();
                sqlString = "INSERT INTO " + INDICATOR_STATE_NAME + " (" + INDICATOR_STATE_NAME + ") VALUES (?);";
                auto insertStatement = m_database.createStatement(sqlString);
                if (!insertStatement) {
                    ACSDK_ERROR(LX("setIndicatorStateFailed").m("Could not create insertStatement"));
                    return false;
                }
                if (!insertStatement->bindIntParameter(1, indicatorStateToInt(state))) {
                    ACSDK_ERROR(LX("setIndicatorStateFailed").m("Could not bind parameter"));
                    return false;
                }
                if (!insertStatement->step()) {
                    ACSDK_ERROR(LX("setIndicatorStateFailed").m("Could not perform step"));
                    return false;
                }
                insertStatement->finalize();
                if (!transaction->commit()) {
                    ACSDK_ERROR(LX("setIndicatorStateFailed").m("Could not commit transaction"));
                    return false;
                }
            }
            return true;
        }
        bool SQLiteNotificationsStorage::getIndicatorState(IndicatorState* state) {
            if (!state) {
                ACSDK_ERROR(LX("getIndicatorStateFailed").m("State parameter was nullptr"));
                return false;
            }
            string sqlString = "SELECT * FROM " + INDICATOR_STATE_NAME;
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("getIndicatorStateFailed").m("Could not create statement"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("getIndicatorStateFailed").m("Could not perform step"));
                return false;
            }
            int stepResult = statement->getStepResult();
            if (stepResult != SQLITE_ROW) {
                ACSDK_ERROR(LX("getIndicatorStateFailed").d("reason", "No records in the table. Returning default indicator state."));
                *state = DEFAULT_INDICATOR_STATE;
                return true;
            }
            IndicatorState indicatorState = avsCommon::avs::intToIndicatorState(statement->getColumnInt(0));
            statement->finalize();
            if (IndicatorState::UNDEFINED == indicatorState) {
                ACSDK_ERROR(LX("getIndicatorStateFailed").d("reason", "Unknown indicator state retrieved from the table. Returning "
                            "default indicator state."));
                indicatorState = DEFAULT_INDICATOR_STATE;
            }
            *state = indicatorState;
            return true;
        }
        bool SQLiteNotificationsStorage::checkForEmptyQueue(bool* empty) {
            if (!empty) {
                ACSDK_ERROR(LX("checkForEmptyQueueFailed").m("empty parameter was nullptr"));
                return false;
            }
            string sqlString = "SELECT * FROM " + NOTIFICATION_INDICATOR_TABLE_NAME;
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("checkForEmptyQueueFailed").m("Could not create statement"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("checkForEmptyQueueFailed").m("Could not perform step"));
                return false;
            }
            int stepResult = statement->getStepResult();
            if (stepResult != SQLITE_ROW) *empty = true;
            else *empty = false;
            return true;
        }
        bool SQLiteNotificationsStorage::clearNotificationIndicators() {
            const string sqlString = "DELETE FROM " + NOTIFICATION_INDICATOR_TABLE_NAME;
            lock_guard<mutex> lock{m_databaseMutex};
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("clearNotificationIndicatorsFailed").m("Could not create statement."));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("clearNotificationIndicatorsFailed").m("Could not perform step."));
                return false;
            }
            return true;
        }
        SQLiteNotificationsStorage::~SQLiteNotificationsStorage() {
            close();
        }
        bool SQLiteNotificationsStorage::getNextNotificationIndicatorLocked(NotificationIndicator* notificationIndicator) {
            string sqlString = "SELECT * FROM " + NOTIFICATION_INDICATOR_TABLE_NAME + " ORDER BY ROWID ASC LIMIT 1;";
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("getNextNotificationIndicatorLockedFailed").m("Could not create statement"));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("getNextNotificationIndicatorLockedFailed").m("Could not perform step"));
                return false;
            }
            int stepResult = statement->getStepResult();
            if (stepResult != SQLITE_ROW) {
                ACSDK_ERROR(LX("getNextNotificationIndicatorLockedFailed").m("No records left in table"));
                return false;
            }
            int numberColumns = statement->getColumnCount();
            bool persistVisualIndicator = false;
            bool playAudioIndicator = false;
            string assetId = "";
            string assetUrl = "";
            for (int i = 0; i < numberColumns; i++) {
                string columnName = statement->getColumnName(i);
                if (DATABASE_COLUMN_PERSIST_VISUAL_INDICATOR_NAME == columnName) persistVisualIndicator = statement->getColumnInt(i);
                else if (DATABASE_COLUMN_PLAY_AUDIO_INDICATOR_NAME == columnName) playAudioIndicator = statement->getColumnInt(i);
                else if (DATABASE_COLUMN_ASSET_ID_NAME == columnName) assetId = statement->getColumnText(i);
                else if (DATABASE_COLUMN_ASSET_URL_NAME == columnName) assetUrl = statement->getColumnText(i);
            }
            *notificationIndicator = NotificationIndicator(persistVisualIndicator, playAudioIndicator, assetId, assetUrl);
            return true;
        }
        bool SQLiteNotificationsStorage::getQueueSize(int* size) {
            if (!size) {
                ACSDK_ERROR(LX("getQueueSizeFailed").m("size parameter was nullptr"));
                return false;
            }
            if (!getNumberTableRows(&m_database, NOTIFICATION_INDICATOR_TABLE_NAME, size)) {
                ACSDK_ERROR(LX("getQueueSizeFailed").m("Failed to count rows in table"));
                return false;
            }
            return true;
        }
    }
}