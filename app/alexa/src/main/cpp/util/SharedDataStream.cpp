#include "SharedDataStream.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                template <typename T> const int SharedDataStream<T>::MAX_READER_CREATION_RETRIES = 3;
                template <typename T> const std::string SharedDataStream<T>::TAG = "SharedDataStream";
                template <typename T> size_t SharedDataStream<T>::calculateBufferSize(size_t nWords, size_t wordSize, size_t maxReaders) {
                    if (0 == nWords) {
                        logger::acsdkError(logger::LogEntry(TAG, "calculateBufferSizeFailed").d("reason", "numWordsZero"));
                        return 0;
                    } else if (0 == wordSize) {
                        logger::acsdkError(logger::LogEntry(TAG, "calculateBufferSizeFailed").d("reason", "wordSizeZero"));
                        return 0;
                    }
                    size_t overhead = BufferLayout::calculateDataOffset(wordSize, maxReaders);
                    size_t dataSize = nWords * wordSize;
                    return overhead + dataSize;
                }
                template <typename T> std::unique_ptr<SharedDataStream<T>> SharedDataStream<T>::create(std::shared_ptr<Buffer> buffer, size_t wordSize, size_t maxReaders) {
                    return create(buffer, wordSize, maxReaders, maxReaders);
                }
                template <typename T> std::unique_ptr<SharedDataStream<T>> SharedDataStream<T>::create(std::shared_ptr<Buffer> buffer, size_t wordSize, size_t maxReaders,
                                                                                     size_t maxEphemeralReaders) {
                    size_t expectedSize = calculateBufferSize(1, wordSize, maxReaders);
                    if (0 == expectedSize) return nullptr;
                    else if (nullptr == buffer) {
                        logger::acsdkError(logger::LogEntry(TAG, "createFailed").d("reason", "nullBuffer"));
                        return nullptr;
                    } else if (expectedSize > buffer->size()) {
                        logger::acsdkError(logger::LogEntry(TAG, "createFailed")
                        .d("reason", "bufferSizeTooSmall")
                        .d("bufferSize", buffer->size())
                        .d("expectedSize", expectedSize));
                        return nullptr;
                    } else if (maxEphemeralReaders > maxReaders) {
                        logger::acsdkError(logger::LogEntry(TAG, "createFailed").d("reason", "maxEphemeralReaders > maxReaders"));
                        return nullptr;
                    }
                    std::unique_ptr<SharedDataStream<T>> sds(new SharedDataStream<T>(buffer));
                    if (!sds->m_bufferLayout->init(wordSize, maxReaders, maxEphemeralReaders)) {
                        return nullptr;
                    }
                    return sds;
                }
                template <typename T> std::unique_ptr<SharedDataStream<T>> SharedDataStream<T>::open(std::shared_ptr<Buffer> buffer) {
                    std::unique_ptr<SharedDataStream<T>> sds(new SharedDataStream<T>(buffer));
                    if (!sds->m_bufferLayout->attach()) return nullptr;
                    return sds;
                }
                template <typename T> size_t SharedDataStream<T>::getMaxReaders() const { return m_bufferLayout->getHeader()->maxReaders; }
                template <typename T> typename SharedDataStream<T>::Index SharedDataStream<T>::getDataSize() const { return m_bufferLayout->getDataSize(); }
                template <typename T> size_t SharedDataStream<T>::getWordSize() const { return m_bufferLayout->getHeader()->wordSize; }
                template <typename T> std::unique_ptr<typename SharedDataStream<T>::Writer> SharedDataStream<T>::createWriter(typename Writer::Policy policy, bool forceReplacement) {
                    auto header = m_bufferLayout->getHeader();
                    std::lock_guard<Mutex> lock(header->writerEnableMutex);
                    if (header->isWriterEnabled && !forceReplacement) {
                        logger::acsdkError(logger::LogEntry(TAG, "createWriterFailed")
                                                   .d("reason", "existingWriterAttached")
                                                   .d("forceReplacement", "false"));
                        return nullptr;
                    } else return std::unique_ptr<Writer>(new Writer(policy, m_bufferLayout));
                }
                template <typename T> std::unique_ptr<typename SharedDataStream<T>::Reader> SharedDataStream<T>::createReader(typename Reader::Policy policy, bool startWithNewData) {
                    std::unique_lock<Mutex> lock(m_bufferLayout->getHeader()->readerEnableMutex);
                    for (size_t id = 0; id < m_bufferLayout->getHeader()->maxEphemeralReaders; ++id) {
                        if (!m_bufferLayout->isReaderEnabled(id)) return createReaderLocked(id, policy, startWithNewData, false, &lock);
                    }
                    logger::acsdkError(logger::LogEntry(TAG, "createWriterFailed").d("reason", "noAvailableReaders"));
                    return nullptr;
                }
                template <typename T> std::unique_ptr<typename SharedDataStream<T>::Reader> SharedDataStream<T>::createReader(size_t id, typename Reader::Policy policy,
                                                                                                            bool startWithNewData, bool forceReplacement) {
                    std::unique_lock<Mutex> lock(m_bufferLayout->getHeader()->readerEnableMutex);
                    return createReaderLocked(id, policy, startWithNewData, forceReplacement, &lock);
                }
                template <typename T> SharedDataStream<T>::SharedDataStream(std::shared_ptr<Buffer> buffer) : m_bufferLayout{std::make_shared<BufferLayout>(buffer)} {}
                template <typename T> std::unique_ptr<typename SharedDataStream<T>::Reader> SharedDataStream<T>::createReaderLocked(size_t id, typename Reader::Policy policy,
                                                                                                                  bool startWithNewData, bool forceReplacement,
                                                                                                                  std::unique_lock<Mutex>* lock) {
                    if (m_bufferLayout->isReaderEnabled(id) && !forceReplacement) {
                        logger::acsdkError(logger::LogEntry(TAG, "createReaderLockedFailed")
                                                   .d("reason", "readerAlreadyAttached")
                                                   .d("readerId", id)
                                                   .d("forceReplacement", "false"));
                        return nullptr;
                    } else {
                        auto reader = std::unique_ptr<Reader>(new Reader(policy, m_bufferLayout, id));
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
                                    logger::acsdkError(logger::LogEntry(TAG, "createReaderLockedError")
                                                               .d("reason", "writeCursorBeyondEndCursor?")
                                                               .d("readerId", id));
                                    continue;
                                }
                                auto wordsBeingWritten = writeEndCursor - writeStartCursor;
                                if (offset < wordsBeingWritten) {
                                    logger::acsdkWarn(logger::LogEntry(TAG, "createReaderLockedWarning")
                                                              .d("reason", "detectedWriterOverflow")
                                                              .d("readerId", id));
                                    continue;
                                }
                                offset -= wordsBeingWritten;
                            }
                            if (reader->seek(offset, Reader::Reference::BEFORE_WRITER)) return reader;
                        }
                        logger::acsdkError(logger::LogEntry(TAG, "createReaderLockedFailed")
                                           .d("reason", "seekRetriesExhausted")
                                           .d("readerId", id));
                        return nullptr;
                    }
                }
            }
        }
    }
}
