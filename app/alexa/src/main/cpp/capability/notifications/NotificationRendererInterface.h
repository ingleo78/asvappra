#ifndef ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONRENDERERINTERFACE_H_
#define ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONRENDERERINTERFACE_H_

#include <iostream>
#include <memory>
#include <string>
#include "NotificationRendererObserverInterface.h"

namespace alexaClientSDK {
    namespace acsdkNotificationsInterfaces {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        class NotificationRendererInterface {
        public:
            virtual ~NotificationRendererInterface() = default;
            virtual void addObserver(shared_ptr<NotificationRendererObserverInterface> observer) = 0;
            virtual void removeObserver(shared_ptr<NotificationRendererObserverInterface> observer) = 0;
            virtual bool renderNotification(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url) = 0;
            virtual bool cancelNotificationRendering() = 0;
        };
    }
}
#endif