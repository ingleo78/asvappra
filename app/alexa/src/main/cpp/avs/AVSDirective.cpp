#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <json/document.h>
#include "AVSDirective.h"
#include "AVSMessageHeader.h"
#include "AVSMessageEndpoint.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace avs;
            using namespace attachment;
            using namespace utils;
            using namespace json;
            using namespace jsonUtils;
            using namespace logger;
            using namespace sds;
            using namespace rapidjson;
            static const string JSON_MESSAGE_DIRECTIVE_KEY = "directive";
            static const string JSON_MESSAGE_HEADER_KEY = "header";
            static const string JSON_MESSAGE_NAMESPACE_KEY = "namespace";
            static const string JSON_MESSAGE_NAME_KEY = "name";
            static const string JSON_MESSAGE_ID_KEY = "messageId";
            static const string JSON_MESSAGE_DIALOG_REQUEST_ID_KEY = "dialogRequestId";
            static const string JSON_MESSAGE_PAYLOAD_KEY = "payload";
            static const string JSON_MESSAGE_INSTANCE_KEY = "instance";
            static const string JSON_MESSAGE_PAYLOAD_VERSION_KEY = "payloadVersion";
            static const string JSON_CORRELATION_TOKEN_KEY = "correlationToken";
            static const string JSON_EVENT_CORRELATION_TOKEN_KEY = "eventCorrelationToken";
            static const string JSON_ENDPOINT_KEY = "endpoint";
            static const string JSON_ENDPOINT_ID_KEY = "endpointId";
            static const string JSON_ENDPOINT_COOKIE_KEY = "cookie";
            static const string TAG("AvsDirective");
            #define LX(event) LogEntry(TAG, event)
            static bool parseDocument(const string& unparsedDirective, Document* document) {
                if (!document) {
                    ACSDK_ERROR(LX("parseDocumentFailed").m("nullptr document"));
                    return false;
                }
                if (!parseJSON(unparsedDirective, document)) {
                    ACSDK_ERROR(LX("parseDocumentFailed").d("uparsedDirective", unparsedDirective));
                    return false;
                }
                return true;
            }
            static shared_ptr<AVSMessageHeader> parseHeader(const Document& document, AVSDirective::ParseStatus* parseStatus) {
                if (!parseStatus) {
                    ACSDK_ERROR(LX("parseHeaderFailed").m("nullptr parseStatus"));
                    return nullptr;
                }
                *parseStatus = AVSDirective::ParseStatus::SUCCESS;
                Value::ConstMemberIterator directiveIt;
                if (!findNode((Value&)document, JSON_MESSAGE_DIRECTIVE_KEY, &directiveIt)) {
                    *parseStatus = AVSDirective::ParseStatus::ERROR_MISSING_DIRECTIVE_KEY;
                    return nullptr;
                }
                Value::ConstMemberIterator headerIt;
                if (!findNode(directiveIt->value, JSON_MESSAGE_HEADER_KEY, &headerIt)) {
                    *parseStatus = AVSDirective::ParseStatus::ERROR_MISSING_HEADER_KEY;
                    return nullptr;
                }
                string avsNamespace;
                if (!retrieveValue(headerIt->value, JSON_MESSAGE_NAMESPACE_KEY, &avsNamespace)) {
                    *parseStatus = AVSDirective::ParseStatus::ERROR_MISSING_NAMESPACE_KEY;
                    return nullptr;
                }
                string avsName;
                if (!retrieveValue(headerIt->value, JSON_MESSAGE_NAME_KEY, &avsName)) {
                    *parseStatus = AVSDirective::ParseStatus::ERROR_MISSING_NAME_KEY;
                    return nullptr;
                }
                string avsMessageId;
                if (!retrieveValue(headerIt->value, JSON_MESSAGE_ID_KEY, &avsMessageId)) {
                    *parseStatus = AVSDirective::ParseStatus::ERROR_MISSING_MESSAGE_ID_KEY;
                    return nullptr;
                }
                string avsDialogRequestId;
                auto it = headerIt->value.FindMember(JSON_MESSAGE_DIALOG_REQUEST_ID_KEY.data());
                if (it != headerIt->value.MemberEnd()) convertToValue(it->value, &avsDialogRequestId);
                string instance;
                if (retrieveValue(headerIt->value, JSON_MESSAGE_INSTANCE_KEY, &instance)) {
                    ACSDK_DEBUG5(LX(__func__).d(JSON_MESSAGE_INSTANCE_KEY, instance));
                }
                string payloadVersion;
                if (retrieveValue(headerIt->value, JSON_MESSAGE_PAYLOAD_VERSION_KEY, &payloadVersion)) {
                    ACSDK_DEBUG5(LX(__func__).d(JSON_MESSAGE_PAYLOAD_VERSION_KEY, payloadVersion));
                }
                string correlationToken;
                if (retrieveValue(headerIt->value, JSON_CORRELATION_TOKEN_KEY, &correlationToken)) {
                    ACSDK_DEBUG5(LX(__func__).d(JSON_CORRELATION_TOKEN_KEY, correlationToken));
                }
                string eventCorrelationToken;
                if (retrieveValue(headerIt->value, JSON_EVENT_CORRELATION_TOKEN_KEY, &eventCorrelationToken)) {
                    ACSDK_DEBUG5(LX(__func__).d(JSON_EVENT_CORRELATION_TOKEN_KEY, eventCorrelationToken));
                }
                return make_shared<AVSMessageHeader>(avsNamespace, avsName, avsMessageId, avsDialogRequestId, correlationToken, eventCorrelationToken, payloadVersion,
                                                     instance);
            }
            static string parsePayload(const Document& document, AVSDirective::ParseStatus* parseStatus) {
                if (!parseStatus) {
                    ACSDK_ERROR(LX("parsePayloadFailed").m("nullptr parseStatus"));
                    return "";
                }
                Value::ConstMemberIterator directiveIt;
                if (!findNode((Value&)document, JSON_MESSAGE_DIRECTIVE_KEY, &directiveIt)) {
                    *parseStatus = AVSDirective::ParseStatus::ERROR_MISSING_DIRECTIVE_KEY;
                    return "";
                }
                string payload;
                if (!retrieveValue(directiveIt->value, JSON_MESSAGE_PAYLOAD_KEY, &payload)) {
                    *parseStatus = AVSDirective::ParseStatus::ERROR_MISSING_PAYLOAD_KEY;
                    return "";
                }
                *parseStatus = AVSDirective::ParseStatus::SUCCESS;
                return payload;
            }
            static Optional<AVSMessageEndpoint> parseEndpoint(const Document& document) {
                Value::ConstMemberIterator directiveIt;
                if (!findNode((Value&)document, JSON_MESSAGE_DIRECTIVE_KEY, &directiveIt)) {
                    ACSDK_ERROR(LX("parseEndpointFailed").d("reason", "noDirectiveKey"));
                    return Optional<AVSMessageEndpoint>();
                }
                Value::ConstMemberIterator endpointIt;
                if (!findNode(directiveIt->value, JSON_ENDPOINT_KEY, &endpointIt)) {
                    ACSDK_DEBUG0(LX(__func__).m("noEndpoint"));
                    return Optional<AVSMessageEndpoint>();
                }
                string endpointId;
                if (!retrieveValue(endpointIt->value, JSON_ENDPOINT_ID_KEY, &endpointId)) {
                    ACSDK_ERROR(LX(__func__).m("noEndpointId"));
                    return Optional<AVSMessageEndpoint>();
                }
                AVSMessageEndpoint messageEndpoint{endpointId};
                messageEndpoint.cookies = retrieveStringMap(endpointIt->value, JSON_ENDPOINT_COOKIE_KEY);
                ACSDK_DEBUG5(LX(__func__).sensitive("endpointId", endpointId));
                return utils::Optional<AVSMessageEndpoint>(messageEndpoint);
            }
            pair<unique_ptr<AVSDirective>, AVSDirective::ParseStatus> AVSDirective::create(const string& unparsedDirective, shared_ptr<AttachmentManagerInterface> attachmentManager,
                                                                                           const string& attachmentContextId) {
                pair<unique_ptr<AVSDirective>, ParseStatus> result;
                result.second = ParseStatus::SUCCESS;
                Document document;
                if (!parseDocument(unparsedDirective, &document)) {
                    ACSDK_ERROR(LX("createFailed").m("failed to parse JSON"));
                    result.second = ParseStatus::ERROR_INVALID_JSON;
                    return result;
                }
                auto header = parseHeader(document, &(result.second));header->getMessageId();
                if (ParseStatus::SUCCESS != result.second) {
                    ACSDK_ERROR(LX("createFailed").m("failed to parse header"));
                    return result;
                }
                auto payload = parsePayload(document, &(result.second));
                if (ParseStatus::SUCCESS != result.second) {
                    ACSDK_ERROR(LX("createFailed").m("failed to parse payload"));
                    return result;
                }
                auto endpoint = parseEndpoint(document);
                result.first = unique_ptr<AVSDirective>(new AVSDirective(unparsedDirective, header, payload, attachmentManager, attachmentContextId, endpoint));
                return result;
            }
            unique_ptr<AVSDirective> AVSDirective::create(const string& unparsedDirective, const shared_ptr<AVSMessageHeader> avsMessageHeader, const string& payload,
                                                          const shared_ptr<AttachmentManagerInterface> attachmentManager, const string& attachmentContextId,
                                                          const Optional<AVSMessageEndpoint>& endpoint) {
                if (!avsMessageHeader) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageHeader"));
                    return nullptr;
                }
                if (!attachmentManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullAttachmentManager"));
                    return nullptr;
                }
                unique_ptr<AVSDirective> result = unique_ptr<AVSDirective>(new AVSDirective(unparsedDirective, avsMessageHeader, payload, attachmentManager, attachmentContextId, endpoint));
                AVSMessage(avsMessageHeader, payload, endpoint);
                return unique_ptr<AVSDirective>(new AVSDirective(unparsedDirective, avsMessageHeader, payload, attachmentManager, attachmentContextId, endpoint));
            }
            unique_ptr<AttachmentReader> AVSDirective::getAttachmentReader(const string& contentId, ReaderPolicy readerPolicy) const {
                auto attachmentId = m_attachmentManager->generateAttachmentId(m_attachmentContextId, contentId);
                return m_attachmentManager->createReader(attachmentId, readerPolicy);
            }
            AVSDirective::AVSDirective(const string& unparsedDirective, shared_ptr<AVSMessageHeader> avsMessageHeader, const string& payload,
                                       shared_ptr<AttachmentManagerInterface> attachmentManager, const string& attachmentContextId,
                                       const Optional<AVSMessageEndpoint>& endpoint) : AVSMessage{avsMessageHeader, payload, endpoint},
                                       m_unparsedDirective{unparsedDirective}, m_attachmentManager{attachmentManager}, m_attachmentContextId{attachmentContextId} {}
            string AVSDirective::getUnparsedDirective() const { return m_unparsedDirective; }
            string AVSDirective::getAttachmentContextId() const { return m_attachmentContextId; }
        }
    }
}