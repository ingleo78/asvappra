#ifndef ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONSOBSERVERINTERFACE_H_
#define ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONSOBSERVERINTERFACE_H_

#include <avs/IndicatorState.h>

namespace alexaClientSDK {
    namespace acsdkNotificationsInterfaces {
        class NotificationsObserverInterface {
        public:
            virtual ~NotificationsObserverInterface() = default;
            virtual void onSetIndicator(avsCommon::avs::IndicatorState state);
            virtual void onNotificationReceived();
        };
    }
}
#endif