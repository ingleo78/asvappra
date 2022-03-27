#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_AUDIOFORMAT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_AUDIOFORMAT_H_

#include <ostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            struct AudioFormat {
                enum class Encoding {
                    LPCM,
                    OPUS
                };
                enum class Layout {
                    NON_INTERLEAVED,
                    INTERLEAVED
                };
                enum class Endianness {
                    LITTLE,
                    BIG
                };
                Encoding encoding;
                Endianness endianness;
                unsigned int sampleRateHz;
                unsigned int sampleSizeInBits;
                unsigned int numChannels;
                bool dataSigned;
                Layout layout;
            };
            inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::Encoding& encoding) {
                switch(encoding) {
                    case AudioFormat::Encoding::LPCM: stream << "LPCM"; break;
                    case AudioFormat::Encoding::OPUS: stream << "OPUS"; break;
                }
                return stream;
            }
            inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::Endianness& endianness) {
                switch(endianness) {
                    case AudioFormat::Endianness::LITTLE: stream << "LITTLE"; break;
                    case AudioFormat::Endianness::BIG: stream << "BIG"; break;
                }
                return stream;
            }
        }
    }
}
#endif