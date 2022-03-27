#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTWRITER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTWRITER_H_

#include <chrono>
#include <cstddef>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                class AttachmentWriter {
                public:
                    enum class WriteStatus {
                        OK,
                        CLOSED,
                        OK_BUFFER_FULL,
                        ERROR_BYTES_LESS_THAN_WORD_SIZE,
                        ERROR_INTERNAL,
                        TIMEDOUT
                    };
                    virtual ~AttachmentWriter() = default;
                    virtual size_t write(const void* buf, size_t numBytes, WriteStatus* writeStatus, milliseconds timeout = milliseconds(0));
                    virtual void close();
                };
            }
        }
    }
}
#endif