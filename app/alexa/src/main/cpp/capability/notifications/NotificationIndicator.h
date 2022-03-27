#ifndef ACSDKNOTIFICATIONS_NOTIFICATIONINDICATOR_H_
#define ACSDKNOTIFICATIONS_NOTIFICATIONINDICATOR_H_

#include <string>

namespace alexaClientSDK {
    namespace acsdkNotifications {
        using namespace std;
        struct NotificationIndicator {
            NotificationIndicator();
            NotificationIndicator(bool persistVisualIndicator, bool playAudioIndicator, const string& assetId, const string& url);
            bool persistVisualIndicator;
            bool playAudioIndicator;
            struct Asset {
                string assetId;
                string url;
            } asset;
        };
    }
}
#endif