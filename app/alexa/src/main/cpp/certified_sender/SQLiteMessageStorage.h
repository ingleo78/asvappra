#ifndef ALEXA_CLIENT_SDK_CERTIFIEDSENDER_INCLUDE_CERTIFIEDSENDER_SQLITEMESSAGESTORAGE_H_
#define ALEXA_CLIENT_SDK_CERTIFIEDSENDER_INCLUDE_CERTIFIEDSENDER_SQLITEMESSAGESTORAGE_H_

#include <configuration/ConfigurationNode.h>
#include <storage/SQLiteDatabase.h>
#include "MessageStorageInterface.h"

namespace alexaClientSDK {
    namespace certifiedSender {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace configuration;
        using namespace alexaClientSDK::storage;
        using namespace sqliteStorage;
        class SQLiteMessageStorage : public MessageStorageInterface {
        public:
            static unique_ptr<SQLiteMessageStorage> create(const ConfigurationNode& configurationRoot);
            SQLiteMessageStorage(const string& databaseFilePath);
            ~SQLiteMessageStorage();
            bool createDatabase() override;
            bool open() override;
            void close() override;
            bool store(const string& message, int* id) override;
            bool store(const string& message, const string& uriPathExtension, int* id) override;
            bool load(queue<StoredMessage>* messageContainer) override;
            bool erase(int messageId) override;
            bool clearDatabase() override;
        private:
            SQLiteDatabase m_database;
        };
    }
}
#endif