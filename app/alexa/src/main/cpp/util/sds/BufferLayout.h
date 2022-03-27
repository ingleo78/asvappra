#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_BUFFERLAYOUT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_BUFFERLAYOUT_H_

#include <cstdint>
#include <cstddef>
#include <mutex>
#include <string>
#include <vector>
#include <logger/LoggerUtils.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                using namespace std;
                using namespace logger;
                template <typename T> class BufferLayout {
                private:
                    using Index = uint64_t;
                    using AtomicIndex = typename T::AtomicIndex;
                    using AtomicBool = typename T::AtomicBool;
                    using Buffer = typename T::Buffer;
                    using Mutex = typename T::Mutex;
                    using ConditionVariable = typename T::ConditionVariable;
                public:
                    static const uint32_t MAGIC_NUMBER = 0x53445348;
                    static const uint32_t VERSION = 2;
                    BufferLayout(std::shared_ptr<Buffer> buffer);
                    ~BufferLayout();
                    struct Header {
                        uint32_t magic;
                        uint8_t version;
                        uint32_t traitsNameHash;
                        uint16_t wordSize;
                        uint8_t maxReaders;
                        uint8_t maxEphemeralReaders;
                        ConditionVariable dataAvailableConditionVariable;
                        Mutex dataAvailableMutex;
                        ConditionVariable spaceAvailableConditionVariable;
                        Mutex backwardSeekMutex;
                        AtomicBool isWriterEnabled;
                        AtomicBool hasWriterBeenClosed;
                        Mutex writerEnableMutex;
                        AtomicIndex writeStartCursor;
                        AtomicIndex writeEndCursor;
                        AtomicIndex oldestUnconsumedCursor;
                        uint32_t referenceCount;
                        Mutex attachMutex;
                        Mutex readerEnableMutex;
                    };
                    Header* getHeader() const;
                    AtomicBool* getReaderEnabledArray() const;
                    AtomicIndex* getReaderCursorArray() const;
                    AtomicIndex* getReaderCloseIndexArray() const;
                    Index getDataSize() const;
                    uint8_t* getData(Index at = 0) const;
                    bool init(size_t wordSize, size_t maxReaders, size_t maxEphemeralReaders);
                    bool attach();
                    void detach();
                    bool isReaderEnabled(size_t id) const;
                    void enableReaderLocked(size_t id);
                    void disableReaderLocked(size_t id);
                    Index wordsUntilWrap(Index after) const;
                    static size_t calculateDataOffset(size_t wordSize, size_t maxReaders);
                    void updateOldestUnconsumedCursor();
                    void updateOldestUnconsumedCursorLocked();
                private:
                    static uint32_t stableHash(const char* string);
                    static size_t alignSizeTo(size_t size, size_t align);
                    static size_t calculateReaderEnabledArrayOffset();
                    static size_t calculateReaderCursorArrayOffset(size_t maxReaders);
                    static size_t calculateReaderCloseIndexArrayOffset(size_t maxReaders);
                    void calculateAndCacheConstants(size_t wordSize, size_t maxReaders);
                    static const std::string TAG;
                    bool isAttached() const;
                    std::shared_ptr<Buffer> m_buffer;
                    AtomicBool* m_readerEnabledArray;
                    AtomicIndex* m_readerCursorArray;
                    AtomicIndex* m_readerCloseIndexArray;
                    Index m_dataSize;
                    uint8_t* m_data;
                };
            }
        }
    }
}
#endif