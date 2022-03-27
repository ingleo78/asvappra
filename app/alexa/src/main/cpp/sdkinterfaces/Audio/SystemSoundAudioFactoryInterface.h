#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_SYSTEMSOUNDAUDIOFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_SYSTEMSOUNDAUDIOFACTORYINTERFACE_H_

#include <functional>
#include <istream>
#include <memory>
#include <utility>
#include <util/MediaType.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                using namespace std;
                using namespace avsCommon::utils;
                class SystemSoundAudioFactoryInterface {
                public:
                    virtual ~SystemSoundAudioFactoryInterface() = default;
                    virtual function<pair<unique_ptr<istream>, const MediaType>()> endSpeechTone() const = 0;
                    virtual function<pair<unique_ptr<istream>, const MediaType>()> wakeWordNotificationTone() const = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_SYSTEMSOUNDAUDIOFACTORYINTERFACE_H_
