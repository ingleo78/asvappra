#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_AUDIOFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_AUDIOFACTORYINTERFACE_H_

#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/memory>
#include "/media/draico/Informacion/Proyectos_de_android/AmazingGPS/alexa/src/main/jni/sdkinterfaces/Audio/AlertsAudioFactoryInterface.h"
#include "/media/draico/Informacion/Proyectos_de_android/AmazingGPS/alexa/src/main/jni/sdkinterfaces/Audio/CommunicationsAudioFactoryInterface.h"
#include "/media/draico/Informacion/Proyectos_de_android/AmazingGPS/alexa/src/main/jni/sdkinterfaces/Audio/NotificationsAudioFactoryInterface.h"
#include "/media/draico/Informacion/Proyectos_de_android/AmazingGPS/alexa/src/main/jni/sdkinterfaces/Audio/SystemSoundAudioFactoryInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                class AudioFactoryInterface {
                public:
                    virtual ~AudioFactoryInterface() = default;
                    virtual std::shared_ptr<AlertsAudioFactoryInterface> alerts() const = 0;
                    virtual std::shared_ptr<NotificationsAudioFactoryInterface> notifications() const = 0;
                    virtual std::shared_ptr<CommunicationsAudioFactoryInterface> communications() const = 0;
                    virtual std::shared_ptr<SystemSoundAudioFactoryInterface> systemSounds() const = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_AUDIOFACTORYINTERFACE_H_
