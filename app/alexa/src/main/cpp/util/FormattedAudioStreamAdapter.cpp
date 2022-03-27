#include <logger/Logger.h>
#include "bluetooth/FormattedAudioStreamAdapter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                static const std::string TAG("FormattedAudioStreamAdapter");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                FormattedAudioStreamAdapter::FormattedAudioStreamAdapter(const AudioFormat& audioFormat) {
                    m_audioFormat = audioFormat;
                }
                AudioFormat FormattedAudioStreamAdapter::getAudioFormat() const {
                    return m_audioFormat;
                }
                void FormattedAudioStreamAdapter::setListener(std::shared_ptr<FormattedAudioStreamAdapterListener> listener) {
                    std::lock_guard<std::mutex> guard(m_readerFunctionMutex);
                    m_listener = listener;
                }
                size_t FormattedAudioStreamAdapter::send(const unsigned char* buffer, size_t size) {
                    if (!buffer) {
                        ACSDK_ERROR(LX("sendFailed").d("reason", "buffer is null"));
                        return 0;
                    }
                    if (0 == size) {
                        ACSDK_ERROR(LX("sendFailed").d("reason", "size is 0"));
                        return 0;
                    }
                    std::shared_ptr<FormattedAudioStreamAdapterListener> listener;
                    {
                        std::lock_guard<std::mutex> guard(m_readerFunctionMutex);
                        listener = m_listener.lock();
                    }
                    if (listener) {
                        listener->onFormattedAudioStreamAdapterData(m_audioFormat, buffer, size);
                        return size;
                    } else return 0;
                }
            }
        }
    }
}
