#include <memory>
#include <random>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <util/file/FileUtils.h>
#include "NotificationIndicator.h"
#include "SQLiteNotificationsStorage.h"


using namespace ::testing;
namespace alexaClientSDK {
    namespace acsdkNotifications {
        namespace test {
            using namespace file;
            using IndicatorState = avs::IndicatorState;
            static const string TEST_DATABASE_FILE_PATH = "notificationsStorageTestDatabase.db";
            static const string PATH_DELIMITER = "/";
            static const string TEST_ASSET_ID1 = "testAssetId1";
            static const string TEST_ASSET_ID2 = "testAssetId2";
            static const string TEST_ASSET_URL1 = "testAssetUrl1";
            static const string TEST_ASSET_URL2 = "testAssetUrl2";
            static const string INDICATOR_STATE_NAME = "indicatorState";
            static const int INVALID_STATE_VALUE = 123;
            static const unsigned int NUM_TEST_INDICATORS = 15;
            static const unsigned int NOTIFICATION_INDICATOR_SEED = 1;
            static const unsigned int MAX_RANDOM_INT = 100;
            static bool isOpen(const shared_ptr<NotificationsStorageInterface>& storage) {
                int dummySize;
                return storage->getQueueSize(&dummySize);
            }
            class NotificationsStorageTest : public Test {
            public:
                NotificationsStorageTest();
                ~NotificationsStorageTest();
                void createDatabase();
                void cleanupLocalDbFile();
                void checkNotificationIndicatorsEquality(const NotificationIndicator& actual, const NotificationIndicator& expected);
            protected:
                shared_ptr<SQLiteNotificationsStorage> m_storage;
            };
            NotificationsStorageTest::NotificationsStorageTest() : m_storage{make_shared<SQLiteNotificationsStorage>(TEST_DATABASE_FILE_PATH)} {
                cleanupLocalDbFile();
            }
            NotificationsStorageTest::~NotificationsStorageTest() {
                m_storage->close();
                cleanupLocalDbFile();
            }
            void NotificationsStorageTest::createDatabase() {
                m_storage->createDatabase();
            }
            void NotificationsStorageTest::cleanupLocalDbFile() {
                if (fileExists(TEST_DATABASE_FILE_PATH)) removeFile(TEST_DATABASE_FILE_PATH.c_str());
            }
            void NotificationsStorageTest::checkNotificationIndicatorsEquality(
                const NotificationIndicator& actual,
                const NotificationIndicator& expected) {
                ASSERT_EQ(actual.persistVisualIndicator, expected.persistVisualIndicator);
                ASSERT_EQ(actual.playAudioIndicator, expected.playAudioIndicator);
                ASSERT_EQ(actual.asset.assetId, expected.asset.assetId);
                ASSERT_EQ(actual.asset.url, expected.asset.url);
            }
            TEST_F(NotificationsStorageTest, test_constructionAndDestruction) {
                ASSERT_FALSE(isOpen(m_storage));
            }
            TEST_F(NotificationsStorageTest, test_databaseCreation) {
                ASSERT_FALSE(isOpen(m_storage));
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
            }
            TEST_F(NotificationsStorageTest, test_openAndCloseDatabase) {
                ASSERT_FALSE(isOpen(m_storage));
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                m_storage->close();
                ASSERT_FALSE(isOpen(m_storage));
                ASSERT_TRUE(m_storage->open());
                ASSERT_TRUE(isOpen(m_storage));
                m_storage->close();
                ASSERT_FALSE(isOpen(m_storage));
            }
            TEST_F(NotificationsStorageTest, test_databaseEnqueueAndDequeue) {
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                NotificationIndicator secondIndicator(false, true, TEST_ASSET_ID2, TEST_ASSET_URL2);
                ASSERT_FALSE(m_storage->enqueue(firstIndicator));
                ASSERT_FALSE(m_storage->dequeue());
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                ASSERT_TRUE(m_storage->enqueue(secondIndicator));
                NotificationIndicator firstDequeue;
                ASSERT_TRUE(m_storage->peek(&firstDequeue));
                ASSERT_TRUE(m_storage->dequeue());
                checkNotificationIndicatorsEquality(firstDequeue, firstIndicator);
                NotificationIndicator secondDequeue;
                ASSERT_TRUE(m_storage->peek(&secondDequeue));
                ASSERT_TRUE(m_storage->dequeue());
                checkNotificationIndicatorsEquality(secondDequeue, secondIndicator);
                ASSERT_FALSE(m_storage->dequeue());
            }
            TEST_F(NotificationsStorageTest, test_settingAndGettingIndicatorState) {
                IndicatorState state;
                ASSERT_FALSE(m_storage->setIndicatorState(IndicatorState::ON));
                ASSERT_FALSE(m_storage->getIndicatorState(&state));
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                ASSERT_TRUE(m_storage->setIndicatorState(IndicatorState::ON));
                ASSERT_TRUE(m_storage->getIndicatorState(&state));
                ASSERT_EQ(state, IndicatorState::ON);
                ASSERT_TRUE(m_storage->setIndicatorState(IndicatorState::OFF));
                ASSERT_TRUE(m_storage->getIndicatorState(&state));
                ASSERT_EQ(state, IndicatorState::OFF);
            }
            TEST_F(NotificationsStorageTest, test_clearingNotificationIndicators) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                NotificationIndicator secondIndicator(false, true, TEST_ASSET_ID2, TEST_ASSET_URL2);
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                ASSERT_TRUE(m_storage->enqueue(secondIndicator));
                ASSERT_TRUE(m_storage->clearNotificationIndicators());
                ASSERT_FALSE(m_storage->dequeue());
            }
            TEST_F(NotificationsStorageTest, test_defaultValueForEmptyStorage) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                ASSERT_TRUE(m_storage->clearNotificationIndicators());
                IndicatorState indicatorState = IndicatorState::UNDEFINED;
                ASSERT_TRUE(m_storage->getIndicatorState(&indicatorState));
                ASSERT_TRUE(IndicatorState::UNDEFINED != indicatorState);
            }
            TEST_F(NotificationsStorageTest, test_defaultValueForInvalidDBContents) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                SQLiteDatabase database(TEST_DATABASE_FILE_PATH);
                ASSERT_TRUE(database.open());
                string sqlString = "UPDATE " + INDICATOR_STATE_NAME + " SET " + INDICATOR_STATE_NAME + " = (?);";
                auto updateStatement = database.createStatement(sqlString);
                ASSERT_THAT(updateStatement, NotNull());
                ASSERT_TRUE(updateStatement->bindIntParameter(1, INVALID_STATE_VALUE));
                ASSERT_TRUE(updateStatement->step());
                updateStatement->finalize();
                IndicatorState indicatorState = IndicatorState::UNDEFINED;
                ASSERT_TRUE(m_storage->getIndicatorState(&indicatorState));
                ASSERT_TRUE(IndicatorState::UNDEFINED != indicatorState);
            }
            TEST_F(NotificationsStorageTest, test_checkingEmptyQueue) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                bool empty = false;
                ASSERT_TRUE(m_storage->checkForEmptyQueue(&empty));
                ASSERT_TRUE(empty);
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                NotificationIndicator secondIndicator(false, true, TEST_ASSET_ID2, TEST_ASSET_URL2);
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                ASSERT_TRUE(m_storage->enqueue(secondIndicator));
                ASSERT_TRUE(m_storage->checkForEmptyQueue(&empty));
                ASSERT_FALSE(empty);
                ASSERT_TRUE(m_storage->dequeue());
                ASSERT_TRUE(m_storage->checkForEmptyQueue(&empty));
                ASSERT_FALSE(empty);
                ASSERT_TRUE(m_storage->dequeue());
                ASSERT_TRUE(m_storage->checkForEmptyQueue(&empty));
                ASSERT_TRUE(empty);
            }
            TEST_F(NotificationsStorageTest, test_databasePersistence) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                NotificationIndicator secondIndicator(false, true, TEST_ASSET_ID2, TEST_ASSET_URL2);
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                ASSERT_TRUE(m_storage->enqueue(secondIndicator));
                m_storage->close();
                ASSERT_FALSE(isOpen(m_storage));
                ASSERT_TRUE(m_storage->open());
                ASSERT_TRUE(isOpen(m_storage));
                NotificationIndicator firstDequeue;
                ASSERT_TRUE(m_storage->peek(&firstDequeue));
                ASSERT_TRUE(m_storage->dequeue());
                checkNotificationIndicatorsEquality(firstDequeue, firstIndicator);
                m_storage->close();
                ASSERT_FALSE(isOpen(m_storage));
                ASSERT_TRUE(m_storage->open());
                ASSERT_TRUE(isOpen(m_storage));
                NotificationIndicator secondDequeue;
                ASSERT_TRUE(m_storage->peek(&secondDequeue));
                ASSERT_TRUE(m_storage->dequeue());
                checkNotificationIndicatorsEquality(secondDequeue, secondIndicator);
            }
            TEST_F(NotificationsStorageTest, test_queueOrder) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                default_random_engine intGenerator;
                intGenerator.seed(NOTIFICATION_INDICATOR_SEED);
                uniform_int_distribution<int>(1, MAX_RANDOM_INT);
                vector<NotificationIndicator> notificationIndicators;
                for (unsigned int i = 0; i < NUM_TEST_INDICATORS; i++) {
                    bool r_persistVisualIndicator = intGenerator() % 2;
                    bool r_playAudioIndicator = intGenerator() % 2;
                    string r_assetId = intGenerator() % 2 ? TEST_ASSET_ID1 : TEST_ASSET_ID2;
                    string r_assetUrl = intGenerator() % 2 ? TEST_ASSET_URL1 : TEST_ASSET_URL2;
                    NotificationIndicator ni(r_persistVisualIndicator, r_playAudioIndicator, r_assetId, r_assetUrl);
                    notificationIndicators.push_back(ni);
                    m_storage->enqueue(ni);
                }
                NotificationIndicator dequeuePtr;
                for (unsigned int i = 0; i < NUM_TEST_INDICATORS; i++) {
                    ASSERT_TRUE(m_storage->peek(&dequeuePtr));
                    ASSERT_TRUE(m_storage->dequeue());
                    checkNotificationIndicatorsEquality(dequeuePtr, notificationIndicators[i]);
                }
            }
            TEST_F(NotificationsStorageTest, test_peek) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                NotificationIndicator secondIndicator(false, true, TEST_ASSET_ID2, TEST_ASSET_URL2);
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                ASSERT_TRUE(m_storage->enqueue(secondIndicator));
                NotificationIndicator peekedAt;
                ASSERT_TRUE(m_storage->peek(&peekedAt));
                checkNotificationIndicatorsEquality(peekedAt, firstIndicator);
                ASSERT_TRUE(m_storage->dequeue());
                ASSERT_TRUE(m_storage->peek(&peekedAt));
                checkNotificationIndicatorsEquality(peekedAt, secondIndicator);
            }
            TEST_F(NotificationsStorageTest, test_size) {
                createDatabase();
                ASSERT_TRUE(isOpen(m_storage));
                int size = 0;
                ASSERT_TRUE(m_storage->getQueueSize(&size));
                ASSERT_EQ(size, 0);
                NotificationIndicator firstIndicator(true, false, TEST_ASSET_ID1, TEST_ASSET_URL1);
                ASSERT_TRUE(m_storage->enqueue(firstIndicator));
                ASSERT_TRUE(m_storage->getQueueSize(&size));
                ASSERT_EQ(size, 1);
                NotificationIndicator secondIndicator(false, true, TEST_ASSET_ID2, TEST_ASSET_URL2);
                ASSERT_TRUE(m_storage->enqueue(secondIndicator));
                ASSERT_TRUE(m_storage->getQueueSize(&size));
                ASSERT_EQ(size, 2);
                ASSERT_TRUE(m_storage->dequeue());
                ASSERT_TRUE(m_storage->getQueueSize(&size));
                ASSERT_EQ(size, 1);
                ASSERT_TRUE(m_storage->dequeue());
                ASSERT_TRUE(m_storage->getQueueSize(&size));
                ASSERT_EQ(size, 0);
            }
        }
    }
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc < 1) {
        std::cerr << "USAGE: " << std::string(argv[0]) << std::endl;
        return 1;
    }
    return RUN_ALL_TESTS();
}