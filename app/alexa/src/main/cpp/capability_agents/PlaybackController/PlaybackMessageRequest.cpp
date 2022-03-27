#include "PlaybackMessageRequest.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            PlaybackMessageRequest::PlaybackMessageRequest(const PlaybackCommand& command, const string& jsonContent,
                                                           shared_ptr<PlaybackController> playbackController) : MessageRequest(jsonContent),
                                                           m_playbackController{playbackController}, m_command(command) {}
            void PlaybackMessageRequest::sendCompleted(MessageRequestObserverInterface::Status status) {
                m_playbackController->messageSent(m_command, status);
            }
        }
    }
}