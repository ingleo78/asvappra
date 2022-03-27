#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_READER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_READER_H_

#include <cstdint>
#include <cstddef>
#include <vector>
#include <mutex>
#include <limits>
#include <cstring>
#include <logger/LoggerUtils.h>
#include "PlatformDefinitions.h"
#include "SharedDataStream.h"
#include "ReaderPolicy.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                template <typename T> class SharedDataStream<T>::Reader {
                    public:
                        using Policy = ReaderPolicy;
                        enum class Reference {
                            AFTER_READER,
                            BEFORE_READER,
                            BEFORE_WRITER,
                            ABSOLUTE
                        };
                        struct Error {
                            enum {
                                CLOSED = 0,
                                OVERRUN = -1,
                                WOULDBLOCK = -2,
                                TIMEDOUT = -3,
                                INVALID = -4
                            };
                        };
                        Reader(Policy policy, std::shared_ptr<BufferLayout> bufferLayout, uint8_t id);
                        ~Reader();
                        ssize_t read(void* buf, size_t nWords, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
                        bool seek(Index offset, Reference reference = Reference::ABSOLUTE);
                        Index tell(Reference reference = Reference::ABSOLUTE) const;
                        void close(Index offset = 0, Reference reference = Reference::AFTER_READER);
                        size_t getId() const;
                        size_t getWordSize() const;
                        static std::string errorToString(Error error);
                    private:
                        static const std::string TAG;
                        Policy m_policy;
                        std::shared_ptr<BufferLayout> m_bufferLayout;
                        uint8_t m_id;
                        AtomicIndex* m_readerCursor;
                        AtomicIndex* m_readerCloseIndex;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_READER_H_
