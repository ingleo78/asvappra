#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_SHAREDDATASTREAM_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_SDS_SHAREDDATASTREAM_H_

#include <cstdint>
#include <cstddef>
#include <memory>
#include <logger/LoggerUtils.h>
#include "BufferLayout.h"
#include "Reader.h"
#include "Writer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                using namespace std;
                using namespace logger;
                using BufferedLayout = BufferedLayout<T>;
                template <typename T> class SharedDataStream {
                public:
                    using Index = uint64_t;
                    using AtomicIndex = typename T::AtomicIndex;
                    using AtomicBool = typename T::AtomicBool;
                    using Buffer = typename T::Buffer;
                    using Mutex = typename T::Mutex;
                    using ConditionVariable = typename T::ConditionVariable;
                    using Writer = Writer<T>;
                    using Reader = Reader<T>;
                    static size_t calculateBufferSize(size_t nWords, size_t wordSize = 1, size_t maxReaders = 1);
                    static unique_ptr<SharedDataStream> create(shared_ptr<Buffer> buffer, size_t wordSize = 1, size_t maxReaders = 1);
                    static unique_ptr<SharedDataStream> create(shared_ptr<Buffer> buffer, size_t wordSize, size_t maxReaders, size_t maxEphemeralReaders);
                    static unique_ptr<SharedDataStream> open(shared_ptr<Buffer> buffer);
                    SharedDataStream(size_t nWords, size_t wordSize = 1, size_t maxReaders = 1) { *this = create(nWords, wordSize, maxReaders); }
                    size_t getMaxReaders() const;
                    Index getDataSize() const;
                    size_t getWordSize() const;
                    unique_ptr<Writer> createWriter(Writer::Policy policy, bool forceReplacement = false);
                    unique_ptr<Reader> createReader(Reader::Policy policy, bool startWithNewData = false);
                    unique_ptr<Reader> createReader(size_t id, Reader::Policy policy, bool startWithNewData = false, bool forceReplacement = false);
                private:
                    SharedDataStream(shared_ptr<typename T::Buffer> buffer);
                    unique_ptr<Reader> createReaderLocked(size_t id, Reader::Policy policy, bool startWithNewData, bool forceReplacement, unique_lock<Mutex>* lock);
                    static const string TAG;
                    static const int MAX_READER_CREATION_RETRIES;
                    shared_ptr<BufferLayout> m_bufferLayout;
                };
                template <typename T> const int SharedDataStream<T>::MAX_READER_CREATION_RETRIES = 3;
                template <typename T> const string SharedDataStream<T>::TAG = "SharedDataStream";
                template <typename T> size_t SharedDataStream<T>::calculateBufferSize(size_t nWords, size_t wordSize, size_t maxReaders) {
                    if (0 == nWords) {
                        acsdkError(LogEntry(TAG, "calculateBufferSizeFailed").d("reason", "numWordsZero"));
                        return 0;
                    } else if (0 == wordSize) {
                        acsdkError(LogEntry(TAG, "calculateBufferSizeFailed").d("reason", "wordSizeZero"));
                        return 0;
                    }
                    size_t overhead = BufferLayout::calculateDataOffset(wordSize, maxReaders);
                    size_t dataSize = nWords * wordSize;
                    return overhead + dataSize;
                }
                template <typename T> unique_ptr<SharedDataStream<T>> SharedDataStream<T>::create(shared_ptr<Buffer> buffer, size_t wordSize, size_t maxReaders) {
                    return create(buffer, wordSize, maxReaders, maxReaders);
                }
                template <typename T> unique_ptr<SharedDataStream<T>> SharedDataStream<T>::create(shared_ptr<Buffer> buffer, size_t wordSize, size_t maxReaders,
                                                                                size_t maxEphemeralReaders) {
                    size_t expectedSize = calculateBufferSize(1, wordSize, maxReaders);
                    if (0 == expectedSize) return nullptr;
                    else if (nullptr == buffer) {
                        acsdkError(LogEntry(TAG, "createFailed").d("reason", "nullBuffer"));
                        return nullptr;
                    } else if (expectedSize > buffer->size()) {
                        acsdkError(LogEntry(TAG, "createFailed").d("reason", "bufferSizeTooSmall").d("bufferSize", buffer->size())
                                   .d("expectedSize", expectedSize));
                        return nullptr;
                    } else if (maxEphemeralReaders > maxReaders) {
                        acsdkError(LogEntry(TAG, "createFailed").d("reason", "maxEphemeralReaders > maxReaders"));
                        return nullptr;
                    }
                    unique_ptr<SharedDataStream<T>> sds(new SharedDataStream<T>(buffer));
                    if (!sds->m_bufferLayout->init(wordSize, maxReaders, maxEphemeralReaders)) return nullptr;
                    return sds;
                }
                template <typename T> unique_ptr<SharedDataStream<T>> SharedDataStream<T>::open(shared_ptr<Buffer> buffer) {
                    unique_ptr<SharedDataStream<T>> sds(new SharedDataStream<T>(buffer));
                    if (!sds->m_bufferLayout->attach()) return nullptr;
                    else return sds;
                }
                template <typename T> size_t SharedDataStream<T>::getMaxReaders() const {
                    return m_bufferLayout->getHeader()->maxReaders;
                }
                template <typename T> typename SharedDataStream<T>::Index SharedDataStream<T>::getDataSize() const {
                    return m_bufferLayout->getDataSize();
                }
                template <typename T> size_t SharedDataStream<T>::getWordSize() const {
                    return m_bufferLayout->getHeader()->wordSize;
                }
                template <typename T> unique_ptr<Writer> SharedDataStream<T>::createWriter(Writer::Policy policy, bool forceReplacement) {
                    auto header = m_bufferLayout->getHeader();
                    lock_guard<Mutex> lock(header->writerEnableMutex);
                    if (header->isWriterEnabled && !forceReplacement) {
                        acsdkError(LogEntry(TAG, "createWriterFailed").d("reason", "existingWriterAttached").d("forceReplacement", "false"));
                        return nullptr;
                    } else return unique_ptr<Writer>(new Writer(policy, m_bufferLayout));
                }
                template <typename T> unique_ptr<Reader> SharedDataStream<T>::createReader(Reader::Policy policy, bool startWithNewData) {
                    unique_lock<Mutex> lock(m_bufferLayout->getHeader()->readerEnableMutex);
                    for (size_t id = 0; id < m_bufferLayout->getHeader()->maxEphemeralReaders; ++id) {
                        if (!m_bufferLayout->isReaderEnabled(id)) return createReaderLocked(id, policy, startWithNewData, false, &lock);
                    }
                    acsdkError(LogEntry(TAG, "createWriterFailed").d("reason", "noAvailableReaders"));
                    return nullptr;
                }
                template <typename T> unique_ptr<Reader> SharedDataStream<T>::createReader(size_t id, Reader::Policy policy, bool startWithNewData, bool forceReplacement) {
                    unique_lock<Mutex> lock(m_bufferLayout->getHeader()->readerEnableMutex);
                    return createReaderLocked(id, policy, startWithNewData, forceReplacement, &lock);
                }
                template <typename T> SharedDataStream<T>::SharedDataStream(shared_ptr<Buffer> buffer) : m_bufferLayout{make_shared<BufferLayout>(buffer)} {}
                template <typename T> unique_ptr<Reader> SharedDataStream<T>::createReaderLocked(size_t id, Reader::Policy policy, bool startWithNewData, bool forceReplacement,
                                                                               unique_lock<Mutex>* lock) {
                    if (m_bufferLayout->isReaderEnabled(id) && !forceReplacement) {
                        acsdkError(LogEntry(TAG, "createReaderLockedFailed").d("reason", "readerAlreadyAttached").d("readerId", id)
                                   .d("forceReplacement", "false"));
                        return nullptr;
                    } else {
                        auto reader = unique_ptr<Reader>(new Reader(policy, m_bufferLayout, id));
                        lock->unlock();
                        if (startWithNewData) {
                            m_bufferLayout->updateOldestUnconsumedCursor();
                            return reader;
                        }
                        auto headerPtr = m_bufferLayout->getHeader();
                        for (int i = 0; i < MAX_READER_CREATION_RETRIES; i++) {
                            auto offset = m_bufferLayout->getDataSize();
                            auto writeStartCursor = headerPtr->writeStartCursor.load();
                            if (writeStartCursor < offset) offset = writeStartCursor;
                            else {
                                auto writeEndCursor = headerPtr->writeEndCursor.load();
                                if (writeEndCursor < writeStartCursor) {
                                    acsdkError(LogEntry(TAG,"createReaderLockedError").d("reason", "writeCursorBeyondEndCursor?").d("readerId", id));
                                    continue;
                                }
                                auto wordsBeingWritten = writeEndCursor - writeStartCursor;
                                if (offset < wordsBeingWritten) {
                                    acsdkWarn(LogEntry(TAG, "createReaderLockedWarning").d("reason", "detectedWriterOverflow").d("readerId", id));
                                    continue;
                                }
                                offset -= wordsBeingWritten;
                            }
                            if (reader->seek(offset, Reader::Reference::BEFORE_WRITER)) return reader;
                        }
                        acsdkError(LogEntry(TAG, "createReaderLockedFailed").d("reason", "seekRetriesExhausted").d("readerId", id));
                        return nullptr;
                    }
                }
            }
        }
    }
}
#endif