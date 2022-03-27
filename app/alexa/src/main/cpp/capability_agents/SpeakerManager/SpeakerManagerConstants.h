#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_SPEAKERMANAGERCONSTANTS_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_SPEAKERMANAGERCONSTANTS_H_

#include <avs/NamespaceAndName.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            const std::string NAMESPACE = "Speaker";
            const avsCommon::avs::NamespaceAndName SET_VOLUME{NAMESPACE, "SetVolume"};
            const avsCommon::avs::NamespaceAndName ADJUST_VOLUME{NAMESPACE, "AdjustVolume"};
            const avsCommon::avs::NamespaceAndName SET_MUTE{NAMESPACE, "SetMute"};
            const avsCommon::avs::NamespaceAndName VOLUME_STATE{NAMESPACE, "VolumeState"};
            const char VOLUME_KEY[] = "volume";
            const char MUTE_KEY[] = "mute";
            const char MUTED_KEY[] = "muted";
            const std::string VOLUME_CHANGED = "VolumeChanged";
            const std::string MUTE_CHANGED = "MuteChanged";
        }
    }
}
#endif