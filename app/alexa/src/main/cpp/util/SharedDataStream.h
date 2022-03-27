#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_SHAREDDATASTREAM_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_SHAREDDATASTREAM_H_

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <logger/LoggerUtils.h>
#include "BufferLayout.h"
#include "Reader.h"
#include "Writer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                template <typename T> class SharedDataStream {
                    private:
                        class BufferLayout;
                    public:
                        using Index = uint64_t;
                        using AtomicIndex = typename T::AtomicIndex;
                        using AtomicBool = typename T::AtomicBool;
                        using Buffer = typename T::Buffer;
                        using Mutex = typename T::Mutex;
                        using ConditionVariable = typename T::ConditionVariable;
                        class Reader;
                        class Writer;
                        static size_t calculateBufferSize(size_t nWords, size_t wordSize = 1, size_t maxReaders = 1);
                        static std::unique_ptr<SharedDataStream> create(std::shared_ptr<Buffer> buffer, size_t wordSize = 1, size_t maxReaders = 1);
                        static std::unique_ptr<SharedDataStream> create(std::shared_ptr<Buffer> buffer, size_t wordSize, size_t maxReaders, size_t maxEphemeralReaders);
                        static std::unique_ptr<SharedDataStream> open(std::shared_ptr<Buffer> buffer);
                        size_t getMaxReaders() const;
                        Index getDataSize() const;
                        size_t getWordSize() const;
                        std::unique_ptr<Writer> createWriter(typename Writer::Policy policy, bool forceReplacement = false);
                        std::unique_ptr<Reader> createReader(typename Reader::Policy policy, bool startWithNewData = false);
                        std::unique_ptr<Reader> createReader(size_t id, typename Reader::Policy policy, bool startWithNewData = false, bool forceReplacement = false);
                    private:
                        SharedDataStream(std::shared_ptr<typename T::Buffer> buffer);
                        std::unique_ptr<Reader> createReaderLocked(size_t id, typename Reader::Policy policy, bool startWithNewData, bool forceReplacement,
                                                                   std::unique_lock<Mutex>* lock);
                        static const std::string TAG;
                        static const int MAX_READER_CREATION_RETRIES;
                        std::shared_ptr<BufferLayout> m_bufferLayout;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_SHAREDDATASTREAM_H_
