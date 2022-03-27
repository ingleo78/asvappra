#include "Writer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                using namespace std;
                using namespace logger;
                using namespace chrono;
                template <typename T> const string SharedDataStream<T>::Writer::TAG = "SdsWriter";
                template <typename T> SharedDataStream<T>::Writer::Writer(Policy policy, shared_ptr<BufferLayout> bufferLayout) : m_policy{policy},
                                                        m_bufferLayout{bufferLayout}, m_closed{false} {
                    auto header = m_bufferLayout->getHeader();
                    header->isWriterEnabled = true;
                    header->writeEndCursor = header->writeStartCursor.load();
                }
                template <typename T> SharedDataStream<T>::Writer::~Writer() {
                    close();
                }
                template <typename T> ssize_t SharedDataStream<T>::Writer::write(const void* buf, size_t nWords, milliseconds timeout) {
                    if (nullptr == buf) {
                        acsdkError(LogEntry(TAG, "writeFailed").d("reason", "nullBuffer"));
                        return Error::INVALID;
                    }
                    if (0 == nWords) {
                        acsdkError(LogEntry(TAG, "writeFailed").d("reason", "zeroNumWords"));
                        return Error::INVALID;
                    }
                    auto header = m_bufferLayout->getHeader();
                    if (!header->isWriterEnabled) {
                        acsdkError(LogEntry(TAG, "writeFailed").d("reason", "writerDisabled"));
                        return Error::CLOSED;
                    }
                    auto wordsToCopy = nWords;
                    auto buf8 = static_cast<const uint8_t*>(buf);
                    unique_lock<Mutex> backwardSeekLock(header->backwardSeekMutex, defer_lock);
                    Index writeEnd = header->writeStartCursor + nWords;
                    switch (m_policy) {
                        case Policy::NONBLOCKABLE:
                            if (nWords > m_bufferLayout->getDataSize()) {
                                wordsToCopy = nWords = m_bufferLayout->getDataSize();
                                writeEnd = header->writeStartCursor + nWords;
                            }
                            break;
                        case Policy::ALL_OR_NOTHING:
                            backwardSeekLock.lock();
                            if ((writeEnd >= header->oldestUnconsumedCursor) &&
                                ((writeEnd - header->oldestUnconsumedCursor) > m_bufferLayout->getDataSize())) {
                                return Error::WOULDBLOCK;
                            }
                            break;
                        case Policy::BLOCKING:
                            auto predicate = [this, header] {
                                return (header->writeStartCursor < header->oldestUnconsumedCursor) ||
                                       (header->writeStartCursor - header->oldestUnconsumedCursor) < m_bufferLayout->getDataSize();
                            };
                            backwardSeekLock.lock();
                            if (milliseconds::zero() == timeout) {
                                header->spaceAvailableConditionVariable.wait(backwardSeekLock, predicate);
                            } else if (!header->spaceAvailableConditionVariable.wait_for(backwardSeekLock, timeout, predicate)) {
                                return Error::TIMEDOUT;
                            }
                            auto spaceAvailable = m_bufferLayout->getDataSize();
                            if (header->writeStartCursor >= header->oldestUnconsumedCursor) {
                                auto wordsToOverrun = m_bufferLayout->getDataSize() - (header->writeStartCursor - header->oldestUnconsumedCursor);
                                if (wordsToOverrun < spaceAvailable) {
                                    spaceAvailable = wordsToOverrun;
                                }
                            }
                            if (spaceAvailable < nWords) {
                                wordsToCopy = nWords = spaceAvailable;
                                writeEnd = header->writeStartCursor + nWords;
                            }
                            break;
                    }
                    header->writeEndCursor = writeEnd;
                    if (backwardSeekLock) backwardSeekLock.unlock();
                    if (Policy::ALL_OR_NOTHING == m_policy) {
                        if (wordsToCopy > m_bufferLayout->getDataSize()) {
                            wordsToCopy = m_bufferLayout->getDataSize();
                            buf8 += (nWords - wordsToCopy) * getWordSize();
                        }
                    }
                    size_t beforeWrap = m_bufferLayout->wordsUntilWrap(header->writeStartCursor);
                    if (beforeWrap > wordsToCopy) {
                        beforeWrap = wordsToCopy;
                    }
                    size_t afterWrap = wordsToCopy - beforeWrap;
                    memcpy(m_bufferLayout->getData(header->writeStartCursor), buf8, beforeWrap * getWordSize());
                    if (afterWrap > 0) {
                        memcpy(m_bufferLayout->getData(header->writeStartCursor + beforeWrap), buf8 + beforeWrap * getWordSize(), afterWrap * getWordSize());
                    }
                    unique_lock<Mutex> dataAvailableLock(header->dataAvailableMutex, std::defer_lock);
                    if (Policy::NONBLOCKABLE != m_policy) {
                        dataAvailableLock.lock();
                    }
                    header->writeStartCursor = header->writeEndCursor.load();
                    if (Policy::NONBLOCKABLE != m_policy) {
                        dataAvailableLock.unlock();
                    }
                    header->dataAvailableConditionVariable.notify_all();
                    return nWords;
                }
                template <typename T> typename SharedDataStream<T>::Index SharedDataStream<T>::Writer::tell() const {
                    return m_bufferLayout->getHeader()->writeStartCursor;
                }
                template <typename T> void SharedDataStream<T>::Writer::close() {
                    auto header = m_bufferLayout->getHeader();
                    lock_guard<Mutex> lock(header->writerEnableMutex);
                    if (m_closed) return;
                    if (header->isWriterEnabled) {
                        header->isWriterEnabled = false;
                        unique_lock<Mutex> dataAvailableLock(header->dataAvailableMutex);
                        header->hasWriterBeenClosed = true;
                        header->dataAvailableConditionVariable.notify_all();
                    }
                    m_closed = true;
                }
                template <typename T> size_t SharedDataStream<T>::Writer::getWordSize() const {
                    return m_bufferLayout->getHeader()->wordSize;
                }
                template <typename T> string SharedDataStream<T>::Writer::errorToString(Error error) {
                    switch (error) {
                        case Error::CLOSED: return "CLOSED";
                        case Error::WOULDBLOCK: return "WOULDBLOCK";
                        case Error::INVALID: return "INVALID";
                        case Error::TIMEDOUT: return "TIMEDOUT";
                    }
                    acsdkError(LogEntry(TAG, "errorToStringFailed").d("reason", "invalidError").d("error", error));
                    return "(unknown error " + to_string(error) + ")";
                }
            }
        }
    }
}