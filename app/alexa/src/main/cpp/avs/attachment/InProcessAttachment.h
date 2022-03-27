#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_INPROCESSATTACHMENT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_INPROCESSATTACHMENT_H_

#include <util/sds/InProcessSDS.h>
#include "Attachment.h"
#include "InProcessAttachmentReader.h"
#include "InProcessAttachmentWriter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                class InProcessAttachment : public Attachment {
                public:
                    using SDSType = avsCommon::utils::sds::InProcessSDS;
                    using SDSBufferType = avsCommon::utils::sds::InProcessSDSTraits::Buffer;
                    static const int SDS_BUFFER_DEFAULT_SIZE_IN_BYTES = 0x100000;
                    InProcessAttachment(const string& id, unique_ptr<SDSType> sds = nullptr, size_t maxNumReaders = 1);
                    unique_ptr<AttachmentWriter> createWriter(InProcessAttachmentWriter::Writer::Policy policy = InProcessAttachmentWriter::Writer::Policy::ALL_OR_NOTHING) override;
                    unique_ptr<AttachmentReader> createReader(InProcessAttachmentReader::Reader::Policy policy) override;
                private:
                    shared_ptr<SDSType> m_sds;
                    const size_t m_maxNumReaders;
                };
            }
        }
    }
}
#endif