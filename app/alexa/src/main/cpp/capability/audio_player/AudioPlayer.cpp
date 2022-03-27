#include <algorithm>
#include <cctype>
#include <cstdio>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <json/en.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/Audio/MixingBehavior.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <metrics/MetricEventBuilder.h>
#include <metrics/DataPointStringBuilder.h>
#include <metrics/DataPointCounterBuilder.h>
#include "AudioPlayer.h"

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        using namespace acsdkAudioPlayerInterfaces;
        using namespace rapidjson;
        using namespace sdkInterfaces;
        using namespace json;
        using namespace logger;
        using namespace metrics;
        using namespace captions;
        using MediaPlayerState = MediaPlayerState;
        static const string AUDIOPLAYER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
        static const string AUDIOPLAYER_CAPABILITY_INTERFACE_NAME = "AudioPlayer";
        static const string AUDIOPLAYER_CAPABILITY_INTERFACE_VERSION = "1.4";
        static const string FINGERPRINT_KEY = "fingerprint";
        static const string PACKAGE_KEY = "package";
        static const string BUILD_TYPE_KEY = "buildType";
        static const string VERSION_NUMBER_KEY = "versionNumber";
        static const string TAG("AudioPlayer");
        static const AudioPlayer::SourceId ERROR_SOURCE_ID = MediaPlayerInterface::ERROR;
        #define LX(event) LogEntry(TAG, event)
        static const string CHANNEL_NAME = FocusManagerInterface::CONTENT_CHANNEL_NAME;
        static const string NAMESPACE = "AudioPlayer";
        static const NamespaceAndName PLAY{NAMESPACE, "Play"};
        static const NamespaceAndName STOP{NAMESPACE, "Stop"};
        static const NamespaceAndName CLEAR_QUEUE{NAMESPACE, "ClearQueue"};
        static const NamespaceAndName UPDATE_PROGRESS_REPORT_INTERVAL{NAMESPACE, "UpdateProgressReportInterval"};
        static const NamespaceAndName STATE{NAMESPACE, "PlaybackState"};
        static const string CID_PREFIX{"cid:"};
        static const char TOKEN_KEY[] = "token";
        static const char OFFSET_KEY[] = "offsetInMilliseconds";
        static const char SEEK_START_OFFSET_KEY[] = "seekStartOffsetInMilliseconds";
        static const char SEEK_END_OFFSET_KEY[] = "seekEndOffsetInMilliseconds";
        static const char PLAYBACK_ATTRIBUTES_KEY[] = "playbackAttributes";
        static const char NAME_KEY[] = "name";
        static const char CODEC_KEY[] = "codec";
        static const char SAMPLING_RATE_IN_HERTZ_KEY[] = "samplingRateInHertz";
        static const char DATA_RATE_IN_BITS_PER_SECOND_KEY[] = "dataRateInBitsPerSecond";
        static const char PLAYBACK_REPORTS_KEY[] = "playbackReports";
        static const char START_OFFSET_KEY[] = "startOffsetInMilliseconds";
        static const char END_OFFSET_KEY[] = "endOffsetInMilliseconds";
        static const char ACTIVITY_KEY[] = "playerActivity";
        static const char CAPTION_KEY[] = "caption";
        static const char CAPTION_TYPE_KEY[] = "type";
        static const char CAPTION_CONTENT_KEY[] = "content";
        static const char STUTTER_DURATION_KEY[] = "stutterDurationInMilliseconds";
        static const seconds TIMEOUT{2};
        static const audio::MixingBehavior DEFAULT_MIXING_BEHAVIOR = audio::MixingBehavior::BEHAVIOR_PAUSE;
        static const string AUDIO_PLAYER_METRIC_PREFIX = "AUDIO_PLAYER-";
        static const string UNSUPPORTED_INTERRUPTED_BEHAVIOR = "UNSUPPORTED_INTERRUPTED_BEHAVIOR_PLAY_DIRECTIVE";
        static const string MISSING_INTERRUPTED_BEHAVIOR = "MISSING_INTERRUPTED_BEHAVIOR_PLAY_DIRECTIVE";
        static const string PREPARE_PLAY_DIRECTIVE_RECEIVED = "PREPARE_PLAY_DIRECTIVE_RECEIVED";
        static const string PLAY_DIRECTIVE_RECEIVED = "PLAY_DIRECTIVE_RECEIVED";
        static const string STOP_DIRECTIVE_RECEIVED = "STOP_DIRECTIVE_RECEIVED";
        static const string PLAYBACK_STARTED = "PLAYBACK_STARTED";
        static const string PLAYBACK_STOPPED = "PLAYBACK_STOPPED";
        static const string METADATA_UNFILTERED_ENCOUNTERED = "UnfilteredMetadataEncountered";
        static const string METADATA_FILTERED_ENCOUNTERED = "FilteredMetadataEncountered";
        static const string MEDIA_PLAYBACK_TIME = "MediaPlaybackTime";
        static const string TRACK_TO_TRACK_TIME = "AutoProgressionLatency";
        static const string TRACK_PROGRESSION_FATAL = "TrackProgressionFatalError";
        static const string MID_PLAYBACK_FATAL = "PlaybackFatalError";
        static const string TRACK_TIME_ON_QUEUE = "TrackTimeOnQueue";
        static const vector<string> METADATA_WHITELIST = {"title"};
        static const seconds METADATA_EVENT_RATE{1};
        static const seconds LOCAL_STOP_DEFAULT_PIPELINE_OPEN_TIME(900);
        static bool compareUrlNotQuery(const std::string& url1, const std::string& url2) {
            string::size_type p1 = url1.find('?');
            string::size_type p2 = url2.find('?');
            if (p1 != p2) return false;
            if (p1 == string::npos) return url1 == url2;
            string url1Trunc = url1.substr(0, p1);
            string url2Trunc = url2.substr(0, p2);
            return url1Trunc == url2Trunc;
        }
        static shared_ptr<CapabilityConfiguration> getAudioPlayerCapabilityConfiguration(Fingerprint fingerprint);
        static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const string& metricActivityName, const DataPoint& dataPoint,
                                 const string& msgId, const string& trackId, const string& requesterType = "") {
            auto metricBuilder = MetricEventBuilder{}.setActivityName(metricActivityName).addDataPoint(dataPoint);
            if (!msgId.empty()) {
                metricBuilder.addDataPoint(DataPointStringBuilder{}.setName("DIRECTIVE_MESSAGE_ID").setValue(msgId).build());
            }
            if (!trackId.empty()) {
                metricBuilder.addDataPoint(DataPointStringBuilder{}.setName("TRACK_ID").setValue(trackId).build());
            }
            if (!requesterType.empty()) {
                metricBuilder.addDataPoint(DataPointStringBuilder{}.setName("REQUESTER_TYPE").setValue(requesterType).build());
            }
            auto metric = metricBuilder.build();
            if (metric == nullptr) {
                ACSDK_ERROR(LX("Error creating metric."));
                return;
            }
            recordMetric(metricRecorder, metric);
        }
        static void submitAnnotatedMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const string& baseMetric, const string& annotation) {
            auto metricName = baseMetric + "_" + annotation;
            submitMetric(metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + metricName,DataPointCounterBuilder{}.setName(metricName).increment(1).build(),
                  "","");
        }
        AudioPlayer::PlayDirectiveInfo::PlayDirectiveInfo(const std::string& messageId) : messageId{messageId}, playBehavior{PlayBehavior::ENQUEUE},
                                                          sourceId{ERROR_SOURCE_ID}, isBuffered{false} {}
        shared_ptr<AudioPlayer> AudioPlayer::create(unique_ptr<MediaPlayerFactoryInterface> mediaPlayerFactory, shared_ptr<MessageSenderInterface> messageSender,
                                                    shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ContextManagerInterface> contextManager,
                                                    shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender, shared_ptr<PlaybackRouterInterface> playbackRouter,
                                                    vector<shared_ptr<ChannelVolumeInterface>> audioChannelVolumeInterfaces, shared_ptr<captions::CaptionManagerInterface> captionManager,
                                                    shared_ptr<MetricRecorderInterface> metricRecorder) {
            if (nullptr == mediaPlayerFactory) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullMediaPlayerFactory"));
                return nullptr;
            } else if (nullptr == messageSender) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                return nullptr;
            } else if (nullptr == focusManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullFocusManager"));
                return nullptr;
            } else if (nullptr == contextManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                return nullptr;
            } else if (nullptr == exceptionSender) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
                return nullptr;
            } else if (nullptr == playbackRouter) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullPlaybackRouter"));
                return nullptr;
            } else if (audioChannelVolumeInterfaces.empty()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "emptyAudioChannelVolumeInterfaces"));
                return nullptr;
            }
            auto audioPlayer = shared_ptr<AudioPlayer>(new AudioPlayer(move(mediaPlayerFactory), messageSender, focusManager, contextManager, exceptionSender,
                                                       playbackRouter, audioChannelVolumeInterfaces, captionManager, metricRecorder));
            contextManager->setStateProvider(STATE, audioPlayer);
            audioPlayer->m_mediaPlayerFactory->addObserver(audioPlayer);
            return audioPlayer;
        }
        void AudioPlayer::provideState(const NamespaceAndName& stateProviderName, unsigned int stateRequestToken) {
            ACSDK_DEBUG(LX("provideState").d("stateRequestToken", stateRequestToken));
            m_executor.submit([this, stateRequestToken] { executeProvideState(true, stateRequestToken); });
        }
        void AudioPlayer::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
            auto info = std::make_shared<DirectiveInfo>(directive, nullptr);
            if (info && (info->directive->getName() == PLAY.name || info->directive->getName() == CLEAR_QUEUE.name)) preHandleDirective(info);
            handleDirective(info);
        }
        void AudioPlayer::preHandleDirective(shared_ptr<DirectiveInfo> info) {
            if (!info || !info->directive) {
                ACSDK_ERROR(LX("preHandleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                return;
            }
            ACSDK_DEBUG5(LX(__func__).d("name", info->directive->getName()).d("messageId", info->directive->getMessageId()));
            if (info->directive->getName() == PLAY.name) preHandlePlayDirective(info);
            else if (info->directive->getName() == CLEAR_QUEUE.name) handleClearQueueDirective(info);
            else if (info->directive->getName() == STOP.name) handleStopDirective(info);
            else { ACSDK_DEBUG(LX("preHandleDirective NO-OP").d("reason", "NoPreHandleBehavior").d("directiveName", info->directive->getName())); }
        }
        void AudioPlayer::handleDirective(shared_ptr<DirectiveInfo> info) {
            if (!info || !info->directive) {
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                return;
            }
            ACSDK_DEBUG5(LX("handleDirective").d("name", info->directive->getName()).d("messageId", info->directive->getMessageId()));
            if (info->directive->getName() == PLAY.name) handlePlayDirective(info);
            else if (info->directive->getName() == STOP.name);
            else if (info->directive->getName() == CLEAR_QUEUE.name);
            else if (info->directive->getName() == UPDATE_PROGRESS_REPORT_INTERVAL.name) handleUpdateProgressReportIntervalDirective(info);
            else {
                sendExceptionEncounteredAndReportFailed(info,"unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName(),
                                                  ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "unknownDirective").d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));
            }
        }
        void AudioPlayer::cancelDirective(shared_ptr<DirectiveInfo> info) {
            removeDirective(info);
            if (info && info->directive) {
                ACSDK_DEBUG(LX("cancelDirective").d("name", info->directive->getName()));
                auto messageId = info->directive->getMessageId();
                m_executor.submit([this, messageId] {
                    auto okToRequest = m_okToRequestNextTrack;
                    m_okToRequestNextTrack = false;
                    for (auto it = m_audioPlayQueue.begin(); it != m_audioPlayQueue.end(); it++) {
                        if (messageId == (*it)->messageId) {
                            if (it->get()->mediaPlayer) stopAndReleaseMediaPlayer(*it);
                            m_audioPlayQueue.erase(it);
                            break;
                        }
                    }
                    m_okToRequestNextTrack = okToRequest;
                });
            }
        }
        void AudioPlayer::onDeregistered() {
            ACSDK_DEBUG(LX("onDeregistered"));
            m_executor.submit([this] {
                executeStop();
                clearPlayQueue(false);
            });
        }
        DirectiveHandlerConfiguration AudioPlayer::getConfiguration() const {
            DirectiveHandlerConfiguration configuration;
            auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
            configuration[PLAY] = audioNonBlockingPolicy;
            configuration[STOP] = audioNonBlockingPolicy;
            configuration[CLEAR_QUEUE] = audioNonBlockingPolicy;
            configuration[UPDATE_PROGRESS_REPORT_INTERVAL] = audioNonBlockingPolicy;
            return configuration;
        }
        void AudioPlayer::onFocusChanged(FocusState newFocus, MixingBehavior behavior) {
            ACSDK_DEBUG(LX("onFocusChanged").d("newFocus", newFocus).d("MixingBehavior", behavior));
            m_executor.submit([this, newFocus, behavior] { executeOnFocusChanged(newFocus, behavior); });
            switch(newFocus) {
                case FocusState::FOREGROUND: return;
                case FocusState::BACKGROUND: {
                        if (MixingBehavior::MAY_DUCK == behavior) return;
                        auto predicate = [this] {
                            switch(m_currentActivity) {
                                case PlayerActivity::IDLE: case PlayerActivity::PAUSED: case PlayerActivity::STOPPED: case PlayerActivity::FINISHED: return true;
                                case PlayerActivity::PLAYING: case PlayerActivity::BUFFER_UNDERRUN: return false;
                            }
                            ACSDK_ERROR(LX("onFocusChangedFailed").d("reason", "unexpectedActivity").d("m_currentActivity", m_currentActivity));
                            return false;
                        };
                        unique_lock<mutex> lock(m_currentActivityMutex);
                        if (!m_currentActivityConditionVariable.wait_for(lock, TIMEOUT, predicate)) {
                            ACSDK_ERROR(LX("onFocusChangedTimedOut").d("newFocus", newFocus).d("m_currentActivity", m_currentActivity));
                        }
                    }
                    return;
                case FocusState::NONE: {
                    auto predicate = [this] {
                        switch (m_currentActivity) {
                            case PlayerActivity::IDLE: case PlayerActivity::STOPPED: case PlayerActivity::FINISHED: return true;
                            case PlayerActivity::PLAYING: case PlayerActivity::PAUSED: case PlayerActivity::BUFFER_UNDERRUN: return false;
                        }
                        ACSDK_ERROR(LX("onFocusChangedFailed").d("reason", "unexpectedActivity").d("m_currentActivity", m_currentActivity));
                        return false;
                    };
                    unique_lock<mutex> lock(m_currentActivityMutex);
                    if (!m_currentActivityConditionVariable.wait_for(lock, TIMEOUT, predicate)) {
                        ACSDK_ERROR(LX("onFocusChangedFailed").d("reason", "activityChangeTimedOut").d("newFocus", newFocus)
                            .d("m_currentActivity", m_currentActivity));
                    }
                }
                    return;
            }
            ACSDK_ERROR(LX("onFocusChangedFailed").d("reason", "unexpectedFocusState").d("newFocus", newFocus));
        }
        void AudioPlayer::onFirstByteRead(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX(__func__).d("id", id).d("state", state));
        }
        void AudioPlayer::onPlaybackStarted(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onPlaybackStarted").d("id", id).d("state", state));
            submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + PLAYBACK_STARTED,DataPointCounterBuilder{}.setName(PLAYBACK_STARTED).increment(1).build(),
                         m_currentlyPlaying->messageId, m_currentlyPlaying->audioItem.id);
            m_executor.submit([this, id, state] { executeOnPlaybackStarted(id, state); });
        }
        void AudioPlayer::onPlaybackStopped(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onPlaybackStopped").d("id", id).d("state", state));
            submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + PLAYBACK_STOPPED,DataPointCounterBuilder{}.setName(PLAYBACK_STOPPED).increment(1).build(),
                         m_currentlyPlaying->stopMessageId, m_currentlyPlaying->audioItem.id);
            m_executor.submit([this, id, state] { executeOnPlaybackStopped(id, state); });
        }
        void AudioPlayer::onPlaybackFinished(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onPlaybackFinished").d("id", id).d("state", state));
            m_executor.submit([this, id, state] { executeOnPlaybackFinished(id, state); });
        }
        void AudioPlayer::onPlaybackError(SourceId id, const ErrorType& type, string error, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onPlaybackError").d("type", type).d("error", error).d("id", id).d("state", state));
            m_executor.submit([this, id, type, error, state] { executeOnPlaybackError(id, type, error, state); });
        }
        void AudioPlayer::onPlaybackPaused(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onPlaybackPaused").d("id", id).d("state", state));
            m_executor.submit([this, id, state] { executeOnPlaybackPaused(id, state); });
        }
        void AudioPlayer::onPlaybackResumed(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onPlaybackResumed").d("id", id).d("state", state));
            m_executor.submit([this, id, state] { executeOnPlaybackResumed(id, state); });
        }
        void AudioPlayer::onBufferUnderrun(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onBufferUnderrun").d("id", id).d("state", state));
            m_executor.submit([this, id, state] { executeOnBufferUnderrun(id, state); });
        }
        void AudioPlayer::onBufferRefilled(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onBufferRefilled").d("id", id).d("state", state));
            m_executor.submit([this, id, state] { executeOnBufferRefilled(id, state); });
        }
        void AudioPlayer::onSeeked(SourceId id, const MediaPlayerState& startState, const MediaPlayerState& endState) {
            ACSDK_DEBUG(LX("onSeeked").d("id", id).d("startState", startState).d("endState", endState));
            m_executor.submit([this, id, startState, endState] { executeOnSeeked(id, startState, endState); });
        }
        void AudioPlayer::onTags(SourceId id, std::unique_ptr<const VectorOfTags> vectorOfTags, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onTags").d("id", id).d("state", state));
            if (nullptr == vectorOfTags || vectorOfTags->empty()) {
                ACSDK_ERROR(LX("onTagsFailed").d("reason", "noTags"));
                return;
            }
            shared_ptr<const VectorOfTags> sharedVectorOfTags(move(vectorOfTags));
            m_executor.submit([this, id, sharedVectorOfTags, state] { executeOnTags(id, sharedVectorOfTags, state); });
        }
        void AudioPlayer::onBufferingComplete(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("onBufferingComplete").d("id", id).d("state", state));
            m_executor.submit([this, id, state] { executeOnBufferingComplete(id, state); });
        }
        void AudioPlayer::onReadyToProvideNextPlayer() {
            ACSDK_DEBUG(LX(__func__));
            m_executor.submit([this] { executeOnReadyToProvideNextPlayer(); });
        }
        void AudioPlayer::onProgressReportDelayElapsed() {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.submit([this] { sendEventWithTokenAndOffset("ProgressReportDelayElapsed"); });
        }
        void AudioPlayer::onProgressReportIntervalElapsed() {
            ACSDK_DEBUG9(LX(__func__));
            m_executor.submit([this] { sendEventWithTokenAndOffset("ProgressReportIntervalElapsed", true); });
        }
        void AudioPlayer::onProgressReportIntervalUpdated() {
            ACSDK_DEBUG9(LX(__func__));
            m_executor.submit([this] { sendEventWithTokenAndOffset("ProgressReportIntervalUpdated"); });
        }
        void AudioPlayer::requestProgress() {
            ACSDK_DEBUG9(LX(__func__));
            m_executor.submit([this] {
                auto progress = getOffset();
                m_progressTimer.onProgress(progress);
            });
        }
        void AudioPlayer::addObserver(shared_ptr<AudioPlayerObserverInterface> observer) {
            ACSDK_DEBUG1(LX("addObserver"));
            if (!observer) {
                ACSDK_ERROR(LX("addObserver").m("Observer is null."));
                return;
            }
            m_executor.submit([this, observer] {
                if (!m_observers.insert(observer).second) { ACSDK_ERROR(LX("addObserver").m("Duplicate observer.")); }
            });
        }
        void AudioPlayer::removeObserver(shared_ptr<AudioPlayerObserverInterface> observer) {
            ACSDK_DEBUG1(LX("removeObserver"));
            if (!observer) {
                ACSDK_ERROR(LX("removeObserver").m("Observer is null."));
                return;
            }
            m_executor.submit([this, observer] {
                if (m_observers.erase(observer) == 0) { ACSDK_WARN(LX("removeObserver").m("Nonexistent observer.")); }
            });
        }
        void AudioPlayer::setObserver(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer) {
            ACSDK_DEBUG1(LX(__func__));
            m_executor.submit([this, observer] { m_renderPlayerObserver = observer; });
        }
        milliseconds AudioPlayer::getAudioItemOffset() {
            ACSDK_DEBUG1(LX(__func__));
            auto offset = m_executor.submit([this] { return getOffset(); });
            return offset.get();
        }
        milliseconds AudioPlayer::getAudioItemDuration() {
            ACSDK_DEBUG5(LX(__func__));
            auto duration = m_executor.submit([this] { return getDuration(); });
            return duration.get();
        }
        void AudioPlayer::stopPlayback() {
            localOperation(PlaybackOperation::STOP_PLAYBACK);
        }
        bool AudioPlayer::localOperation(PlaybackOperation op) {
            ACSDK_DEBUG5(LX(__func__));
            promise<bool> opSuccess;
            auto successFuture = opSuccess.get_future();
            m_executor.submit([this, op, &opSuccess] { executeLocalOperation(op, std::move(opSuccess)); });
            if (successFuture.wait_for(milliseconds(1000)) == future_status::ready) return successFuture.get();
            return false;
        }
        bool AudioPlayer::localSeekTo(milliseconds location, bool fromStart) {
            ACSDK_DEBUG5(LX(__func__));
            auto result = m_executor.submit([this, location, fromStart] { return executeLocalSeekTo(location, fromStart); });
            return result.get();
        }
        AudioPlayer::AudioPlayer(unique_ptr<MediaPlayerFactoryInterface> mediaPlayerFactory, shared_ptr<MessageSenderInterface> messageSender,
                                 shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ContextManagerInterface> contextManager,
                                 shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender, shared_ptr<PlaybackRouterInterface> playbackRouter,
                                 vector<shared_ptr<ChannelVolumeInterface>> audioChannelVolumeInterfaces,
                                 shared_ptr<captions::CaptionManagerInterface> captionManager, shared_ptr<MetricRecorderInterface> metricRecorder) :
                                 CapabilityAgent{NAMESPACE, exceptionSender}, RequiresShutdown{"AudioPlayer"}, m_mediaPlayerFactory{move(mediaPlayerFactory)},
                                 m_messageSender{messageSender}, m_focusManager{focusManager}, m_contextManager{contextManager}, m_playbackRouter{playbackRouter},
                                 m_captionManager{captionManager}, m_metricRecorder{metricRecorder}, m_currentActivity{PlayerActivity::IDLE},
                                 m_focus{FocusState::NONE}, m_currentlyPlaying(make_shared<PlayDirectiveInfo>("")), m_offset{milliseconds{milliseconds::zero()}},
                                 m_isStopCalled{false}, m_okToRequestNextTrack{false}, m_isAcquireChannelRequestPending{false},
                                 m_currentMixability{ContentType::UNDEFINED}, m_mixingBehavior{MixingBehavior::UNDEFINED}, m_isAutoProgressing{false},
                                 m_isLocalResumePending(false), m_isStartingPlayback{false}, m_audioChannelVolumeInterfaces{audioChannelVolumeInterfaces} {
            Fingerprint fingerprint = m_mediaPlayerFactory->getFingerprint();
            m_capabilityConfigurations.insert(getAudioPlayerCapabilityConfiguration(fingerprint));
            m_currentlyPlaying->sourceId = ERROR_SOURCE_ID;
        }
        shared_ptr<CapabilityConfiguration> getAudioPlayerCapabilityConfiguration(Fingerprint fingerprint) {
            std::unordered_map<string, string> configMap;
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, AUDIOPLAYER_CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, AUDIOPLAYER_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, AUDIOPLAYER_CAPABILITY_INTERFACE_VERSION});
            if (!fingerprint.package.empty() && !fingerprint.buildType.empty() && !fingerprint.versionNumber.empty()) {
                JsonGenerator configurations;
                configurations.startObject(FINGERPRINT_KEY);
                configurations.addMember(PACKAGE_KEY, fingerprint.package);
                configurations.addMember(BUILD_TYPE_KEY, fingerprint.buildType);
                configurations.addMember(VERSION_NUMBER_KEY, fingerprint.versionNumber);
                configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, configurations.toString()});
            }
            return std::make_shared<CapabilityConfiguration>(configMap);
        }
        void AudioPlayer::doShutdown() {
            m_progressTimer.stop();
            m_executor.shutdown();
            executeStop();
            releaseMediaPlayer(m_currentlyPlaying);
            m_mediaPlayerFactory->removeObserver(shared_from_this());
            m_mediaPlayerFactory.reset();
            m_messageSender.reset();
            m_focusManager.reset();
            m_contextManager->setStateProvider(STATE, nullptr);
            m_contextManager.reset();
            clearPlayQueue(true);
            m_playbackRouter.reset();
            m_captionManager.reset();
        }
        bool AudioPlayer::parseDirectivePayload(shared_ptr<DirectiveInfo> info, Document* document) {
            ParseResult result = document->Parse(info->directive->getPayload().data());
            if (result) return true;
            ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", rapidjson::GetParseError_En(result.Code())).d("offset", result.Offset())
                .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }
        static audio::MixingBehavior getMixingBehavior(const string& messageID, const Value& payload, const shared_ptr<MetricRecorderInterface>& metricRecorder) {
            string interruptedBehavior;
            if (jsonUtils::retrieveValue(payload, "interruptedBehavior", &interruptedBehavior)) {
                if (interruptedBehavior == "PAUSE") return audio::MixingBehavior::BEHAVIOR_PAUSE;
                else if (interruptedBehavior == "ATTENUATE") return audio::MixingBehavior::BEHAVIOR_DUCK;
                else {
                    submitMetric(metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + UNSUPPORTED_INTERRUPTED_BEHAVIOR,
                        DataPointCounterBuilder{}.setName(UNSUPPORTED_INTERRUPTED_BEHAVIOR).increment(1).build(), messageID,"");
                }
            } else {
                submitMetric(metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + MISSING_INTERRUPTED_BEHAVIOR,
                    DataPointCounterBuilder{}.setName(MISSING_INTERRUPTED_BEHAVIOR).increment(1).build(), messageID,"");
            }
            ACSDK_ERROR(LX("getMixingBehaviorFailed").d("reason", "interruptedBehavior flag missing").m("using Default Behavior (PAUSE)"));
            return DEFAULT_MIXING_BEHAVIOR;
        }
        void AudioPlayer::preHandlePlayDirective(shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG1(LX("preHandlePlayDirective"));
            ACSDK_DEBUG9(LX("prePLAY").d("payload", info->directive->getPayload()));
            Document payload;
            if (!parseDirectivePayload(info, &payload)) return;
            shared_ptr<PlayDirectiveInfo> playItem = make_shared<PlayDirectiveInfo>(info->directive->getMessageId());
            if (!jsonUtils::retrieveValue(payload, "playBehavior", &playItem->playBehavior)) playItem->playBehavior = PlayBehavior::ENQUEUE;
            Value::ConstMemberIterator audioItemJson;
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            if (!jsonUtils::findNode(_payload, "audioItem", &audioItemJson)) {
                ACSDK_ERROR(LX("preHandlePlayDirectiveFailed").d("reason", "missingAudioItem").d("messageId", playItem->messageId));
                sendExceptionEncounteredAndReportFailed(info, "missing AudioItem");
                return;
            }
            AudioItem audioItem;
            if (!jsonUtils::retrieveValue(audioItemJson->value, "audioItemId", &audioItem.id)) audioItem.id = "anonymous";
            Value::ConstMemberIterator stream;
            if (!jsonUtils::findNode(audioItemJson->value, "stream", &stream)) {
                ACSDK_ERROR(LX("preHandlePlayDirectiveFailed").d("reason", "missingStream").d("messageId", playItem->messageId));
                sendExceptionEncounteredAndReportFailed(info, "missing stream");
                return;
            }
            playItem->mixingBehavior = getMixingBehavior(info->directive->getMessageId(), stream->value, m_metricRecorder);
            if (!jsonUtils::retrieveValue(stream->value, "url", &audioItem.stream.url)) {
                ACSDK_ERROR(LX("preHandlePlayDirectiveFailed").d("reason", "missingUrl").d("messageId", playItem->messageId));
                sendExceptionEncounteredAndReportFailed(info, "missing URL");
                return;
            }
            if (!jsonUtils::retrieveValue(stream->value, "streamFormat", &audioItem.stream.format)) audioItem.stream.format = StreamFormat::AUDIO_MPEG;
            if (audioItem.stream.url.compare(0, CID_PREFIX.size(), CID_PREFIX) == 0) {
                string contentId = audioItem.stream.url.substr(CID_PREFIX.length());
                audioItem.stream.reader = info->directive->getAttachmentReader(contentId, sds::ReaderPolicy::NONBLOCKING);
                if (nullptr == audioItem.stream.reader) {
                    ACSDK_ERROR(LX("preHandlePlayDirectiveFailed").d("reason", "getAttachmentReaderFailed").d("messageId", playItem->messageId));
                    sendExceptionEncounteredAndReportFailed(info, "unable to obtain attachment reader");
                    return;
                }
                if (audioItem.stream.format != StreamFormat::AUDIO_MPEG) {
                    ACSDK_ERROR(LX("preHandlePlayDirectiveFailed").d("reason", "unsupportedFormat").d("format", audioItem.stream.format)
                        .d("messageId", playItem->messageId));
                    string message = "unsupported format " + streamFormatToString(audioItem.stream.format);
                    sendExceptionEncounteredAndReportFailed(info, message);
                    return;
                }
            }
            int64_t milliseconds;
            if (jsonUtils::retrieveValue(stream->value, "offsetInMilliseconds", &milliseconds)) {
                audioItem.stream.offset = chrono::milliseconds(milliseconds);
            } else audioItem.stream.offset = chrono::milliseconds::zero();
            if (jsonUtils::retrieveValue(stream->value, "endOffsetInMilliseconds", &milliseconds)) {
                auto chronoMillis = chrono::milliseconds(milliseconds);
                if (chronoMillis <= audioItem.stream.offset) {
                    ACSDK_ERROR(LX("preHandlePlayDirectiveFailed").d("reason", "endOffsetLessThanOffset").d("offset", audioItem.stream.offset.count())
                        .d("endOffset", chronoMillis.count()));
                    string message = "bad endOffset value";
                    sendExceptionEncounteredAndReportFailed(info, message, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return;
                }
                audioItem.stream.endOffset = chronoMillis;
            } else audioItem.stream.endOffset = chrono::milliseconds::zero();
            string expiryTimeString;
            audioItem.stream.expiryTime = steady_clock::time_point::max();
            if (jsonUtils::retrieveValue(stream->value, "expiryTime", &expiryTimeString)) {
                int64_t unixTime;
                if (m_timeUtils.convert8601TimeStringToUnix(expiryTimeString, &unixTime)) {
                    int64_t currentTime;
                    if (m_timeUtils.getCurrentUnixTime(&currentTime)) {
                        std::chrono::seconds timeToExpiry(unixTime - currentTime);
                        audioItem.stream.expiryTime = std::chrono::steady_clock::now() + timeToExpiry;
                    }
                }
            }
            Value::ConstMemberIterator progressReport;
            audioItem.stream.progressReport.delay = ProgressTimer::getNoDelay();
            audioItem.stream.progressReport.interval = ProgressTimer::getNoInterval();
            if (!jsonUtils::findNode(stream->value, "progressReport", &progressReport)) progressReport = stream->value.MemberEnd();
            else {
                if (jsonUtils::retrieveValue(progressReport->value, "progressReportDelayInMilliseconds", &milliseconds)) {
                    audioItem.stream.progressReport.delay = std::chrono::milliseconds(milliseconds);
                }
                if (jsonUtils::retrieveValue(progressReport->value, "progressReportIntervalInMilliseconds", &milliseconds)) {
                    audioItem.stream.progressReport.interval = std::chrono::milliseconds(milliseconds);
                }
            }
            if (!jsonUtils::retrieveValue(stream->value, "token", &audioItem.stream.token)) audioItem.stream.token = "";
            if (!jsonUtils::retrieveValue(stream->value, "expectedPreviousToken", &audioItem.stream.expectedPreviousToken)) {
                audioItem.stream.expectedPreviousToken = "";
            }
            Value::ConstMemberIterator httpHeadersIterator;
            if (jsonUtils::findNode(stream->value, PlaybackContext::HTTP_HEADERS, &httpHeadersIterator)) {
                parseHeadersFromPlayDirective(httpHeadersIterator->value, audioItem);
            }
            auto messageId = info->directive->getMessageId();
            if (playItem->playBehavior != PlayBehavior::ENQUEUE && !messageId.empty()) {
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + PREPARE_PLAY_DIRECTIVE_RECEIVED,
                    DataPointCounterBuilder{}.setName(PREPARE_PLAY_DIRECTIVE_RECEIVED).increment(1).build(), messageId, audioItem.stream.token);
            }
            if (!m_captionManager) { ACSDK_DEBUG5(LX("captionsNotParsed").d("reason", "captionManagerIsNull")); }
            else {
                Value::ConstMemberIterator captionIterator;
                if (!jsonUtils::findNode(stream->value, CAPTION_KEY, &captionIterator)) {
                    captionIterator = stream->value.MemberEnd();
                    ACSDK_DEBUG3(LX("captionsNotParsed").d("reason", "keyNotFoundInPayload"));
                } else {
                    auto captionFormat = captions::CaptionFormat::UNKNOWN;
                    string captionFormatString;
                    if (jsonUtils::retrieveValue(captionIterator->value, CAPTION_TYPE_KEY, &captionFormatString)) {
                        captionFormat = captions::avsStringToCaptionFormat(captionFormatString);
                    } else { ACSDK_WARN(LX("captionParsingIncomplete").d("reason", "failedToParseField").d("field", "type")); }
                    string captionContentString;
                    if (!jsonUtils::retrieveValue(captionIterator->value, CAPTION_CONTENT_KEY, &captionContentString)) {
                        ACSDK_WARN(LX("captionParsingIncomplete").d("reason", "failedToParseField").d("field", "content"));
                    }
                    auto captionData = captions::CaptionData(captionFormat, captionContentString);
                    ACSDK_DEBUG5(LX("captionPayloadParsed").d("type", captionData.format));
                    audioItem.captionData = captions::CaptionData(captionFormat, captionContentString);
                }
            }
            Value::ConstMemberIterator playRequestorJson;
            Value __payload{payload.GetString(), strlen(payload.GetString())};
            if (jsonUtils::findNode(__payload, "playRequestor", &playRequestorJson)) {
                if (!jsonUtils::retrieveValue(playRequestorJson->value, "type", &playItem->playRequestor.type)) {
                    ACSDK_ERROR(LX("handlePlayDirectiveFailed").d("reason", "missingPlayRequestorType")
                                    .d("messageId", info->directive->getMessageId()));
                    sendExceptionEncounteredAndReportFailed(info, "missing playRequestor type field");
                    return;
                }
                if (!jsonUtils::retrieveValue(playRequestorJson->value, "id", &playItem->playRequestor.id)) {
                    ACSDK_ERROR(LX("handlePlayDirectiveFailed").d("reason", "missingPlayRequestorId")
                                    .d("messageId", info->directive->getMessageId()));
                    sendExceptionEncounteredAndReportFailed(info, "missing playRequestor id field");
                    return;
                }
            }
            playItem->audioItem = audioItem;
            m_executor.submit([this, info, playItem] {
                if (isMessageInQueue(playItem->messageId)) {
                    ACSDK_ERROR(LX("preHandlePlayDirectiveFailed").d("reason", "messageIdAlreadyInPreHandleQueue"));
                    sendExceptionEncounteredAndReportFailed(info,"duplicated messageId " + playItem->messageId,
                        ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return;
                }
                executePrePlay(playItem);
            });
        }
        void AudioPlayer::handlePlayDirective(std::shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG1(LX("handlePlayDirective"));
            ACSDK_DEBUG9(LX("PLAY").d("payload", info->directive->getPayload()));
            Document payload;
            if (!parseDirectivePayload(info, &payload)) return;
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            Value::ConstMemberIterator audioItemJson;
            if (!jsonUtils::findNode(_payload, "audioItem", &audioItemJson)) {
                ACSDK_ERROR(LX("handlePlayDirectiveFailed").d("reason", "missingAudioItem").d("messageId", info->directive->getMessageId()));
                sendExceptionEncounteredAndReportFailed(info, "missing AudioItem");
                return;
            }
            Value::ConstMemberIterator stream;
            if (!jsonUtils::findNode(audioItemJson->value, "stream", &stream)) {
                ACSDK_ERROR(LX("handlePlayDirectiveFailed").d("reason", "missingStream").d("messageId", info->directive->getMessageId()));
                sendExceptionEncounteredAndReportFailed(info, "missing stream");
                return;
            }
            PlayBehavior playBehavior;
            if (!jsonUtils::retrieveValue(payload, "playBehavior", &playBehavior)) playBehavior = PlayBehavior::ENQUEUE;
            string token;
            if (!jsonUtils::retrieveValue(stream->value, "token", &token)) token = "";
            setHandlingCompleted(info);
            auto messageId = info->directive->getMessageId();
            if (playBehavior != PlayBehavior::ENQUEUE && !messageId.empty()) {
                Value::ConstMemberIterator playRequestorJson;
                string requestorType;
                Value __payload{payload.GetString(), strlen(payload.GetString())};
                if (jsonUtils::findNode(__payload, "playRequestor", &playRequestorJson)) {
                    if (!jsonUtils::retrieveValue(playRequestorJson->value, "type", &requestorType)) {
                        ACSDK_WARN(LX("handlePlayDirectiveWarning").d("reason", "missingPlayRequestorType").d("messageId", info->directive->getMessageId()));
                    }
                }
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + PLAY_DIRECTIVE_RECEIVED,
                    DataPointCounterBuilder{}.setName(PLAY_DIRECTIVE_RECEIVED).increment(1).build(), messageId, token, requestorType);
            }
            m_executor.submit([this, messageId] { executePlay(messageId); });
        }
        void AudioPlayer::handleStopDirective(shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG1(LX("handleStopDirective"));
            submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + STOP_DIRECTIVE_RECEIVED,
                DataPointCounterBuilder{}.setName(STOP_DIRECTIVE_RECEIVED).increment(1).build(), info->directive->getMessageId(),
                         m_currentlyPlaying->audioItem.id);
            auto messageId = info->directive->getMessageId();
            setHandlingCompleted(info);
            m_executor.submit([this, messageId] { executeStop(messageId); });
        }
        void AudioPlayer::handleClearQueueDirective(shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG1(LX("handleClearQueue"));
            Document payload;
            if (!parseDirectivePayload(info, &payload)) return;
            ClearBehavior clearBehavior;
            if (!jsonUtils::retrieveValue(payload, "clearBehavior", &clearBehavior)) clearBehavior = ClearBehavior::CLEAR_ENQUEUED;
            setHandlingCompleted(info);
            m_executor.submit([this, clearBehavior] { executeClearQueue(clearBehavior); });
        }
        void AudioPlayer::handleUpdateProgressReportIntervalDirective(shared_ptr<DirectiveInfo> info) {
            ACSDK_DEBUG1(LX("handleUpdateProgressReportIntervalDirective"));
            Document payload;
            if (!parseDirectivePayload(info, &payload)) return;
            int64_t milliseconds;
            if (!jsonUtils::retrieveValue(payload, "progressReportIntervalInMilliseconds", &milliseconds)) {
                sendExceptionEncounteredAndReportFailed(info, "missing progressReportIntervalInMilliseconds");
                return;
            }
            setHandlingCompleted(info);
            m_executor.submit([this, milliseconds] { executeUpdateProgressReportInterval(chrono::milliseconds(milliseconds)); });
        }
        void AudioPlayer::removeDirective(std::shared_ptr<DirectiveInfo> info) {
            if (info->directive && info->result) {
                auto messageId = info->directive->getMessageId();
                CapabilityAgent::removeDirective(messageId);
            }
        }
        void AudioPlayer::setHandlingCompleted(shared_ptr<DirectiveInfo> info) {
            if (info && info->result) info->result->setCompleted();
            removeDirective(info);
        }
        void AudioPlayer::executeProvideState(bool sendToken, unsigned int stateRequestToken) {
            ACSDK_DEBUG(LX("executeProvideState").d("sendToken", sendToken).d("stateRequestToken", stateRequestToken));
            auto policy = StateRefreshPolicy::NEVER;
            if (PlayerActivity::PLAYING == m_currentActivity) policy = StateRefreshPolicy::ALWAYS;
            Document state(kObjectType);
            state.AddMember(TOKEN_KEY, m_currentlyPlaying->audioItem.stream.token.data(), state.GetAllocator());
            string count;
            ostringstream o;
            o << (int64_t)duration_cast<milliseconds>(getOffset()).count();
            count += o.str();
            state.AddMember(OFFSET_KEY, count.data(), state.GetAllocator());
            state.AddMember(ACTIVITY_KEY, playerActivityToString(m_currentActivity).data(), state.GetAllocator());
            Value _state{state.GetString(), strlen(state.GetString())};
            attachPlaybackAttributesIfAvailable(_state, state.GetAllocator());
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!state.Accept(writer)) {
                ACSDK_ERROR(LX("executeProvideState").d("reason", "writerRefusedJsonObject"));
                return;
            }
            SetStateResult result;
            if (sendToken) result = m_contextManager->setState(STATE, buffer.GetString(), policy, stateRequestToken);
            else result = m_contextManager->setState(STATE, buffer.GetString(), policy);
            if (result != SetStateResult::SUCCESS) {
                ACSDK_ERROR(LX("executeProvideState").d("reason", "contextManagerSetStateFailed").d("token", m_currentlyPlaying->audioItem.stream.token));
            }
        }
        bool AudioPlayer::executeStopDucking() {
            for (auto& it : m_audioChannelVolumeInterfaces) {
                if (!it->stopDucking()) {
                    ACSDK_WARN(LX("executeOnFocusChanged").m("Failed to Restore Audio Channel Volume"));
                    return false;
                } else { ACSDK_DEBUG9(LX("executeOnFocusChanged").m("Restored Audio Channel Volume")); }
            }
            return true;
        }
        bool AudioPlayer::executeStartDucking() {
            for (auto& it : m_audioChannelVolumeInterfaces) {
                if (it->startDucking()) { ACSDK_DEBUG9(LX("executeOnFocusChanged").m("Attenuated Audio Channel Volume")); }
                else {
                    ACSDK_WARN(LX("executeOnFocusChanged").m("Failed to Attenuate Audio Channel Volume"));
                    return false;
                }
            }
            return true;
        }
        void AudioPlayer::executeOnFocusChanged(FocusState newFocus, MixingBehavior behavior) {
            ACSDK_DEBUG1(LX("executeOnFocusChanged").d("from", m_focus).d("to", newFocus).d("m_currentActivity", m_currentActivity).d("MixingBehavior", behavior));
            if ((m_focus == newFocus) && (m_mixingBehavior == behavior)) return;
            auto previousFocus = m_focus;
            m_focus = newFocus;
            m_mixingBehavior = behavior;
            switch(newFocus) {
                case FocusState::FOREGROUND: {
                        if (m_isAcquireChannelRequestPending) m_isAcquireChannelRequestPending = false;
                        executeStopDucking();
                        if (m_isLocalResumePending) {
                            m_isLocalResumePending = false;
                            bool success = false;
                            if (m_currentlyPlaying->mediaPlayer && m_currentlyPlaying->sourceId != ERROR_SOURCE_ID) {
                                success = m_currentlyPlaying->mediaPlayer->play(m_currentlyPlaying->sourceId);
                            }
                            m_localResumeSuccess.set_value(success);
                            if (!success) {
                                ACSDK_ERROR(LX("executeOnFocusChangedFailed").d("reason", "localResumeFailed"));
                                m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                            }
                            return;
                        }
                        switch(m_currentActivity) {
                            case PlayerActivity::IDLE: case PlayerActivity::STOPPED: case PlayerActivity::FINISHED:
                                if (!m_audioPlayQueue.empty() && !m_isStartingPlayback) {
                                    ACSDK_DEBUG1(LX("executeOnFocusChanged").d("action", "playNextItem"));
                                    playNextItem();
                                }
                                return;
                            case PlayerActivity::PAUSED: {
                                    if (m_isStopCalled) {
                                        ACSDK_DEBUG1(LX("executeOnFocusChanged").d("action", "stoppingAlreadyDoNothing"));
                                        return;
                                    }
                                    ACSDK_DEBUG1(LX("executeOnFocusChanged").d("action", "resumeMediaPlayer"));
                                    if (!m_currentlyPlaying->mediaPlayer->resume(m_currentlyPlaying->sourceId)) {
                                        sendPlaybackFailedEvent(m_currentlyPlaying->audioItem.stream.token,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                                                       "failed to resume media player",getMediaPlayerState());
                                        ACSDK_ERROR(LX("executeOnFocusChangedFailed").d("reason", "resumeFailed"));
                                        m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                                        return;
                                    }
                                }
                                return;
                            case PlayerActivity::PLAYING: case PlayerActivity::BUFFER_UNDERRUN: break;
                        }
                    }
                    break;
                case FocusState::BACKGROUND:
                    if (m_isAcquireChannelRequestPending) m_isAcquireChannelRequestPending = false;
                    switch(m_currentActivity) {
                        case PlayerActivity::STOPPED:
                            if (m_playNextItemAfterStopped && !m_audioPlayQueue.empty()) {
                                m_playNextItemAfterStopped = false;
                                return;
                            }
                        case PlayerActivity::FINISHED: case PlayerActivity::IDLE:
                            if (previousFocus == FocusState::NONE && !m_audioPlayQueue.empty()) {
                                if (m_audioPlayQueue.front()->mixingBehavior == audio::MixingBehavior::BEHAVIOR_DUCK &&
                                    !m_isStartingPlayback) {
                                    ACSDK_DEBUG1(LX("transitionToBackgroundWhileNotPlayingAndNextItemDuckable").d("result", "playNextItemInDuck"));
                                    playNextItem();
                                }
                                else if (m_audioPlayQueue.front()->mixingBehavior == audio::MixingBehavior::BEHAVIOR_PAUSE) {
                                    ACSDK_DEBUG1(LX("transitionToBackgroundWhileNotPlayingAndNextItemNotDuckable")
                                        .d("result", "delayNextItemUntilForegroundAcquired"));
                                    return;
                                }
                            } else if (m_audioPlayQueue.empty()) {
                                ACSDK_DEBUG1(LX("transitionToBackGroundWhileNotPlaying"));
                                return;
                            }
                        case PlayerActivity::PAUSED: case PlayerActivity::PLAYING: case PlayerActivity::BUFFER_UNDERRUN: {
                                if (behavior == MixingBehavior::MAY_DUCK) {
                                    if (executeStartDucking())  return;
                                } else if (MixingBehavior::MUST_PAUSE != behavior) {
                                    ACSDK_WARN(LX("executeOnFocusChanged").d("Unhandled MixingBehavior", behavior));
                                }
                                if (m_currentlyPlaying->mediaPlayer) m_currentlyPlaying->mediaPlayer->pause(m_currentlyPlaying->sourceId);
                            }
                            return;
                    }
                    break;
                case FocusState::NONE:
                    executeStopDucking();
                    switch(m_currentActivity) {
                        case PlayerActivity::IDLE: case PlayerActivity::STOPPED: case PlayerActivity::FINISHED: return;
                        case PlayerActivity::PLAYING: case PlayerActivity::PAUSED: case PlayerActivity::BUFFER_UNDERRUN:
                            if (m_isAcquireChannelRequestPending) {
                                ACSDK_DEBUG1(LX("executeOnFocusChanged").d("action", "Ignore-Transient"));
                                return;
                            }
                            clearPlayQueue(false);
                            ACSDK_DEBUG1(LX("executeOnFocusChanged").d("action", "executeStop"));
                            executeStop();
                            return;
                    }
                    break;
            }
            ACSDK_WARN(LX("unexpectedExecuteOnFocusChanged").d("newFocus", newFocus).d("m_currentActivity", m_currentActivity));
        }
        void AudioPlayer::executeOnPlaybackStarted(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnPlaybackStarted").d("id", id).d("state", state));
            m_isStartingPlayback = false;
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_ERROR(LX("executeOnPlaybackStartedFailed").d("reason", "invalidSourceId").d("id", id).d("state", state)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            if (FocusState::NONE == m_focus) {
                ACSDK_WARN(LX(__func__).d("reason", "callbackAfterFocusLost").d("action", "stopping"));
                if (!m_currentlyPlaying->mediaPlayer->stop(m_currentlyPlaying->sourceId)) { ACSDK_ERROR(LX(__func__).d("reason", "stopFailed")); }
                return;
            }
            sendPlaybackStartedEvent(state);
            m_playbackRouter->useDefaultHandlerWith(shared_from_this());
            changeActivity(PlayerActivity::PLAYING);
            if (m_isAutoProgressing) {
                m_isAutoProgressing = false;
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + TRACK_TO_TRACK_TIME,
                    m_autoProgressTimeMetricData.setName(TRACK_TO_TRACK_TIME).stopDurationTimer().build(),"","");
            }
            m_progressTimer.start();
            if (m_mediaPlayerFactory->isMediaPlayerAvailable() && m_currentlyPlaying->isBuffered) sendPlaybackNearlyFinishedEvent(state);
            else m_okToRequestNextTrack = true;
        }
        void AudioPlayer::executeOnBufferingComplete(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG(LX("executeOnBufferingComplete").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                if (!m_audioPlayQueue.empty()) {
                    for (auto& it : m_audioPlayQueue) {
                        if (it->sourceId == id) {
                            it->isBuffered = true;
                            ACSDK_DEBUG2(LX(__func__).m("FullyBufferedBeforeStarted").d("id", id).d("state", state));
                            return;
                        }
                    }
                }
                ACSDK_DEBUG(LX("executeOnBufferingComplete").d("reason", "sourceIdDoesNotMatchCurrentTrack").d("id", id).d("state", state)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            if (m_mediaPlayerFactory->isMediaPlayerAvailable() && m_okToRequestNextTrack) {
                sendPlaybackNearlyFinishedEvent(state);
                m_okToRequestNextTrack = false;
            } else {
                m_currentlyPlaying->isBuffered = true;
                ACSDK_DEBUG2(LX(__func__).m("FullyBufferedBeforeStarted").d("id", id).d("state", state));
            }
        }
        void AudioPlayer::executeOnPlaybackStopped(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnPlaybackStopped").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_DEBUG(LX("executeOnPlaybackStopped").d("reason", "sourceIdDoesNotMatchCurrentTrack").d("id", id).d("state", state)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            m_isAutoProgressing = false;
            switch(m_currentActivity) {
                case PlayerActivity::PLAYING: case PlayerActivity::PAUSED: case PlayerActivity::BUFFER_UNDERRUN:
                    changeActivity(PlayerActivity::STOPPED);
                    m_progressTimer.stop();
                    sendPlaybackStoppedEvent(state);
                    m_okToRequestNextTrack = false;
                    m_isStopCalled = false;
                    if (!m_playNextItemAfterStopped || m_audioPlayQueue.empty()) handlePlaybackCompleted();
                    else if (avsCommon::avs::FocusState::FOREGROUND == m_focus) playNextItem();
                    return;
                case PlayerActivity::IDLE: case PlayerActivity::STOPPED: case PlayerActivity::FINISHED:
                    if (m_focus != FocusState::NONE) {
                        handlePlaybackCompleted();
                        return;
                    }
                    ACSDK_ERROR(LX("executeOnPlaybackStoppedFailed").d("reason", "alreadyStopped").d("m_currentActivity", m_currentActivity));
                    break;
            }
            ACSDK_ERROR(LX("executeOnPlaybackStoppedFailed").d("reason", "unexpectedActivity").d("m_currentActivity", m_currentActivity));
        }
        void AudioPlayer::executeOnPlaybackFinished(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnPlaybackFinished").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_ERROR(LX("executeOnPlaybackFinishedFailed").d("reason", "invalidSourceId").d("id", id).d("state", state)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            m_isAutoProgressing = false;
            switch(m_currentActivity) {
                case PlayerActivity::PLAYING:
                    changeActivity(PlayerActivity::FINISHED);
                    m_progressTimer.stop();
                    {
                        if (m_audioPlayQueue.empty()) releaseMediaPlayer(m_currentlyPlaying);
                        m_executor.submit([this, state] {
                            if (m_okToRequestNextTrack) {
                                sendPlaybackNearlyFinishedEvent(state);
                                m_okToRequestNextTrack = false;
                            }
                            sendPlaybackFinishedEvent(state);
                            m_okToRequestNextTrack = false;
                            if (m_audioPlayQueue.empty()) handlePlaybackCompleted();
                            else {
                                m_autoProgressTimeMetricData.startDurationTimer();
                                m_isAutoProgressing = true;
                                playNextItem();
                            }
                        });
                    }
                    return;
                case PlayerActivity::IDLE: case PlayerActivity::STOPPED: case PlayerActivity::PAUSED: case PlayerActivity::BUFFER_UNDERRUN:
                case PlayerActivity::FINISHED:
                    ACSDK_ERROR(LX("executeOnPlaybackFinishedFailed").d("reason", "notPlaying").d("m_currentActivity", m_currentActivity));
                    return;
            }
            ACSDK_ERROR(LX("executeOnPlaybackFinishedFailed").d("reason", "unexpectedActivity").d("m_currentActivity", m_currentActivity));
        }
        void AudioPlayer::handlePlaybackCompleted() {
            m_progressTimer.stop();
            if (m_focus != avsCommon::avs::FocusState::NONE) m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
        }
        void AudioPlayer::executeOnPlaybackError(SourceId id, const ErrorType& type, string error, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX(__func__).d("id", id).d("state", state).d("type", type).d("error", error));
            m_isStartingPlayback = false;
            if (id != m_currentlyPlaying->sourceId) {
                if (!m_audioPlayQueue.empty()) {
                    for (auto& it : m_audioPlayQueue) {
                        if (it->sourceId == id) {
                            it->errorMsg = move(error);
                            it->errorType = type;
                            ACSDK_WARN(LX(__func__).m("ErrorWhileBuffering").d("id", id).d("state", state));
                            return;
                        }
                    }
                }
                ACSDK_ERROR(LX(__func__).d("reason", "invalidSourceId").d("id", id).d("state", state).d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            if (m_isAutoProgressing) {
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + TRACK_PROGRESSION_FATAL,DataPointCounterBuilder{}.setName(TRACK_PROGRESSION_FATAL).increment(1).build(),
                             m_currentlyPlaying->messageId, m_currentlyPlaying->audioItem.id);
            } else {
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + MID_PLAYBACK_FATAL,DataPointCounterBuilder{}.setName(MID_PLAYBACK_FATAL).increment(1).build(),
                             m_currentlyPlaying->messageId, m_currentlyPlaying->audioItem.id);
            }
            m_isAutoProgressing = false;
            m_okToRequestNextTrack = false;
            m_progressTimer.stop();
            if (m_currentActivity != PlayerActivity::IDLE) {
                auto offsetDelta = getOffset() - m_currentlyPlaying->initialOffset;
                if (offsetDelta < milliseconds(500) || (m_currentActivity != PlayerActivity::STOPPED && m_currentActivity != PlayerActivity::FINISHED)) {
                    sendPlaybackFailedEvent(m_currentlyPlaying->audioItem.stream.token, type, error, state);
                }
            }
            submitAnnotatedMetric(m_metricRecorder, "MediaError", error);
            executeOnPlaybackStopped(m_currentlyPlaying->sourceId, state);
        }
        void AudioPlayer::executeOnPlaybackPaused(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnPlaybackPaused").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_ERROR(LX("executeOnPlaybackPausedFailed").d("reason", "invalidSourceId").d("id", id).d("state", state)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            m_progressTimer.pause();
            sendPlaybackPausedEvent(state);
            changeActivity(PlayerActivity::PAUSED);
        }
        void AudioPlayer::executeOnPlaybackResumed(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnPlaybackResumed").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_ERROR(LX("executeOnPlaybackResumedFailed").d("reason", "invalidSourceId").d("id", id).d("state", state)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            if (m_currentActivity == PlayerActivity::STOPPED) {
                ACSDK_ERROR(LX("executeOnPlaybackResumedAborted").d("reason", "currentActivity:STOPPED"));
                return;
            }
            sendPlaybackResumedEvent(state);
            m_progressTimer.resume();
            changeActivity(PlayerActivity::PLAYING);
        }
        void AudioPlayer::executeOnBufferUnderrun(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnBufferUnderrun").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_ERROR(LX("executeOnBufferUnderrunFailed").d("reason", "invalidSourceId").d("id", id).d("state", state)
                   .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            if (PlayerActivity::BUFFER_UNDERRUN == m_currentActivity) {
                ACSDK_ERROR(LX("executeOnBufferUnderrunFailed").d("reason", "alreadyInUnderrun"));
                return;
            }
            m_bufferUnderrunTimestamp = steady_clock::now();
            sendPlaybackStutterStartedEvent(state);
            m_progressTimer.pause();
            changeActivity(PlayerActivity::BUFFER_UNDERRUN);
        }
        void AudioPlayer::executeOnBufferRefilled(SourceId id, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnBufferRefilled").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_ERROR(LX("executeOnBufferRefilledFailed").d("reason", "invalidSourceId").d("id", id).d("state", state)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            sendPlaybackStutterFinishedEvent(state);
            m_progressTimer.resume();
            changeActivity(PlayerActivity::PLAYING);
        }
        void AudioPlayer::executeOnSeeked(SourceId id, const MediaPlayerState& startState, const MediaPlayerState& endState) {
            ACSDK_DEBUG1(LX(__func__).d("id", id).d("startState", startState).d("endState", endState));
            if (id != m_currentlyPlaying->sourceId) {
                ACSDK_ERROR(LX("executeOnSeekStartFailed").d("reason", "invalidSourceId").d("id", id).d("startState", startState).d("endState", endState)
                    .d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            if (m_currentActivity == PlayerActivity::BUFFER_UNDERRUN) {
                sendPlaybackStutterFinishedEvent(endState);
                m_progressTimer.resume();
                changeActivity(PlayerActivity::PLAYING);
            }
            sendPlaybackSeekedEvent(startState, endState);
        }
        void AudioPlayer::executeOnTags(SourceId id, shared_ptr<const VectorOfTags> vectorOfTags, const MediaPlayerState& state) {
            ACSDK_DEBUG1(LX("executeOnTags").d("id", id).d("state", state));
            if (id != m_currentlyPlaying->sourceId) {
                for (const auto& it : m_audioPlayQueue) {
                    if (it->sourceId == id) {
                        sendStreamMetadataExtractedEvent(it->audioItem, vectorOfTags, state);
                        return;
                    }
                }
                ACSDK_ERROR(LX("executeOnTags").d("reason", "invalidSourceId").d("id", id).d("state", state).d("m_sourceId", m_currentlyPlaying->sourceId));
                return;
            }
            sendStreamMetadataExtractedEvent(m_currentlyPlaying->audioItem, vectorOfTags, state);
        }
        void AudioPlayer::clearPlayQueue(const bool stopCurrentPlayer) {
            for (auto& it : m_audioPlayQueue) {
                if (it->mediaPlayer) {
                    if (!stopCurrentPlayer && it->sourceId == m_currentlyPlaying->sourceId) {
                        releaseMediaPlayer(it);
                        continue;
                    }
                    stopAndReleaseMediaPlayer(it);
                }
            }
            m_audioPlayQueue.clear();
        }
        void AudioPlayer::stopAndReleaseMediaPlayer(shared_ptr<PlayDirectiveInfo> playbackItem) {
            m_isStartingPlayback = false;
            if (playbackItem->mediaPlayer) playbackItem->mediaPlayer->stop(playbackItem->sourceId);
            releaseMediaPlayer(playbackItem);
        }
        void AudioPlayer::releaseMediaPlayer(shared_ptr<PlayDirectiveInfo> playbackItem) {
            if (playbackItem->mediaPlayer) {
                ACSDK_DEBUG5(LX(__func__).d("sourceId", playbackItem->sourceId));
                playbackItem->mediaPlayer->removeObserver(shared_from_this());
                if (!m_mediaPlayerFactory->releaseMediaPlayer(playbackItem->mediaPlayer)) {
                    ACSDK_ERROR(LX(__func__).m("releaseMediaPlayerFailed").d("reason", "Factory Release Failed - Invalid MediaPlayer"));
                }
                playbackItem->mediaPlayer.reset();
                playbackItem->sourceId = ERROR_SOURCE_ID;
            }
        }
        bool AudioPlayer::isMessageInQueue(const string& messageId) {
            for (const auto& it : m_audioPlayQueue) {
                if (messageId == it->messageId) return true;
            }
            return false;
        }
        bool AudioPlayer::configureMediaPlayer(shared_ptr<PlayDirectiveInfo>& playbackItem) {
            if (!playbackItem->mediaPlayer) {
                shared_ptr<MediaPlayerInterface> mediaPlayer = m_mediaPlayerFactory->acquireMediaPlayer();
                AudioPlayer::SourceId sourceId = ERROR_SOURCE_ID;
                if (mediaPlayer == nullptr) {
                    ACSDK_ERROR(LX("configureMediaPlayerFailed").d("reason", "nullMediaPlayer"));
                    return false;
                }
                if (playbackItem->audioItem.stream.reader) {
                    ACSDK_DEBUG9(LX("configureMediaPlayer"));
                    sourceId = mediaPlayer->setSource(std::move(playbackItem->audioItem.stream.reader));
                    if (ERROR_SOURCE_ID == sourceId) {
                        sendPlaybackFailedEvent(playbackItem->audioItem.stream.token,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                                       "failed to set attachment media source",getMediaPlayerState());
                        ACSDK_ERROR(LX("configureMediaPlayerFailed").d("reason", "setSourceFailed").d("type", "attachment"));
                        return false;
                    }
                } else {
                    ACSDK_DEBUG9(LX("configureMediaPlayer").d("offset", playbackItem->audioItem.stream.offset.count()));
                    SourceConfig cfg = emptySourceConfig();
                    cfg.endOffset = playbackItem->audioItem.stream.endOffset;
                    sourceId = mediaPlayer->setSource(playbackItem->audioItem.stream.url, playbackItem->audioItem.stream.offset, cfg,false,
                                                      playbackItem->audioItem.playbackContext);
                    if (ERROR_SOURCE_ID == sourceId) {
                        sendPlaybackFailedEvent(playbackItem->audioItem.stream.token,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                                       "failed to set URL media source",getMediaPlayerState());
                        ACSDK_ERROR(LX("configureMediaPlayerFailed").d("reason", "setSourceFailed").d("type", "URL"));
                        return false;
                    }
                }
                ACSDK_DEBUG5(LX(__func__).d("sourceId", sourceId).d("audioItemId", playbackItem->audioItem.id));
                playbackItem->mediaPlayer = mediaPlayer;
                playbackItem->sourceId = sourceId;
                playbackItem->mediaPlayer->addObserver(shared_from_this());
            }
            return true;
        }
        void AudioPlayer::executeOnReadyToProvideNextPlayer() {
            ACSDK_DEBUG1(LX(__func__).d("queueSize", m_audioPlayQueue.size()));
            if (!m_mediaPlayerFactory->isMediaPlayerAvailable()) {
                ACSDK_DEBUG1(LX(__func__).m("AvailablePlayerInUse"));
                return;
            }
            if (m_audioPlayQueue.empty() && m_okToRequestNextTrack) {
                sendPlaybackNearlyFinishedEvent(getMediaPlayerState());
                m_okToRequestNextTrack = false;
            } else {
                for (auto& it : m_audioPlayQueue) {
                    if (!it->mediaPlayer) {
                        ACSDK_INFO(LX(__func__).m("providing available MediaPlayer to queued item"));
                        configureMediaPlayer(it);
                        break;
                    }
                }
            }
        }
        void AudioPlayer::executePrePlay(std::shared_ptr<PlayDirectiveInfo> info) {
            ACSDK_DEBUG1(LX(__func__).d("messageId", info->messageId).d("preBufferBehavior", info->playBehavior));
            if (!info->audioItem.stream.expectedPreviousToken.empty()) {
                auto previousToken = m_audioPlayQueue.empty() ? m_currentlyPlaying->audioItem.stream.token : m_audioPlayQueue.back()->audioItem.stream.token;
                if (previousToken != info->audioItem.stream.expectedPreviousToken) {
                    ACSDK_INFO(LX("executePrePlayDropped").d("reason", "unexpectedPreviousToken").d("previous", previousToken)
                        .d("expected", info->audioItem.stream.expectedPreviousToken));
                    return;
                }
            }
            bool isNextItem = false;
            if (!isNextItem && !m_audioPlayQueue.empty() && info->playBehavior != PlayBehavior::ENQUEUE) {
                auto& existingItem = m_audioPlayQueue.front();
                isNextItem = (m_audioPlayQueue.size() == 1 && existingItem->audioItem.id == info->audioItem.id &&
                              compareUrlNotQuery(existingItem->audioItem.stream.url, info->audioItem.stream.url));
                if (!isNextItem && m_audioPlayQueue.size() > 1) {
                        for (auto& it : m_audioPlayQueue) {
                        if (it->audioItem.id == info->audioItem.id &&
                            compareUrlNotQuery(it->audioItem.stream.url, info->audioItem.stream.url)) {
                            isNextItem = true;
                            existingItem = it;
                            break;
                        }
                    }
                }
                if (isNextItem) {
                    ACSDK_DEBUG(LX(__func__).d("usingExistingBufferedItem", info->audioItem.id).d("sourceId", existingItem->sourceId));
                    info->mediaPlayer = existingItem->mediaPlayer;
                    info->sourceId = existingItem->sourceId;
                    existingItem->mediaPlayer.reset();
                }
            }
            if (!isNextItem) {
                if (m_mediaPlayerFactory->isMediaPlayerAvailable() && (m_audioPlayQueue.empty() || m_audioPlayQueue.front()->mediaPlayer)) {
                    ACSDK_DEBUG(LX(__func__).m("acquiringPlayerSetSource"));
                    if (!configureMediaPlayer(info)) return;
                } else { ACSDK_DEBUG5(LX(__func__).m("enqueueWithoutPlayer")); }
            }
            ACSDK_INFO(LX(__func__).d("enqueuing", info->audioItem.id).d("sourceId", info->sourceId).d("messageId", info->messageId));
            switch(info->playBehavior) {
                case PlayBehavior::REPLACE_ALL: case PlayBehavior::REPLACE_ENQUEUED: clearPlayQueue(false);
                case PlayBehavior::ENQUEUE:
                    info->queueTimeMetricData.startDurationTimer();
                    m_audioPlayQueue.push_back(info);
                    break;
            }
        }
        void AudioPlayer::executePlay(const string& messageId) {
            ACSDK_DEBUG1(LX(__func__));
            if (m_audioPlayQueue.empty()) {
                ACSDK_ERROR(LX("executePlayFailed").d("reason", "emptyPlayQueue"));
                return;
            }
            auto playItem = m_audioPlayQueue.front();
            if (playItem->playBehavior != PlayBehavior::ENQUEUE && playItem->messageId != messageId) {
                ACSDK_ERROR(LX("executePlayFailed").d("reason", "TrackNotHeadOfQueue"));
                return;
            }
            if (playItem->playBehavior == PlayBehavior::REPLACE_ALL) executeStop("", true);
            auto mixability = playItem->mixingBehavior == audio::MixingBehavior::BEHAVIOR_PAUSE ? avsCommon::avs::ContentType::NONMIXABLE : ContentType::MIXABLE;
            switch(m_currentActivity) {
                case PlayerActivity::IDLE: case PlayerActivity::STOPPED: case PlayerActivity::FINISHED:
                    if (FocusState::NONE == m_focus) {
                        auto activity = FocusManagerInterface::Activity::create(NAMESPACE, shared_from_this(), milliseconds::zero(), mixability);
                        if (m_isAcquireChannelRequestPending) {
                            ACSDK_INFO(LX("executePlay").d("No-op m_isAcquireChannelRequestPending", m_isAcquireChannelRequestPending));
                        } else if (m_focusManager->acquireChannel(CHANNEL_NAME, activity)) {
                            ACSDK_INFO(LX("executePlay").d("acquiring Channel", CHANNEL_NAME));
                            m_currentMixability = mixability;
                        } else {
                            ACSDK_ERROR(LX("executePlayFailed").d("reason", "CouldNotAcquireChannel"));
                            m_progressTimer.stop();
                            sendPlaybackFailedEvent(playItem->audioItem.stream.token,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                                           string("Could not acquire ") + CHANNEL_NAME + " for " + NAMESPACE,getMediaPlayerState());
                        }
                    }
                    return;
                case PlayerActivity::PAUSED:
                    if (m_focus == FocusState::BACKGROUND) {
                        auto activity = FocusManagerInterface::Activity::create(NAMESPACE, shared_from_this(), milliseconds::zero(), mixability);
                        if (m_focusManager->acquireChannel(CHANNEL_NAME, activity)) {
                            ACSDK_INFO(LX("executePlay").d("acquiring Channel", CHANNEL_NAME));
                            m_isAcquireChannelRequestPending = true;
                        }
                    }
                    return;
                case PlayerActivity::PLAYING: case PlayerActivity::BUFFER_UNDERRUN: return;
            }
            ACSDK_ERROR(LX("executePlayFailed").d("reason", "unexpectedActivity").d("m_currentActivity", m_currentActivity));
        }
        void AudioPlayer::playNextItem() {
            ACSDK_DEBUG1(LX(__func__).d("m_audioPlayQueue.size", m_audioPlayQueue.size()));
            m_progressTimer.stop();
            if (m_audioPlayQueue.empty()) {
                sendPlaybackFailedEvent(m_currentlyPlaying->audioItem.stream.token,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                               "queu(int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(offset).count()e is empty",getMediaPlayerState());
                ACSDK_ERROR(LX("playNextItemFailed").d("reason", "emptyQueue"));
                executeStop();
                return;
            }
            auto playItem = m_audioPlayQueue.front();
            m_audioPlayQueue.pop_front();
            submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + TRACK_TIME_ON_QUEUE,
                         playItem->queueTimeMetricData.setName(TRACK_TIME_ON_QUEUE).stopDurationTimer().build(), playItem->messageId, playItem->audioItem.id);
            releaseMediaPlayer(m_currentlyPlaying);
            m_currentlyPlaying = playItem;
            m_currentlyPlaying->initialOffset = m_currentlyPlaying->audioItem.stream.offset;
            m_offset = m_currentlyPlaying->initialOffset;
            auto mixability = m_currentlyPlaying->mixingBehavior == audio::MixingBehavior::BEHAVIOR_PAUSE ? ContentType::NONMIXABLE : ContentType::MIXABLE;
            if (m_currentMixability != mixability) {
                m_currentMixability = mixability;
                m_focusManager->modifyContentType(CHANNEL_NAME, NAMESPACE, mixability);
            }
            if (!m_currentlyPlaying->mediaPlayer) {
                if (m_mediaPlayerFactory->isMediaPlayerAvailable()) {
                    if (!configureMediaPlayer(m_currentlyPlaying)) return;
                }
            }
            if (!m_currentlyPlaying->mediaPlayer) {
                ACSDK_ERROR(LX("playNextItemFailed").m("playerNotConfigured"));
                sendPlaybackFailedEvent(m_currentlyPlaying->audioItem.stream.token,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                               "player not configured",getMediaPlayerState());
                return;
            }
            if (!m_currentlyPlaying->errorMsg.empty()) {
                ACSDK_ERROR(LX("playNextItemFailed").m("reportingPrebufferedError"));
                executeOnPlaybackError(m_currentlyPlaying->sourceId, m_currentlyPlaying->errorType,m_currentlyPlaying->errorMsg,getMediaPlayerState());
                return;
            }
            ACSDK_DEBUG1(LX(__func__).d("playingSourceId", m_currentlyPlaying->sourceId));
            if (!m_currentlyPlaying->mediaPlayer->play(m_currentlyPlaying->sourceId)) {
                executeOnPlaybackError(m_currentlyPlaying->sourceId,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,"playFailed",getMediaPlayerState());
                return;
            }
            m_isStartingPlayback = true;
            if (m_captionManager && m_currentlyPlaying->audioItem.captionData.isValid()) {
                m_captionManager->onCaption(m_currentlyPlaying->sourceId, m_currentlyPlaying->audioItem.captionData);
            }
            m_progressTimer.init(shared_from_this(),m_currentlyPlaying->audioItem.stream.progressReport.delay,
                         m_currentlyPlaying->audioItem.stream.progressReport.interval,m_currentlyPlaying->initialOffset);
        }
        void AudioPlayer::executeStop(const std::string& messageId, bool playNextItem) {
            ACSDK_DEBUG1(LX("executeStop").d("playNextItem", playNextItem).d("m_currentActivity", m_currentActivity).d("sourceId", m_currentlyPlaying->sourceId));
            m_currentlyPlaying->stopMessageId = messageId;
            switch(m_currentActivity) {
                case PlayerActivity::IDLE: case PlayerActivity::STOPPED: case PlayerActivity::FINISHED: return;
                case PlayerActivity::PLAYING: case PlayerActivity::PAUSED: case PlayerActivity::BUFFER_UNDERRUN:
                    getOffset();
                    m_playNextItemAfterStopped = playNextItem;
                    if (m_currentlyPlaying->mediaPlayer && !m_currentlyPlaying->mediaPlayer->stop(m_currentlyPlaying->sourceId)) {
                        ACSDK_ERROR(LX("executeStopFailed").d("reason", "stopFailed").d("sourceId", m_currentlyPlaying->sourceId));
                    } else m_isStopCalled = true;
                    return;
            }
            ACSDK_ERROR(LX("executeStopFailed").d("reason", "unexpectedActivity").d("m_currentActivity", m_currentActivity));
        }
        void AudioPlayer::executeClearQueue(ClearBehavior clearBehavior) {
            ACSDK_DEBUG1(LX("executeClearQueue").d("clearBehavior", clearBehavior));
            switch (clearBehavior) {
                case ClearBehavior::CLEAR_ALL:
                    executeStop();
                    clearPlayQueue(false);
                    break;
                case ClearBehavior::CLEAR_ENQUEUED: clearPlayQueue(true); break;
            }
            sendPlaybackQueueClearedEvent();
        }
        void AudioPlayer::executeUpdateProgressReportInterval(milliseconds progressReportInterval) {
            ACSDK_DEBUG1(LX("executeUpdateProgressReportInterval").d("progressReportInterval", progressReportInterval.count()));
            m_progressTimer.updateInterval(progressReportInterval);
        }
        void AudioPlayer::executeLocalOperation(PlaybackOperation op, promise<bool> success) {
            ACSDK_DEBUG1(LX(__func__).d("m_currentActivity", m_currentActivity).d("sourceId", m_currentlyPlaying->sourceId).d("op", op));
            if (m_currentlyPlaying->mediaPlayer && m_currentlyPlaying->sourceId != ERROR_SOURCE_ID) {
                switch(m_currentActivity) {
                    case PlayerActivity::STOPPED:
                        if (op == PlaybackOperation::RESUME_PLAYBACK) {
                            if (FocusState::FOREGROUND != m_focus) {
                                auto activity = FocusManagerInterface::Activity::create(
                                    NAMESPACE, shared_from_this(), std::chrono::milliseconds::zero(), m_currentMixability);
                                if (m_isAcquireChannelRequestPending) {
                                    ACSDK_INFO(LX("executeLocalResume").d("No-op m_isAcquireChannelRequestPending", m_isAcquireChannelRequestPending));
                                    m_isLocalResumePending = true;
                                    m_localResumeSuccess.swap(success);
                                } else if (m_focusManager->acquireChannel(CHANNEL_NAME, activity)) {
                                    ACSDK_INFO(LX("executeLocalResume").d("acquiring Channel", CHANNEL_NAME));
                                    m_isAcquireChannelRequestPending = true;
                                    m_isLocalResumePending = true;
                                    m_localResumeSuccess.swap(success);
                                } else {
                                    ACSDK_ERROR(LX("executeLocalResumeFailed").d("reason", "CouldNotAcquireChannel"));
                                    m_progressTimer.stop();
                                    success.set_value(false);
                                    sendPlaybackFailedEvent(m_currentlyPlaying->audioItem.stream.token,ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                                                   std::string("Could not acquire ") + CHANNEL_NAME + " for " + NAMESPACE, getMediaPlayerState());
                                }
                            } else {
                                m_currentlyPlaying->initialOffset = getOffset();
                                success.set_value(m_currentlyPlaying->mediaPlayer->play(m_currentlyPlaying->sourceId));
                            }
                        } else success.set_value(true);
                        break;
                    case PlayerActivity::IDLE: case PlayerActivity::FINISHED: success.set_value(false); break;
                    case PlayerActivity::PLAYING: case PlayerActivity::PAUSED: case PlayerActivity::BUFFER_UNDERRUN:
                        if (op != PlaybackOperation::RESUME_PLAYBACK) {
                            m_currentlyPlaying->initialOffset = getOffset();
                            if (op == PlaybackOperation::STOP_PLAYBACK) m_isStopCalled = m_currentlyPlaying->mediaPlayer->stop(m_currentlyPlaying->sourceId);
                            else m_isStopCalled = m_currentlyPlaying->mediaPlayer->stop(m_currentlyPlaying->sourceId, LOCAL_STOP_DEFAULT_PIPELINE_OPEN_TIME);
                            success.set_value(m_isStopCalled);
                        } else success.set_value(true);
                }
            }
        }
        bool AudioPlayer::executeLocalSeekTo(milliseconds location, bool fromStart) {
            ACSDK_DEBUG1(LX(__func__).d("m_currentActivity", m_currentActivity).d("sourceId", m_currentlyPlaying->sourceId).d("seekTo", location.count())
                .d("fromStart", fromStart));
            switch(m_currentActivity) {
                case PlayerActivity::IDLE: case PlayerActivity::FINISHED: return false;
                case PlayerActivity::STOPPED: case PlayerActivity::PLAYING: case PlayerActivity::PAUSED: case PlayerActivity::BUFFER_UNDERRUN:
                    if (m_currentlyPlaying->mediaPlayer && m_currentlyPlaying->sourceId != ERROR_SOURCE_ID) {
                        return m_currentlyPlaying->mediaPlayer->seekTo(m_currentlyPlaying->sourceId, location, fromStart);
                    }
            }
            return false;
        }
        milliseconds AudioPlayer::getDuration() {
            if (m_currentlyPlaying->mediaPlayer && m_currentlyPlaying->sourceId != ERROR_SOURCE_ID) {
                auto state = m_currentlyPlaying->mediaPlayer->getMediaPlayerState(m_currentlyPlaying->sourceId);
                if (state.hasValue()) return state.value().duration;
            }
            ACSDK_ERROR(LX(__func__).m("getDurationFailed"));
            return milliseconds::zero();
        }
        void AudioPlayer::changeActivity(PlayerActivity activity) {
            ACSDK_DEBUG(LX("changeActivity").d("from", m_currentActivity).d("to", activity));
            unique_lock<mutex> lock(m_currentActivityMutex);
            auto oldActivity = m_currentActivity;
            m_currentActivity = activity;
            lock.unlock();
            m_currentActivityConditionVariable.notify_all();
            if (oldActivity != m_currentActivity && oldActivity == PlayerActivity::PLAYING) {
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + MEDIA_PLAYBACK_TIME,
                    m_playbackTimeMetricData.setName(MEDIA_PLAYBACK_TIME).stopDurationTimer().build(),"","");
            } else if (oldActivity != m_currentActivity && m_currentActivity == PlayerActivity::PLAYING) m_playbackTimeMetricData.startDurationTimer();
            executeProvideState();
            notifyObserver();
        }
        void AudioPlayer::sendEventWithTokenAndOffset(const string& eventName, bool includePlaybackReports, milliseconds offset) {
            rapidjson::Document payload(kObjectType);
            payload.AddMember(TOKEN_KEY, m_currentlyPlaying->audioItem.stream.token.data(), payload.GetAllocator());
            if (MEDIA_PLAYER_INVALID_OFFSET == offset) {
                offset = getOffset();
            }
            ACSDK_DEBUG1(LX("sendEventWithTokenAndOffset").d("eventName", eventName).d("offset", offset.count()).d("report", includePlaybackReports));
            string count;
            ostringstream o;
            o << (int64_t)duration_cast<milliseconds>(offset).count();
            count += o.str();
            payload.AddMember(OFFSET_KEY, count.data(), payload.GetAllocator());
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            attachPlaybackAttributesIfAvailable(_payload, payload.GetAllocator());
            if (includePlaybackReports) attachPlaybackReportsIfAvailable(_payload, payload.GetAllocator());
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!_payload.Accept(writer)) {
                ACSDK_ERROR(LX("sendEventWithTokenAndOffsetFailed").d("reason", "writerRefusedJsonObject"));
                return;
            }
            sendEvent(eventName, "", buffer.GetString());
        }
        void AudioPlayer::sendPlaybackStartedEvent(const MediaPlayerState& state) {
            sendEventWithTokenAndOffset("PlaybackStarted", false, m_currentlyPlaying->initialOffset);
        }
        void AudioPlayer::sendPlaybackNearlyFinishedEvent(const MediaPlayerState& state) {
            sendEventWithTokenAndOffset("PlaybackNearlyFinished", false, state.offset);
        }
        void AudioPlayer::sendPlaybackStutterStartedEvent(const MediaPlayerState& state) {
            sendEventWithTokenAndOffset("PlaybackStutterStarted", false, state.offset);
        }
        void AudioPlayer::sendPlaybackStutterFinishedEvent(const MediaPlayerState& state) {
            Document payload(kObjectType);
            payload.AddMember(TOKEN_KEY, m_currentlyPlaying->audioItem.stream.token.data(), payload.GetAllocator());
            ostringstream o;
            string countOffset;
            string _stutterDuration;
            o << (int64_t)duration_cast<milliseconds>(state.offset).count();
            countOffset += o.str();
            o.clear();
            payload.AddMember( OFFSET_KEY, countOffset.data(), payload.GetAllocator());
            auto stutterDuration = steady_clock::now() - m_bufferUnderrunTimestamp;
            o << (int64_t)duration_cast<milliseconds>(stutterDuration).count();
            _stutterDuration += o.str();
            payload.AddMember(STUTTER_DURATION_KEY, _stutterDuration.data(), payload.GetAllocator());
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            attachPlaybackAttributesIfAvailable(_payload, payload.GetAllocator());
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!payload.Accept(writer)) {
                ACSDK_ERROR(LX("sendPlaybackStutterFinishedEventFailed").d("reason", "writerRefusedJsonObject"));
                return;
            }
            sendEvent("PlaybackStutterFinished", "", buffer.GetString());
        }
        void AudioPlayer::sendPlaybackFinishedEvent(const MediaPlayerState& state) {
            sendEventWithTokenAndOffset("PlaybackFinished", true, state.offset);
        }
        void AudioPlayer::sendPlaybackFailedEvent(const string& failingToken, ErrorType errorType, const string& message, const MediaPlayerState& state) {
            Document payload(kObjectType);
            payload.AddMember(TOKEN_KEY, failingToken.data(), payload.GetAllocator());
            Value currentPlaybackState(kObjectType);
            Value tokenKey{TOKEN_KEY, strlen(TOKEN_KEY)};
            string streamToken = m_currentlyPlaying->audioItem.stream.token;
            Value token{streamToken.data(), strlen(streamToken.data())};
            currentPlaybackState.AddMember(tokenKey, token);
            ostringstream o;
            o << (int64_t)duration_cast<milliseconds>(state.offset).count();
            string stateOffset = o.str();
            Value offsetKey{OFFSET_KEY, strlen(OFFSET_KEY)};
            Value _stateOffset{stateOffset.data(), strlen(stateOffset.data())};
            currentPlaybackState.AddMember(offsetKey, _stateOffset);
            string _playerActivityToString = playerActivityToString(m_currentActivity).data();
            Value __playerActivityToString{_playerActivityToString.data(), strlen(_playerActivityToString.data())};
            Value activityKey{ACTIVITY_KEY, strlen(ACTIVITY_KEY)};
            currentPlaybackState.AddMember(activityKey, __playerActivityToString);
            attachPlaybackAttributesIfAvailable(currentPlaybackState, payload.GetAllocator());
            payload.AddMember("currentPlaybackState", currentPlaybackState, payload.GetAllocator());
            Value error(rapidjson::kObjectType);
            Value type{"type", strlen("type")};
            Value _errorTypeToString{errorTypeToString(errorType).data(), strlen(errorTypeToString(errorType).data())};
            error.AddMember(type, _errorTypeToString);
            Value _message{"message", strlen("message")};
            Value __message{message.data(), strlen(message.data())};
            error.AddMember(_message, __message);
            payload.AddMember("error", error, payload.GetAllocator());
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            attachPlaybackReportsIfAvailable(_payload, payload.GetAllocator());
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!_payload.Accept(writer)) {
                ACSDK_ERROR(LX("sendPlaybackStartedEventFailed").d("reason", "writerRefusedJsonObject"));
                return;
            }
            sendEvent("PlaybackFailed", "", buffer.GetString());
        }
        void AudioPlayer::sendPlaybackStoppedEvent(const MediaPlayerState& state) {
            sendEventWithTokenAndOffset("PlaybackStopped", true, state.offset);
        }
        void AudioPlayer::sendPlaybackPausedEvent(const MediaPlayerState& state) {
            sendEventWithTokenAndOffset("PlaybackPaused", false, state.offset);
        }

        void AudioPlayer::sendPlaybackResumedEvent(const MediaPlayerState& state) {
            sendEventWithTokenAndOffset("PlaybackResumed", false, state.offset);
        }
        void AudioPlayer::sendPlaybackSeekedEvent(const MediaPlayerState& startState, const MediaPlayerState& endState) {
            Document payload(rapidjson::kObjectType);
            payload.AddMember(TOKEN_KEY, m_currentlyPlaying->audioItem.stream.token.data(), payload.GetAllocator());
            ostringstream o;
            o << (int64_t)startState.offset.count();
            string startStateOffset = o.str();
            o.clear();
            o << (int64_t)endState.offset.count();
            string endStateOffset = o.str();
            payload.AddMember(SEEK_START_OFFSET_KEY, startStateOffset.data(), payload.GetAllocator());
            payload.AddMember(SEEK_END_OFFSET_KEY, endStateOffset.data(), payload.GetAllocator());
            payload.AddMember(ACTIVITY_KEY, playerActivityToString(m_currentActivity).data(), payload.GetAllocator());
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            attachPlaybackAttributesIfAvailable(_payload, payload.GetAllocator());
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            if (!_payload.Accept(writer)) {
                ACSDK_ERROR(LX("sendPlaybackSeekedEventFailed").d("reason", "writerRefusedJsonObject"));
                return;
            }
            if (m_currentActivity == PlayerActivity::PLAYING) {
                m_currentlyPlaying->initialOffset = endState.offset;
                sendPlaybackStartedEvent(endState);
            } else {
                sendPlaybackStoppedEvent(endState);
                m_currentlyPlaying->initialOffset = endState.offset;
            }
        }
        void AudioPlayer::sendPlaybackQueueClearedEvent() {
            sendEvent("PlaybackQueueCleared");
        }
        void AudioPlayer::sendStreamMetadataExtractedEvent(AudioItem& audioItem, shared_ptr<const VectorOfTags> vectorOfTags, const MediaPlayerState&) {
            const string& token = audioItem.stream.token;
            Document payload(kObjectType);
            payload.AddMember(TOKEN_KEY, token.data(), payload.GetAllocator());
            Value metadata(kObjectType);
            bool passFilter = false;
            VectorOfTags newCache(audioItem.cachedMetadata.begin(), audioItem.cachedMetadata.end());
            for (auto& tag : *vectorOfTags) {
                string lowerTag = tag.key;
                transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(), ::tolower);
                if (find(begin(METADATA_WHITELIST), std::end(METADATA_WHITELIST), lowerTag) != end(METADATA_WHITELIST) && !tag.value.empty()) {
                    bool dupFound = false;
                    for (auto iter = newCache.begin(); iter != newCache.end(); ++iter) {
                        if (iter->key == tag.key) {
                            if (iter->value == tag.value && iter->type == tag.type) dupFound = true;
                            else newCache.erase(iter);
                            break;
                        }
                    }
                    if (dupFound) continue;
                    passFilter = true;
                    newCache.push_back(tag);
                    Value tagKey(tag.key.c_str(), tag.key.length());
                    if (TagType::BOOLEAN == tag.type) {
                        string value = tag.value;
                        transform(value.begin(), value.end(), value.begin(), ::tolower);
                        if (value == "true") {
                            Value _true{true};
                            metadata.AddMember(tagKey, _true);
                        } else {
                            Value _false{false};
                            metadata.AddMember(tagKey, _false);
                        }
                    } else {
                        Value tagValue(tag.value.c_str(), tag.value.length());
                        metadata.AddMember(tagKey, tagValue);
                    }
                }
            }
            if (!passFilter) {
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + METADATA_UNFILTERED_ENCOUNTERED,
                    DataPointCounterBuilder{}.setName(METADATA_UNFILTERED_ENCOUNTERED).increment(1).build(),"", token);
                ACSDK_DEBUG(LX("sendStreamMetadataExtractedEvent").d("eventNotSent", "noWhitelistedData"));
                return;
            } else {
                submitMetric(m_metricRecorder, AUDIO_PLAYER_METRIC_PREFIX + METADATA_FILTERED_ENCOUNTERED,
                    DataPointCounterBuilder{}.setName(METADATA_FILTERED_ENCOUNTERED).increment(1).build(),"", token);
            }
            time_point<std::chrono::steady_clock> now = steady_clock::now();
            if (audioItem.lastMetadataEvent.time_since_epoch().count() == 0 ||
                now - audioItem.lastMetadataEvent > METADATA_EVENT_RATE) {
                audioItem.lastMetadataEvent = now;
                audioItem.cachedMetadata = newCache;
            } else {
                ACSDK_DEBUG(LX("sendStreamMetadataExtractedEvent").d("eventNotSent", "tooFrequent"));
                return;
            }
            payload.AddMember("metadata", metadata, payload.GetAllocator());
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            Value _payload{payload.GetString(), strlen(payload.GetString())};
            if (!_payload.Accept(writer)) {
                ACSDK_ERROR(LX("sendStreamMetadataExtractedEvent").d("reason", "writerRefusedJsonObject"));
                return;
            }
            sendEvent("StreamMetadataExtracted", "", buffer.GetString());
        }
        void AudioPlayer::notifyObserver() {
            AudioPlayerObserverInterface::Context context;
            context.audioItemId = m_currentlyPlaying->audioItem.id;
            context.offset = getOffset();
            context.playRequestor = m_currentlyPlaying->playRequestor;
            ACSDK_DEBUG1(LX("notifyObserver").d("playerActivity", playerActivityToString(m_currentActivity)));
            for (const auto& observer : m_observers) observer->onPlayerActivityChanged(m_currentActivity, context);
            if (m_renderPlayerObserver) {
                RenderPlayerInfoCardsObserverInterface::Context renderPlayerInfoContext;
                renderPlayerInfoContext.audioItemId = m_currentlyPlaying->audioItem.id;
                renderPlayerInfoContext.offset = getOffset();
                renderPlayerInfoContext.mediaProperties = shared_from_this();
                m_renderPlayerObserver->onRenderPlayerCardsInfoChanged(m_currentActivity, renderPlayerInfoContext);
            }
        }
        milliseconds AudioPlayer::getOffset() {
            if (m_currentlyPlaying->mediaPlayer && m_currentlyPlaying->sourceId != ERROR_SOURCE_ID) {
                auto offset = m_currentlyPlaying->mediaPlayer->getOffset(m_currentlyPlaying->sourceId);
                if (offset != MEDIA_PLAYER_INVALID_OFFSET) m_offset = offset;
            }
            return m_offset;
        }
        void AudioPlayer::sendEvent(const string& eventName, const string& dialogRequestIdString, const string& payload, const string& context) {
            auto event = buildJsonEventString(eventName, dialogRequestIdString, payload, context);
            auto request = std::make_shared<MessageRequest>(event.second, "");
            m_messageSender->sendMessage(request);
        }
        MediaPlayerState AudioPlayer::getMediaPlayerState() {
            MediaPlayerState state = MediaPlayerState(getOffset(), getDuration());
            return state;
        }
        unordered_set<shared_ptr<CapabilityConfiguration>> AudioPlayer::getCapabilityConfigurations() {
            return m_capabilityConfigurations;
        }
        void AudioPlayer::attachPlaybackAttributesIfAvailable(Value& parent, Document::AllocatorType& allocator) {
            if (!m_currentlyPlaying->mediaPlayer) return;
            Optional<PlaybackAttributes> playbackAttributes = m_currentlyPlaying->mediaPlayer->getPlaybackAttributes();
            if (!playbackAttributes.hasValue()) return;
            Value jsonPlaybackAttributes(kObjectType);
            Value nameKey{NAME_KEY, strlen(NAME_KEY)};
            Value name{playbackAttributes.value().name.data(), strlen(playbackAttributes.value().name.data())};
            Value codecKey{CODEC_KEY, strlen(CODEC_KEY)};
            Value codec{playbackAttributes.value().codec.data(), strlen(playbackAttributes.value().codec.data())};
            jsonPlaybackAttributes.AddMember(nameKey, name);
            jsonPlaybackAttributes.AddMember(codecKey, codec);
            ostringstream o;
            o << (int64_t)playbackAttributes.value().samplingRateInHertz;
            string samplingRateInHertz = o.str();
            o.clear();
            o << (int64_t)playbackAttributes.value().dataRateInBitsPerSecond;
            string dataRateInBitsPerSecond = o.str();
            Value samplingRateInHertzKey{SAMPLING_RATE_IN_HERTZ_KEY, strlen(SAMPLING_RATE_IN_HERTZ_KEY)};
            Value _samplingRateInHertz{samplingRateInHertz.data(), samplingRateInHertz.length()};
            Value dataRateInBitsPerSecondKey{DATA_RATE_IN_BITS_PER_SECOND_KEY, strlen(DATA_RATE_IN_BITS_PER_SECOND_KEY)};
            Value _dataRateInBitsPerSecond{dataRateInBitsPerSecond.data(), dataRateInBitsPerSecond.length()};
            jsonPlaybackAttributes.AddMember(samplingRateInHertzKey, _samplingRateInHertz);
            jsonPlaybackAttributes.AddMember(dataRateInBitsPerSecondKey, _dataRateInBitsPerSecond);
            Value playbackAttributesKey{PLAYBACK_ATTRIBUTES_KEY, strlen(PLAYBACK_ATTRIBUTES_KEY)};
            parent.AddMember(playbackAttributesKey, jsonPlaybackAttributes);
        }
        void AudioPlayer::attachPlaybackReportsIfAvailable(Value& parent, Document::AllocatorType& allocator) {
            if (!m_currentlyPlaying->mediaPlayer) return;
            vector<PlaybackReport> playbackReports = m_currentlyPlaying->mediaPlayer->getPlaybackReports();
            if (playbackReports.empty()) return;
            Value jsonPlaybackReports(kArrayType);
            ostringstream o;
            for (auto& report : playbackReports) {
                Value jsonReport(kObjectType);
                Value startOffsetKey{START_OFFSET_KEY, strlen(START_OFFSET_KEY)};
                o << (int64_t)report.startOffset.count();
                string _startOffset = o.str();
                Value startOffset{_startOffset.data(), _startOffset.length()};
                o.clear();
                Value endOffsetKey{END_OFFSET_KEY, strlen(END_OFFSET_KEY)};
                o << (int64_t)report.endOffset.count();
                string endOffset = o.str();
                Value _endOffset{endOffset.data(), endOffset.length()};
                jsonReport.AddMember(startOffsetKey, startOffset);
                jsonReport.AddMember(endOffsetKey, _endOffset);
                Value jsonPlaybackAttributes(kObjectType);
                Value nameKey{NAME_KEY, strlen(NAME_KEY)};
                Value name{report.playbackAttributes.name.data(), report.playbackAttributes.name.length()};
                Value codecKey{CODEC_KEY, strlen(CODEC_KEY)};
                Value codec{report.playbackAttributes.codec.data(), report.playbackAttributes.codec.length()};
                jsonPlaybackAttributes.AddMember(nameKey, name);
                jsonPlaybackAttributes.AddMember(codecKey, codec);
                o.clear();
                o << (int64_t)report.playbackAttributes.samplingRateInHertz;
                string samplingRateInHertz = o.str();
                o.clear();
                o << (int64_t)report.playbackAttributes.dataRateInBitsPerSecond;
                string dataRateInBitsPerSecond = o.str();
                o.clear();
                Value samplingRateInHertzKey{SAMPLING_RATE_IN_HERTZ_KEY, strlen(SAMPLING_RATE_IN_HERTZ_KEY)};
                Value _samplingRateInHertz{samplingRateInHertz.data(), samplingRateInHertz.length()};
                Value dataRateInBitsPerSecondKey{DATA_RATE_IN_BITS_PER_SECOND_KEY, strlen(DATA_RATE_IN_BITS_PER_SECOND_KEY)};
                Value _dataRateInBitsPerSecond{dataRateInBitsPerSecond.data(), dataRateInBitsPerSecond.length()};
                jsonPlaybackAttributes.AddMember(samplingRateInHertzKey, _samplingRateInHertz);
                jsonPlaybackAttributes.AddMember(dataRateInBitsPerSecondKey, _dataRateInBitsPerSecond);
                Value playbackAttributesKey{PLAYBACK_ATTRIBUTES_KEY, strlen(PLAYBACK_ATTRIBUTES_KEY)};
                jsonReport.AddMember(playbackAttributesKey, jsonPlaybackAttributes);
                jsonPlaybackReports.PushBack(jsonReport);
            }
            Value playbackReportKey{PLAYBACK_REPORTS_KEY, strlen(PLAYBACK_REPORTS_KEY)};
            parent.AddMember(playbackReportKey, jsonPlaybackReports);
        }
        void AudioPlayer::parseHeadersFromPlayDirective(const Value& httpHeaders, AudioItem& audioItem) {
            jsonUtils::retrieveStringMapFromArray(httpHeaders, PlaybackContext::HTTP_KEY_HEADERS, audioItem.playbackContext.keyConfig);
            jsonUtils::retrieveStringMapFromArray(httpHeaders, PlaybackContext::HTTP_MANIFEST_HEADERS, audioItem.playbackContext.manifestConfig);
            jsonUtils::retrieveStringMapFromArray(httpHeaders, PlaybackContext::HTTP_AUDIOSEGMENT_HEADERS, audioItem.playbackContext.audioSegmentConfig);
            jsonUtils::retrieveStringMapFromArray(httpHeaders, PlaybackContext::HTTP_ALL_HEADERS, audioItem.playbackContext.allConfig);
            if (validatePlaybackContextHeaders(&(audioItem.playbackContext))) {
                ACSDK_WARN(LX("validatePlaybackContext").m("Error in validation of headers-attempt to playback with valid data"));
            }
        }
    }
}