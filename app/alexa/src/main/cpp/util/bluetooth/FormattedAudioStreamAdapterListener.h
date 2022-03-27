#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTERLISTENER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTERLISTENER_H_

#include <util/AudioFormat.h>

typedef unsigned int size_t;

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                class FormattedAudioStreamAdapterListener {
                public:
                    virtual void onFormattedAudioStreamAdapterData(AudioFormat audioFormat, const unsigned char* buffer, size_t size) = 0;
                    virtual ~FormattedAudioStreamAdapterListener() = default;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTERLISTENER_H_
