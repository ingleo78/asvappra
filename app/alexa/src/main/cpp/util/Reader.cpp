#include "Reader.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                template <typename T> const std::string SharedDataStream<T>::Reader::TAG = "SdsReader";
                template <typename T> SharedDataStream<T>::Reader::Reader(Policy policy, std::shared_ptr<BufferLayout> bufferLayout, uint8_t id) : m_policy{policy},
                                                        m_bufferLayout{bufferLayout}, m_id{id}, m_readerCursor{&m_bufferLayout->getReaderCursorArray()[m_id]},
                                                        m_readerCloseIndex{&m_bufferLayout->getReaderCloseIndexArray()[m_id]} {
                    *m_readerCursor = m_bufferLayout->getHeader()->writeStartCursor.load();
                    *m_readerCloseIndex = std::numeric_limits<Index>::max();
                    m_bufferLayout->enableReaderLocked(m_id);
                }
                template <typename T>
                SharedDataStream<T>::Reader::~Reader() {
                    seek(0, Reference::BEFORE_WRITER);
                    std::lock_guard<Mutex> lock(m_bufferLayout->getHeader()->readerEnableMutex);
                    m_bufferLayout->disableReaderLocked(m_id);
                    m_bufferLayout->updateOldestUnconsumedCursor();
                }
                template <typename T> ssize_t SharedDataStream<T>::Reader::read(void* buf, size_t nWords, std::chrono::milliseconds timeout) {
                    if (nullptr == buf) {
                        logger::acsdkError(logger::LogEntry(TAG, "readFailed").d("reason", "nullBuffer"));
                        return Error::INVALID;
                    }
                    if (0 == nWords) {
                        logger::acsdkError(logger::LogEntry(TAG, "readFailed").d("reason", "invalidNumWords").d("numWords", nWords));
                        return Error::INVALID;
                    }
                    auto readerCloseIndex = m_readerCloseIndex->load();
                    if (*m_readerCursor >= readerCloseIndex) return Error::CLOSED;
                    auto header = m_bufferLayout->getHeader();
                    if ((header->writeEndCursor >= *m_readerCursor) &&
                        (header->writeEndCursor - *m_readerCursor) > m_bufferLayout->getDataSize()) {
                        return Error::OVERRUN;
                    }
                    std::unique_lock<Mutex> lock(header->dataAvailableMutex, std::defer_lock);
                    if (Policy::BLOCKING == m_policy) lock.lock();
                    size_t wordsAvailable = tell(Reference::BEFORE_WRITER);
                    if (0 == wordsAvailable) {
                        if (header->writeEndCursor > 0 && !header->isWriterEnabled) return Error::CLOSED;
                        else if (Policy::NONBLOCKING == m_policy) return Error::WOULDBLOCK;
                        else if (Policy::BLOCKING == m_policy) {
                            auto predicate = [this, header] { return header->hasWriterBeenClosed || tell(Reference::BEFORE_WRITER) > 0; };
                            if (std::chrono::milliseconds::zero() == timeout) header->dataAvailableConditionVariable.wait(lock, predicate);
                            else if (!header->dataAvailableConditionVariable.wait_for(lock, timeout, predicate)) return Error::TIMEDOUT;
                        }
                        wordsAvailable = tell(Reference::BEFORE_WRITER);
                        if (0 == wordsAvailable) return Error::CLOSED;
                    }
                    if (Policy::BLOCKING == m_policy) lock.unlock();
                    if (nWords > wordsAvailable) nWords = wordsAvailable;
                    if ((*m_readerCursor + nWords) > readerCloseIndex) nWords = readerCloseIndex - *m_readerCursor;
                    size_t beforeWrap = m_bufferLayout->wordsUntilWrap(*m_readerCursor);
                    if (beforeWrap > nWords) beforeWrap = nWords;
                    size_t afterWrap = nWords - beforeWrap;
                    auto buf8 = static_cast<uint8_t*>(buf);
                    memcpy(buf8, m_bufferLayout->getData(*m_readerCursor), beforeWrap * getWordSize());
                    if (afterWrap > 0) {
                        memcpy(buf8 + (beforeWrap * getWordSize()), m_bufferLayout->getData(*m_readerCursor + beforeWrap), afterWrap * getWordSize());
                    }
                    *m_readerCursor += nWords;
                    bool overrun = ((header->writeEndCursor - *m_readerCursor) > m_bufferLayout->getDataSize());
                    m_bufferLayout->updateOldestUnconsumedCursor();
                    if (overrun) return Error::OVERRUN;
                    return nWords;
                }
                template <typename T> bool SharedDataStream<T>::Reader::seek(Index offset, Reference reference) {
                    auto header = m_bufferLayout->getHeader();
                    auto writeStartCursor = &header->writeStartCursor;
                    auto writeEndCursor = &header->writeEndCursor;
                    Index absolute = std::numeric_limits<Index>::max();
                    switch (reference) {
                        case Reference::AFTER_READER: absolute = *m_readerCursor + offset; break;
                        case Reference::BEFORE_READER:
                            if (offset > *m_readerCursor) {
                                logger::acsdkError(logger::LogEntry(TAG, "seekFailed")
                                                           .d("reason", "seekBeforeStreamStart")
                                                           .d("reference", "BEFORE_READER")
                                                           .d("seekOffset", offset)
                                                           .d("readerCursor", m_readerCursor->load()));
                                return false;
                            }
                            absolute = *m_readerCursor - offset;
                            break;
                        case Reference::BEFORE_WRITER:
                            if (offset > *writeStartCursor) {
                                logger::acsdkError(logger::LogEntry(TAG, "seekFailed")
                                                           .d("reason", "seekBeforeStreamStart")
                                                           .d("reference", "BEFORE_WRITER")
                                                           .d("seekOffset", offset)
                                                           .d("writeStartCursor", writeStartCursor->load()));
                                return false;
                            }
                            absolute = *writeStartCursor - offset;
                            break;
                        case Reference::ABSOLUTE:
                            absolute = offset;
                    }
                    if (absolute > *m_readerCloseIndex) {
                        logger::acsdkError(logger::LogEntry(TAG, "seekFailed")
                                                   .d("reason", "seekBeyondCloseIndex")
                                                   .d("position", absolute)
                                                   .d("readerCloseIndex", m_readerCloseIndex->load()));
                        return false;
                    }
                    bool backward = absolute < *m_readerCursor;
                    std::unique_lock<Mutex> lock(header->backwardSeekMutex, std::defer_lock);
                    if (backward) lock.lock();
                    if (*writeEndCursor >= absolute && *writeEndCursor - absolute > m_bufferLayout->getDataSize()) {
                        logger::acsdkError(logger::LogEntry(TAG, "seekFailed").d("reason", "seekOverwrittenData"));
                        return false;
                    }
                    *m_readerCursor = absolute;
                    if (backward) {
                        header->oldestUnconsumedCursor = 0;
                        m_bufferLayout->updateOldestUnconsumedCursorLocked();
                        lock.unlock();
                    } else m_bufferLayout->updateOldestUnconsumedCursor();
                    return true;
                }
                template <typename T> typename SharedDataStream<T>::Index SharedDataStream<T>::Reader::tell(Reference reference) const {
                    auto writeStartCursor = &m_bufferLayout->getHeader()->writeStartCursor;
                    switch (reference) {
                        case Reference::AFTER_READER: return 0;
                        case Reference::BEFORE_READER: return 0;
                        case Reference::BEFORE_WRITER: return (*writeStartCursor >= *m_readerCursor) ? *writeStartCursor - *m_readerCursor : 0;
                        case Reference::ABSOLUTE: return *m_readerCursor;
                    }
                    logger::acsdkError(logger::LogEntry(TAG, "tellFailed").d("reason", "invalidReference"));
                    return std::numeric_limits<Index>::max();
                }
                template <typename T> void SharedDataStream<T>::Reader::close(Index offset, Reference reference) {
                    auto writeStartCursor = &m_bufferLayout->getHeader()->writeStartCursor;
                    Index absolute = 0;
                    bool validReference = false;
                    switch (reference) {
                        case Reference::AFTER_READER: absolute = *m_readerCursor + offset; validReference = true; break;
                        case Reference::BEFORE_READER: absolute = *m_readerCursor; validReference = true; break;
                        case Reference::BEFORE_WRITER:
                            if (*writeStartCursor < offset) {
                                logger::acsdkError(logger::LogEntry(TAG, "closeFailed")
                                                           .d("reason", "invalidIndex")
                                                           .d("reference", "BEFORE_WRITER")
                                                           .d("offset", offset)
                                                           .d("writeStartCursor", writeStartCursor->load()));
                            } else absolute = *writeStartCursor - offset;
                            validReference = true;
                            break;
                        case Reference::ABSOLUTE: absolute = offset; validReference = true; break;
                    }
                    if (!validReference) logger::acsdkError(logger::LogEntry(TAG, "closeFailed").d("reason", "invalidReference"));
                    *m_readerCloseIndex = absolute;
                }
                template <typename T> size_t SharedDataStream<T>::Reader::getId() const { return m_id; }
                template <typename T> size_t SharedDataStream<T>::Reader::getWordSize() const { return m_bufferLayout->getHeader()->wordSize; }
                template <typename T> std::string SharedDataStream<T>::Reader::errorToString(Error error) {
                    switch (error) {
                        case Error::CLOSED: return "CLOSED";
                        case Error::OVERRUN: return "OVERRUN";
                        case Error::WOULDBLOCK: return "WOULDBLOCK";
                        case Error::TIMEDOUT: return "TIMEDOUT";
                        case Error::INVALID: return "INVALID";
                    }
                    logger::acsdkError(logger::LogEntry(TAG, "errorToStringFailed").d("reason", "invalidError").d("error", error));
                    return "(unknown error " + to_string(error) + ")";
                }
            }
        }
    }
}