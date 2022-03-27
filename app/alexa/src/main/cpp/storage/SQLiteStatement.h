#ifndef ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITESTATEMENT_H_
#define ALEXA_CLIENT_SDK_STORAGE_SQLITESTORAGE_INCLUDE_SQLITESTORAGE_SQLITESTATEMENT_H_

#include <list>
#include <string>
#include "sqlite3.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            class SQLiteStatement {
            public:
                SQLiteStatement(sqlite3* dbHandle, const std::string& sqlString);
                ~SQLiteStatement();
                bool isValid();
                bool step();
                bool reset();
                bool bindIntParameter(int index, int value);
                bool bindInt64Parameter(int index, int64_t value);
                bool bindStringParameter(int index, const std::string& value);
                int getStepResult() const;
                int getColumnCount() const;
                std::string getColumnName(int index) const;
                std::string getColumnText(int index) const;
                int getColumnInt(int index) const;
                int64_t getColumnInt64(int index) const;
                void finalize();
            private:
                sqlite3_stmt* m_handle;
                int m_stepResult;
                std::list<std::string> m_boundValues;
            };
        }
    }
}
#endif