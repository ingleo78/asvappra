#include <map>
#include <json/JSONGenerator.h>
#include <logger/LogEntry.h>
#include <logger/Logger.h>
#include <util/Metrics.h>
#include <uuid_generation/UUIDGeneration.h>
#include "EventBuilder.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils;
            using namespace json;
            using namespace logger;
            using namespace constants;
            static const string TAG("EventBuilder");
            #define LX(event) LogEntry(TAG, event)
            static const string CONTEXT_KEY_STRING = "context";
            static const string COOKIE_KEY_STRING = "cookie";
            static const string DIALOG_REQUEST_ID_KEY_STRING = "dialogRequestId";
            static const string ENDPOINT_KEY_STRING = "endpoint";
            static const string ENDPOINT_ID_KEY_STRING = "endpointId";
            static const string EVENT_KEY_STRING = "event";
            static const string INSTANCE_KEY_STRING = "instance";
            static const string MESSAGE_KEY_STRING = "message";
            static const string MESSAGE_ID_KEY_STRING = "messageId";
            static const string SCOPE_KEY_STRING = "scope";
            static const string SCOPE_TYPE_KEY_STRING = "type";
            static const string SCOPE_TOKEN_KEY_STRING = "token";
            static const string SCOPE_PARTITION_KEY_STRING = "partition";
            static const string SCOPE_USER_ID_KEY_STRING = "userId";
            static const string BEARER_TOKEN_TYPE = "BearerToken";
            static const string BEARER_TOKEN_WITH_PARTITION_TYPE = "BearerTokenWithPartition";
            static const string TYPE_KEY_STRING = "type";
            static string buildHeader(const string& nameSpace, const string& eventName, const string& dialogRequestIdValue, JsonGenerator& jsonGenerator) {
                jsonGenerator.startObject(HEADER_KEY_STRING);
                auto messageId = avsCommon::utils::uuidGeneration::generateUUID();
                jsonGenerator.addMember(NAMESPACE_KEY_STRING, nameSpace);
                jsonGenerator.addMember(NAME_KEY_STRING, eventName);
                jsonGenerator.addMember(MESSAGE_ID_KEY_STRING, messageId);
                if (!dialogRequestIdValue.empty()) jsonGenerator.addMember(DIALOG_REQUEST_ID_KEY_STRING, dialogRequestIdValue);
                jsonGenerator.finishObject();
                return messageId;
            }
            const pair<string, string> buildJsonEventString(const string& nameSpace, const string& eventName, const string& dialogRequestIdValue,
                                                            const string& jsonPayloadValue, const string& jsonContext) {
                const pair<string, string> emptyPair;
                json::JsonGenerator jsonGenerator;
                if (!jsonContext.empty()) {
                    if (!jsonGenerator.addRawJsonMember(CONTEXT_KEY_STRING, jsonContext)) {
                        ACSDK_ERROR(LX("buildJsonEventStringFailed").d("reason", "parseContextFailed").sensitive("context", jsonContext));
                        return emptyPair;
                    }
                }
                jsonGenerator.startObject(EVENT_KEY_STRING);
                auto messageId = buildHeader(nameSpace, eventName, dialogRequestIdValue, jsonGenerator);
                if (eventName == "SpeechStarted" || eventName == "SpeechFinished" || eventName == "Recognize") {
                    ACSDK_METRIC_IDS(TAG, eventName, messageId, dialogRequestIdValue, Metrics::Location::BUILDING_MESSAGE);
                }
                jsonGenerator.addRawJsonMember(PAYLOAD_KEY_STRING, jsonPayloadValue);
                jsonGenerator.finishObject();
                ACSDK_DEBUG(LX("buildJsonEventString").d("messageId", messageId).d("namespace", nameSpace).d("name", eventName));
                auto eventJson = jsonGenerator.toString();
                ACSDK_DEBUG0(LX(__func__).d("event", eventJson));
                return make_pair(messageId, eventJson);
            }
            static void addJsonObjectFromMap(const string& name, const map<std::string_view, string_view>& map, JsonGenerator& jsonGenerator) {
                if (!map.empty()) {
                    jsonGenerator.startObject(name);
                    for (const auto& element : map) jsonGenerator.addMember(element.first.data(), element.second.data());
                    jsonGenerator.finishObject();
                }
            }
            static void addEndpointToJson(const AVSMessageEndpoint& endpoint, JsonGenerator& jsonGenerator) {
                jsonGenerator.startObject(ENDPOINT_KEY_STRING);
                jsonGenerator.addMember(ENDPOINT_ID_KEY_STRING, endpoint.endpointId);
                addJsonObjectFromMap(COOKIE_KEY_STRING, endpoint.cookies, jsonGenerator);
                jsonGenerator.finishObject();
            }
            string buildJsonEventString(const AVSMessageHeader& eventHeader, const Optional<AVSMessageEndpoint>& endpoint, const string& jsonPayloadValue,
                                        const Optional<AVSContext>& context) {
                json::JsonGenerator jsonGenerator;
                jsonGenerator.startObject(EVENT_KEY_STRING);
                {
                    if (endpoint.hasValue()) addEndpointToJson(endpoint.value(), jsonGenerator);
                    jsonGenerator.addRawJsonMember(HEADER_KEY_STRING, eventHeader.toJson());
                    jsonGenerator.addRawJsonMember(PAYLOAD_KEY_STRING, jsonPayloadValue);
                }
                jsonGenerator.finishObject();
                if (context.hasValue()) jsonGenerator.addRawJsonMember(CONTEXT_KEY_STRING, context.value().toJson());
                return jsonGenerator.toString();
            }
        }
    }
}