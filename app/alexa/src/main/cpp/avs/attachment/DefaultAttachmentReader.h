#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_DEFAULTATTACHMENTREADER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_DEFAULTATTACHMENTREADER_H_

#include <chrono>
#include <memory>
#include <stdint.h>
#include <logger/Logger.h>
#include <logger/LogEntry.h>
#include <util/sds/InProcessSDS.h>
#include <util/sds/Reader.h>
#include "AttachmentReader.h"

typedef unsigned int size_t;

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                using namespace utils;
                using namespace logger;
                using namespace sds;
                template <typename SDSType> class DefaultAttachmentReader : public AttachmentReader {
                private:
                    using Reader = Reader<InProcessSDSTraits>;
                public:
                    static unique_ptr<AttachmentReader> create(typename SDSType::Reader::Policy policy, shared_ptr<SDSType> sds,
                                                               typename SDSType::Index offset = 0,
                                                               typename SDSType::Reader::Reference reference = SDSType::Reader::Reference::ABSOLUTE,
                                                               bool resetOnOverrun = false) {
                        auto reader = unique_ptr<DefaultAttachmentReader>(new DefaultAttachmentReader<SDSType>(policy, sds, resetOnOverrun));
                        if (!reader->m_reader) {
                            ACSDK_ERROR(LogEntry(TAG, "createFailed").d("reason", "object not fully created"));
                            return nullptr;
                        }
                        if (!reader->m_reader->seek(offset, reference)) {
                            ACSDK_ERROR(LogEntry(TAG, "ConstructorFailed").d("reason", "seek failed"));
                            return nullptr;
                        }
                        return std::unique_ptr<AttachmentReader>(reader.release());
                    }
                    ~DefaultAttachmentReader();
                    size_t read(void* buf, size_t numBytes, ReadStatus* readStatus, milliseconds timeoutMs = milliseconds(0));
                    void close(ClosePoint closePoint = ClosePoint::AFTER_DRAINING_CURRENT_BUFFER) override;
                    bool seek(uint64_t offset) override;
                    uint64_t getNumUnreadBytes() override;
                private:
                    DefaultAttachmentReader(Reader::Policy policy, shared_ptr<SDSType> sds, bool resetOnOverrun);
                    static const std::string TAG;
                    shared_ptr<Reader> m_reader;
                    bool m_resetOnOverrun;
                };
                template <typename SDSType> const std::string DefaultAttachmentReader<SDSType>::TAG = "DefaultAttachmentReader";
                template <typename SDSType> DefaultAttachmentReader<SDSType>::~DefaultAttachmentReader() {
                    close();
                }
                template <typename SDSType> size_t DefaultAttachmentReader<SDSType>::read(void* buf, size_t numBytes, AttachmentReader::ReadStatus* readStatus, milliseconds timeoutMs) {
                    if (!readStatus) {
                        ACSDK_ERROR(LogEntry(TAG, "readFailed").d("reason", "read status is nullptr"));
                        return 0;
                    }
                    if (!buf) {
                        ACSDK_ERROR(LogEntry(TAG, "readFailed").d("reason", "buf is nullptr"));
                        *readStatus = ReadStatus::ERROR_INTERNAL;
                        return 0;
                    }
                    if (!m_reader) {
                        ACSDK_INFO(LogEntry(TAG, "readFailed").d("reason", "closed or uninitialized SDS"));
                        *readStatus = ReadStatus::CLOSED;
                        return 0;
                    }
                    if (timeoutMs.count() < 0) {
                        ACSDK_ERROR(LogEntry(TAG, "readFailed").d("reason", "negative timeout"));
                        *readStatus = ReadStatus::ERROR_INTERNAL;
                        return 0;
                    }
                    *readStatus = ReadStatus::OK;
                    if (0 == numBytes) return 0;
                    const auto wordSize = m_reader->getWordSize();
                    if (numBytes < wordSize) {
                        ACSDK_ERROR(LogEntry(TAG, "readFailed").d("reason", "bytes requested smaller than SDS word size"));
                        *readStatus = ReadStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE;
                        return 0;
                    }
                    size_t bytesRead = 0;
                    const auto numWords = numBytes / wordSize;
                    const auto readResult = m_reader->read(buf, numWords, timeoutMs);
                    if (readResult < 0) {
                        switch(readResult) {
                            case Reader::Error::OVERRUN:
                                if (m_resetOnOverrun) {
                                    *readStatus = ReadStatus::OK_OVERRUN_RESET;
                                    ACSDK_DEBUG5(utils::logger::LogEntry(TAG, "readFailed").d("reason", "memory overrun by writer"));
                                    m_reader->seek(0, Reader::Reference::BEFORE_WRITER);
                                } else {
                                    *readStatus = ReadStatus::ERROR_OVERRUN;
                                    ACSDK_ERROR(LogEntry(TAG, "readFailed").d("reason", "memory overrun by writer"));
                                    close();
                                }
                                break;
                            case Reader::Error::WOULDBLOCK: *readStatus = ReadStatus::OK_WOULDBLOCK; break;
                            case Reader::Error::TIMEDOUT: *readStatus = ReadStatus::OK_TIMEDOUT; break;
                        }
                        if (ReadStatus::OK == *readStatus) {
                            ACSDK_ERROR(LogEntry(TAG, "readFailed").d("reason", "unhandled error code").d("code", readResult));
                            *readStatus = ReadStatus::ERROR_INTERNAL;
                        }
                    } else if (0 == readResult) {
                        *readStatus = ReadStatus::CLOSED;
                        ACSDK_DEBUG0(LogEntry(TAG, "readFailed").d("reason", "SDS is closed"));
                    } else bytesRead = static_cast<size_t>(readResult) * wordSize;
                    return bytesRead;
                }
                template <typename SDSType> void DefaultAttachmentReader<SDSType>::close(AttachmentReader::ClosePoint closePoint) {
                    if (m_reader) {
                        switch(closePoint) {
                            case ClosePoint::IMMEDIATELY: m_reader->close(); return;
                            case ClosePoint::AFTER_DRAINING_CURRENT_BUFFER: m_reader->close(0, Reader::Reference::BEFORE_WRITER); return;
                        }
                    }
                }
                template <typename SDSType> bool DefaultAttachmentReader<SDSType>::seek(uint64_t offset) {
                    if (m_reader) return m_reader->seek(offset);
                    return false;
                }
                template <typename SDSType> uint64_t DefaultAttachmentReader<SDSType>::getNumUnreadBytes() {
                    if (m_reader) return m_reader->tell(Reader::Reference::BEFORE_WRITER);
                    ACSDK_ERROR(LogEntry(TAG, "getNumUnreadBytesFailed").d("reason", "noReader"));
                    return 0;
                }
                template <typename SDSType> DefaultAttachmentReader<SDSType>::DefaultAttachmentReader(Reader::Policy policy, std::shared_ptr<SDSType> sds, bool resetOnOverrun) :
                                                                                    m_resetOnOverrun{resetOnOverrun} {
                    if (!sds) {
                        ACSDK_ERROR(LogEntry(TAG, "ConstructorFailed").d("reason", "SDS parameter is nullptr"));
                        return;
                    }
                    m_reader = sds->createReader(policy);
                    if (!m_reader) {
                        ACSDK_ERROR(LogEntry(TAG, "ConstructorFailed").d("reason", "could not create an SDS reader"));
                        return;
                    }
                }
            }
        }
    }
}
#endif