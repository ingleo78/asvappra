#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_FORMATTEDAUDIOSTREAMADAPTER_H_

#include <memory>
#include <mutex>
#include <util/AudioFormat.h>
#include "FormattedAudioStreamAdapterListener.h"

typedef unsigned int size_t;

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                class FormattedAudioStreamAdapter {
                public:
                    explicit FormattedAudioStreamAdapter(const AudioFormat& audioFormat);
                    AudioFormat getAudioFormat() const;
                    void setListener(std::shared_ptr<FormattedAudioStreamAdapterListener> listener);
                    size_t send(const unsigned char* buffer, size_t size);
                private:
                    AudioFormat m_audioFormat;
                    std::weak_ptr<FormattedAudioStreamAdapterListener> m_listener;
                    std::mutex m_readerFunctionMutex;
                };
            }
        }
    }
}
#endif