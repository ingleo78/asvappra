#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENT_H_

#include <atomic>
#include <mutex>
#include <string>
#include <util/sds/InProcessSDS.h>
#include <util/sds/ReaderPolicy.h>
#include <util/sds/WriterPolicy.h>
#include "AttachmentReader.h"
#include "AttachmentWriter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                class Attachment {
                public:
                    Attachment(const std::string& attachmentId);
                    virtual ~Attachment() = default;
                    virtual std::unique_ptr<AttachmentWriter> createWriter(utils::sds::WriterPolicy policy = utils::sds::WriterPolicy::ALL_OR_NOTHING);
                    virtual std::unique_ptr<AttachmentReader> createReader(utils::sds::ReaderPolicy policy);
                    std::string getId() const;
                    bool hasCreatedReader();
                    bool hasCreatedWriter();
                protected:
                    const std::string m_id;
                    std::mutex m_mutex;
                    std::atomic<bool> m_hasCreatedWriter;
                    std::atomic<size_t> m_numReaders;
                };
            }
        }
    }
}
#endif