#ifndef ACSDKNOTIFICATIONS_SQLITENOTIFICATIONSSTORAGE_H_
#define ACSDKNOTIFICATIONS_SQLITENOTIFICATIONSSTORAGE_H_

#include <mutex>
#include <configuration/ConfigurationNode.h>
#include <storage/SQLiteDatabase.h>
#include <acsdk_alerts_interfaces/NotificationsStorageInterface.h>

namespace alexaClientSDK {
    namespace acsdkNotifications {
        using namespace std;
        using namespace storage;
        using namespace sqliteStorage;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace configuration;
        using namespace acsdkNotificationsInterfaces;
        class SQLiteNotificationsStorage : public NotificationsStorageInterface {
        public:
            static unique_ptr<SQLiteNotificationsStorage> create(const ConfigurationNode& configurationRoot);
            SQLiteNotificationsStorage(const string& databaseFilePath);
            ~SQLiteNotificationsStorage();
            bool createDatabase() override;
            bool open() override;
            void close() override;
            bool enqueue(const NotificationIndicator& notificationIndicator) override;
            bool dequeue() override;
            bool peek(NotificationIndicator* notificationIndicator) override;
            bool setIndicatorState(IndicatorState state) override;
            bool getIndicatorState(IndicatorState* state) override;
            bool checkForEmptyQueue(bool* empty) override;
            bool clearNotificationIndicators() override;
            bool getQueueSize(int* size) override;
        private:
            bool getNextNotificationIndicatorLocked(NotificationIndicator* notificationIndicator);
            mutex m_databaseMutex;
            SQLiteDatabase m_database;
        };
    }
}
#endif