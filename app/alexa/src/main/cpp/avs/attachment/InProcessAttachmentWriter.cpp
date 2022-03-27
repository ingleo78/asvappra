#include <logger/Logger.h>
#include <util/sds/SharedDataStream.h>
#include <util/sds/InProcessSDS.h>
#include <util/sds/Writer.h>
#include "InProcessAttachmentWriter.h"


namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                using namespace utils;
                using namespace logger;
                using namespace sds;
                using Writer = Writer<InProcessSDSTraits>;
                static const string TAG("InProcessAttachmentWriter");
                #define LX(event) LogEntry(TAG, event)
                unique_ptr<InProcessAttachmentWriter> InProcessAttachmentWriter::create(
                    shared_ptr<SDSType> sds,
                    SDSTypeWriter::Policy policy) {
                    auto writer = unique_ptr<InProcessAttachmentWriter>(new InProcessAttachmentWriter(sds, policy));
                    if (!writer->m_writer) {
                        ACSDK_ERROR(LX("createFailed").d("reason", "could not create instance"));
                        return nullptr;
                    }
                    return writer;
                }
                InProcessAttachmentWriter::InProcessAttachmentWriter(shared_ptr<SharedDataStream<InProcessSDSTraits>> sds, SDSTypeWriter::Policy policy) {
                    if (!sds) {
                        ACSDK_ERROR(LX("constructorFailed").d("reason", "SDS parameter is nullptr"));
                        return;
                    }
                    m_writer = sds->createWriter(policy);
                }
                InProcessAttachmentWriter::~InProcessAttachmentWriter() { close(); }
                size_t InProcessAttachmentWriter::write(
                    const void* buff,
                    size_t numBytes,
                    WriteStatus* writeStatus,
                    milliseconds timeout) {
                    if (!writeStatus) {
                        ACSDK_ERROR(LX("writeFailed").d("reason", "writeStatus is nullptr"));
                        return 0;
                    }
                    if (!m_writer) {
                        ACSDK_ERROR(LX("writeFailed").d("reason", "SDS is closed or uninitialized"));
                        *writeStatus = WriteStatus::CLOSED;
                        ACSDK_ERROR(LX("InProcessAttachmentWriter : SDS is closed!"));
                        return 0;
                    }
                    *writeStatus = WriteStatus::OK;
                    if (0 == numBytes) return 0;
                    auto wordSize = m_writer->getWordSize();
                    if (numBytes < wordSize) {
                        ACSDK_ERROR(LX("writeFailed").d("reason", "number of bytes are smaller than the underlying word size"));
                        *writeStatus = WriteStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE;
                        return 0;
                    }
                    size_t bytesWritten = 0;
                    auto numWords = numBytes / wordSize;
                    auto writeResult = m_writer->write(buff, numWords, timeout);
                    if (writeResult < 0) {
                        switch(writeResult) {
                            case Writer::Error::WOULDBLOCK:
                                *writeStatus = WriteStatus::OK_BUFFER_FULL;
                                break;
                            case Writer::Error::INVALID:
                                ACSDK_ERROR(LX("AttachmentWriter has generated an internal error."));
                                *writeStatus = WriteStatus::ERROR_INTERNAL;
                                break;
                            case Writer::Error::TIMEDOUT:
                                ACSDK_DEBUG9(LX("AttachmentWriter has timed out while attempting a write."));
                                *writeStatus = WriteStatus::TIMEDOUT;
                                break;
                        }
                        if (WriteStatus::OK == *writeStatus) {
                            ACSDK_ERROR(LX("writeFailed").d("reason", "unhandled error code from underlying SDS").d("code", std::to_string(writeResult)));
                            *writeStatus = WriteStatus::ERROR_INTERNAL;
                        }
                    } else if (0 == writeResult) {
                        *writeStatus = WriteStatus::CLOSED;
                        ACSDK_INFO(LX("writeFailed").d("reason", "underlying SDS is closed"));
                        close();
                    } else bytesWritten = static_cast<size_t>(writeResult) * wordSize;
                    return bytesWritten;
                }
                void InProcessAttachmentWriter::close() {
                    if (m_writer) m_writer->close();
                }
            }
        }
    }
}