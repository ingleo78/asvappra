#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "AuthorizedSender.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace externalMediaPlayer {
            using namespace rapidjson;
            using namespace json::jsonUtils;
            static const string TAG("AuthorizedSender");
            #define LX(event) LogEntry(TAG, event)
            static const string EVENT_KEY = "event";
            static const string HEADER_KEY = "header";
            static const string PAYLOAD_KEY = "payload";
            static const string PLAYER_ID_KEY = "playerId";
            shared_ptr<AuthorizedSender> AuthorizedSender::create(shared_ptr<MessageSenderInterface> messageSender) {
                ACSDK_DEBUG5(LX(__func__));
                if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                }
                return std::shared_ptr<AuthorizedSender>(new AuthorizedSender(messageSender));
            }
            AuthorizedSender::AuthorizedSender(shared_ptr<MessageSenderInterface> messageSender) : m_messageSender{messageSender} {}
            AuthorizedSender::~AuthorizedSender() {}
            void AuthorizedSender::sendMessage(shared_ptr<MessageRequest> request) {
                ACSDK_DEBUG5(LX(__func__));
                Document document;
                ParseResult result = document.Parse(request->getJsonContent().data());
                if (!result) {
                    ACSDK_ERROR(LX("parseMessageFailed").d("reason", "parseFailed").d("error", GetParseError_En(result.Code()))
                        .d("offset", result.Offset()));
                    request->sendCompleted(MessageRequestObserverInterface::Status::BAD_REQUEST);
                    return;
                }
                Value::ConstMemberIterator event;
                Value _document{document.GetString(), strlen(document.GetString())};
                if (!findNode(_document, EVENT_KEY, &event)) {
                    request->sendCompleted(MessageRequestObserverInterface::Status::BAD_REQUEST);
                    return;
                }
                Value::ConstMemberIterator header;
                if (!findNode(event->value, HEADER_KEY, &header)) {
                    request->sendCompleted(MessageRequestObserverInterface::Status::BAD_REQUEST);
                    return;
                }
                Value::ConstMemberIterator payload;
                if (!findNode(event->value, PAYLOAD_KEY, &payload)) {
                    request->sendCompleted(MessageRequestObserverInterface::Status::BAD_REQUEST);
                    return;
                }
                string playerId;
                if (!retrieveValue(payload->value, PLAYER_ID_KEY, &playerId)) {
                    request->sendCompleted(MessageRequestObserverInterface::Status::BAD_REQUEST);
                    return;
                }
                {
                    lock_guard<mutex> lock(m_updatePlayersMutex);
                    if (m_authorizedPlayerIds.count(playerId) == 0) {
                        ACSDK_ERROR(LX("sendMessageFailed").d("reason", "unauthorizedPlayer").d("playerId", playerId));
                        request->sendCompleted(MessageRequestObserverInterface::Status::BAD_REQUEST);
                        return;
                    }
                    m_messageSender->sendMessage(request);
                }
            }
            void AuthorizedSender::updateAuthorizedPlayers(const unordered_set<string>& playerIds) {
                ACSDK_DEBUG5(LX(__func__));
                lock_guard<mutex> lock(m_updatePlayersMutex);
                m_authorizedPlayerIds = playerIds;
            }
        }
    }
}