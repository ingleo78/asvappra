#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTMANAGERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTMANAGERINTERFACE_H_

#include <chrono>
#include <string>
#include <memory>
#include "Attachment.h"
#include "AttachmentReader.h"
#include "AttachmentWriter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace utils;
                using namespace sds;
                class AttachmentManagerInterface {
                public:
                    virtual ~AttachmentManagerInterface() = default;
                    virtual string generateAttachmentId(const string& contextId, const string& contentId) const;
                    virtual bool setAttachmentTimeoutMinutes(minutes timeoutMinutes);
                    virtual unique_ptr<AttachmentWriter> createWriter(const string& attachmentId, WriterPolicy policy = WriterPolicy::ALL_OR_NOTHING);
                    virtual unique_ptr<AttachmentReader> createReader(const string& attachmentId, ReaderPolicy policy);
                };
            }
        }
    }
}
#endif