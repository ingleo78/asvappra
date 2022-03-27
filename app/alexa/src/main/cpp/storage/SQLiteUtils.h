#ifndef ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITEUTILS_H_
#define ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITEUTILS_H_

#include <string>
#include "sqlite3.h"
#include "SQLiteDatabase.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            sqlite3* createSQLiteDatabase(const std::string& filePath);
            sqlite3* openSQLiteDatabase(const std::string& filePath);
            bool closeSQLiteDatabase(sqlite3* dbHandle);
            bool performQuery(sqlite3* dbHandle, const std::string& sqlString);
            bool getNumberTableRows(SQLiteDatabase* db, const std::string& tableName, int* numberRows);
            bool getTableMaxIntValue(SQLiteDatabase* db, const std::string& tableName, const std::string& columnName, int* maxId);
            bool tableExists(sqlite3* dbHandle, const std::string& tableName);
            bool clearTable(sqlite3* dbHandle, const std::string& tableName);
            bool dropTable(sqlite3* dbHandle, const std::string& tableName);
        }
    }
}
#endif