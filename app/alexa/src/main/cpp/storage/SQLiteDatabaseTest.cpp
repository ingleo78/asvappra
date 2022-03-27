#include <chrono>
#include <cstdlib>
#include <gtest/gtest.h>
#include <util/file/FileUtils.h>
#include "SQLiteDatabase.h"

namespace alexaClientSDK {
    namespace storage {
        namespace sqliteStorage {
            namespace test {
                using namespace std;
                using namespace chrono;
                using namespace avsCommon;
                using namespace utils;
                using namespace file;
                static string g_workingDirectory;
                static const string BAD_PATH = "_/_/_/there/is/no/way/this/path/should/exist/,/so/it/should/cause/an/error/when/creating/the/db";
                static std::string generateDbFilePath() {
                    auto currentTime = high_resolution_clock::now();
                    auto nanosecond = static_cast<int64_t>(duration_cast<nanoseconds>(currentTime.time_since_epoch()).count());
                    string filePath = g_workingDirectory + "/SQLiteDatabaseTest-" + to_string(nanosecond) + to_string(rand());
                    EXPECT_FALSE(fileExists(filePath));
                    return filePath;
                }
                TEST(SQLiteDatabaseTest, test_closeThenOpen) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db(dbFilePath);
                    ASSERT_TRUE(db.initialize());
                    db.close();
                    ASSERT_TRUE(db.open());
                }
                TEST(SQLiteDatabaseTest, test_initializeAlreadyExisting) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db1(dbFilePath);
                    ASSERT_TRUE(db1.initialize());
                    SQLiteDatabase db2(dbFilePath);
                    ASSERT_FALSE(db2.initialize());
                    db2.close();
                    db1.close();
                }
                TEST(SQLiteDatabaseTest, test_initializeBadPath) {
                    SQLiteDatabase db(BAD_PATH);
                    ASSERT_FALSE(db.initialize());
                }
                TEST(SQLiteDatabaseTest, test_initializeOnDirectory) {
                    SQLiteDatabase db(g_workingDirectory);
                    ASSERT_FALSE(db.initialize());
                }
                TEST(SQLiteDatabaseTest, test_initializeTwice) {
                    SQLiteDatabase db(generateDbFilePath());
                    ASSERT_TRUE(db.initialize());
                    ASSERT_FALSE(db.initialize());
                    db.close();
                }
                TEST(SQLiteDatabaseTest, test_openAlreadyExisting) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db1(dbFilePath);
                    ASSERT_TRUE(db1.initialize());
                    SQLiteDatabase db2(dbFilePath);
                    ASSERT_TRUE(db2.open());
                    db2.close();
                    db1.close();
                }
                TEST(SQLiteDatabaseTest, test_openBadPath) {
                    SQLiteDatabase db(BAD_PATH);
                    ASSERT_FALSE(db.open());
                }
                TEST(SQLiteDatabaseTest, test_openDirectory) {
                    SQLiteDatabase db(g_workingDirectory);
                    ASSERT_FALSE(db.open());
                }
                TEST(SQLiteDatabaseTest, test_openTwice) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db1(dbFilePath);
                    ASSERT_TRUE(db1.initialize());
                    SQLiteDatabase db2(dbFilePath);
                    ASSERT_TRUE(db2.open());
                    ASSERT_FALSE(db2.open());
                    db2.close();
                    db1.close();
                }
                TEST(SQLiteDatabaseTest, test_transactionsCommit) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db(dbFilePath);
                    ASSERT_TRUE(db.initialize());
                    {
                        auto transaction1 = db.beginTransaction();
                        ASSERT_NE(transaction1, nullptr);
                        ASSERT_TRUE(transaction1->commit());
                    }
                    auto transaction2 = db.beginTransaction();
                    ASSERT_NE(transaction2, nullptr);
                    db.close();
                }
                TEST(SQLiteDatabaseTest, test_transactionsRollback) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db(dbFilePath);
                    ASSERT_TRUE(db.initialize());
                    {
                        auto transaction1 = db.beginTransaction();
                        ASSERT_NE(transaction1, nullptr);
                        ASSERT_TRUE(transaction1->rollback());
                    }
                    auto transaction2 = db.beginTransaction();
                    ASSERT_NE(transaction2, nullptr);
                    db.close();
                }
                TEST(SQLiteDatabaseTest, test_nestedTransactions) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db(dbFilePath);
                    ASSERT_TRUE(db.initialize());
                    auto transaction1 = db.beginTransaction();
                    ASSERT_NE(transaction1, nullptr);
                    auto transaction2 = db.beginTransaction();
                    ASSERT_EQ(transaction2, nullptr);
                    db.close();
                }
                TEST(SQLiteDatabaseTest, test_doubleCommit) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db(dbFilePath);
                    ASSERT_TRUE(db.initialize());
                    auto transaction1 = db.beginTransaction();
                    ASSERT_NE(transaction1, nullptr);
                    ASSERT_TRUE(transaction1->commit());
                    ASSERT_FALSE(transaction1->commit());
                    db.close();
                }
                TEST(SQLiteDatabaseTest, test_autoRollback) {
                    auto dbFilePath = generateDbFilePath();
                    SQLiteDatabase db(dbFilePath);
                    ASSERT_TRUE(db.initialize());
                    {
                        auto transaction1 = db.beginTransaction();
                        ASSERT_NE(transaction1, nullptr);
                    }
                    auto transaction2 = db.beginTransaction();
                    ASSERT_NE(transaction2, nullptr);
                    db.close();
                }
            }
        }
    }
}
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    if (argc < 2) {
        std::cerr << "Usage: " << std::string(argv[0]) << " <path to folder for test>" << std::endl;
        return -1;
    } else {
        alexaClientSDK::storage::sqliteStorage::test::g_workingDirectory = std::string(argv[1]);
        return RUN_ALL_TESTS();
    }
}
