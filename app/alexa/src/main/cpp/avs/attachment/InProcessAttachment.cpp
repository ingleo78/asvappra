#include <string>
#include "InProcessAttachment.h"
#include "memory/Memory.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace alexaClientSDK::avsCommon::utils::memory;
                static constexpr size_t WORD_SIZE = 1;
                InProcessAttachment::InProcessAttachment(const string& id, unique_ptr<SDSType> sds, size_t maxNumReaders) : Attachment(id), m_sds{std::move(sds)},
                                                                                                                            m_maxNumReaders{maxNumReaders} {
                    if (!m_sds) {
                        auto buffSize = SDSType::calculateBufferSize(SDS_BUFFER_DEFAULT_SIZE_IN_BYTES, WORD_SIZE, maxNumReaders);
                        auto buff = make_shared<SDSBufferType>(buffSize);
                        m_sds = SDSType::create(buff, WORD_SIZE, m_maxNumReaders);
                    }
                }
                unique_ptr<AttachmentWriter> InProcessAttachment::createWriter(InProcessAttachmentWriter::Writer::Policy policy) {
                    lock_guard<mutex> lock(m_mutex);
                    if (m_hasCreatedWriter) return nullptr;
                    auto writer = InProcessAttachmentWriter::create(m_sds, policy);
                    if (writer) m_hasCreatedWriter = true;
                    return move(writer);
                }
                unique_ptr<AttachmentReader> InProcessAttachment::createReader(InProcessAttachmentReader::Reader::Policy policy) {
                    lock_guard<mutex> lock(m_mutex);
                    if (m_numReaders > m_maxNumReaders) return nullptr;
                    auto reader = InProcessAttachmentReader::create(policy, m_sds);
                    if (reader) ++m_numReaders;
                    return move(reader);
                }
            }
        }
    }
}