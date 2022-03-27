#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGENTRYBUFFER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGENTRYBUFFER_H_

#include <memory>
#include <streambuf>
#include <vector>

#ifndef ACSDK_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE
    #define ACSDK_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE 128
#endif

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class LogEntryBuffer : public std::streambuf {
                    public:
                        LogEntryBuffer();
                        ~LogEntryBuffer() { free(m_smallBuffer); m_base = NULL; }
                        int_type overflow(int_type ch) override;
                        const char* c_str() const;
                    private:
                        char m_smallBuffer[ACSDK_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE];
                        char* m_base;
                        std::unique_ptr<std::vector<char>> m_largeBuffer;
                };
            }
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGENTRYBUFFER_H_
