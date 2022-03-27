#include <fstream>
#include <storage/SQLiteUtils.h>
#include <storage/SQLiteStatement.h>
#include <util/file/FileUtils.h>
#include <logger/Logger.h>
#include "SQLiteMessageStorage.h"

namespace alexaClientSDK {
    namespace certifiedSender {
        using namespace file;
        using namespace logger;
        static const string TAG("SQLiteMessageStorage");
        #define LX(event) LogEntry(TAG, event)
        static const string CERTIFIED_SENDER_CONFIGURATION_ROOT_KEY = "certifiedSender";
        static const string CERTIFIED_SENDER_DB_FILE_PATH_KEY = "databaseFilePath";
        static const string MESSAGES_TABLE_NAME = "messages_with_uri";
        static const string DATABASE_COLUMN_ID_NAME = "id";
        static const string DATABASE_COLUMN_MESSAGE_TEXT_NAME = "message_text";
        static const string DATABASE_COLUMN_URI = "uri";
        static const string CREATE_MESSAGES_TABLE_SQL_STRING = string("CREATE TABLE ") + MESSAGES_TABLE_NAME + " (" +
                                                               DATABASE_COLUMN_ID_NAME + " INT PRIMARY KEY NOT NULL," +
                                                               DATABASE_COLUMN_URI + " TEXT NOT NULL," +
                                                               DATABASE_COLUMN_MESSAGE_TEXT_NAME + " TEXT NOT NULL);";
        unique_ptr<SQLiteMessageStorage> SQLiteMessageStorage::create(const ConfigurationNode& configurationRoot) {
            auto certifiedSenderConfigurationRoot = configurationRoot[CERTIFIED_SENDER_CONFIGURATION_ROOT_KEY];
            if (!certifiedSenderConfigurationRoot) {
                ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config for the Message Storage database")
                    .d("key", CERTIFIED_SENDER_CONFIGURATION_ROOT_KEY));
                return nullptr;
            }
            string certifiedSenderDatabaseFilePath;
            if (!certifiedSenderConfigurationRoot.getString(CERTIFIED_SENDER_DB_FILE_PATH_KEY, &certifiedSenderDatabaseFilePath) ||
                certifiedSenderDatabaseFilePath.empty()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "Could not load config value").d("key", CERTIFIED_SENDER_DB_FILE_PATH_KEY));
                return nullptr;
            }
            return unique_ptr<SQLiteMessageStorage>(new SQLiteMessageStorage(certifiedSenderDatabaseFilePath));
        }
        SQLiteMessageStorage::SQLiteMessageStorage(const string& certifiedSenderDatabaseFilePath) : m_database{certifiedSenderDatabaseFilePath} {}
        SQLiteMessageStorage::~SQLiteMessageStorage() {
            close();
        }
        bool SQLiteMessageStorage::createDatabase() {
            if (!m_database.initialize()) {
                ACSDK_ERROR(LX("createDatabaseFailed"));
                return false;
            }
            if (!m_database.performQuery(CREATE_MESSAGES_TABLE_SQL_STRING)) {
                ACSDK_ERROR(LX("createDatabaseFailed").m("Table could not be created."));
                close();
                return false;
            }
            return true;
        }
        bool SQLiteMessageStorage::open() {
            if (!m_database.open()) return false;
            if (!m_database.tableExists(MESSAGES_TABLE_NAME) && !m_database.performQuery(CREATE_MESSAGES_TABLE_SQL_STRING)) {
                ACSDK_ERROR(LX("openFailed").d("sqlString", CREATE_MESSAGES_TABLE_SQL_STRING));
                close();
                return false;
            }
            return true;
        }
        void SQLiteMessageStorage::close() {
            m_database.close();
        }
        bool SQLiteMessageStorage::store(const string& message, int* id) {
            return store(message, "", id);
        }
        bool SQLiteMessageStorage::store(const string& message, const string& uriPathExtension, int* id) {
            if (!id) {
                ACSDK_ERROR(LX("storeFailed").m("id parameter was nullptr."));
                return false;
            }
            string sqlString = string("INSERT INTO " + MESSAGES_TABLE_NAME + " (") + DATABASE_COLUMN_ID_NAME + ", " +
                                    DATABASE_COLUMN_URI + ", " + DATABASE_COLUMN_MESSAGE_TEXT_NAME + ") VALUES (?, ?, ?);";
            int nextId = 0;
            if (!getTableMaxIntValue(&m_database, MESSAGES_TABLE_NAME, DATABASE_COLUMN_ID_NAME, &nextId)) {
                ACSDK_ERROR(LX("storeFailed").m("Cannot generate message id."));
                return false;
            }
            nextId++;
            if (nextId <= 0) {
                ACSDK_ERROR(LX("storeFailed").m("Invalid computed row id.  Possible numerical overflow.").d("id", nextId));
                return false;
            }
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("storeFailed").m("Could not create statement."));
                return false;
            }
            int boundParam = 1;
            if (!statement->bindIntParameter(boundParam++, nextId) ||
                !statement->bindStringParameter(boundParam++, uriPathExtension) ||
                !statement->bindStringParameter(boundParam, message)) {
                ACSDK_ERROR(LX("storeFailed").m("Could not bind parameter."));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("storeFailed").m("Could not perform step."));
                return false;
            }
            *id = nextId;
            return true;
        }
        bool SQLiteMessageStorage::load(std::queue<StoredMessage>* messageContainer) {
            if (!messageContainer) {
                ACSDK_ERROR(LX("loadFailed").m("Alert container parameter is nullptr."));
                return false;
            }
            string sqlString = "SELECT * FROM " + MESSAGES_TABLE_NAME + " ORDER BY id;";
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("loadFailed").m("Could not create statement."));
                return false;
            }
            int id = 0;
            string uriPathExtension;
            string message;
            if (!statement->step()) {
                ACSDK_ERROR(LX("loadFailed").m("Could not perform step."));
                return false;
            }
            while (SQLITE_ROW == statement->getStepResult()) {
                int numberColumns = statement->getColumnCount();
                for (int i = 0; i < numberColumns; i++) {
                    string columnName = statement->getColumnName(i);
                    if (DATABASE_COLUMN_ID_NAME == columnName) id = statement->getColumnInt(i);
                    else if (DATABASE_COLUMN_MESSAGE_TEXT_NAME == columnName) message = statement->getColumnText(i);
                    else if (DATABASE_COLUMN_URI == columnName) uriPathExtension = statement->getColumnText(i);
                }
                StoredMessage storedMessage(id, message, uriPathExtension);
                messageContainer->push(storedMessage);
                statement->step();
            }
            return true;
        }
        bool SQLiteMessageStorage::erase(int messageId) {
            string sqlString = "DELETE FROM " + MESSAGES_TABLE_NAME + " WHERE id=?;";
            auto statement = m_database.createStatement(sqlString);
            if (!statement) {
                ACSDK_ERROR(LX("eraseFailed").m("Could not create statement."));
                return false;
            }
            int boundParam = 1;
            if (!statement->bindIntParameter(boundParam, messageId)) {
                ACSDK_ERROR(LX("eraseFailed").m("Could not bind messageId."));
                return false;
            }
            if (!statement->step()) {
                ACSDK_ERROR(LX("eraseFailed").m("Could not perform step."));
                return false;
            }
            return true;
        }
        bool SQLiteMessageStorage::clearDatabase() {
            if (!m_database.clearTable(MESSAGES_TABLE_NAME)) {
                ACSDK_ERROR(LX("clearDatabaseFailed").m("could not clear messages table."));
                return false;
            }
            return true;
        }
    }
}