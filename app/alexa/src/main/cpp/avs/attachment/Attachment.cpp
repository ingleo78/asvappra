#include "Attachment.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                Attachment::Attachment(const std::string& attachmentId) : m_id{attachmentId}, m_hasCreatedWriter{false}, m_numReaders{0} {}
                std::string Attachment::getId() const { return m_id; }
                bool Attachment::hasCreatedReader() { return m_numReaders > 0; }
                bool Attachment::hasCreatedWriter() { return m_hasCreatedWriter; }
            }
        }
    }
}