#include <string>
#include <logger/Logger.h>
#include "PlaybackRouter.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            static const string TAG("PlaybackRouter");
            #define LX(event) LogEntry(TAG, event)
            shared_ptr<PlaybackRouter> PlaybackRouter::create(shared_ptr<PlaybackHandlerInterface> defaultHandler) {
                ACSDK_DEBUG9(LX("create").m("called"));
                if (nullptr == defaultHandler) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "null defaultHandler"));
                    return nullptr;
                }
                auto playbackRouter = shared_ptr<PlaybackRouter>(new PlaybackRouter(defaultHandler));
                return playbackRouter;
            }
            void PlaybackRouter::buttonPressed(PlaybackButton button) {
                ACSDK_DEBUG9(LX("buttonPressed").d("button", button));
                unique_lock<mutex> lock(m_handlerMutex);
                if (!m_handler) {
                    ACSDK_ERROR(LX("buttonPressedFailed").m("called but handler is not set"));
                    return;
                }
                auto observer = m_handler;
                lock.unlock();
                observer->onButtonPressed(button);
            }
            void PlaybackRouter::togglePressed(PlaybackToggle toggle, bool action) {
                ACSDK_DEBUG9(LX("togglePressed").d("toggle", toggle).d("action", action));
                unique_lock<mutex> lock(m_handlerMutex);
                if (!m_handler) {
                    ACSDK_ERROR(LX("togglePressedFailed").m("called but handler is not set"));
                    return;
                }
                auto observer = m_handler;
                lock.unlock();
                observer->onTogglePressed(toggle, action);
            }
            void PlaybackRouter::switchToDefaultHandler() {
                ACSDK_DEBUG9(LX("switchToDefaultHandler"));
                setHandler(m_defaultHandler);
            }
            PlaybackRouter::PlaybackRouter(shared_ptr<PlaybackHandlerInterface> defaultHandler) : RequiresShutdown{"PlaybackRouter"},
                                           m_handler{defaultHandler}, m_defaultHandler{defaultHandler} {}
            void PlaybackRouter::doShutdown() {
                lock_guard<mutex> lock(m_handlerMutex);
                m_handler.reset();
                m_defaultHandler.reset();
            }
            void PlaybackRouter::setHandler(shared_ptr<PlaybackHandlerInterface> handler, shared_ptr<LocalPlaybackHandlerInterface> localHandler) {
                ACSDK_DEBUG9(LX("setHandler").d("handler", handler));
                lock_guard<mutex> guard(m_handlerMutex);
                if (!handler) {
                    ACSDK_ERROR(LX("setHandler").d("handler", handler));
                    return;
                }
                m_handler = handler;
                m_localHandler = localHandler;
            }
            void PlaybackRouter::useDefaultHandlerWith(shared_ptr<LocalPlaybackHandlerInterface> localHandler) {
                ACSDK_DEBUG9(LX("useDefaultHandlerWith"));
                setHandler(m_defaultHandler, localHandler);
            }
            bool PlaybackRouter::localOperation(LocalPlaybackHandlerInterface::PlaybackOperation op) {
                ACSDK_DEBUG9(LX("localOperation"));
                bool useFallback = true;
                {
                    lock_guard<std::mutex> guard(m_handlerMutex);
                    if (m_localHandler) {
                        useFallback = !m_localHandler->localOperation(op);
                        ACSDK_DEBUG(LX("localOperation").d("usingFallback", useFallback));
                    }
                }
                if (useFallback) {
                    switch(op) {
                        case LocalPlaybackHandlerInterface::PlaybackOperation::STOP_PLAYBACK:
                        case LocalPlaybackHandlerInterface::PlaybackOperation::PAUSE_PLAYBACK:
                            buttonPressed(PlaybackButton::PAUSE);
                            break;
                        case LocalPlaybackHandlerInterface::PlaybackOperation::RESUME_PLAYBACK:
                            buttonPressed(PlaybackButton::PLAY);
                            break;
                        default: return false;
                    }
                }
                return true;
            }
            bool PlaybackRouter::localSeekTo(milliseconds location, bool fromStart) {
                ACSDK_DEBUG9(LX("localSeekTo").d("location", location.count()).d("fromStart", fromStart));
                lock_guard<mutex> guard(m_handlerMutex);
                if (m_localHandler) return m_localHandler->localSeekTo(location, fromStart);
                return false;
            }
        }
    }
}