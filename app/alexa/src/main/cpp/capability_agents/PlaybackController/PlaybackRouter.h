#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKROUTER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKROUTER_H_

#include <sdkinterfaces/PlaybackHandlerInterface.h>
#include <sdkinterfaces/PlaybackRouterInterface.h>
#include <util/RequiresShutdown.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            class PlaybackRouter : public PlaybackRouterInterface, public RequiresShutdown, public enable_shared_from_this<PlaybackRouter> {
            public:
                static shared_ptr<PlaybackRouter> create(shared_ptr<PlaybackHandlerInterface> defaultHandler);
                virtual ~PlaybackRouter() = default;
                virtual void buttonPressed(PlaybackButton button) override;
                virtual void togglePressed(PlaybackToggle toggle, bool action) override;
                virtual void switchToDefaultHandler() override;
                virtual void setHandler(shared_ptr<PlaybackHandlerInterface> handler, shared_ptr<LocalPlaybackHandlerInterface> localHandler = nullptr) override;
                virtual void useDefaultHandlerWith(shared_ptr<LocalPlaybackHandlerInterface> localHandler) override;
                virtual bool localOperation(LocalPlaybackHandlerInterface::PlaybackOperation op) override;
                virtual bool localSeekTo(milliseconds location, bool fromStart) override;
            private:
                PlaybackRouter(shared_ptr<PlaybackHandlerInterface> defaultHandler);
                void doShutdown() override;
                shared_ptr<PlaybackHandlerInterface> m_handler;
                shared_ptr<LocalPlaybackHandlerInterface> m_localHandler;
                shared_ptr<PlaybackHandlerInterface> m_defaultHandler;
                mutex m_handlerMutex;
            };
        }
    }
}
#endif