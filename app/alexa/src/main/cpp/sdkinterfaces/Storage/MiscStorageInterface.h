#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_STORAGE_MISCSTORAGEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_STORAGE_MISCSTORAGEINTERFACE_H_

#include <string>
#include <unordered_map>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace storage {
                using namespace std;
                class MiscStorageInterface {
                public:
                    enum class KeyType {
                        UNKNOWN_KEY,
                        STRING_KEY
                    };
                    enum class ValueType {
                        UNKNOWN_VALUE,
                        STRING_VALUE
                    };
                    virtual ~MiscStorageInterface() = default;
                    virtual bool createDatabase();
                    virtual bool open();
                    virtual bool isOpened();
                    virtual void close();
                    virtual bool createTable(const string& componentName, const string& tableName, KeyType keyType, ValueType valueType);
                    virtual bool clearTable(const string& componentName, const string& tableName);
                    virtual bool deleteTable(const string& componentName, const string& tableName);
                    virtual bool get(const string& componentName, const string& tableName, const string& key, string* value);
                    virtual bool add(const string& componentName, const string& tableName, const string& key, const string& value);
                    virtual bool update(const string& componentName, const string& tableName, const string& key, const string& value);
                    virtual bool put(const string& componentName, const string& tableName, const string& key, const string& value);
                    virtual bool remove(const string& componentName, const string& tableName, const string& key);
                    virtual bool tableEntryExists(const string& componentName, const string& tableName, const string& key, bool* tableEntryExistsValue);
                    virtual bool tableExists(const string& componentName, const string& tableName, bool* tableExistsValue);
                    virtual bool load(const string& componentName, const string& tableName, unordered_map<string, string>* valueContainer);
                };
            }
        }
    }
}
#endif