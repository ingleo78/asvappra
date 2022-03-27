#ifndef ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONSSTORAGEINTERFACE_H_
#define ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONSSTORAGEINTERFACE_H_

#include <avs/IndicatorState.h>
#include "NotificationIndicator.h"

namespace alexaClientSDK {
    namespace acsdkNotificationsInterfaces {
        class NotificationsStorageInterface {
        public:
            using IndicatorState = avsCommon::avs::IndicatorState;
            virtual ~NotificationsStorageInterface() = default;
            virtual bool createDatabase() = 0;
            virtual bool open() = 0;
            virtual void close() = 0;
            virtual bool enqueue(const acsdkNotifications::NotificationIndicator& notificationIndicator) = 0;
            virtual bool dequeue() = 0;
            virtual bool peek(acsdkNotifications::NotificationIndicator* notificationIndicator) = 0;
            virtual bool setIndicatorState(IndicatorState state) = 0;
            virtual bool getIndicatorState(IndicatorState* state) = 0;
            virtual bool checkForEmptyQueue(bool* empty) = 0;
            virtual bool clearNotificationIndicators() = 0;
            virtual bool getQueueSize(int* size) = 0;
        };
    }
}
#endif