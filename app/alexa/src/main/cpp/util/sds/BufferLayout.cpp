#include <util/playlist_parser/IterativePlaylistParserInterface.h>
#include <util/playlist_parser/PlaylistParserInterface.h>
#include "BufferLayout.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace sds {
                using namespace std;
                using namespace logger;
                template <typename T> const string SharedDataStream<T>::BufferLayout::TAG = "SdsBufferLayout";
                template <typename T> SharedDataStream<T>::BufferLayout::BufferLayout(std::shared_ptr<Buffer> buffer) : m_buffer{buffer}, m_readerEnabledArray{nullptr},
                                                                    m_readerCursorArray{nullptr}, m_readerCloseIndexArray{nullptr}, m_dataSize{0},
                                                                    m_data{nullptr} {}
                template <typename T> SharedDataStream<T>::BufferLayout::~BufferLayout() {
                    detach();
                }
                template <typename T> typename SharedDataStream<T>::BufferLayout::Header* SharedDataStream<T>::BufferLayout::getHeader() const {
                    return reinterpret_cast<Header*>(m_buffer->data());
                }
                template <typename T> typename SharedDataStream<T>::AtomicBool* SharedDataStream<T>::BufferLayout::getReaderEnabledArray() const {
                    return m_readerEnabledArray;
                }
                template <typename T> typename SharedDataStream<T>::AtomicIndex* SharedDataStream<T>::BufferLayout::getReaderCursorArray() const {
                    return m_readerCursorArray;
                }
                template <typename T> typename SharedDataStream<T>::AtomicIndex* SharedDataStream<T>::BufferLayout::getReaderCloseIndexArray() const {
                    return m_readerCloseIndexArray;
                }
                template <typename T> typename SharedDataStream<T>::Index SharedDataStream<T>::BufferLayout::getDataSize() const {
                    return m_dataSize;
                }
                template <typename T> uint8_t* SharedDataStream<T>::BufferLayout::getData(Index at) const {
                    return m_data + (at % getDataSize()) * getHeader()->wordSize;
                }
                template <typename FieldType, typename ClassType> auto inline max_field_limit(FieldType(ClassType::*)) -> decltype(numeric_limits<FieldType>::max()) {
                    return numeric_limits<FieldType>::max();
                }
                template <typename T> bool SharedDataStream<T>::BufferLayout::init(size_t wordSize, size_t maxReaders, size_t maxEphemeralReaders) {
                    if (wordSize > max_field_limit(&Header::wordSize)) {
                        acsdkError(LogEntry(TAG, "initFailed").d("reason", "wordSizeTooLarge").d("wordSize", wordSize)
                                           .d("wordSizeLimit", max_field_limit(&Header::wordSize)));
                        return false;
                    }
                    if (maxReaders > max_field_limit(&Header::maxReaders)) {
                        acsdkError(LogEntry(TAG, "initFailed").d("reason", "maxReadersTooLarge").d("maxReaders", maxReaders)
                                           .d("maxReadersLimit", max_field_limit(&Header::maxReaders)));
                        return false;
                    }
                    calculateAndCacheConstants(wordSize, maxReaders);
                    auto header = new (getHeader()) Header;
                    size_t id;
                    for (id = 0; id < maxReaders; ++id) {
                        new (m_readerEnabledArray + id) AtomicBool;
                        new (m_readerCursorArray + id) AtomicIndex;
                        new (m_readerCloseIndexArray + id) AtomicIndex;
                    }
                    header->magic = MAGIC_NUMBER;
                    header->version = VERSION;
                    header->traitsNameHash = stableHash(T::traitsName);
                    header->wordSize = wordSize;
                    header->maxReaders = maxReaders;
                    header->maxEphemeralReaders = maxEphemeralReaders;
                    header->isWriterEnabled = false;
                    header->hasWriterBeenClosed = false;
                    header->writeStartCursor = 0;
                    header->writeEndCursor = 0;
                    header->oldestUnconsumedCursor = 0;
                    header->referenceCount = 1;
                    for (id = 0; id < maxReaders; ++id) {
                        m_readerEnabledArray[id] = false;
                        m_readerCursorArray[id] = 0;
                        m_readerCloseIndexArray[id] = 0;
                    }
                    return true;
                }
                template <typename T> bool SharedDataStream<T>::BufferLayout::attach() {
                    auto header = getHeader();
                    if (header->magic != MAGIC_NUMBER) {
                        acsdkError(LogEntry(TAG, "attachFailed").d("reason", "magicNumberMismatch").d("magicNumber", header->magic)
                                           .d("expectedMagicNumber", std::to_string(MAGIC_NUMBER)));
                        return false;
                    }
                    if (header->version != VERSION) {
                        acsdkError(LogEntry(TAG, "attachFailed").d("reason", "incompatibleVersion").d("version", header->version)
                                           .d("expectedVersion", std::to_string(VERSION)));
                        return false;
                    }
                    if (header->traitsNameHash != stableHash(T::traitsName)) {
                        acsdkError(LogEntry(TAG, "attachFailed").d("reason", "traitsNameHashMismatch").d("hash", header->traitsNameHash)
                                           .d("expectedHash", stableHash(T::traitsName)));
                        return false;
                    }
                    lock_guard<Mutex> lock(header->attachMutex);
                    if (0 == header->referenceCount) {
                        acsdkError(LogEntry(TAG, "attachFailed").d("reason", "zeroUsers"));
                        return false;
                    }
                    if (numeric_limits<decltype(header->referenceCount)>::max() == header->referenceCount) {
                        acsdkError(LogEntry(TAG, "attachFailed").d("reason", "bufferMaxUsersExceeded").d("numUsers", header->referenceCount)
                                           .d("maxNumUsers", std::numeric_limits<decltype(header->referenceCount)>::max()));
                        return false;
                    }
                    ++header->referenceCount;
                    calculateAndCacheConstants(header->wordSize, header->maxReaders);
                    return true;
                }
                template <typename T> void SharedDataStream<T>::BufferLayout::detach() {
                    if (!isAttached()) return;
                    auto header = getHeader();
                    {
                        lock_guard<Mutex> lock(header->attachMutex);
                        --header->referenceCount;
                        if (header->referenceCount > 0) return;
                    }
                    for (size_t id = 0; id < header->maxReaders; ++id) {
                        m_readerCloseIndexArray[id].~AtomicIndex();
                        m_readerCursorArray[id].~AtomicIndex();
                        m_readerEnabledArray[id].~AtomicBool();
                    }
                    header->~Header();
                }
                template <typename T> bool SharedDataStream<T>::BufferLayout::isReaderEnabled(size_t id) const {
                    return m_readerEnabledArray[id];
                }
                template <typename T> void SharedDataStream<T>::BufferLayout::enableReaderLocked(size_t id) {
                    m_readerEnabledArray[id] = true;
                }
                template <typename T> void SharedDataStream<T>::BufferLayout::disableReaderLocked(size_t id) {
                    m_readerEnabledArray[id] = false;
                }
                template <typename T> typename SharedDataStream<T>::Index SharedDataStream<T>::BufferLayout::wordsUntilWrap(Index after) const {
                    return getDataSize() - (after % getDataSize());
                }
                template <typename T> size_t SharedDataStream<T>::BufferLayout::calculateDataOffset(size_t wordSize, size_t maxReaders) {
                    return alignSizeTo(calculateReaderCloseIndexArrayOffset(maxReaders) + (maxReaders * sizeof(AtomicIndex)), wordSize);
                }
                template <typename T> void SharedDataStream<T>::BufferLayout::updateOldestUnconsumedCursor() {
                    lock_guard<Mutex> backwardSeekLock(getHeader()->backwardSeekMutex);
                    updateOldestUnconsumedCursorLocked();
                }
                template <typename T> void SharedDataStream<T>::BufferLayout::updateOldestUnconsumedCursorLocked() {
                    auto header = getHeader();
                    Index oldest = std::numeric_limits<Index>::max();
                    for (size_t id = 0; id < header->maxReaders; ++id) {
                        if (isReaderEnabled(id) && getReaderCursorArray()[id] < oldest) {
                            oldest = getReaderCursorArray()[id];
                        }
                    }
                    if (numeric_limits<Index>::max() == oldest) {
                        oldest = header->writeStartCursor;
                    }
                    if (oldest > header->oldestUnconsumedCursor) {
                        header->oldestUnconsumedCursor = oldest;
                        header->spaceAvailableConditionVariable.notify_all();
                    }
                }
                template <typename T> uint32_t SharedDataStream<T>::BufferLayout::stableHash(const char* string) {
                    uint32_t hashed = 0;
                    size_t pos = 0;
                    while (*string) {
                        hashed ^= *string << ((pos % sizeof(uint32_t)) * 8);
                        ++string;
                        ++pos;
                    }
                    return hashed;
                }
                template <typename T> size_t SharedDataStream<T>::BufferLayout::alignSizeTo(size_t size, size_t align) {
                    if (size) return (((size - 1) / align) + 1) * align;
                    else return 0;
                }
                template <typename T> size_t SharedDataStream<T>::BufferLayout::calculateReaderEnabledArrayOffset() {
                    return alignSizeTo(sizeof(Header), alignof(AtomicBool));
                }
                template <typename T> size_t SharedDataStream<T>::BufferLayout::calculateReaderCursorArrayOffset(size_t maxReaders) {
                    return alignSizeTo(calculateReaderEnabledArrayOffset() + (maxReaders * sizeof(AtomicBool)), alignof(AtomicIndex));
                }
                template <typename T> size_t SharedDataStream<T>::BufferLayout::calculateReaderCloseIndexArrayOffset(size_t maxReaders) {
                    return calculateReaderCursorArrayOffset(maxReaders) + (maxReaders * sizeof(AtomicIndex));
                }
                template <typename T> void SharedDataStream<T>::BufferLayout::calculateAndCacheConstants(size_t wordSize, size_t maxReaders) {
                    auto buffer = reinterpret_cast<uint8_t*>(m_buffer->data());
                    m_readerEnabledArray = reinterpret_cast<AtomicBool*>(buffer + calculateReaderEnabledArrayOffset());
                    m_readerCursorArray = reinterpret_cast<AtomicIndex*>(buffer + calculateReaderCursorArrayOffset(maxReaders));
                    m_readerCloseIndexArray = reinterpret_cast<AtomicIndex*>(buffer + calculateReaderCloseIndexArrayOffset(maxReaders));
                    m_dataSize = (m_buffer->size() - calculateDataOffset(wordSize, maxReaders)) / wordSize;
                    m_data = buffer + calculateDataOffset(wordSize, maxReaders);
                }
                template <typename T> bool SharedDataStream<T>::BufferLayout::isAttached() const {
                    return m_data != nullptr;
                }
            }
        }
    }
}
