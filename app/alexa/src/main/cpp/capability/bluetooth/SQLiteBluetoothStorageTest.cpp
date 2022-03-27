#include <fstream>
#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <configuration/ConfigurationNode.h>
#include "SQLiteBluetoothStorage.h"

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        namespace test {
            using namespace testing;
            static const string TEST_DATABASE = "SQLiteBluetoothStorageTestDatabase.db";
            static const string BLUETOOTH_JSON = R"({"bluetooth" : {"databaseFilePath":")" + TEST_DATABASE + R"("}})";
            static const string FILE_EXISTS_ERROR = "Database File " + TEST_DATABASE + " already exists.";
            static const string TEST_MAC = "01:23:45:67:89:ab";
            static const string TEST_MAC_2 = "11:23:45:67:89:ab";
            static const string TEST_UUID = "650f973b-c2ab-4c6e-bff4-3788cd521340";
            static const string TEST_UUID_2 = "750f973b-c2ab-4c6e-bff4-3788cd521340";
            static const string TEST_UNKNOWN = "UNKNOWN";
            static const string TEST_OTHER = "OTHER";
            static const string TEST_PHONE = "PHONE";
            static const string UUID_TABLE_NAME = "uuidMapping";
            static const string COLUMN_UUID = "uuid";
            static const string COLUMN_MAC = "mac";
            static bool fileExists(const string& file) {
                ifstream dbFile(file);
                return dbFile.good();
            }
            class SQLiteBluetoothStorageTest : public ::testing::Test {
            public:
                void SetUp();
                void TearDown();
            protected:
                void closeAndDeleteDB();
                bool createLegacyDatabase();
                bool insertEntryLegacy(const string& uuid, const string& mac);
                bool setupDatabase(bool migratedDatabase);
                void getOrderedMacHelper(bool ascending);
                void getRowsHelper(function<bool(SQLiteBluetoothStorage&, unordered_map<string, string>*)> retrieveRows, const unordered_map<string, string>& macToUuids,
                                   const unordered_map<string, string>& expected);
                void getRetrieveValueHelper(function<bool(SQLiteBluetoothStorage&, const string&, string*)> retrieveValue, const string& key,
                                            const string& expectedValue, const unordered_map<string, string>& macToUuids);
                unique_ptr<SQLiteBluetoothStorage> m_db;
                unique_ptr<SQLiteDatabase> m_sqLiteDb;
            };
            void SQLiteBluetoothStorageTest::closeAndDeleteDB() {
                if (m_db) m_db->close();
                if (m_sqLiteDb) m_sqLiteDb->close();
                m_db.reset();
                m_sqLiteDb.reset();
                if (fileExists(TEST_DATABASE)) remove(TEST_DATABASE.c_str());
            }
            bool SQLiteBluetoothStorageTest::createLegacyDatabase() {
                m_sqLiteDb = unique_ptr<SQLiteDatabase>(new SQLiteDatabase(TEST_DATABASE));
                if (!m_sqLiteDb || !m_sqLiteDb->initialize()) return false;
                if (!m_sqLiteDb->performQuery("CREATE TABLE " + UUID_TABLE_NAME + "(" + COLUMN_UUID + " text not null unique, " + COLUMN_MAC +
                    " text not null unique);")) {
                    m_sqLiteDb->close();
                    return false;
                }
                return true;
            }
            bool SQLiteBluetoothStorageTest::insertEntryLegacy(const string& uuid, const string& mac) {
                const string sqlString = "INSERT INTO " + UUID_TABLE_NAME + " (" + COLUMN_UUID + "," + COLUMN_MAC + ") VALUES (?,?);";
                auto statement = m_sqLiteDb->createStatement(sqlString);
                if (!statement) return false;
                const int UUID_INDEX = 1;
                const int MAC_INDEX = 2;
                if (!statement->bindStringParameter(UUID_INDEX, uuid) || !statement->bindStringParameter(MAC_INDEX, mac)) return false;
                if (!statement->step()) return false;
                return true;
            }
            bool SQLiteBluetoothStorageTest::setupDatabase(bool migratedDatabase) {
                if (migratedDatabase) {
                    createLegacyDatabase();
                    m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                    if (!m_db || !(m_db->open())) return false;
                } else {
                    m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                    if (!m_db || !(m_db->createDatabase())) return false;
                }
                return true;
            }
            void SQLiteBluetoothStorageTest::SetUp() {
                if (fileExists(TEST_DATABASE)) {
                    ADD_FAILURE() << FILE_EXISTS_ERROR;
                    exit(1);
                }
                auto json = shared_ptr<stringstream>(new stringstream());
                *json << BLUETOOTH_JSON;
                vector<shared_ptr<istream>> jsonStream;
                jsonStream.push_back(json);
                ASSERT_TRUE(ConfigurationNode::initialize(jsonStream));
            }
            void SQLiteBluetoothStorageTest::TearDown() {
                configuration::ConfigurationNode::uninitialize();
                closeAndDeleteDB();
            }
            void SQLiteBluetoothStorageTest::getOrderedMacHelper(bool ascending) {
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC_2, TEST_UUID_2));
                list<string> expected;
                if (ascending) {
                    expected.push_back(TEST_MAC);
                    expected.push_back(TEST_MAC_2);
                } else {
                    expected.push_back(TEST_MAC_2);
                    expected.push_back(TEST_MAC);
                }
                list<string> rows;
                ASSERT_TRUE(m_db->getOrderedMac(ascending, &rows));
                ASSERT_THAT(rows, Eq(expected));
            }
            void SQLiteBluetoothStorageTest::getRowsHelper(
                function<bool(SQLiteBluetoothStorage&, unordered_map<string, string>*)> retrieveRows,
                const unordered_map<string, string>& macToUuids,
                const unordered_map<string, string>& expected) {
                for (const auto& macAndUuid : macToUuids) {
                    ASSERT_TRUE(m_db->insertByMac(macAndUuid.first, macAndUuid.second));
                }
                unordered_map<string, string> rows;
                ASSERT_TRUE(retrieveRows(*m_db, &rows));
                ASSERT_THAT(rows, Eq(expected));
            }
            void SQLiteBluetoothStorageTest::getRetrieveValueHelper(function<bool(SQLiteBluetoothStorage&, const string&, string*)> retrieveValue,
                                                                    const string& key, const string& expectedValue, const unordered_map<string,
                                                                    string>& macToUuids) {
                for (const auto& macAndUuid : macToUuids) {
                    ASSERT_TRUE(m_db->insertByMac(macAndUuid.first, macAndUuid.second));
                }
                string value;
                ASSERT_TRUE(retrieveValue(*m_db, key, &value));
                ASSERT_THAT(value, Eq(expectedValue));
            }
            TEST_F(SQLiteBluetoothStorageTest, uninitializedDatabase) {
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_FALSE(m_db->open());
            }
            TEST_F(SQLiteBluetoothStorageTest, openDatabase) {
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->createDatabase());
                m_db->close();
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->open());
            }
            TEST_F(SQLiteBluetoothStorageTest, openLegacyDatabase) {
                createLegacyDatabase();
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->open());
            }
            TEST_F(SQLiteBluetoothStorageTest, retrieveCategoryforUnknownUUID) {
                createLegacyDatabase();
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->open());
                string category;
                ASSERT_FALSE(m_db->getCategory(TEST_UUID, &category));
                ASSERT_THAT(category, Eq(""));
            }
            TEST_F(SQLiteBluetoothStorageTest, insertByMacPostDatabaseUpgrade) {
                createLegacyDatabase();
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->open());
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                string category;
                ASSERT_TRUE(m_db->getCategory(TEST_UUID, &category));
                ASSERT_THAT(category, Eq(TEST_UNKNOWN));
            }
            TEST_F(SQLiteBluetoothStorageTest, retrieveMacforKnownUUID) {
                createLegacyDatabase();
                insertEntryLegacy(TEST_UUID, TEST_MAC);
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->open());
                string mac;
                ASSERT_TRUE(m_db->getMac(TEST_UUID, &mac));
                ASSERT_THAT(mac, Eq(TEST_MAC));
            }
            TEST_F(SQLiteBluetoothStorageTest, retrieveCategoryforKnownUUID) {
                createLegacyDatabase();
                insertEntryLegacy(TEST_UUID, TEST_MAC);
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->open());
                string category;
                ASSERT_TRUE(m_db->getCategory(TEST_UUID, &category));
                ASSERT_THAT(category, Eq(TEST_OTHER));
            }
            TEST_F(SQLiteBluetoothStorageTest, retrieveCategoryforKnownMultipleUUID) {
                createLegacyDatabase();
                insertEntryLegacy(TEST_UUID, TEST_MAC);
                insertEntryLegacy(TEST_UUID_2, TEST_MAC_2);
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->open());
                string category;
                ASSERT_TRUE(m_db->getCategory(TEST_UUID, &category));
                ASSERT_THAT(category, Eq(TEST_OTHER));
                ASSERT_TRUE(m_db->getCategory(TEST_UUID_2, &category));
                ASSERT_THAT(category, Eq(TEST_OTHER));
            }
            class SQLiteBluetoothStorageParameterizedTests : public SQLiteBluetoothStorageTest , public WithParamInterface<bool> {};
            INSTANTIATE_TEST_CASE_P(Parameterized, SQLiteBluetoothStorageParameterizedTests, Values(true, false));
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_createInvalidConfigurationRoot) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ConfigurationNode::uninitialize();
                vector<shared_ptr<istream>> empty;
                ConfigurationNode::initialize(empty);
                ASSERT_THAT(SQLiteBluetoothStorage::create(ConfigurationNode::getRoot()), IsNull());
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_createValidConfigurationRoot) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_THAT(SQLiteBluetoothStorage::create(ConfigurationNode::getRoot()), NotNull());
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_createDatabaseSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                closeAndDeleteDB();
                m_db = SQLiteBluetoothStorage::create(ConfigurationNode::getRoot());
                ASSERT_THAT(m_db, NotNull());
                ASSERT_TRUE(m_db->createDatabase());
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_createExistingDatabaseFails) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_FALSE(m_db->createDatabase());
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_openExistingDatabaseSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                m_db->close();
                ASSERT_TRUE(m_db->open());
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_clearOnOneRowSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                ASSERT_TRUE(m_db->clear());
                unordered_map<string, string> rows;
                ASSERT_TRUE(m_db->getUuidToMac(&rows));
                ASSERT_THAT(rows.size(), Eq(0U));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_clearOnMultipleRowsSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC_2, TEST_UUID_2));
                ASSERT_TRUE(m_db->clear());
                unordered_map<string, string> rows;
                ASSERT_TRUE(m_db->getUuidToMac(&rows));
                ASSERT_THAT(rows.size(), Eq(0U));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_clearOnEmptySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_TRUE(m_db->clear());
                unordered_map<string, string> rows;
                ASSERT_TRUE(m_db->getUuidToMac(&rows));
                ASSERT_THAT(rows.size(), Eq(0U));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getUuidWithOneSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getUuid, TEST_MAC, TEST_UUID, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getUuidWithMultipleSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getUuid, TEST_MAC, TEST_UUID, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getUuidNoMatchingFails) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                string uuid;
                ASSERT_FALSE(m_db->getUuid(TEST_MAC, &uuid));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getMacWithOneSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getMac, TEST_UUID, TEST_MAC, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getMacWithMultipleSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getMac, TEST_UUID, TEST_MAC, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getMacNoMatchingFails) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                string mac;
                ASSERT_FALSE(m_db->getMac(TEST_UUID, &mac));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getCategoryWithOneSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getCategory, TEST_UUID, TEST_UNKNOWN, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getCategoryWithMultipleSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getCategory, TEST_UUID, TEST_UNKNOWN, data);
                ASSERT_TRUE(m_db->updateByCategory(TEST_UUID, TEST_PHONE));
                string category;
                ASSERT_TRUE(m_db->getCategory(TEST_UUID, &category));
                ASSERT_THAT(category, Eq(TEST_PHONE));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getCategoryWithMultipleInsertByMacSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getCategory, TEST_UUID, TEST_UNKNOWN, data);
                ASSERT_TRUE(m_db->updateByCategory(TEST_UUID, TEST_PHONE));
                getRetrieveValueHelper(&SQLiteBluetoothStorage::getCategory, TEST_UUID, TEST_PHONE, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getCategoryNoMatchingFails) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                string category;
                ASSERT_FALSE(m_db->getCategory(TEST_UUID, &category));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getMacToUuidWithOneRowSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                getRowsHelper(&SQLiteBluetoothStorage::getMacToUuid, data, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getMacToUuidWithMultipleRowsSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                getRowsHelper(&SQLiteBluetoothStorage::getMacToUuid, data, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getMacToUuidWithEmptySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                unordered_map<string, string> data;
                getRowsHelper(&SQLiteBluetoothStorage::getMacToUuid, data, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getUuidToMacWithOneRowSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                const unordered_map<string, string> expected{{TEST_UUID, TEST_MAC}};
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToMac, data, expected);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getUuidToMacWithMultipleRowsSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                const unordered_map<string, string> expected{{TEST_UUID, TEST_MAC}, {TEST_UUID_2, TEST_MAC_2}};
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToMac, data, expected);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getUuidToMacWithEmptySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                unordered_map<string, string> data;
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToMac, data, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getUuidToCategoryWithOneRowSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                const unordered_map<string, string> expected{{TEST_UUID, TEST_UNKNOWN}};
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToCategory, data, expected);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getUuidToCategoryWithOneRowUpdateCategorySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                const unordered_map<string, string> expected{{TEST_UUID, TEST_UNKNOWN}};
                const unordered_map<string, string> expectedUpdate{{TEST_UUID, TEST_PHONE}};
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToCategory, data, expected);
                ASSERT_TRUE(m_db->updateByCategory(TEST_UUID, TEST_PHONE));
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToCategory, data, expectedUpdate);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getUuidToCategoryWithMultipleRowsSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                const unordered_map<string, string> expected{{TEST_UUID, TEST_UNKNOWN}, {TEST_UUID_2, TEST_UNKNOWN}};
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToCategory, data, expected);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getUuidToCategoryWithEmptySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                unordered_map<string, string> data;
                getRowsHelper(&SQLiteBluetoothStorage::getUuidToCategory, data, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getMacToCategoryWithOneRowSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                const unordered_map<string, string> expected{{TEST_MAC, TEST_UNKNOWN}};
                getRowsHelper(&SQLiteBluetoothStorage::getMacToCategory, data, expected);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getMacToCategoryWithOneRowUpdateCategorySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}};
                const unordered_map<string, string> expected{{TEST_MAC, TEST_UNKNOWN}};
                const unordered_map<string, string> expectedUpdate{{TEST_MAC, TEST_PHONE}};
                getRowsHelper(&SQLiteBluetoothStorage::getMacToCategory, data, expected);
                ASSERT_TRUE(m_db->updateByCategory(TEST_UUID, TEST_PHONE));
                getRowsHelper(&SQLiteBluetoothStorage::getMacToCategory, data, expectedUpdate);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getMacToCategoryWithMultipleRowsSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> data{{TEST_MAC, TEST_UUID}, {TEST_MAC_2, TEST_UUID_2}};
                const unordered_map<string, string> expected{{TEST_MAC, TEST_UNKNOWN}, {TEST_MAC_2, TEST_UNKNOWN}};
                getRowsHelper(&SQLiteBluetoothStorage::getMacToCategory, data, expected);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, getMacToCategoryWithEmptySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                unordered_map<string, string> data;
                getRowsHelper(&SQLiteBluetoothStorage::getMacToCategory, data, data);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getOrderedMacAscending) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                getOrderedMacHelper(true);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_getOrderedMacDescending) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                getOrderedMacHelper(false);
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, updateByCategorySucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                ASSERT_TRUE(m_db->updateByCategory(TEST_UUID, TEST_PHONE));
                string category;
                ASSERT_TRUE(m_db->getCategory(TEST_UUID, &category));
                ASSERT_THAT(category, Eq(TEST_PHONE));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, updateByCategoryNoMatchingFails) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                string category;
                ASSERT_FALSE(m_db->updateByCategory(TEST_UUID, TEST_PHONE));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_insertByMacSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> expected{{TEST_MAC, TEST_UUID}};
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                unordered_map<string, string> rows;
                ASSERT_TRUE(m_db->getMacToUuid(&rows));
                ASSERT_THAT(rows, Eq(expected));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_insertByMacDuplicateFails) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                ASSERT_FALSE(m_db->insertByMac(TEST_MAC, TEST_UUID, false));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_insertByMacOverwriteSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                const unordered_map<string, string> expected{{TEST_MAC, TEST_UUID}};
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID_2));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                unordered_map<string, string> rows;
                ASSERT_TRUE(m_db->getMacToUuid(&rows));
                ASSERT_THAT(rows, Eq(expected));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_removeExistingSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_TRUE(m_db->insertByMac(TEST_MAC, TEST_UUID));
                ASSERT_TRUE(m_db->remove(TEST_MAC));
                unordered_map<string, string> rows;
                ASSERT_TRUE(m_db->getMacToUuid(&rows));
                ASSERT_THAT(rows.size(), Eq(0U));
            }
            TEST_P(SQLiteBluetoothStorageParameterizedTests, test_removeNonExistingSucceeds) {
                ASSERT_TRUE(setupDatabase(GetParam()));
                ASSERT_TRUE(m_db->remove(TEST_MAC));
                unordered_map<string, string> rows;
                ASSERT_TRUE(m_db->getMacToUuid(&rows));
                ASSERT_THAT(rows.size(), Eq(0U));
            }
        }
    }
}