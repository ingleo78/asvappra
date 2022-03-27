#include <fstream>
#include <queue>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <util/file/FileUtils.h>
#include "SQLiteMessageStorage.h"

namespace alexaClientSDK {
    namespace certifiedSender {
        namespace test {
            using namespace testing;
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace file;
            static const string TEST_DATABASE_FILE_PATH = "messageStorageTestDatabase.db";
            static const string TEST_SECOND_DATABASE_FILE_PATH = "messageStorageSecondTestDatabase.db";
            static const string PATH_DELIMITER = "/";
            static string g_dbTestFilePath;
            static const string TEST_MESSAGE_ONE = "test_message_one";
            static const string TEST_MESSAGE_TWO = "test_message_two";
            static const string TEST_MESSAGE_THREE = "test_message_three";
            static const string TEST_MESSAGE_URI = "/v20160207/events/SpeechRecognizer/Recognize";
            class MessageStorageTest : public Test {
            public:
                MessageStorageTest() : m_storage{make_shared<SQLiteMessageStorage>(g_dbTestFilePath)} {
                    cleanupLocalDbFile();
                }
                ~MessageStorageTest() {
                    m_storage->close();
                    cleanupLocalDbFile();
                }
                void createDatabase() {
                    m_storage->createDatabase();
                }
                void cleanupLocalDbFile() {
                    if (g_dbTestFilePath.empty()) return;
                    if (fileExists(g_dbTestFilePath)) removeFile(g_dbTestFilePath.c_str());
                }
            protected:
                shared_ptr<MessageStorageInterface> m_storage;
            };
            static bool isOpen(const shared_ptr<MessageStorageInterface>& storage) {
                queue<MessageStorageInterface::StoredMessage> dummyMessages;
                return storage->load(&dummyMessages);
            }
            TEST_F(MessageStorageTest, test_constructionAndDestruction) {
                EXPECT_FALSE(isOpen(m_storage));
            }
            TEST_F(MessageStorageTest, test_databaseCreation) {
                EXPECT_FALSE(isOpen(m_storage));
                createDatabase();
                EXPECT_TRUE(isOpen(m_storage));
            }
            TEST_F(MessageStorageTest, test_openAndCloseDatabase) {
                EXPECT_FALSE(isOpen(m_storage));
                createDatabase();
                EXPECT_TRUE(isOpen(m_storage));
                m_storage->close();
                EXPECT_FALSE(isOpen(m_storage));
                EXPECT_TRUE(m_storage->open());
                EXPECT_TRUE(isOpen(m_storage));
                m_storage->close();
                EXPECT_FALSE(isOpen(m_storage));
            }
            TEST_F(MessageStorageTest, test_databaseStoreAndLoad) {
                createDatabase();
                EXPECT_TRUE(isOpen(m_storage));
                queue<MessageStorageInterface::StoredMessage> dbMessages;
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 0);
                int dbId = 0;
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_ONE, &dbId));
                EXPECT_EQ(dbId, 1);
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 1);
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_ONE);
                dbMessages.pop();
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_TWO, &dbId));
                EXPECT_EQ(dbId, 2);
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_THREE, &dbId));
                EXPECT_EQ(dbId, 3);
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 3);
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_ONE);
                dbMessages.pop();
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_TWO);
                dbMessages.pop();
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_THREE);
            }
            TEST_F(MessageStorageTest, test_databaseErase) {
                createDatabase();
                EXPECT_TRUE(isOpen(m_storage));
                int dbId = 0;
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_ONE, &dbId));
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_TWO, &dbId));
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_THREE, &dbId));
                queue<MessageStorageInterface::StoredMessage> dbMessages;
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 3);
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_ONE);
                EXPECT_TRUE(m_storage->erase(dbMessages.front().id));
                while(!dbMessages.empty()) dbMessages.pop();
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 2);
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_TWO);
                dbMessages.pop();
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_THREE);
            }
            TEST_F(MessageStorageTest, test_databaseClear) {
                createDatabase();
                EXPECT_TRUE(isOpen(m_storage));
                int dbId = 0;
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_ONE, &dbId));
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_TWO, &dbId));
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_THREE, &dbId));
                queue<MessageStorageInterface::StoredMessage> dbMessages;
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 3);
                while(!dbMessages.empty()) dbMessages.pop();
                EXPECT_TRUE(m_storage->clearDatabase());
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 0);
            }
            TEST_F(MessageStorageTest, testDatabaseStoreAndLoadWithURI) {
                createDatabase();
                EXPECT_TRUE(isOpen(m_storage));
                queue<MessageStorageInterface::StoredMessage> dbMessages;
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(static_cast<int>(dbMessages.size()), 0);
                int dbId = 0;
                EXPECT_TRUE(m_storage->store(TEST_MESSAGE_ONE, TEST_MESSAGE_URI, &dbId));
                EXPECT_EQ(dbId, 1);
                EXPECT_TRUE(m_storage->load(&dbMessages));
                EXPECT_EQ(dbMessages.front().message, TEST_MESSAGE_ONE);
                EXPECT_EQ(dbMessages.front().uriPathExtension, TEST_MESSAGE_URI);
            }
        }
    }
}
int main(int argc, char** argv) {
    using namespace std;
    using namespace alexaClientSDK::certifiedSender::test;
    ::testing::InitGoogleTest(&argc, argv);
    if (argc < 2) {
        cerr << "USAGE: " << string(argv[0]) << " <path_to_test_directory_location>" << std::endl;
        return 1;
    } else {
        g_dbTestFilePath = string(argv[1]) + PATH_DELIMITER + TEST_DATABASE_FILE_PATH;
        return RUN_ALL_TESTS();
    }
}
