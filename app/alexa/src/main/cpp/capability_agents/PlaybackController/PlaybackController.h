#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKCONTROLLER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKCONTROLLER_H_

#include <memory>
#include <queue>
#include <string>
#include <avs/CapabilityConfiguration.h>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ContextRequesterInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/PlaybackHandlerInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include "PlaybackCommand.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace threading;
            class PlaybackController : public ContextRequesterInterface, public PlaybackHandlerInterface, public CapabilityConfigurationInterface,
                                       public RequiresShutdown, public enable_shared_from_this<PlaybackController> {
            public:
                static shared_ptr<PlaybackController> create(shared_ptr<ContextManagerInterface> contextManager,
                                                             shared_ptr<MessageSenderInterface> messageSender);
                virtual ~PlaybackController() = default;
                void onContextAvailable(const string& jsonContext) override;
                void onContextFailure(const ContextRequestError error) override;
                void onButtonPressed(PlaybackButton button) override;
                void onTogglePressed(PlaybackToggle toggle, bool action) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
                void messageSent(const PlaybackCommand&, MessageRequestObserverInterface::Status messageStatus);
            private:
                PlaybackController(shared_ptr<ContextManagerInterface> contextManager, shared_ptr<MessageSenderInterface> messageSender);
                void doShutdown() override;
                void handleCommand(const PlaybackCommand& command);
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<ContextManagerInterface> m_contextManager;
                queue<const PlaybackCommand*> m_commands;
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                Executor m_executor;
            };
        }
    }
}
#endif