#include <cstring>
#include "logger/LogEntryBuffer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                LogEntryBuffer::LogEntryBuffer() : m_base(m_smallBuffer) {
                    auto end = m_base + ACSDK_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE - 1;
                    setg(m_base, m_base, end);
                    setp(m_base, end);
                }
                LogEntryBuffer::int_type LogEntryBuffer::overflow(int_type ch) {
                    if (traits_type::eof() == ch) return traits_type::eof();
                    auto size = pptr() - m_base;
                    if (!m_largeBuffer) {
                        m_largeBuffer.reset(new std::vector<char>(ACSDK_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE * 2));
                        memcpy(m_largeBuffer->data(), m_base, size);
                    } else m_largeBuffer->resize(m_largeBuffer->size() * 2);
                    auto newBase = m_largeBuffer->data();
                    auto delta = newBase - m_base;
                    auto newEnd = newBase + m_largeBuffer->size() - 1;
                    setp(newBase + size, newEnd);
                    setg(newBase, gptr() + delta, newEnd);
                    m_base = newBase;
                    *pptr() = ch;
                    pbump(1);
                    return ch;
                }
                const char* LogEntryBuffer::c_str() const {
                    *pptr() = 0;
                    return m_base;
                }
            }
        }
    }
}
