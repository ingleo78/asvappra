#include <util/file/FileUtils.h>
#include <logger/Logger.h>
#include <utility>
#include "SQLiteUtils.h"
#include "SQLiteDatabase.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace file;
            using namespace logger;
            static const string TAG("SQLiteDatabase");
            #define LX(event) LogEntry(TAG, event)
            SQLiteDatabase::SQLiteDatabase(const string& storageFilePath) : m_storageFilePath{storageFilePath}, m_transactionIsInProgress{false},
                                           m_dbHandle{nullptr} {
                m_sharedThisPlaceholder = shared_ptr<SQLiteDatabase>(this, [](SQLiteDatabase*) {});
            }
            SQLiteDatabase::~SQLiteDatabase() {
                if (m_dbHandle) {
                    ACSDK_WARN(LX(__func__).m("DB wasn't closed before destruction of SQLiteDatabase").d("file path", m_storageFilePath));
                    close();
                }
                if (m_transactionIsInProgress) {
                    ACSDK_ERROR(LX(__func__).d("reason", "There is an incomplete transaction. Rolling it back."));
                    rollbackTransaction();
                }
                m_sharedThisPlaceholder.reset();
            }
            bool SQLiteDatabase::initialize() {
                if (m_dbHandle) {
                    ACSDK_ERROR(LX(__func__).m("Database is already open."));
                    return false;
                }
                if (fileExists(m_storageFilePath)) {
                    ACSDK_ERROR(LX(__func__).m("File specified already exists.").d("file path", m_storageFilePath));
                    return false;
                }
                m_dbHandle = createSQLiteDatabase(m_storageFilePath);
                if (!m_dbHandle) {
                    ACSDK_ERROR(LX(__func__).m("Database could not be created.").d("file path", m_storageFilePath));
                    return false;
                }
                return true;
            }
            bool SQLiteDatabase::open() {
                if (m_dbHandle) {
                    ACSDK_ERROR(LX(__func__).m("Database is already open."));
                    return false;
                }
                if (!fileExists(m_storageFilePath)) {
                    ACSDK_DEBUG0(LX(__func__).m("File specified does not exist.").d("file path", m_storageFilePath));
                    return false;
                }
                m_dbHandle = openSQLiteDatabase(m_storageFilePath);
                if (!m_dbHandle) {
                    ACSDK_ERROR(LX(__func__).m("Database could not be opened.").d("file path", m_storageFilePath));
                    return false;
                }
                return true;
            }
            bool SQLiteDatabase::isDatabaseReady() {
                return (m_dbHandle != nullptr);
            }
            bool SQLiteDatabase::performQuery(const string& sqlString) {
                if (!sqliteStorage::performQuery(m_dbHandle, sqlString)) {
                    ACSDK_ERROR(LX("performQueryFailed").d("SQL string", sqlString));
                    return false;
                }
                return true;
            }
            bool SQLiteDatabase::tableExists(const string& tableName) {
                if (!sqliteStorage::tableExists(m_dbHandle, tableName)) {
                    ACSDK_DEBUG0(LX(__func__).d("reason", "table doesn't exist or there was an error checking").d("table", tableName));
                    return false;
                }
                return true;
            }
            bool SQLiteDatabase::clearTable(const string& tableName) {
                if (!sqliteStorage::clearTable(m_dbHandle, tableName)) {
                    ACSDK_ERROR(LX(__func__).d("could not clear table", tableName));
                    return false;
                }
                return true;
            }
            void SQLiteDatabase::close() {
                if (m_dbHandle) {
                    closeSQLiteDatabase(m_dbHandle);
                    m_dbHandle = nullptr;
                }
            }
            unique_ptr<sqliteStorage::SQLiteStatement> SQLiteDatabase::createStatement(const string& sqlString) {
                unique_ptr<sqliteStorage::SQLiteStatement> statement(new SQLiteStatement(m_dbHandle, sqlString));
                if (!statement->isValid()) {
                    ACSDK_ERROR(LX("createStatementFailed").d("sqlString", sqlString));
                    statement = nullptr;
                }
                return statement;
            }
            unique_ptr<SQLiteDatabase::Transaction> SQLiteDatabase::beginTransaction() {
                if (m_transactionIsInProgress) {
                    ACSDK_ERROR(LX("beginTransactionFailed").d("reason", "Only one transaction at a time is allowed"));
                    return nullptr;
                }
                const string sqlString = "BEGIN TRANSACTION;";
                if (!performQuery(sqlString)) {
                    ACSDK_ERROR(LX("beginTransactionFailed").d("reason", "Query failed"));
                    return nullptr;
                }
                m_transactionIsInProgress = true;
                return unique_ptr<Transaction>(new Transaction(m_sharedThisPlaceholder));
            }
            bool SQLiteDatabase::commitTransaction() {
                if (!m_transactionIsInProgress) {
                    ACSDK_ERROR(LX("commitTransactionFailed").d("reason", "No transaction in progress"));
                    return false;
                }
                const string sqlString = "COMMIT TRANSACTION;";
                if (!performQuery(sqlString)) {
                    ACSDK_ERROR(LX("commitTransactionFailed").d("reason", "Query failed"));
                    return false;
                }
                m_transactionIsInProgress = false;
                return true;
            }
            bool SQLiteDatabase::rollbackTransaction() {
                if (!m_transactionIsInProgress) {
                    ACSDK_ERROR(LX("rollbackTransactionFailed").d("reason", "No transaction in progress"));
                    return false;
                }
                const string sqlString = "ROLLBACK TRANSACTION;";
                if (!performQuery(sqlString)) {
                    ACSDK_ERROR(LX("rollbackTransactionFailed").d("reason", "Query failed"));
                    return false;
                }
                m_transactionIsInProgress = false;
                return true;
            }
            bool SQLiteDatabase::Transaction::commit() {
                if (m_transactionCompleted) {
                    ACSDK_ERROR(LX("commitFailed").d("reason", "Transaction has already been completed"));
                    return false;
                }
                auto database = m_database.lock();
                if (!database) {
                    ACSDK_ERROR(LX("commitFailed").d("reason", "Database has already been finalized"));
                    return false;
                }
                m_transactionCompleted = true;
                return database->commitTransaction();
            }
            bool SQLiteDatabase::Transaction::rollback() {
                if (m_transactionCompleted) {
                    ACSDK_ERROR(LX("rollbackFailed").d("reason", "Transaction has already been completed"));
                    return false;
                }
                bool isExpired = m_database.expired();
                auto database = m_database.lock();
                if (!database && !isExpired) {
                    ACSDK_ERROR(LX("rollbackFailed").d("reason", "Database has already been finalized"));
                    return false;
                }
                m_transactionCompleted = true;
                return database->rollbackTransaction();
            }
            SQLiteDatabase::Transaction::Transaction(std::weak_ptr<SQLiteDatabase> database) :
                    m_database{std::move(database)},
                    m_transactionCompleted{false} {
            }
            SQLiteDatabase::Transaction::~Transaction() {
                if (m_transactionCompleted) return;
                ACSDK_ERROR(LX(__func__).m("Transaction was not completed manually, rolling it back automatically"));
                rollback();
            }
        }
    }
}
