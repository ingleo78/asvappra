#ifndef ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITEDATABASE_H_
#define ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITEDATABASE_H_

#include <memory>
#include <string>
#include <vector>
#include "sqlite3.h"
#include "SQLiteStatement.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            class SQLiteDatabase {
            public:
                class Transaction {
                    friend SQLiteDatabase;
                public:
                    ~Transaction();
                    bool commit();
                    bool rollback();
                private:
                    Transaction(std::weak_ptr<SQLiteDatabase> dbHandle);
                    std::weak_ptr<SQLiteDatabase> m_database;
                    bool m_transactionCompleted;
                };
                SQLiteDatabase(const std::string& filePath);
                ~SQLiteDatabase();
                bool initialize();
                bool open();
                bool performQuery(const std::string& sqlString);
                bool tableExists(const std::string& tableName);
                bool clearTable(const std::string& tableName);
                void close();
                std::unique_ptr<SQLiteStatement> createStatement(const std::string& sqlString);
                bool isDatabaseReady();
                std::unique_ptr<Transaction> beginTransaction();
            private:
                bool commitTransaction();
                bool rollbackTransaction();
                const std::string m_storageFilePath;
                bool m_transactionIsInProgress;
                sqlite3* m_dbHandle;
                std::shared_ptr<SQLiteDatabase> m_sharedThisPlaceholder;
            };
        }
    }
}
#endif