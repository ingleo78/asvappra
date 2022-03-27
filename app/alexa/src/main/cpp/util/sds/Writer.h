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
                using namespace std;
                using namespace logger;
                using namespace chrono;
                template <typename T> class Writer {
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
                    Writer(Policy policy, shared_ptr<BufferLayout> bufferLayout);
                    ~Writer();
                    ssize_t write(const void* buf, size_t nWords, milliseconds timeout = milliseconds(0));
                    Index tell() const;
                    void close();
                    size_t getWordSize() const;
                    static string errorToString(Error error);
                private:
                    static const string TAG;
                    Policy m_policy;
                    shared_ptr<BufferLayout> m_bufferLayout;
                    bool m_closed;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_WRITER_H_
