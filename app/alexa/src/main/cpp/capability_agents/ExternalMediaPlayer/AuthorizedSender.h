#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_EXTERNALMEDIAPLAYER_INCLUDE_EXTERNALMEDIAPLAYER_AUTHORIZEDSENDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_EXTERNALMEDIAPLAYER_INCLUDE_EXTERNALMEDIAPLAYER_AUTHORIZEDSENDER_H_

#include <mutex>
#include <string>
#include <unordered_set>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <threading/Executor.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace externalMediaPlayer {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace json;
            class AuthorizedSender : public MessageSenderInterface {
            public:
                ~AuthorizedSender() override;
                static shared_ptr<AuthorizedSender> create(shared_ptr<MessageSenderInterface> messageSender);
                void sendMessage(shared_ptr<MessageRequest> request) override;
                void updateAuthorizedPlayers(const unordered_set<string>& playerIds);
            private:
                AuthorizedSender(shared_ptr<MessageSenderInterface> messageSender);
                mutex m_updatePlayersMutex;
                shared_ptr<MessageSenderInterface> m_messageSender;
                unordered_set<string> m_authorizedPlayerIds;
            };
        }
    }
}
#endif