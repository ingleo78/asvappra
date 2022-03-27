#include <util/file/FileUtils.h>
#include <logger/Logger.h>
#include <util/string/StringUtils.h>
#include <fstream>
#include "SQLiteUtils.h"
#include "SQLiteStatement.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            using namespace avsCommon;
            using namespace utils;
            using namespace file;
            using namespace logger;
            using namespace string;
            static const std::string TAG("SQLiteUtils");
            #define LX(event) LogEntry(TAG, event)
            static sqlite3* openSQLiteDatabaseHelper(const std::string& filePath, int sqliteFlags) {
                sqlite3* dbHandle = nullptr;
                int rcode = sqlite3_open_v2(filePath.c_str(), &dbHandle, sqliteFlags,nullptr);
                if (rcode != SQLITE_OK) {
                    ACSDK_ERROR(LX("openSQLiteDatabaseHelperFailed").m("Could not open database.").d("rcode", rcode)
                                    .d("file path", filePath).d("error message", sqlite3_errmsg(dbHandle)));
                    sqlite3_close(dbHandle);
                    dbHandle = nullptr;
                }
                return dbHandle;
            }
            sqlite3* createSQLiteDatabase(const std::string& filePath) {
                if (fileExists(filePath)) {
                    ACSDK_ERROR(LX("createSQLiteDatabaseFailed").m("File already exists.").d("file", filePath));
                    return nullptr;
                }
                int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
                sqlite3* dbHandle = openSQLiteDatabaseHelper(filePath, flags);
                if (!dbHandle) {
                    ACSDK_ERROR(LX("createSQLiteDatabaseFailed").m("Could not create database."));
                }
                return dbHandle;
            }
            sqlite3* openSQLiteDatabase(const std::string& filePath) {
                if (!fileExists(filePath)) {
                    ACSDK_ERROR(LX("openSQLiteDatabaseFailed").m("File could not be found.").d("file", filePath));
                    return nullptr;
                }
                int flags = SQLITE_OPEN_READWRITE;
                sqlite3* dbHandle = openSQLiteDatabaseHelper(filePath, flags);
                if (!dbHandle) {
                    ACSDK_ERROR(LX("openSQLiteDatabaseFailed").m("Could not open database."));
                }
                return dbHandle;
            }

            bool closeSQLiteDatabase(sqlite3* dbHandle) {
                if (!dbHandle) {
                    ACSDK_ERROR(LX("closeSQLiteDatabaseFailed").m("dbHandle is nullptr."));
                }
                int rcode = sqlite3_close(dbHandle);
                if (rcode != SQLITE_OK) {
                    ACSDK_ERROR(LX("closeSQLiteDatabaseFailed").d("rcode", rcode).d("error message", sqlite3_errmsg(dbHandle)));
                    return false;
                }
                return true;
            }

            bool performQuery(sqlite3* dbHandle, const std::string& sqlString) {
                if (!dbHandle) {
                    ACSDK_ERROR(LX("performQueryFailed").m("dbHandle was nullptr."));
                    return false;
                }
                int rcode = sqlite3_exec(dbHandle, sqlString.c_str(),nullptr, nullptr,nullptr);
                if (rcode != SQLITE_OK) {
                    ACSDK_ERROR(LX("performQueryFailed").m("Could not execute SQL:" + sqlString).d("rcode", rcode)
                                    .d("error message", sqlite3_errmsg(dbHandle)));
                    return false;
                }
                return true;
            }
            bool getNumberTableRows(SQLiteDatabase* db, const std::string& tableName, int* numberRows) {
                if (!db) {
                    ACSDK_ERROR(LX("getNumberTableRowsFailed").m("db was nullptr."));
                    return false;
                }
                if (!numberRows) {
                    ACSDK_ERROR(LX("getNumberTableRowsFailed").m("numberRows was nullptr."));
                    return false;
                }
                std::string sqlString = "SELECT COUNT(*) FROM " + tableName + ";";
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("getNumberTableRowsFailed").m("Could not create statement."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("getNumberTableRowsFailed").m("Could not step to next row."));
                    return false;
                }
                const int RESULT_COLUMN_POSITION = 0;
                std::string rowValue = statement->getColumnText(RESULT_COLUMN_POSITION);
                if (!stringToInt(rowValue.c_str(), numberRows)) {
                    ACSDK_ERROR(LX("getNumberTableRowsFailed").d("Could not convert string to integer", rowValue));
                    return false;
                }
                return true;
            }
            bool getTableMaxIntValue(SQLiteDatabase* db, const std::string& tableName, const std::string& columnName, int* maxInt) {
                if (!db) {
                    ACSDK_ERROR(LX("getTableMaxIntValue").m("db was nullptr."));
                    return false;
                }
                if (!maxInt) {
                    ACSDK_ERROR(LX("getMaxIntFailed").m("maxInt was nullptr."));
                    return false;
                }
                std::string sqlString = "SELECT " + columnName + " FROM " + tableName + " ORDER BY " + columnName + " DESC LIMIT 1;";
                auto statement = db->createStatement(sqlString);
                if (!statement) {
                    ACSDK_ERROR(LX("getTableMaxIntValueFailed").m("Could not create statement."));
                    return false;
                }
                if (!statement->step()) {
                    ACSDK_ERROR(LX("getTableMaxIntValueFailed").m("Could not step to next row."));
                    return false;
                }
                int stepResult = statement->getStepResult();
                if (stepResult != SQLITE_ROW && stepResult != SQLITE_DONE) {
                    ACSDK_ERROR(LX("getTableMaxIntValueFailed").m("Step did not evaluate to either row or completion."));
                    return false;
                }
                if (SQLITE_DONE == stepResult) {
                    *maxInt = 0;
                }
                if (SQLITE_ROW == stepResult) {
                    const int RESULT_COLUMN_POSITION = 0;
                    std::string rowValue = statement->getColumnText(RESULT_COLUMN_POSITION);
                    if (!stringToInt(rowValue.c_str(), maxInt)) {
                        ACSDK_ERROR(LX("getTableMaxIntValueFailed").d("Could not convert string to integer", rowValue));
                        return false;
                    }
                }
                return true;
            }
            bool tableExists(sqlite3* dbHandle, const std::string& tableName) {
                if (!dbHandle) {
                    ACSDK_ERROR(LX("tableExistsFailed").m("dbHandle was nullptr."));
                    return false;
                }
                std::string sqlString = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
                SQLiteStatement statement(dbHandle, sqlString);
                if (!statement.isValid()) {
                    ACSDK_ERROR(LX("tableExistsFailed").m("Could not create statement."));
                    return false;
                }
                if (!statement.step()) {
                    ACSDK_ERROR(LX("tableExistsFailed").m("Could not step to next row."));
                    return false;
                }
                int stepResult = statement.getStepResult();
                if (stepResult != SQLITE_ROW && stepResult != SQLITE_DONE) {
                    ACSDK_ERROR(LX("tableExistsFailed").m("Step did not evaluate to either row or completion."));
                    return false;
                }
                if (SQLITE_DONE == stepResult) return false;
                const int RESULT_COLUMN_POSITION = 0;
                std::string rowValue = statement.getColumnText(RESULT_COLUMN_POSITION);
                int count = 0;
                if (!stringToInt(rowValue.c_str(), &count)) {
                    ACSDK_ERROR(LX("tableExistsFailed").d("Could not convert string to integer", rowValue));
                    return false;
                }
                return (1 == count);
            }
            bool clearTable(sqlite3* dbHandle, const std::string& tableName) {
                if (!dbHandle) {
                    ACSDK_ERROR(LX("clearTableFailed").m("dbHandle was nullptr."));
                    return false;
                }
                std::string sqlString = "DELETE FROM " + tableName + ";";
                SQLiteStatement statement(dbHandle, sqlString);
                if (!statement.isValid()) {
                    ACSDK_ERROR(LX("clearTableFailed").m("Could not create statement."));
                    return false;
                }
                if (!statement.step()) {
                    ACSDK_ERROR(LX("clearTableFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
            bool dropTable(sqlite3* dbHandle, const std::string& tableName) {
                if (!dbHandle) {
                    ACSDK_ERROR(LX("dropTableFailed").m("dbHandle was nullptr."));
                    return false;
                }
                std::string sqlString = "DROP TABLE IF EXISTS " + tableName + ";";
                SQLiteStatement statement(dbHandle, sqlString);
                if (!statement.isValid()) {
                    ACSDK_ERROR(LX("dropTableFailed").m("Could not create statement."));
                    return false;
                }
                if (!statement.step()) {
                    ACSDK_ERROR(LX("dropTableFailed").m("Could not perform step."));
                    return false;
                }
                return true;
            }
        }
    }
}
