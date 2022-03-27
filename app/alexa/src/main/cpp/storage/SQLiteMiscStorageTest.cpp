#include <gtest/gtest.h>
#include <avs/Initialization/AlexaClientSDKInit.h>
#include <configuration/ConfigurationNode.h>
#include "SQLiteDatabase.h"
#include "SQLiteMiscStorage.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            namespace test {
                using namespace std;
                using namespace avsCommon;
                using namespace avs;
                using namespace sdkInterfaces;
                using namespace utils;
                using namespace configuration;
                using namespace initialization;
                using namespace sdkInterfaces::storage;
                using namespace testing;
                static const string COMPONENT_NAME = "SQLiteMiscStorageTest";
                static const string MISC_DB_CONFIG_JSON = "{\"miscDatabase\":{\"databaseFilePath\":\"miscDBSQLiteMiscStorageTest.db\"}}";
                class SQLiteMiscStorageTest : public Test {
                public:
                    SQLiteMiscStorageTest();
                    ~SQLiteMiscStorageTest();
                    void SetUp() override;
                protected:
                    void createTestTable(const string& tableName, const SQLiteMiscStorage::KeyType keyType, const SQLiteMiscStorage::ValueType valueType);
                    void deleteTestTable(const string& tableName);
                    shared_ptr<SQLiteMiscStorage> m_miscStorage;
                };
                void SQLiteMiscStorageTest::createTestTable(const string& tableName, const SQLiteMiscStorage::KeyType keyType,
                                                            const SQLiteMiscStorage::ValueType valueType) {
                    if (m_miscStorage) {
                        bool tableExists;
                        m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists);
                        if (tableExists) {
                            m_miscStorage->clearTable(COMPONENT_NAME, tableName);
                        } else {
                            m_miscStorage->createTable(COMPONENT_NAME, tableName, keyType, valueType);
                        }
                    }
                }
                void SQLiteMiscStorageTest::deleteTestTable(const string& tableName) {
                    if (m_miscStorage) {
                        bool tableExists;
                        m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists);
                        if (tableExists) {
                            m_miscStorage->clearTable(COMPONENT_NAME, tableName);
                            m_miscStorage->deleteTable(COMPONENT_NAME, tableName);
                        }
                    }
                }
                SQLiteMiscStorageTest::SQLiteMiscStorageTest() {
                    auto inString = shared_ptr<istringstream>(new istringstream(MISC_DB_CONFIG_JSON));
                    AlexaClientSDKInit::initialize({inString});
                    auto config = ConfigurationNode::getRoot();
                    if (config) {
                        m_miscStorage = SQLiteMiscStorage::create(config);
                        if (m_miscStorage) {
                            if (!m_miscStorage->open()) {
                                m_miscStorage->createDatabase();
                            }
                        }
                    }
                }
                SQLiteMiscStorageTest::~SQLiteMiscStorageTest() {
                    if (m_miscStorage) m_miscStorage->close();
                    AlexaClientSDKInit::uninitialize();
                }
                void SQLiteMiscStorageTest::SetUp() {
                    ASSERT_NE(m_miscStorage, nullptr);
                }
                TEST_F(SQLiteMiscStorageTest, test_createStringKeyValueTable) {
                    const string tableName = "SQLiteMiscStorageCreateTableTest";
                    deleteTestTable(tableName);
                    bool tableExists;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_miscStorage->createTable(COMPONENT_NAME, tableName, SQLiteMiscStorage::KeyType::STRING_KEY,
                                SQLiteMiscStorage::ValueType::STRING_VALUE));
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists));
                    ASSERT_TRUE(tableExists);
                    deleteTestTable(tableName);
                }
                TEST_F(SQLiteMiscStorageTest, test_removeWithNonEscapedStringKey) {
                    const string tableName = "SQLiteMiscStorageTableEntryTest";
                    const string tableEntryKey = R"(non-escaped'\\%$#*?!`\"key)";
                    const string tableEntryAddedValue = R"(non-escaped'\\%$#*?!`\"tableEntryAddedValue)";
                    const string tableEntryPutValue = R"(non-escaped'\\%$#*?!`\"tableEntryPutValue)";
                    const string tableEntryAnotherPutValue = R"(non-escaped'\\%$#*?!`\"tableEntryAnotherPutValue)";
                    const string tableEntryUpdatedValue = R"(non-escaped'\\%$#*?!`\"tableEntryUpdatedValue)";
                    string tableEntryValue;
                    deleteTestTable(tableName);
                    createTestTable(tableName, SQLiteMiscStorage::KeyType::STRING_KEY, SQLiteMiscStorage::ValueType::STRING_VALUE);
                    bool tableEntryExists;
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, R"(non-escaped'\\%$#*?!`\"key)", &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, tableEntryKey, tableEntryAddedValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryAddedValue);
                    ASSERT_TRUE(m_miscStorage->remove(COMPONENT_NAME, tableName, tableEntryKey));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    deleteTestTable(tableName);
                }
                TEST_F(SQLiteMiscStorageTest, test_updateWithNonEscapedStringKey) {
                    const string tableName = "SQLiteMiscStorageTableEntryTest";
                    const string tableEntryKey = R"(non-escaped'\\%$#*?!`\"key)";
                    const string tableEntryAddedValue = R"(non-escaped'\\%$#*?!`\"tableEntryAddedValue)";
                    const string tableEntryUpdatedValue = R"(non-escaped'\\%$#*?!`\"tableEntryUpdatedValue)";
                    string tableEntryValue;
                    deleteTestTable(tableName);
                    createTestTable(tableName, SQLiteMiscStorage::KeyType::STRING_KEY, SQLiteMiscStorage::ValueType::STRING_VALUE);
                    bool tableEntryExists;
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, R"(non-escaped'\\%$#*?!`\"key)", &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, tableEntryKey, tableEntryAddedValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryAddedValue);
                    ASSERT_TRUE(m_miscStorage->update(COMPONENT_NAME, tableName, tableEntryKey, tableEntryUpdatedValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryUpdatedValue);
                    deleteTestTable(tableName);
                }
                TEST_F(SQLiteMiscStorageTest, test_putWithNonEscapedStringKey) {
                    const string tableName = "SQLiteMiscStorageTableEntryTest";
                    const string tableEntryKey = R"(non-escaped'\\%$#*?!`\"key)";
                    const string tableEntryAddedValue = R"(non-escaped'\\%$#*?!`\"tableEntryAddedValue)";
                    const string tableEntryPutValue = R"(non-escaped'\\%$#*?!`\"tableEntryPutValue)";
                    const string tableEntryAnotherPutValue = R"(non-escaped'\\%$#*?!`\"tableEntryAnotherPutValue)";
                    string tableEntryValue;
                    deleteTestTable(tableName);
                    createTestTable(tableName, SQLiteMiscStorage::KeyType::STRING_KEY, SQLiteMiscStorage::ValueType::STRING_VALUE);
                    bool tableEntryExists;
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, R"(non-escaped'\\%$#*?!`\"key)", &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, tableEntryKey, tableEntryAddedValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryAddedValue);
                    ASSERT_TRUE(m_miscStorage->put(COMPONENT_NAME, tableName, tableEntryKey, tableEntryPutValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryPutValue);
                    ASSERT_TRUE(m_miscStorage->put(COMPONENT_NAME, tableName, tableEntryKey, tableEntryAnotherPutValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryAnotherPutValue);
                    deleteTestTable(tableName);
                }
                TEST_F(SQLiteMiscStorageTest, test_addWithNonEscapedStringKey) {
                    const string tableName = "SQLiteMiscStorageTableEntryTest";
                    const string tableEntryKey = R"(non-escaped'\\%$#*?!`\"key)";
                    const string tableEntryAddedValue = R"(non-escaped'\\%$#*?!`\"tableEntryAddedValue)";
                    string tableEntryValue;
                    deleteTestTable(tableName);
                    createTestTable(tableName, SQLiteMiscStorage::KeyType::STRING_KEY, SQLiteMiscStorage::ValueType::STRING_VALUE);
                    bool tableEntryExists;
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, R"(non-escaped'\\%$#*?!`\"key)", &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, tableEntryKey, tableEntryAddedValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryAddedValue);
                    deleteTestTable(tableName);
                }
                TEST_F(SQLiteMiscStorageTest, test_tableEntryTests) {
                    const string tableName = "SQLiteMiscStorageTableEntryTest";
                    const string tableEntryKey = "tableEntryTestsKey";
                    const string tableEntryAddedValue = "tableEntryAddedValue";
                    const string tableEntryPutValue = "tableEntryPutValue";
                    const string tableEntryAnotherPutValue = "tableEntryAnotherPutValue";
                    const string tableEntryUpdatedValue = "tableEntryUpdatedValue";
                    string tableEntryValue;
                    deleteTestTable(tableName);
                    createTestTable(tableName, SQLiteMiscStorage::KeyType::STRING_KEY, SQLiteMiscStorage::ValueType::STRING_VALUE);
                    bool tableEntryExists;
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, tableEntryKey, tableEntryAddedValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryAddedValue);
                    ASSERT_TRUE(m_miscStorage->update(COMPONENT_NAME, tableName, tableEntryKey, tableEntryUpdatedValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryUpdatedValue);
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->remove(COMPONENT_NAME, tableName, tableEntryKey));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->put(COMPONENT_NAME, tableName, tableEntryKey, tableEntryPutValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryPutValue);
                    ASSERT_TRUE(m_miscStorage->put(COMPONENT_NAME, tableName, tableEntryKey, tableEntryAnotherPutValue));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, tableEntryAnotherPutValue);
                    deleteTestTable(tableName);
                }
                TEST_F(SQLiteMiscStorageTest, test_loadAndClear) {
                    const string tableName = "SQLiteMiscStorageLoadClearTest";
                    size_t numOfEntries = 3;
                    string keyPrefix = "key";
                    string valuePrefix = "value";
                    unordered_map<string, string> valuesContainer;
                    deleteTestTable(tableName);
                    createTestTable(tableName, SQLiteMiscStorage::KeyType::STRING_KEY, SQLiteMiscStorage::ValueType::STRING_VALUE);
                    for (size_t entryIndx = 1; entryIndx <= numOfEntries; entryIndx++) {
                        string key = keyPrefix + to_string(entryIndx);
                        string value = valuePrefix + to_string(entryIndx);
                        ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, key, value));
                    }
                    ASSERT_TRUE(m_miscStorage->load(COMPONENT_NAME, tableName, &valuesContainer));
                    ASSERT_EQ(valuesContainer.size(), numOfEntries);
                    for (size_t entryIndx = 1; entryIndx <= numOfEntries; entryIndx++) {
                        string keyExpected = keyPrefix + to_string(entryIndx);
                        string valueExpected = valuePrefix + to_string(entryIndx);
                        auto mapIterator = valuesContainer.find(keyExpected);
                        ASSERT_NE(mapIterator, valuesContainer.end());
                        ASSERT_EQ(mapIterator->first, keyExpected);
                        ASSERT_EQ(mapIterator->second, valueExpected);
                    }
                    valuesContainer.clear();
                    ASSERT_TRUE(m_miscStorage->clearTable(COMPONENT_NAME, tableName));
                    m_miscStorage->load(COMPONENT_NAME, tableName, &valuesContainer);
                    ASSERT_TRUE(valuesContainer.empty());
                    deleteTestTable(tableName);
                }
                TEST_F(SQLiteMiscStorageTest, test_createDeleteTable) {
                    const std::string tableName = "SQLiteMiscStorageCreateDeleteTest";
                    deleteTestTable(tableName);
                    bool tableExists;
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists));
                    ASSERT_FALSE(tableExists);
                    ASSERT_TRUE(m_miscStorage->createTable(COMPONENT_NAME, tableName, SQLiteMiscStorage::KeyType::STRING_KEY,
                                SQLiteMiscStorage::ValueType::STRING_VALUE));
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists));
                    ASSERT_TRUE(tableExists);
                    ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, "randomKey", "randomValue"));
                    ASSERT_FALSE(m_miscStorage->deleteTable(COMPONENT_NAME, tableName));
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists));
                    ASSERT_TRUE(tableExists);
                    ASSERT_TRUE(m_miscStorage->clearTable(COMPONENT_NAME, tableName));
                    ASSERT_TRUE(m_miscStorage->deleteTable(COMPONENT_NAME, tableName));
                    ASSERT_TRUE(m_miscStorage->tableExists(COMPONENT_NAME, tableName, &tableExists));
                    ASSERT_FALSE(tableExists);
                }
                TEST_F(SQLiteMiscStorageTest, test_escapeSingleQuoteCharacters) {
                    const string tableName = "SQLiteMiscStorageTableEntryTest";
                    const string tableEntryKey = "tableEntryTestsKey";
                    const string valueWithSingleQuote = "This table's entry";
                    string tableEntryValue;
                    deleteTestTable(tableName);
                    createTestTable(tableName, SQLiteMiscStorage::KeyType::STRING_KEY, SQLiteMiscStorage::ValueType::STRING_VALUE);
                    bool tableEntryExists;
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_FALSE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->add(COMPONENT_NAME, tableName, tableEntryKey, valueWithSingleQuote));
                    ASSERT_TRUE(m_miscStorage->tableEntryExists(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryExists));
                    ASSERT_TRUE(tableEntryExists);
                    ASSERT_TRUE(m_miscStorage->get(COMPONENT_NAME, tableName, tableEntryKey, &tableEntryValue));
                    ASSERT_EQ(tableEntryValue, valueWithSingleQuote);
                    deleteTestTable(tableName);
                }
            }
        }
    }
}
