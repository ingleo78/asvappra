#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_WRITER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_WRITER_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <mutex>
#include <vector>
#include <logger/LoggerUtils.h>
#include "SharedDataStream.h"
#include "WriterPolicy.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                template <typename T> class SharedDataStream<T>::Writer {
                    public:
                        using Policy = WriterPolicy;
                        struct Error {
                            enum {
                                CLOSED = 0,
                                WOULDBLOCK = -1,
                                INVALID = -2,
                                TIMEDOUT = -3,
                            };
                        };
                        Writer(Policy policy, std::shared_ptr<BufferLayout> bufferLayout);
                        ~Writer();
                        ssize_t write(const void* buf, size_t nWords, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
                        Index tell() const;
                        void close();
                        size_t getWordSize() const;
                        static std::string errorToString(Error error);
                    private:
                        static const std::string TAG;
                        Policy m_policy;
                        std::shared_ptr<BufferLayout> m_bufferLayout;
                        bool m_closed;
                };
            }
        }
    }
}
#endif