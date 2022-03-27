#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_NOTIFICATIONSAUDIOFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_NOTIFICATIONSAUDIOFACTORYINTERFACE_H_

#include <istream>
#include <functional>
#include <memory>
#include <utility>
#include <util/MediaType.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                using namespace std;
                using namespace utils;
                class NotificationsAudioFactoryInterface {
                public:
                    virtual ~NotificationsAudioFactoryInterface() = default;
                    virtual function<pair<unique_ptr<istream>, const MediaType>()> notificationDefault() const;
                };
            }
        }
    }
}
#endif