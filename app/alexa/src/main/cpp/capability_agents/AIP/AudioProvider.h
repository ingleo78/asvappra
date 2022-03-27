#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_AUDIOPROVIDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_AUDIOPROVIDER_H_

#include <avs/AudioInputStream.h>
#include <util/AudioFormat.h>
#include "ASRProfile.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace aip {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace utils;
            struct AudioProvider {
                AudioProvider(shared_ptr<AudioInputStream> stream, const AudioFormat& format, ASRProfile profile, bool alwaysReadable, bool canOverride,
                              bool canBeOverridden);
                static const AudioProvider& null();
                operator bool() const;
                shared_ptr<AudioInputStream> stream;
                AudioFormat format;
                ASRProfile profile;
                bool alwaysReadable;
                bool canOverride;
                bool canBeOverridden;
            };
            inline AudioProvider::AudioProvider(shared_ptr<AudioInputStream> stream, const AudioFormat& format, ASRProfile profile, bool alwaysReadable,
                                                bool canOverride, bool canBeOverridden) : stream{stream}, format(format), profile{profile},
                                                alwaysReadable{alwaysReadable}, canOverride{canOverride}, canBeOverridden{canBeOverridden} {}
            inline const AudioProvider& AudioProvider::null() {
                static AudioProvider nullAudioProvider{nullptr,{AudioFormat::Encoding::LPCM,AudioFormat::Endianness::LITTLE,
                                                       0,0,0,true,AudioFormat::Layout::NON_INTERLEAVED},
                                                       ASRProfile::CLOSE_TALK,false,false,false};
                return nullAudioProvider;
            }
            inline AudioProvider::operator bool() const {
                return stream != nullptr;
            }
        }
    }
}
#endif