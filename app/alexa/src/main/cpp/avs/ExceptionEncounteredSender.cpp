#include <sdkinterfaces/MessageSenderInterface.h>
#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <logger/Logger.h>
#include <json/JSONUtils.h>
#include <uuid_generation/UUIDGeneration.h>
#include "AVSContext.h"
#include "MessageRequest.h"
#include "EventBuilder.h"
#include "ExceptionEncounteredSender.h"
#include "ExceptionErrorType.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace rapidjson;
            using namespace utils;
            using namespace logger;
            using namespace sdkInterfaces;
            static const string TAG("ExceptionEncountered");
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE = "System";
            static const string EXCEPTION_ENCOUNTERED_EVENT_NAME = "ExceptionEncountered";
            static const char UNPARSED_DIRECTIVE_KEY_STRING[] = "unparsedDirective";
            static const char ERROR_KEY[] = "error";
            static const char ERROR_TYPE_KEY[] = "type";
            static const char ERROR_MESSAGE_KEY[] = "message";
            unique_ptr<ExceptionEncounteredSender> ExceptionEncounteredSender::create(shared_ptr<MessageSenderInterface> messagesender) {
                if (!messagesender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                }
                return unique_ptr<ExceptionEncounteredSender>(new ExceptionEncounteredSender(messagesender));
            }
            void ExceptionEncounteredSender::sendExceptionEncountered(const string& unparsedDirective, ExceptionErrorType error, const string& errorDescription) {
                string contextJson = AVSContext().toJson();
                Document payloadDataDocument(kObjectType);
                Value unparsed_directive_key_string{UNPARSED_DIRECTIVE_KEY_STRING, strlen(UNPARSED_DIRECTIVE_KEY_STRING)};
                Value _unparsedDirective{unparsedDirective.data(), strlen(unparsedDirective.data())};
                payloadDataDocument.AddMember(unparsed_directive_key_string, _unparsedDirective, payloadDataDocument.GetAllocator());
                Document errorDataDocument(kObjectType);
                ostringstream errorStringVal;
                errorStringVal << error;
                Value error_type_key{ERROR_TYPE_KEY, strlen(ERROR_TYPE_KEY)};
                Value _errorStringVal{errorStringVal.str().data(), strlen(errorStringVal.str().data())};
                errorDataDocument.AddMember(error_type_key, _errorStringVal, errorDataDocument.GetAllocator());
                Value messageJson(StringRef(errorDescription.data()));
                Value error_message_key{ERROR_TYPE_KEY, strlen(ERROR_MESSAGE_KEY)};
                errorDataDocument.AddMember(error_message_key, messageJson, errorDataDocument.GetAllocator());
                Value _errorDataDocument{errorDataDocument.GetString(), strlen(errorDataDocument.GetString())};
                payloadDataDocument.AddMember(ERROR_KEY, _errorDataDocument, payloadDataDocument.GetAllocator());
                Writer<StringBuffer> payloadWriter;
                payloadDataDocument.Accept(payloadWriter);
                StringBuffer payloadJson(payloadWriter.getOs());
                string payload = payloadJson.GetString();
                if (payload.empty()) {
                    ACSDK_ERROR(LX("sendExceptionEncounteredFailed").d("reason", "payloadEmpty"));
                    return;
                }
                auto msgIdAndJsonEvent = buildJsonEventString(NAMESPACE, EXCEPTION_ENCOUNTERED_EVENT_NAME, "", payload, contextJson);
                if (msgIdAndJsonEvent.first.empty()) {
                    ACSDK_ERROR(LX("sendExceptionEncounteredFailed").d("reason", "msgIdEmpty"));
                    return;
                }
                if (msgIdAndJsonEvent.second.empty()) {
                    ACSDK_ERROR(LX("sendExceptionEncounteredFailed").d("reason", "JsonEventEmpty"));
                    return;
                }
                shared_ptr<MessageRequest> request = make_shared<MessageRequest>(msgIdAndJsonEvent.second);
                m_messageSender->sendMessage(request);
            }
            ExceptionEncounteredSender::ExceptionEncounteredSender(shared_ptr<MessageSenderInterface> messageSender) : m_messageSender{messageSender} {}
        }
    }
}