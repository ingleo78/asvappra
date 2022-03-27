#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_STORAGE_STUBMISCSTORAGE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_STORAGE_STUBMISCSTORAGE_H_

#include <gmock/gmock.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "MiscStorageInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace storage {
                namespace test {
                    using namespace std;
                    class StubMiscStorage : public MiscStorageInterface {
                    public:
                        static shared_ptr<StubMiscStorage> create();
                        bool createDatabase() override;
                        bool open() override;
                        bool isOpened() override;
                        void close() override;
                        bool createTable(const string& componentName, const string& tableName, KeyType keyType, ValueType valueType) override;
                        bool clearTable(const string& componentName, const string& tableName) override;
                        bool deleteTable(const string& componentName, const string& tableName) override;
                        bool get(const string& componentName, const string& tableName, const string& key, string* value) override;
                        bool add(const string& componentName, const string& tableName, const string& key, const string& value) override;
                        bool update(const string& componentName, const string& tableName, const string& key, const string& value) override;
                        bool put(const string& componentName, const string& tableName, const string& key, const string& value) override;
                        bool remove(const string& componentName, const string& tableName, const string& key) override;
                        bool tableEntryExists(const string& componentName, const string& tableName, const string& key, bool* tableEntryExistsValue) override;
                        bool tableExists(const string& componentName, const string& tableName, bool* tableExistsValue) override;
                        bool load(const string& componentName, const string& tableName, unordered_map<string, string>* valueContainer) override;
                    private:
                        StubMiscStorage();
                        unordered_map<string, string> m_storage;
                        unordered_set<string> m_tables;
                        bool m_isOpened;
                    };
                }
            }
        }
    }
}
#endif