#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_BLOCKINGPOLICY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_BLOCKINGPOLICY_H_

#include <bitset>
#include <iostream>
#include <util/PlatformDefinitions.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            class BlockingPolicy {
            public:
                struct Medium {
                    enum MediumEnum {
                        AUDIO,
                        VISUAL,
                        COUNT
                    };
                };
                using Mediums = std::bitset<Medium::COUNT>;
                static const avscommon_EXPORT Mediums MEDIUM_AUDIO;
                static const avscommon_EXPORT Mediums MEDIUM_VISUAL;
                static const avscommon_EXPORT Mediums MEDIUMS_AUDIO_AND_VISUAL;
                static const avscommon_EXPORT Mediums MEDIUMS_NONE;
                BlockingPolicy(const Mediums& mediums = MEDIUMS_NONE, bool isBlocking = true);
                bool isValid() const;
                bool isBlocking() const;
                Mediums getMediums() const;
            private:
                Mediums m_mediums;
                bool m_isBlocking;
            };
            inline std::ostream& operator<<(std::ostream& stream, const BlockingPolicy& policy) {
                stream << " Mediums:";
                auto mediums = policy.getMediums();
                if (BlockingPolicy::MEDIUM_AUDIO == mediums) stream << "MEDIUM_AUDIO";
                else if (BlockingPolicy::MEDIUM_VISUAL == mediums) stream << "MEDIUM_VISUAL";
                else if (BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL == mediums) stream << "MEDIUMS_AUDIO_AND_VISUAL";
                else if (BlockingPolicy::MEDIUMS_NONE == mediums) stream << "MEDIUMS_NONE";
                else stream << "Unknown";
                stream << policy.getMediums() << " .isBlocking:" << (policy.isBlocking() ? "True" : "False");
                return stream;
            }
            bool operator==(const BlockingPolicy& lhs, const BlockingPolicy& rhs);
            bool operator!=(const BlockingPolicy& lhs, const BlockingPolicy& rhs);
        }
    }
}
#endif