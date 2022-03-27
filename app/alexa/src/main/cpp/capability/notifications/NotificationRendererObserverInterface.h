#ifndef ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONRENDEREROBSERVERINTERFACE_H_
#define ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONRENDEREROBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace acsdkNotificationsInterfaces {
        class NotificationRendererObserverInterface {
        public:
            virtual ~NotificationRendererObserverInterface() = default;
            virtual void onNotificationRenderingFinished() = 0;
        };
    }
}
#endif