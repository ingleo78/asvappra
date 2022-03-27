#ifndef ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONRENDERERINTERFACE_H_
#define ACSDKNOTIFICATIONSINTERFACES_NOTIFICATIONRENDERERINTERFACE_H_

#include <iostream>
#include <memory>
#include <string>
#include <util/MediaType.h>
#include "NotificationRendererObserverInterface.h"

namespace alexaClientSDK {
    namespace acsdkNotificationsInterfaces {
        using namespace std;
        using namespace avsCommon::utils;
        class NotificationRendererInterface {
        public:
            virtual ~NotificationRendererInterface() = default;
            virtual void addObserver(shared_ptr<NotificationRendererObserverInterface> observer);
            virtual void removeObserver(shared_ptr<NotificationRendererObserverInterface> observer);
            virtual bool renderNotification(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url);
            virtual bool cancelNotificationRendering();
        };
    }
}
#endif