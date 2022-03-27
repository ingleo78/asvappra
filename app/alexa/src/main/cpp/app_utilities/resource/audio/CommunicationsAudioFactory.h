#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_COMMUNICATIONSAUDIOFACTORY_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_RESOURCES_AUDIO_INCLUDE_AUDIO_COMMUNICATIONSAUDIOFACTORY_H_

#include <sdkinterfaces/Audio/CommunicationsAudioFactoryInterface.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace avsCommon;
                using namespace sdkInterfaces::audio;
                using namespace utils;
                class CommunicationsAudioFactory : public CommunicationsAudioFactoryInterface {
                public:
                    function<pair<unique_ptr<istream>, const MediaType>()> callConnectedRingtone() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> callDisconnectedRingtone() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> outboundRingtone() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> dropInIncoming() const override;
                    function<pair<unique_ptr<istream>, const MediaType>()> callIncomingRingtone() const override;
                };
            }
        }
    }
}
#endif