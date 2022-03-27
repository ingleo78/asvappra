#include "NotificationIndicator.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        NotificationIndicator::NotificationIndicator() : persistVisualIndicator{false}, playAudioIndicator{false}, asset{"", ""} {}
        NotificationIndicator::NotificationIndicator(bool persistVisualIndicator, bool playAudioIndicator, const string& assetId,
                                                     const string& url) : persistVisualIndicator{persistVisualIndicator},
                                                     playAudioIndicator{playAudioIndicator}, asset{assetId, url} {}
    }
}