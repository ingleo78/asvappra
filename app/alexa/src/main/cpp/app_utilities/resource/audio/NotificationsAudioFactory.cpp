#include <util/stream/StreamFunctions.h>
#include "data/med_alerts_notification_01.mp3.h"
#include "NotificationsAudioFactory.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace data;
                using namespace avsCommon::utils;
                using namespace stream;
                static pair<unique_ptr<istream>, const MediaType> notificationDefaultFactory() {
                    return make_pair(streamFromData(med_alerts_notification_01_mp3, sizeof(med_alerts_notification_01_mp3)),
                                     MimeTypeToMediaType(med_alerts_notification_01_mp3_mimetype));
                }
                function<pair<unique_ptr<istream>, const MediaType>()> NotificationsAudioFactory::notificationDefault() const {
                    return notificationDefaultFactory;
                }
            }
        }
    }
}