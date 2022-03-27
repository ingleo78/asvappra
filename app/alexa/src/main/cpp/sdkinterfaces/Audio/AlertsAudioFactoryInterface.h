#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_ALERTSAUDIOFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_ALERTSAUDIOFACTORYINTERFACE_H_

#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/functional>
#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/istream>
#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/memory>
#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/utility>
#include "/media/draico/Informacion/Proyectos_de_android/AmazingGPS/alexa/src/main/jni/util/MediaType.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                class AlertsAudioFactoryInterface {
                public:
                    virtual ~AlertsAudioFactoryInterface() = default;
                    virtual std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> alarmDefault() const = 0;
                    virtual std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> alarmShort() const = 0;
                    virtual std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> timerDefault() const = 0;
                    virtual std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> timerShort() const = 0;
                    virtual std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> reminderDefault() const = 0;
                    virtual std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> reminderShort() const = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_ALERTSAUDIOFACTORYINTERFACE_H_
