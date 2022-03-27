#ifndef ACSDKNOTIFICATIONS_NOTIFICATIONRENDERER_H_
#define ACSDKNOTIFICATIONS_NOTIFICATIONRENDERER_H_

#include <mutex>
#include <unordered_set>
#include <avs/MixingBehavior.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <media_player/MediaPlayerInterface.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <util/MediaType.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include "NotificationRendererInterface.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace mediaPlayer;
        using namespace threading;
        using namespace rapidjson;
        using namespace acsdkNotificationsInterfaces;
        class NotificationRenderer : public NotificationRendererInterface, public MediaPlayerObserverInterface , public RequiresShutdown,
                                     public ChannelObserverInterface, public enable_shared_from_this<NotificationRenderer> {
        public:
            using SourceId = MediaPlayerInterface::SourceId;
            static shared_ptr<NotificationRenderer> create(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<FocusManagerInterface> focusManager);
            void addObserver(shared_ptr<NotificationRendererObserverInterface> observer) override;
            void removeObserver(shared_ptr<NotificationRendererObserverInterface> observer) override;
            bool renderNotification(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url) override;
            bool cancelNotificationRendering() override;
            void onFirstByteRead(SourceId sourceId, const MediaPlayerState& state) override;
            void onPlaybackStarted(SourceId sourceId, const MediaPlayerState& state) override;
            void onPlaybackStopped(SourceId sourceId, const MediaPlayerState& state) override;
            void onPlaybackFinished(SourceId sourceId, const MediaPlayerState& state) override;
            void onPlaybackError(SourceId sourceId, const ErrorType& type, string error, const MediaPlayerState& state) override;
            void doShutdown() override;
            void onFocusChanged(FocusState newFocus, MixingBehavior behavior) override;
        private:
            enum class State {
                IDLE,
                RENDERING_PREFERRED,
                RENDERING_DEFAULT,
                CANCELLING,
                NOTIFYING
            };
            NotificationRenderer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<FocusManagerInterface> focusManager);
            void onRenderingFinished(SourceId sourceId);
            bool setState(State newState);
            bool setStateLocked(State newState);
            bool verifyRenderingLocked(const string& caller, SourceId sourceId);
            shared_ptr<MediaPlayerInterface> m_mediaPlayer;
            shared_ptr<FocusManagerInterface> m_focusManager;
            FocusState m_focusState;
            unordered_set<shared_ptr<NotificationRendererObserverInterface>> m_observers;
            mutex m_mutex;
            condition_variable m_wakeTrigger;
            State m_state;
            function<pair<unique_ptr<istream>, const MediaType>()> m_audioFactory;
            SourceId m_sourceId;
            future<void> m_renderFallbackFuture;
            Executor m_executor;
            friend ostream& operator<<(ostream& stream, const NotificationRenderer::State state);
        };
    }
}
#endif