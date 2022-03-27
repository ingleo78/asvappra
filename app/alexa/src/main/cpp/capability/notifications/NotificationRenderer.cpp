#include <thread>
#include <avs/FocusState.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <logger/Logger.h>
#include "NotificationRenderer.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        static const string TAG("NotificationRenderer");
        static const string CHANNEL_NAME = "Earcon";
        static const string NAMESPACE = "NotificationRenderer";
        #define LX(event) LogEntry(TAG, event)
        ostream& operator<<(ostream& stream, const NotificationRenderer::State state) {
            switch(state) {
                case NotificationRenderer::State::IDLE:
                    stream << "IDLE";
                    return stream;
                case NotificationRenderer::State::RENDERING_PREFERRED:
                    stream << "RENDERING_PREFERRED";
                    return stream;
                case NotificationRenderer::State::RENDERING_DEFAULT:
                    stream << "RENDERING_DEFAULT";
                    return stream;
                case NotificationRenderer::State::CANCELLING:
                    stream << "CANCELLING";
                    return stream;
                case NotificationRenderer::State::NOTIFYING:
                    stream << "NOTIFYING";
                    return stream;
            }
            stream << "UNKNOWN: " << static_cast<int>(state);
            return stream;
        }
        void NotificationRenderer::doShutdown() {
            ACSDK_DEBUG5(LX(__func__));
            if (m_mediaPlayer) m_mediaPlayer->removeObserver(shared_from_this());
            m_executor.shutdown();
            m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
            m_focusManager.reset();
            lock_guard<mutex> lock(m_mutex);
            m_observers.clear();
        }
        void NotificationRenderer::onFocusChanged(FocusState newFocus, MixingBehavior behavior) {
            {
                lock_guard<mutex> locker(m_mutex);
                auto currentFocus = m_focusState;
                m_focusState = newFocus;
                if ((FocusState::NONE == newFocus || FocusState::NONE != currentFocus) ||
                    !(behavior == MixingBehavior::UNDEFINED || behavior == MixingBehavior::PRIMARY)) {
                    return;
                }
            }
            m_executor.submit([this]() {
                if (MediaPlayerInterface::ERROR == m_sourceId) {
                    ACSDK_ERROR(LX("renderNotificationPreferredFailed").d("reason", "invalid sourceId"));
                } else if (m_mediaPlayer->play(m_sourceId)) {
                    ACSDK_DEBUG5(LX("renderNotificationSuccess").d("sourceId", m_sourceId));
                    return;
                }
                ACSDK_ERROR(LX("renderNotificationPreferredFailure").d("sourceId", m_sourceId));
                if (!setState(State::RENDERING_DEFAULT)) {
                    ACSDK_ERROR(LX("renderNotificationFailed").d("reason", "setState(RENDERING_DEFAULT) failed"));
                    auto result = m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                    if (!result.valid() || !result.get()) {
                        ACSDK_ERROR(LX("renderNotification").m("UnableToReleaseChannel"));
                    }
                    return;
                }
                shared_ptr<istream> stream;
                MediaType streamFormat = avsCommon::utils::MediaType::UNKNOWN;
                tie(stream, streamFormat) = m_audioFactory();
                m_sourceId = m_mediaPlayer->setSource(stream, false, emptySourceConfig(), streamFormat);
                if (MediaPlayerInterface::ERROR == m_sourceId) {
                    ACSDK_ERROR(LX("renderNotificationDefaultFailed").d("reason", "invalid sourceId"));
                } else if (m_mediaPlayer->play(m_sourceId)) {
                    ACSDK_DEBUG5(LX("renderNotificationDefaultSuccess").d("sourceId", m_sourceId));
                    return;
                }
                ACSDK_ERROR(LX("renderNotificationDefaultFailure").d("sourceId", m_sourceId));
                m_sourceId = MediaPlayerInterface::ERROR;
                m_audioFactory = nullptr;
                setState(State::IDLE);
                auto result = m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                if (!result.valid() || !result.get()) {
                    ACSDK_ERROR(LX("renderNotification").m("UnableToReleaseChannel"));
                }
            });
        }
        shared_ptr<NotificationRenderer> NotificationRenderer::create(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<FocusManagerInterface> focusManager) {
            ACSDK_DEBUG5(LX(__func__));
            if (!mediaPlayer) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullMediaPlayer"));
                return nullptr;
            }
            if (!focusManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullFocusManager"));
                return nullptr;
            }
            shared_ptr<NotificationRenderer> result(new NotificationRenderer(mediaPlayer, focusManager));
            mediaPlayer->addObserver(result);
            return result;
        }
        void NotificationRenderer::addObserver(shared_ptr<acsdkNotificationsInterfaces::NotificationRendererObserverInterface> observer) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_observers.insert(observer);
        }
        void NotificationRenderer::removeObserver(shared_ptr<NotificationRendererObserverInterface> observer) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_observers.erase(observer);
        }
        bool NotificationRenderer::renderNotification(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url) {
            ACSDK_DEBUG5(LX(__func__));
            if (!audioFactory) {
                ACSDK_ERROR(LX("renderNotificationFailed").d("reason", "nullAudioFactory"));
                return false;
            }
            {
                unique_lock<mutex> lock(m_mutex);
                m_wakeTrigger.wait(lock, [this]() { return m_state != State::NOTIFYING; });
            }
            if (!setState(State::RENDERING_PREFERRED)) {
                ACSDK_ERROR(LX("renderNotificationFailed").d("reason", "setState(RENDERING_PREFERRED) failed"));
                return false;
            }
            m_audioFactory = audioFactory;
            m_sourceId = m_mediaPlayer->setSource(url);
            auto activity = FocusManagerInterface::Activity::create(NAMESPACE, shared_from_this(), milliseconds::zero(), ContentType::MIXABLE);
            if (m_focusManager->acquireChannel(CHANNEL_NAME, activity)) {
                ACSDK_DEBUG5(LX("renderNotificationPreferred").m("AcquireChannelSuccess").d("sourceId", m_sourceId));
                return true;
            }
            ACSDK_ERROR(LX("renderNotificationPreferred").m("AcquireChannelFailure").d("sourceId", m_sourceId));
            m_sourceId = MediaPlayerInterface::ERROR;
            m_audioFactory = nullptr;
            setState(State::IDLE);
            return false;
        }
        bool NotificationRenderer::cancelNotificationRendering() {
            ACSDK_DEBUG5(LX(__func__));
            if (!setState(State::CANCELLING)) {
                ACSDK_DEBUG5(LX("cancelNotificationRenderingFailed").d("reason", "setState(CANCELLING) failed"));
                return false;
            }
            if (!m_mediaPlayer->stop(m_sourceId)) {
                ACSDK_ERROR(LX("cancelNotificationRenderingFailed").d("reason", "stopFailed"));
            }
            return true;
        }
        void NotificationRenderer::onFirstByteRead(SourceId sourceId, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", sourceId));
        }
        void NotificationRenderer::onPlaybackStarted(SourceId sourceId, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", sourceId));
            if (sourceId != m_sourceId) {
                ACSDK_ERROR(LX("onPlaybackStartedFailed").d("reason", "unexpectedSourceId").d("expected", m_sourceId));
                return;
            }
            if (State::IDLE == m_state || State::NOTIFYING == m_state) {
                ACSDK_ERROR(LX("onPlaybackStartedFailed").d("reason", "unexpectedState").d("state", m_state));
            }
        }
        void NotificationRenderer::onPlaybackStopped(SourceId sourceId, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", sourceId));
            if (sourceId != m_sourceId) {
                ACSDK_ERROR(LX("onPlaybackStoppedFailed").d("reason", "unexpectedSourceId").d("expected", m_sourceId));
                return;
            }
            onRenderingFinished(sourceId);
        }
        void NotificationRenderer::onPlaybackFinished(SourceId sourceId, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", sourceId));
            if (sourceId != m_sourceId) {
                ACSDK_ERROR(LX("onPlaybackFinishedFailed").d("reason", "unexpectedSourceId").d("expected", m_sourceId));
                return;
            }
            onRenderingFinished(sourceId);
        }
        void NotificationRenderer::onPlaybackError(SourceId sourceId, const ErrorType& type, string error, const MediaPlayerState&) {
            ACSDK_DEBUG5(LX(__func__).d("sourceId", sourceId).d("type", type).d("error", error));
            if (sourceId != m_sourceId) {
                ACSDK_ERROR(LX("onPlaybackErrorFailed").d("reason", "unexpectedSourceId").d("expected", m_sourceId));
                return;
            }
            {
                unique_lock<mutex> lock(m_mutex);
                switch(m_state) {
                    case State::IDLE: case State::NOTIFYING:
                        ACSDK_ERROR(LX("onPlaybackErrorFailed").d("reason", "unexpectedState"));
                        return;
                    case State::RENDERING_DEFAULT: case State::CANCELLING:
                        lock.unlock();
                        onRenderingFinished(sourceId);
                        return;
                    case State::RENDERING_PREFERRED:
                        if (!setStateLocked(State::RENDERING_DEFAULT)) return;
                        break;
                }
            }
            m_renderFallbackFuture = async(launch::async, [this, sourceId]() {
                shared_ptr<istream> stream;
                MediaType streamFormat = MediaType::UNKNOWN;
                tie(stream, streamFormat) = m_audioFactory();
                m_sourceId = m_mediaPlayer->setSource(stream, false, emptySourceConfig(), streamFormat);
                if (m_sourceId != MediaPlayerInterface::ERROR && m_mediaPlayer->play(m_sourceId)) return;
                ACSDK_ERROR(LX("playDefaultAudioFailed"));
                onRenderingFinished(sourceId);
            });
        }
        NotificationRenderer::NotificationRenderer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<FocusManagerInterface> focusManager) :
                                                   RequiresShutdown{"NotificationRenderer"}, m_mediaPlayer{mediaPlayer}, m_focusManager{focusManager},
                                                   m_focusState{FocusState::NONE}, m_state{State::IDLE}, m_sourceId{MediaPlayerInterface::ERROR} {}
        void NotificationRenderer::onRenderingFinished(SourceId sourceId) {
            unordered_set<shared_ptr<acsdkNotificationsInterfaces::NotificationRendererObserverInterface>> localObservers;
            auto result = m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
            if (!result.valid() || !result.get()) {
                ACSDK_ERROR(LX(__func__).m("UnableToReleaseChannel"));
            }
            {
                lock_guard<mutex> lock(m_mutex);
                localObservers = m_observers;
                if (!setStateLocked(State::NOTIFYING)) return;
            }
            for (auto observer : localObservers) observer->onNotificationRenderingFinished();
            setState(State::IDLE);
        }
        bool NotificationRenderer::setState(State newState) {
            lock_guard<mutex> lock(m_mutex);
            return setStateLocked(newState);
        }
        bool NotificationRenderer::setStateLocked(State newState) {
            bool result = true;
            if (newState == m_state) result = false;
            else {
                switch(newState) {
                    case State::IDLE: result = (State::RENDERING_PREFERRED != m_state) && (State::RENDERING_DEFAULT != m_state); break;
                    case State::RENDERING_PREFERRED: result = State::IDLE == m_state; break;
                    case State::RENDERING_DEFAULT: result = State::RENDERING_PREFERRED == m_state; break;
                    case State::CANCELLING: result = (State::RENDERING_DEFAULT == m_state) || (State::RENDERING_PREFERRED == m_state); break;
                    case State::NOTIFYING: result = m_state != State::IDLE; break;
                }
            }
            if (result) {
                ACSDK_DEBUG5(LX("setStateSuccess").d("state", m_state).d("newState", newState));
                m_state = newState;
                m_wakeTrigger.notify_all();
            } else { ACSDK_ERROR(LX("setStateFailed").d("state", m_state).d("newState", newState)); }
            return result;
        }
    }
}