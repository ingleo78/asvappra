#include "util/sds/Reader.h"
#include "DefaultAttachmentReader.h"
#include "InProcessAttachmentReader.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                using Reader = utils::sds::Reader<utils::sds::InProcessSDS>;
                unique_ptr<InProcessAttachmentReader> InProcessAttachmentReader::create(Reader::Policy policy, shared_ptr<SDSType> sds, SDSTypeIndex offset,
                                                                                        Reader::Reference reference , bool resetOnOverrun) {
                    auto readerImpl = DefaultAttachmentReader<SDSType>::create(policy, move(sds), offset, reference, resetOnOverrun);
                    if (!readerImpl) return nullptr;
                    return unique_ptr<InProcessAttachmentReader>(new InProcessAttachmentReader(move(readerImpl)));
                }
                InProcessAttachmentReader::InProcessAttachmentReader(unique_ptr<AttachmentReader> reader) : m_delegate(move(reader)) { }
                std::size_t InProcessAttachmentReader::read(void* buf, size_t numBytes, ReadStatus* readStatus, milliseconds timeoutMs) {
                    return m_delegate->read(buf, numBytes, readStatus, timeoutMs);
                }
                void InProcessAttachmentReader::close(ClosePoint closePoint) { m_delegate->close(closePoint); }
                bool InProcessAttachmentReader::seek(uint64_t offset) { return m_delegate->seek(offset); }
                uint64_t InProcessAttachmentReader::getNumUnreadBytes() { return m_delegate->getNumUnreadBytes(); }
            }
        }
    }
}