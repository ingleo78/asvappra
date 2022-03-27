#include "AVSMessage.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils;
            AVSMessage::AVSMessage(shared_ptr<AVSMessageHeader> avsMessageHeader, string payload, const Optional<AVSMessageEndpoint>& endpoint) :
                                   m_header{avsMessageHeader}, m_payload{std::move(payload)}, m_endpoint{endpoint} {}
            string AVSMessage::getNamespace() const { return m_header->getNamespace(); }
            string AVSMessage::getName() const { return m_header->getName(); }
            string AVSMessage::getMessageId() const { return m_header->getMessageId(); }
            string AVSMessage::getCorrelationToken() const { return m_header->getCorrelationToken(); }
            string AVSMessage::getEventCorrelationToken() const { return m_header->getEventCorrelationToken(); }
            string AVSMessage::getPayloadVersion() const { return m_header->getPayloadVersion(); }
            string AVSMessage::getInstance() const { return m_header->getInstance(); }
            string AVSMessage::getDialogRequestId() const { return m_header->getDialogRequestId(); }
            string AVSMessage::getPayload() const { return m_payload; }
            shared_ptr<const AVSMessageHeader> AVSMessage::getHeader() const { return m_header; }
            string AVSMessage::getHeaderAsString() const { return m_header->getAsString(); }
            Optional<AVSMessageEndpoint> AVSMessage::getEndpoint() const { return m_endpoint; }
        }
    }
}