#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTREADER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTREADER_H_

#include <chrono>
#include <cstddef>
#include <ostream>
#include <util/sds/ReaderPolicy.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                class AttachmentReader {
                public:
                    enum class ReadStatus {
                        OK,
                        OK_WOULDBLOCK,
                        OK_TIMEDOUT,
                        OK_OVERRUN_RESET,
                        CLOSED,
                        ERROR_OVERRUN,
                        ERROR_BYTES_LESS_THAN_WORD_SIZE,
                        ERROR_INTERNAL
                    };
                    enum class ClosePoint {
                        IMMEDIATELY,
                        AFTER_DRAINING_CURRENT_BUFFER
                    };
                    virtual ~AttachmentReader() = default;
                    virtual size_t read(void* buf, size_t numBytes, ReadStatus* readStatus, milliseconds timeoutMs = milliseconds(0));
                    virtual bool seek(uint64_t offset);
                    virtual uint64_t getNumUnreadBytes();
                    virtual void close(ClosePoint closePoint = ClosePoint::AFTER_DRAINING_CURRENT_BUFFER);
                };
                using ReadStatus = AttachmentReader::ReadStatus;
                inline ostream& operator<<(ostream& stream, const ReadStatus& status) {
                    switch(status) {
                        case ReadStatus::OK: stream << "OK"; break;
                        case ReadStatus::OK_WOULDBLOCK: stream << "OK_WOULDBLOCK"; break;
                        case ReadStatus::OK_TIMEDOUT: stream << "OK_TIMEDOUT"; break;
                        case ReadStatus::OK_OVERRUN_RESET: stream << "OK_OVERRUN_RESET"; break;
                        case ReadStatus::CLOSED: stream << "CLOSED"; break;
                        case ReadStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE: stream << "ERROR_BYTES_LESS_THAN_WORD_SIZE"; break;
                        case ReadStatus::ERROR_OVERRUN: stream << "ERROR_OVERRUN"; break;
                        case ReadStatus::ERROR_INTERNAL: stream << "ERROR_INTERNAL"; break;
                    }
                    return stream;
                }
            }
        }
    }
}
#endif