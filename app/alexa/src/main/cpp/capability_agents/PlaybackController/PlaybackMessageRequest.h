#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKMESSAGEREQUEST_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKMESSAGEREQUEST_H_

#include <avs/MessageRequest.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include "PlaybackController.h"
#include "PlaybackCommand.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            class PlaybackMessageRequest : public MessageRequest {
            public:
                PlaybackMessageRequest(const PlaybackCommand& command, const string& jsonContent, shared_ptr<PlaybackController> playbackController);
                void sendCompleted(MessageRequestObserverInterface::Status status) override;
            private:
                shared_ptr<PlaybackController> m_playbackController;
                const PlaybackCommand& m_command;
            };
        }
    }
}
#endif