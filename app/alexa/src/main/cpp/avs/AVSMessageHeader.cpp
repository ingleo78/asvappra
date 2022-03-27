#include <sstream>
#include <json/JSONGenerator.h>
#include <uuid_generation/UUIDGeneration.h>
#include "AVSMessageHeader.h"
#include "EventBuilder.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace constants;
            using namespace utils;
            using namespace json;
            static const string MESSAGE_ID_KEY_STRING = "messageId";
            static const string DIALOG_REQUEST_ID_KEY_STRING = "dialogRequestId";
            static const string EVENT_CORRELATION_KEY_STRING = "eventCorrelationToken";
            static const string CORRELATION_TOKEN_KEY_STRING = "correlationToken";
            static const string PAYLOAD_VERSION_KEY_STRING = "payloadVersion";
            static const string INSTANCE_KEY_STRING = "instance";
            AVSMessageHeader::AVSMessageHeader(const string& avsNamespace, const string& avsName, const string& avsMessageId, const string& avsDialogRequestId,
                                               const string& correlationToken, const string& eventCorrelationToken, const string& payloadVersion,
                                               const string& instance) : m_namespace{avsNamespace}, m_name{avsName}, m_messageId{avsMessageId},
                                               m_dialogRequestId{avsDialogRequestId}, m_correlationToken{correlationToken}, m_eventCorrelationToken{eventCorrelationToken},
                                               m_payloadVersion{payloadVersion}, m_instance{instance} {}
            AVSMessageHeader AVSMessageHeader::createAVSEventHeader(const string& avsNamespace, const string& avsName, const string& avsDialogRequestId,
                                                                    const string& correlationToken, const string& payloadVersion, const string& instance) {
                auto& newId = uuidGeneration::generateUUID();
                return AVSMessageHeader(avsNamespace, avsName, newId, avsDialogRequestId, correlationToken, newId, payloadVersion, instance);
            }
            string AVSMessageHeader::getNamespace() const { return m_namespace; }
            string AVSMessageHeader::getName() const { return m_name; }
            string AVSMessageHeader::getMessageId() const { return m_messageId; }
            string AVSMessageHeader::getDialogRequestId() const { return m_dialogRequestId; }
            string AVSMessageHeader::getCorrelationToken() const { return m_correlationToken; }
            string AVSMessageHeader::getEventCorrelationToken() const { return m_eventCorrelationToken; }
            string AVSMessageHeader::getAsString() const {
                stringstream stream;
                stream << "namespace:" << m_namespace;
                stream << ",name:" << m_name;
                stream << ",messageId:" << m_messageId;
                stream << ",dialogRequestId:" << m_dialogRequestId;
                stream << ",correlationToken:" << m_correlationToken;
                stream << ",eventCorrelationToken:" << m_eventCorrelationToken;
                stream << ",payloadVersion:" << m_payloadVersion;
                stream << ",instance:" << m_instance;
                return stream.str();
            }
            string AVSMessageHeader::getPayloadVersion() const { return m_payloadVersion; }
            string AVSMessageHeader::getInstance() const { return m_instance; }
            string AVSMessageHeader::toJson() const {
                JsonGenerator jsonGenerator;
                jsonGenerator.addMember(NAMESPACE_KEY_STRING, m_namespace);
                jsonGenerator.addMember(NAME_KEY_STRING, m_name);
                jsonGenerator.addMember(MESSAGE_ID_KEY_STRING, m_messageId);
                if (!m_dialogRequestId.empty()) jsonGenerator.addMember(DIALOG_REQUEST_ID_KEY_STRING, m_dialogRequestId);
                if (!m_correlationToken.empty()) jsonGenerator.addMember(CORRELATION_TOKEN_KEY_STRING, m_correlationToken);
                if (!m_eventCorrelationToken.empty()) jsonGenerator.addMember(EVENT_CORRELATION_KEY_STRING, m_eventCorrelationToken);
                if (!m_payloadVersion.empty()) jsonGenerator.addMember(PAYLOAD_VERSION_KEY_STRING, m_payloadVersion);
                if (!m_instance.empty()) jsonGenerator.addMember(INSTANCE_KEY_STRING, m_instance);
                return jsonGenerator.toString();
            }
        }
    }
}