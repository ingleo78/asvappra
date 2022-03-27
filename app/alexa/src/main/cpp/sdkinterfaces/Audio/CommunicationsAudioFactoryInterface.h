#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_COMMUNICATIONSAUDIOFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_COMMUNICATIONSAUDIOFACTORYINTERFACE_H_

#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/functional>
#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/istream>
#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/memory>
#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/utility>
#include "/media/draico/Informacion/Proyectos_de_android/AmazingGPS/alexa/src/main/jni/util/MediaType.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                using namespace std;
                using namespace avsCommon::utils;
                class CommunicationsAudioFactoryInterface {
                public:
                    virtual ~CommunicationsAudioFactoryInterface() = default;
                    virtual function<std::pair<unique_ptr<istream>, const MediaType>()> callConnectedRingtone() const = 0;
                    virtual function<std::pair<unique_ptr<istream>, const MediaType>()> callDisconnectedRingtone() const = 0;
                    virtual function<std::pair<unique_ptr<istream>, const MediaType>()> outboundRingtone() const = 0;
                    virtual function<std::pair<unique_ptr<istream>, const MediaType>()> dropInIncoming() const = 0;
                    virtual function<std::pair<unique_ptr<istream>, const MediaType>()> callIncomingRingtone() const = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_COMMUNICATIONSAUDIOFACTORYINTERFACE_H_
