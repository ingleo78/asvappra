#include <fstream>
#include <string>
#include <unordered_map>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <configuration/ConfigurationNode.h>
#include "SQLiteCapabilitiesDelegateStorage.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace storage {
            namespace test {
                using namespace utils::configuration;
                using namespace testing;
                static const string TEST_DATABASE_FILE_NAME = "SQLiteCapabilitiesDelegateStorageTest.db";
                static const string CAPABILITIES_DELEGATE_JSON = R"({"capabilitiesDelegate" : {"databaseFilePath":")" + TEST_DATABASE_FILE_NAME + R"("}})";
                static const string TEST_ENDPOINT_ID_1 = "EndpointID1";
                static const string TEST_ENDPOINT_ID_2 = "EndpointID2";
                static const string TEST_ENDPOINT_CONFIG_1 = "EndpointConfig1";
                static const string TEST_ENDPOINT_CONFIG_2 = "EndpointConfig2";
                static bool fileExists(const string& file) {
                    ifstream dbFile(file);
                    return dbFile.good();
                }
                class SQLiteCapabilitiesDelegateStorageTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    void closeAndDeleteDB();
                    unique_ptr<SQLiteCapabilitiesDelegateStorage> m_db;
                };
                void SQLiteCapabilitiesDelegateStorageTest::closeAndDeleteDB() {
                    if (m_db) m_db->close();
                    m_db.reset();
                    if (fileExists(TEST_DATABASE_FILE_NAME)) remove(TEST_DATABASE_FILE_NAME.c_str());
                }
                void SQLiteCapabilitiesDelegateStorageTest::SetUp() {
                    auto json = shared_ptr<stringstream>(new stringstream());
                    *json << CAPABILITIES_DELEGATE_JSON;
                    vector<shared_ptr<istream>> jsonStream;
                    jsonStream.push_back(json);
                    ASSERT_TRUE(ConfigurationNode::initialize(jsonStream));
                    m_db = SQLiteCapabilitiesDelegateStorage::create(ConfigurationNode::getRoot());
                    ASSERT_THAT(m_db, NotNull());
                    ASSERT_TRUE(m_db->createDatabase());
                }
                void SQLiteCapabilitiesDelegateStorageTest::TearDown() {
                    ConfigurationNode::uninitialize();
                    closeAndDeleteDB();
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_createInvalidConfigurationRoot) {
                    ConfigurationNode::uninitialize();
                    vector<shared_ptr<istream>> empty;
                    ConfigurationNode::initialize(empty);
                    ASSERT_THAT(SQLiteCapabilitiesDelegateStorage::create(ConfigurationNode::getRoot()), IsNull());
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_createValidConfigurationRoot) {
                    ASSERT_THAT(SQLiteCapabilitiesDelegateStorage::create(ConfigurationNode::getRoot()), NotNull());
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_createExistingDatabaseFails) {
                    ASSERT_FALSE(m_db->createDatabase());
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_openExistingDatabaseSucceeds) {
                    m_db->close();
                    ASSERT_TRUE(m_db->open());
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_storeForEndpointWorks) {
                    ASSERT_TRUE(m_db->store(TEST_ENDPOINT_ID_1, TEST_ENDPOINT_CONFIG_1));
                    string testString;
                    ASSERT_TRUE(m_db->load(TEST_ENDPOINT_ID_1, &testString));
                    ASSERT_EQ(testString, TEST_ENDPOINT_CONFIG_1);
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_storeForEndpointMapWorks) {
                    unordered_map<string, string> storeMap;
                    storeMap.insert({TEST_ENDPOINT_ID_1, TEST_ENDPOINT_CONFIG_1});
                    storeMap.insert({TEST_ENDPOINT_ID_2, TEST_ENDPOINT_CONFIG_2});
                    ASSERT_TRUE(m_db->store(storeMap));
                    unordered_map<string, string> loadMap;
                    ASSERT_TRUE(m_db->load(&loadMap));
                    ASSERT_THAT(loadMap.size(), Eq(2U));
                    auto it1 = loadMap.find(TEST_ENDPOINT_ID_1);
                    ASSERT_NE(it1, loadMap.end());
                    ASSERT_EQ(it1->second, TEST_ENDPOINT_CONFIG_1);
                    auto it2 = loadMap.find(TEST_ENDPOINT_ID_2);
                    ASSERT_NE(it2, loadMap.end());
                    ASSERT_EQ(it2->second, TEST_ENDPOINT_CONFIG_2);
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_storeForExistingEntry) {
                    string storedValue;
                    unordered_map<string, string> storeMap;
                    storeMap.insert({TEST_ENDPOINT_ID_1, TEST_ENDPOINT_CONFIG_1});
                    storeMap.insert({TEST_ENDPOINT_ID_2, TEST_ENDPOINT_CONFIG_2});
                    ASSERT_TRUE(m_db->store(storeMap));
                    ASSERT_TRUE(m_db->load(TEST_ENDPOINT_ID_1, &storedValue));
                    std::string TEST_CONFIG = "TEST_CONFIG";
                    ASSERT_TRUE(m_db->store(TEST_ENDPOINT_ID_1, TEST_CONFIG));
                    ASSERT_TRUE(m_db->load(TEST_ENDPOINT_ID_1, &storedValue));
                    ASSERT_EQ(storedValue, TEST_CONFIG);
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_loadForEndpointWorks) {
                    unordered_map<string, string> storeMap;
                    storeMap.insert({TEST_ENDPOINT_ID_1, TEST_ENDPOINT_CONFIG_1});
                    storeMap.insert({TEST_ENDPOINT_ID_2, TEST_ENDPOINT_CONFIG_2});
                    ASSERT_TRUE(m_db->store(storeMap));
                    std::string endpointConfig1, endpointConfig2;
                    ASSERT_TRUE(m_db->load(TEST_ENDPOINT_ID_1, &endpointConfig1));
                    ASSERT_TRUE(m_db->load(TEST_ENDPOINT_ID_2, &endpointConfig2));
                    ASSERT_EQ(endpointConfig1, TEST_ENDPOINT_CONFIG_1);
                    ASSERT_EQ(endpointConfig2, TEST_ENDPOINT_CONFIG_2);
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_loadForNonExistingEndpoint) {
                    unordered_map<string, string> storeMap;
                    storeMap.insert({TEST_ENDPOINT_ID_2, TEST_ENDPOINT_CONFIG_2});
                    ASSERT_TRUE(m_db->store(storeMap));
                    string endpointConfig1, endpointConfig2;
                    ASSERT_TRUE(m_db->load(TEST_ENDPOINT_ID_1, &endpointConfig1));
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_eraseWorks) {
                    unordered_map<string, string> storeMap;
                    storeMap.insert({TEST_ENDPOINT_ID_1, TEST_ENDPOINT_CONFIG_1});
                    storeMap.insert({TEST_ENDPOINT_ID_2, TEST_ENDPOINT_CONFIG_2});
                    ASSERT_TRUE(m_db->store(storeMap));
                    ASSERT_TRUE(m_db->erase(TEST_ENDPOINT_ID_1));
                    unordered_map<string, string> loadMap;
                    ASSERT_TRUE(m_db->load(&loadMap));
                    ASSERT_THAT(loadMap.size(), Eq(1U));
                    auto it = loadMap.find(TEST_ENDPOINT_ID_1);
                    ASSERT_EQ(it, loadMap.end());
                    it = loadMap.find(TEST_ENDPOINT_ID_2);
                    ASSERT_NE(it, loadMap.end());
                    ASSERT_EQ(it->second, TEST_ENDPOINT_CONFIG_2);
                }
                TEST_F(SQLiteCapabilitiesDelegateStorageTest, test_clearDatabaseWorks) {
                    unordered_map<string, string> testMap;
                    ASSERT_TRUE(m_db->store(TEST_ENDPOINT_ID_1, TEST_ENDPOINT_CONFIG_1));
                    ASSERT_TRUE(m_db->load(&testMap));
                    ASSERT_THAT(testMap.size(), Eq(1U));
                    ASSERT_TRUE(m_db->clearDatabase());
                    testMap.clear();
                    ASSERT_TRUE(m_db->load(&testMap));
                    ASSERT_TRUE(testMap.empty());
                    ASSERT_TRUE(m_db->store(TEST_ENDPOINT_ID_1, TEST_ENDPOINT_CONFIG_1));
                    ASSERT_TRUE(m_db->store(TEST_ENDPOINT_ID_2, TEST_ENDPOINT_CONFIG_2));
                    ASSERT_TRUE(m_db->load(&testMap));
                    ASSERT_THAT(testMap.size(), Eq(2U));
                    ASSERT_TRUE(m_db->clearDatabase());
                    testMap.clear();
                    ASSERT_TRUE(m_db->load(&testMap));
                    ASSERT_TRUE(testMap.empty());
                    testMap.clear();
                    ASSERT_TRUE(m_db->load(&testMap));
                    ASSERT_TRUE(testMap.empty());
                    ASSERT_TRUE(m_db->clearDatabase());
                }
            }
        }
    }
}