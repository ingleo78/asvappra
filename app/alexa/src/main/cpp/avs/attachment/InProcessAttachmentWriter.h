#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_INPROCESSATTACHMENTWRITER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_INPROCESSATTACHMENTWRITER_H_

#include <util/sds/InProcessSDS.h>
#include <util/sds/Writer.h>
#include <util/sds/SharedDataStream.h>
#include "AttachmentWriter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                using namespace utils;
                using namespace sds;
                class InProcessAttachmentWriter : public AttachmentWriter {
                public:
                    using SDSType = SharedDataStream<InProcessSDSTraits>;
                    using Writer = Writer<InProcessSDSTraits>;
                    static unique_ptr<InProcessAttachmentWriter> create(shared_ptr<SDSType> sds, Writer::Policy policy = Writer::Policy::ALL_OR_NOTHING);
                    ~InProcessAttachmentWriter();
                    size_t write(const void* buf, size_t numBytes, WriteStatus* writeStatus, milliseconds timeout = milliseconds(0)) override;
                    void close() override;
                protected:
                    InProcessAttachmentWriter(shared_ptr<SDSType> sds, Writer::Policy policy = Writer::Policy::ALL_OR_NOTHING);
                    shared_ptr<Writer> m_writer;
                };
            }
        }
    }
}
#endif