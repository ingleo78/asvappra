#ifndef ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONSSTORAGEINTERFACE_H_
#define ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONSSTORAGEINTERFACE_H_

#include <avs/IndicatorState.h>
#include <capability_agents/Notifications/NotificationIndicator.h>

namespace alexaClientSDK {
    namespace acsdkNotificationsInterfaces {
        using namespace avsCommon;
        class NotificationsStorageInterface {
        public:
            using IndicatorState = avsCommon::avs::IndicatorState;
            virtual ~NotificationsStorageInterface() = default;
            virtual bool createDatabase();
            virtual bool open();
            virtual void close();
            virtual bool enqueue(const acsdkNotifications::NotificationIndicator& notificationIndicator);
            virtual bool dequeue();
            virtual bool peek(acsdkNotifications::NotificationIndicator* notificationIndicator);
            virtual bool setIndicatorState(IndicatorState state);
            virtual bool getIndicatorState(avs::IndicatorState* state);
            virtual bool checkForEmptyQueue(bool* empty);
            virtual bool clearNotificationIndicators();
            virtual bool getQueueSize(int* size);
        };
    }
}
#endif