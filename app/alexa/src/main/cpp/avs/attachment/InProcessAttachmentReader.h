#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_INPROCESSATTACHMENTREADER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_INPROCESSATTACHMENTREADER_H_

#include <util/sds/InProcessSDS.h>
#include <util/sds/Reader.h>
#include "AttachmentReader.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                using namespace utils;
                using namespace sds;
                class InProcessAttachmentReader : public AttachmentReader {
                public:
                    using SDSType = InProcessSDS;
                    using SDSTypeIndex = uint64_t;
                    using Reader = Reader<InProcessSDSTraits>;
                    using Writer = Writer<InProcessSDSTraits>;
                    static unique_ptr<InProcessAttachmentReader> create(Reader::Policy policy, shared_ptr<SDSType> sds, SDSTypeIndex offset = 0,
                                                                        Reader::Reference reference = Reader::Reference::ABSOLUTE,
                                                                        bool resetOnOverrun = false);
                    ~InProcessAttachmentReader() = default;
                    size_t read(void* buf, size_t numBytes, ReadStatus* readStatus, milliseconds timeoutMs = milliseconds(0)) override;
                    void close(ClosePoint closePoint = ClosePoint::AFTER_DRAINING_CURRENT_BUFFER) override;
                    bool seek(uint64_t offset) override;
                    uint64_t getNumUnreadBytes() override;
                private:
                    explicit InProcessAttachmentReader(unique_ptr<AttachmentReader> delegate);
                    unique_ptr<AttachmentReader> m_delegate;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_INPROCESSATTACHMENTREADER_H_
