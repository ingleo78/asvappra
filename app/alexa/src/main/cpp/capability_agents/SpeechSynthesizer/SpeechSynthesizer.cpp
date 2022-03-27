#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/CapabilityConfiguration.h>
#include <json/JSONGenerator.h>
#include <logger/Logger.h>
#include <util/Metrics.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/DataPointStringBuilder.h>
#include <captions/CaptionData.h>
#include <captions/CaptionFormat.h>
#include "SpeechSynthesizer.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speechSynthesizer {
            using namespace logger;
            using namespace rapidjson;
            using namespace sds;
            using SpeakDirectiveInfo = SpeechSynthesizer::SpeakDirectiveInfo;
            using SpeechSynthesizerState = SpeechSynthesizerObserverInterface::SpeechSynthesizerState;
            using Activity = FocusManagerInterface::Activity;
            static const string SPEECHSYNTHESIZER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string SPEECHSYNTHESIZER_CAPABILITY_INTERFACE_NAME = "SpeechSynthesizer";
            static const string SPEECHSYNTHESIZER_CAPABILITY_INTERFACE_VERSION = "1.3";
            static const string TAG{"SpeechSynthesizer"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE{"SpeechSynthesizer"};
            static const NamespaceAndName SPEAK{NAMESPACE, "Speak"};
            static const NamespaceAndName CONTEXT_MANAGER_SPEECH_STATE{NAMESPACE, "SpeechState"};
            static const string CHANNEL_NAME = FocusManagerInterface::DIALOG_CHANNEL_NAME;
            static string SPEECH_STARTED_EVENT_NAME{"SpeechStarted"};
            static string SPEECH_FINISHED_EVENT_NAME{"SpeechFinished"};
            static string SPEECH_INTERRUPTED_EVENT_NAME{"SpeechInterrupted"};
            static const char KEY_URL[] = "url";
            static const char KEY_TOKEN[] = "token";
            static const char KEY_FORMAT[] = "format";
            static const char KEY_CAPTION[] = "caption";
            static const char KEY_CAPTION_TYPE[] = "type";
            static const char KEY_CAPTION_CONTENT[] = "content";
            static const char KEY_PLAY_BEHAVIOR[] = "playBehavior";
            static const string FORMAT{"AUDIO_MPEG"};
            static const string CID_PREFIX{"cid:"};
            static const char KEY_OFFSET_IN_MILLISECONDS[] = "offsetInMilliseconds";
            static const char KEY_PLAYER_ACTIVITY[] = "playerActivity";
            static const char KEY_ANALYZERS[] = "analyzers";
            static const char KEY_ANALYZERS_INTERFACE[] = "interface";
            static const char KEY_ANALYZERS_ENABLED[] = "enabled";
            static const char PLAYER_STATE_PLAYING[] = "PLAYING";
            static const char PLAYER_STATE_FINISHED[] = "FINISHED";
            static const char PLAYER_STATE_INTERRUPTED[] = "INTERRUPTED";
            static const seconds STATE_CHANGE_TIMEOUT{5};
            static const string POWER_RESOURCE_COMPONENT_NAME{"SpeechSynthesizer"};
            static const string SPEECH_SYNTHESIZER_METRIC_PREFIX = "SPEECH_SYNTHESIZER-";
            static const string FIRST_BYTES_AUDIO = "FIRST_BYTES_AUDIO";
            static const string TTS_STARTED = "TTS_STARTED";
            static const string TTS_FINISHED = "TTS_FINISHED";
            static const string DIALOG_REQUEST_ID_KEY = "DIALOG_REQUEST_ID";
            static const string BUFFER_UNDERRUN = "ERROR.TTS_BUFFER_UNDERRUN";
            static shared_ptr<CapabilityConfiguration> getSpeechSynthesizerCapabilityConfiguration();
            shared_ptr<SpeechSynthesizer> SpeechSynthesizer::create(shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                                    shared_ptr<MessageSenderInterface> messageSender,
                                                                    shared_ptr<FocusManagerInterface> focusManager,
                                                                    shared_ptr<ContextManagerInterface> contextManager,
                                                                    shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                    shared_ptr<avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder,
                                                                    shared_ptr<avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
                                                                    shared_ptr<captions::CaptionManagerInterface> captionManager,
                                                                    shared_ptr<PowerResourceManagerInterface> powerResourceManager) {
                if (!mediaPlayer) {
                    ACSDK_ERROR(LX("SpeechSynthesizerCreationFailed").d("reason", "mediaPlayerNullReference"));
                    return nullptr;
                }
                if (!messageSender) {
                    ACSDK_ERROR(LX("SpeechSynthesizerCreationFailed").d("reason", "messageSenderNullReference"));
                    return nullptr;
                }
                if (!focusManager) {
                    ACSDK_ERROR(LX("SpeechSynthesizerCreationFailed").d("reason", "focusManagerNullReference"));
                    return nullptr;
                }
                if (!contextManager) {
                    ACSDK_ERROR(LX("SpeechSynthesizerCreationFailed").d("reason", "contextManagerNullReference"));
                    return nullptr;
                }
                if (!exceptionSender) {
                    ACSDK_ERROR(LX("SpeechSynthesizerCreationFailed").d("reason", "exceptionSenderNullReference"));
                    return nullptr;
                }
                auto speechSynthesizer = shared_ptr<SpeechSynthesizer>(new SpeechSynthesizer(mediaPlayer, messageSender, focusManager,
                                                                       contextManager, metricRecorder, exceptionSender, captionManager,
                                                                       powerResourceManager));
                speechSynthesizer->init();
                dialogUXStateAggregator->addObserver(speechSynthesizer);
                return speechSynthesizer;
            }
            DirectiveHandlerConfiguration SpeechSynthesizer::getConfiguration() const {
                DirectiveHandlerConfiguration configuration;
                configuration[SPEAK] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                return configuration;
            }
            void SpeechSynthesizer::addObserver(shared_ptr<SpeechSynthesizerObserverInterface> observer) {
                ACSDK_DEBUG9(LX("addObserver").d("observer", observer.get()));
                m_executor.submit([this, observer]() { m_observers.insert(observer); });
            }
            void SpeechSynthesizer::removeObserver(shared_ptr<SpeechSynthesizerObserverInterface> observer) {
                ACSDK_DEBUG9(LX("removeObserver").d("observer", observer.get()));
                m_executor.submit([this, observer]() { m_observers.erase(observer); }).wait();
            }
            void SpeechSynthesizer::onDeregistered() {
                ACSDK_DEBUG9(LX("onDeregistered"));
            }
            void SpeechSynthesizer::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG9(LX("handleDirectiveImmediately").d("messageId", directive->getMessageId()));
                auto info = createDirectiveInfo(directive, nullptr);
                m_executor.submit([this, info]() { executeHandleImmediately(info); });
            }
            void SpeechSynthesizer::preHandleDirective(shared_ptr<DirectiveInfo> info) {
                if (!info || !info->directive) {
                    ACSDK_ERROR(LX("preHandleDirectiveFailed").d("reason", !info ? "nullInfo" : "nullDirective"));
                    return;
                }
                ACSDK_DEBUG9(LX("preHandleDirective").d("messageId", info->directive->getMessageId()));
                m_executor.submit([this, info]() { executePreHandle(info); });
            }
            void SpeechSynthesizer::handleDirective(shared_ptr<DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullInfo"));
                    return;
                }
                ACSDK_DEBUG9(LX("handleDirective").d("messageId", info->directive->getMessageId()));
                if (info->directive->getName() == "Speak") {
                    ACSDK_METRIC_MSG(TAG, info->directive, Metrics::Location::SPEECH_SYNTHESIZER_RECEIVE);
                }
                m_executor.submit([this, info]() { executeHandle(info); });
            }
            void SpeechSynthesizer::cancelDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG9(LX("cancelDirective").d("messageId", info->directive->getMessageId()));
                m_executor.submit([this, info]() { executeCancel(info); });
            }
            void SpeechSynthesizer::onFocusChanged(FocusState newFocus, MixingBehavior behavior) {
                unique_lock<mutex> lock(m_mutex);
                m_currentFocus = newFocus;
                if (m_currentState == m_desiredState) {
                    ACSDK_DEBUG(LX("onFocusChanged").d("newFocus", newFocus).d("result", "skip").d("state", m_currentState));
                    return;
                }
                ACSDK_DEBUG(LX("onFocusChanged").d("newFocus", newFocus).d("MixingBehavior", behavior));
                auto desiredState = m_desiredState;
                switch(newFocus) {
                    case FocusState::FOREGROUND:
                        setCurrentStateLocked(SpeechSynthesizerState::GAINING_FOCUS);
                        break;
                    case FocusState::BACKGROUND:
                        setCurrentStateLocked(SpeechSynthesizerState::LOSING_FOCUS);
                    case FocusState::NONE:
                        if (SpeechSynthesizerState::INTERRUPTED == m_currentState ||
                            SpeechSynthesizerState::FINISHED == m_currentState) {
                            ACSDK_DEBUG5(LX(__func__).d("result", "skip").d("state", m_currentState));
                            return;
                        }
                        desiredState = SpeechSynthesizerState::INTERRUPTED;
                        break;
                }
                auto currentInfo = make_shared<std::shared_ptr<SpeakDirectiveInfo>>(nullptr);
                m_executor.submit([this, desiredState, currentInfo]() {
                    *currentInfo = m_currentInfo;
                    executeStateChange(desiredState);
                });
                if (m_waitOnStateChange.wait_for(lock, STATE_CHANGE_TIMEOUT, [this, desiredState]() {
                        return m_currentState == desiredState;
                    })) {
                    ACSDK_DEBUG9(LX("onFocusChangedSuccess"));
                } else {
                    ACSDK_ERROR(LX("onFocusChangeFailed").d("reason", "stateChangeTimeout").d("initialDesiredState", desiredState)
                        .d("desiredState", m_desiredState).d("currentState", m_currentState));
                    m_executor.submit([this, currentInfo]() {
                        ACSDK_DEBUG9(LX("onFocusChangedLambda").d("currentInfo", *currentInfo && (*currentInfo)->directive ?
                                     (*currentInfo)->directive->getMessageId() : "null").d("m_currentInfo", m_currentInfo && m_currentInfo->directive ?
                                     m_currentInfo->directive->getMessageId() : "null"));
                        if (m_currentInfo && (m_currentInfo == *currentInfo)) {
                            string error{"stateChangeTimeout"};
                            if (m_currentInfo->directive) error += " messageId=" + m_currentInfo->directive->getMessageId();
                            sendExceptionEncounteredAndReportFailed(m_currentInfo,ExceptionErrorType::INTERNAL_ERROR, error);
                        }
                    });
                }
            }
            void SpeechSynthesizer::provideState(const NamespaceAndName& stateProviderName, const unsigned int stateRequestToken) {
                ACSDK_DEBUG9(LX("provideState").d("token", stateRequestToken));
                m_executor.submit([this, stateRequestToken]() {
                    lock_guard<std::mutex> lock(m_mutex);
                    executeProvideStateLocked(stateRequestToken);
                });
            }
            void SpeechSynthesizer::onContextAvailable(const string& jsonContext) {
                ACSDK_DEBUG9(LX("onContextAvailable").d("context", jsonContext));
            }
            void SpeechSynthesizer::onContextFailure(const ContextRequestError error) {
                ACSDK_DEBUG9(LX("onContextFailure").d("error", error));
            }
            void SpeechSynthesizer::onFirstByteRead(SourceId id, const MediaPlayerState&) {
                ACSDK_DEBUG(LX(__func__).d("id", id));
                submitMetric(MetricEventBuilder{}.setActivityName(SPEECH_SYNTHESIZER_METRIC_PREFIX + FIRST_BYTES_AUDIO)
                    .addDataPoint(DataPointCounterBuilder{}.setName(FIRST_BYTES_AUDIO).increment(1).build()));
            }
            void SpeechSynthesizer::onPlaybackStarted(SourceId id, const MediaPlayerState&) {
                ACSDK_DEBUG9(LX("onPlaybackStarted").d("callbackSourceId", id));
                ACSDK_METRIC_IDS(TAG, "SpeechStarted", "", "", Metrics::Location::SPEECH_SYNTHESIZER_RECEIVE);
                m_executor.submit([this, id] {
                    if (id != m_mediaSourceId) {
                        ACSDK_ERROR(LX("queueingExecutePlaybackStartedFailed").d("reason", "mismatchSourceId").d("callbackSourceId", id)
                            .d("sourceId", m_mediaSourceId));
                        executePlaybackError(ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "executePlaybackStartedFailed");
                    } else {
                        submitMetric(MetricEventBuilder{}.setActivityName(SPEECH_SYNTHESIZER_METRIC_PREFIX + TTS_STARTED)
                            .addDataPoint(DataPointCounterBuilder{}.setName(TTS_STARTED).increment(1).build())
                            .addDataPoint(DataPointStringBuilder{}.setName(DIALOG_REQUEST_ID_KEY)
                            .setValue(m_currentInfo->directive->getDialogRequestId()).build()));
                        executePlaybackStarted();
                    }
                });
            }
            void SpeechSynthesizer::onPlaybackFinished(SourceId id, const MediaPlayerState&) {
                ACSDK_DEBUG9(LX("onPlaybackFinished").d("callbackSourceId", id));
                ACSDK_METRIC_IDS(TAG, "SpeechFinished", "", "", Metrics::Location::SPEECH_SYNTHESIZER_RECEIVE);
                m_executor.submit([this, id] {
                    if (id != m_mediaSourceId) {
                        ACSDK_ERROR(LX("queueingExecutePlaybackFinishedFailed").d("reason", "mismatchSourceId").d("callbackSourceId", id)
                            .d("sourceId", m_mediaSourceId));
                        executePlaybackError(ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "executePlaybackFinishedFailed");
                    } else {
                        submitMetric(MetricEventBuilder{}.setActivityName(SPEECH_SYNTHESIZER_METRIC_PREFIX + TTS_FINISHED)
                            .addDataPoint(DataPointCounterBuilder{}.setName(TTS_FINISHED).increment(1).build())
                            .addDataPoint(DataPointStringBuilder{}.setName(DIALOG_REQUEST_ID_KEY)
                            .setValue(m_currentInfo->directive->getDialogRequestId()).build()));
                        executePlaybackFinished();
                    }
                });
            }
            void SpeechSynthesizer::onPlaybackError(SourceId id, const ErrorType& type, string error, const MediaPlayerState&) {
                ACSDK_DEBUG9(LX("onPlaybackError").d("callbackSourceId", id));
                m_executor.submit([this, type, error]() { executePlaybackError(type, error); });
            }
            void SpeechSynthesizer::onPlaybackStopped(SourceId id, const MediaPlayerState&) {
                ACSDK_DEBUG9(LX("onPlaybackStopped").d("callbackSourceId", id));
                m_executor.submit([this, id]() {
                    if (m_currentInfo && m_mediaSourceId == id) {
                        m_currentInfo->sendCompletedMessage = false;
                        if (m_currentInfo->result && !m_currentInfo->isSetFailedCalled) {
                            m_currentInfo->result->setFailed("Stopped due to MediaPlayer stopping.");
                            m_currentInfo->isSetFailedCalled = true;
                        }
                        executePlaybackFinished();
                    } else executePlaybackError(ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "UnexpectedId");
                });
            }
            void SpeechSynthesizer::onBufferUnderrun(SourceId id, const MediaPlayerState&) {
                ACSDK_WARN(LX("onBufferUnderrun").d("callbackSourceId", id));
                submitMetric(MetricEventBuilder{}.setActivityName(SPEECH_SYNTHESIZER_METRIC_PREFIX + BUFFER_UNDERRUN)
                    .addDataPoint(DataPointCounterBuilder{}.setName(BUFFER_UNDERRUN).increment(1).build()));
            }
            SpeakDirectiveInfo::SpeakDirectiveInfo(shared_ptr<DirectiveInfo> directiveInfo) : directive{directiveInfo->directive},
                                                   result{directiveInfo->result}, sendPlaybackStartedMessage{false},
                                                   sendPlaybackFinishedMessage{false}, sendCompletedMessage{false},
                                                   isSetFailedCalled{false}, isPlaybackInitiated{false}, isHandled{false},
                                                   playBehavior{PlayBehavior::REPLACE_ALL} {}
            void SpeakDirectiveInfo::clear() {
                attachmentReader.reset();
                sendPlaybackStartedMessage = false;
                sendPlaybackFinishedMessage = false;
                sendCompletedMessage = false;
                isSetFailedCalled = false;
                isPlaybackInitiated = false;
            }
            SpeechSynthesizer::SpeechSynthesizer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<MessageSenderInterface> messageSender,
                                                 shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ContextManagerInterface> contextManager,
                                                 shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                 shared_ptr<CaptionManagerInterface> captionManager, shared_ptr<PowerResourceManagerInterface> powerResourceManager) :
                                                 CapabilityAgent{NAMESPACE, exceptionSender}, RequiresShutdown{"SpeechSynthesizer"},
                                                 m_mediaSourceId{MediaPlayerInterface::ERROR}, m_offsetInMilliseconds{0},
                                                 m_speechPlayer{mediaPlayer}, m_metricRecorder{metricRecorder}, m_messageSender{messageSender},
                                                 m_focusManager{focusManager}, m_contextManager{contextManager}, m_captionManager{captionManager},
                                                 m_currentState{SpeechSynthesizerState::FINISHED}, m_desiredState{SpeechSynthesizerState::FINISHED},
                                                 m_currentFocus{FocusState::NONE}, m_isShuttingDown{false}, m_initialDialogUXStateReceived{false},
                                                 m_powerResourceManager{powerResourceManager} {
                m_capabilityConfigurations.insert(getSpeechSynthesizerCapabilityConfiguration());
            }
            shared_ptr<CapabilityConfiguration> getSpeechSynthesizerCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, SPEECHSYNTHESIZER_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, SPEECHSYNTHESIZER_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, SPEECHSYNTHESIZER_CAPABILITY_INTERFACE_VERSION});
                return make_shared<CapabilityConfiguration>(configMap);
            }
            void SpeechSynthesizer::doShutdown() {
                ACSDK_DEBUG9(LX("doShutdown"));
                {
                    lock_guard<mutex> lock(m_speakInfoQueueMutex);
                    m_isShuttingDown = true;
                }
                m_contextManager->removeStateProvider(CONTEXT_MANAGER_SPEECH_STATE);
                m_executor.shutdown();
                m_speechPlayer->removeObserver(shared_from_this());
                {
                    unique_lock<mutex> lock(m_mutex);
                    if (SpeechSynthesizerState::PLAYING == m_currentState) {
                        m_desiredState = SpeechSynthesizerState::INTERRUPTED;
                        if (m_currentInfo) {
                            m_currentInfo->sendPlaybackFinishedMessage = false;
                        }
                        lock.unlock();
                        stopPlaying();
                        releaseForegroundFocus();
                        lock.lock();
                        m_currentState = SpeechSynthesizerState::INTERRUPTED;
                    }
                }
                {
                    lock_guard<mutex> lock(m_speakInfoQueueMutex);
                    if (m_currentInfo) m_speakInfoQueue.push_front(m_currentInfo);
                    for (auto& info : m_speakInfoQueue) {
                        if (info->result && !info->isSetFailedCalled) info->result->setFailed("SpeechSynthesizerShuttingDown");
                        removeSpeakDirectiveInfo(info->directive->getMessageId());
                        removeDirective(info->directive->getMessageId());
                    }
                }
                m_speechPlayer.reset();
                m_waitOnStateChange.notify_one();
                m_messageSender.reset();
                m_focusManager.reset();
                m_contextManager.reset();
                m_observers.clear();
            }
            void SpeechSynthesizer::init() {
                m_speechPlayer->addObserver(shared_from_this());
                m_contextManager->setStateProvider(CONTEXT_MANAGER_SPEECH_STATE, shared_from_this());
            }
            void SpeechSynthesizer::executeHandleImmediately(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG(LX("executeHandleImmediately").d("messageId", info->directive->getMessageId()));
                auto speakInfo = validateInfo("executeHandleImmediately", info, false);
                if (!speakInfo) {
                    ACSDK_ERROR(LX("executeHandleImmediatelyFailed").d("reason", "invalidDirective"));
                    return;
                }
                executePreHandleAfterValidation(speakInfo);
                executeHandleAfterValidation(speakInfo);
            }
            void SpeechSynthesizer::executePreHandleAfterValidation(shared_ptr<SpeakDirectiveInfo> speakInfo) {
                if (speakInfo->directive->getName() != SPEAK.name) {
                    const string message("unexpectedDirective " + speakInfo->directive->getName());
                    ACSDK_ERROR(LX("executePreHandleFailed").d("reason", "unexpectedDirective")
                        .d("directiveName", speakInfo->directive->getName()));
                    sendExceptionEncounteredAndReportFailed(speakInfo,ExceptionErrorType::UNSUPPORTED_OPERATION, message);
                    return;
                }
                Document payload;
                if (payload.Parse(speakInfo->directive->getPayload().data()).HasParseError()) {
                    const string message("unableToParsePayload" + speakInfo->directive->getMessageId());
                    ACSDK_ERROR(LX("executePreHandleFailed").d("reason", message).d("messageId", speakInfo->directive->getMessageId()));
                    sendExceptionEncounteredAndReportFailed(speakInfo,ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, message);
                    return;
                }
                Value _payload{payload.GetString(), strlen(payload.GetString())};
                Value::ConstMemberIterator it = _payload.FindMember(KEY_TOKEN);
                if (_payload.MemberEnd() == it) {
                    sendExceptionEncounteredAndReportMissingProperty(speakInfo, KEY_TOKEN);
                    return;
                } else if (!(it->value.IsString())) {
                    sendExceptionEncounteredAndReportUnexpectedPropertyType(speakInfo, KEY_TOKEN);
                    return;
                }
                speakInfo->token = it->value.GetString();
                it = _payload.FindMember(KEY_FORMAT);
                if (_payload.MemberEnd() == it) {
                    sendExceptionEncounteredAndReportMissingProperty(speakInfo, KEY_FORMAT);
                    return;
                } else if (!(it->value.IsString())) {
                    sendExceptionEncounteredAndReportUnexpectedPropertyType(speakInfo, KEY_FORMAT);
                    return;
                }
                string format = it->value.GetString();
                if (format != FORMAT) {
                    const std::string message("unknownFormat " + speakInfo->directive->getMessageId() + " format " + format);
                    ACSDK_ERROR(LX("executePreHandleFailed").d("reason", "unknownFormat").d("messageId", speakInfo->directive->getMessageId())
                        .d("format", format));
                    sendExceptionEncounteredAndReportFailed(speakInfo,ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, message);
                }
                it = _payload.FindMember(KEY_URL);
                if (_payload.MemberEnd() == it) {
                    sendExceptionEncounteredAndReportMissingProperty(speakInfo, KEY_URL);
                    return;
                } else if (!(it->value.IsString())) {
                    sendExceptionEncounteredAndReportUnexpectedPropertyType(speakInfo, KEY_URL);
                    return;
                }
                string urlValue = it->value.GetString();
                auto contentIdPosition = urlValue.find(CID_PREFIX);
                if (contentIdPosition != 0) {
                    const string message("expectedCIDUrlPrefixNotFound");
                    ACSDK_ERROR(LX("executePreHandleFailed").d("reason", message).sensitive("url", urlValue));
                    sendExceptionEncounteredAndReportFailed(speakInfo, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, message);
                    return;
                }
                string contentId = urlValue.substr(contentIdPosition + CID_PREFIX.length());
                speakInfo->attachmentReader = speakInfo->directive->getAttachmentReader(contentId, ReaderPolicy::NONBLOCKING);
                if (!speakInfo->attachmentReader) {
                    const string message("getAttachmentReaderFailed");
                    ACSDK_ERROR(LX("executePreHandleFailed").d("reason", message));
                    sendExceptionEncounteredAndReportFailed(speakInfo,ExceptionErrorType::INTERNAL_ERROR, message);
                }
                it = _payload.FindMember(KEY_PLAY_BEHAVIOR);
                if (_payload.MemberEnd() != it) {
                    if (!(it->value.IsString())) {
                        sendExceptionEncounteredAndReportUnexpectedPropertyType(speakInfo, KEY_PLAY_BEHAVIOR);
                        return;
                    }
                    string behavior = it->value.GetString();
                    if (!stringToPlayBehavior(behavior, &(speakInfo->playBehavior))) {
                        const string message = "failedToParsePlayBehavior";
                        ACSDK_ERROR(LX("executePreHandleFailed").d("reason", message).d("behavior", behavior));
                        sendExceptionEncounteredAndReportFailed(speakInfo,ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, message);
                        return;
                    }
                } else speakInfo->playBehavior = PlayBehavior::REPLACE_ALL;
                if (!m_captionManager) { ACSDK_DEBUG5(LX("captionsNotParsed").d("reason", "captionManagerIsNull")); }
                else {
                    auto captionIterator = _payload.FindMember(KEY_CAPTION);
                    if (_payload.MemberEnd() != captionIterator) {
                        if (captionIterator->value.IsObject()) {
                            Value& captionsPayload = _payload[KEY_CAPTION];
                            auto captionFormat = captions::CaptionFormat::UNKNOWN;
                            captionIterator = captionsPayload.FindMember(KEY_CAPTION_TYPE);
                            if ((_payload.MemberEnd() != captionIterator) && (captionIterator->value.IsString())) {
                                captionFormat = captions::avsStringToCaptionFormat(captionIterator->value.GetString());
                            } else {
                                ACSDK_WARN(LX("captionParsingIncomplete").d("reason", "failedToParseField").d("field", "type"));
                            }
                            string captionContent;
                            captionIterator = captionsPayload.FindMember(KEY_CAPTION_CONTENT);
                            if ((_payload.MemberEnd() != captionIterator) && (captionIterator->value.IsString())) {
                                captionContent = captionIterator->value.GetString();
                            } else {
                                ACSDK_WARN(LX("captionParsingIncomplete").d("reason", "failedToParseField").d("field", "content"));
                            }
                            ACSDK_DEBUG3(LX("captionPayloadParsed").d("type", captionFormat));
                            speakInfo->captionData = captions::CaptionData(captionFormat, captionContent);
                        } else {
                            ACSDK_WARN(LX("captionsNotParsed").d("reason", "keyNotAnObject"));
                        }
                    } else {
                        ACSDK_DEBUG3(LX("captionsNotParsed").d("reason", "keyNotFoundInPayload"));
                    }
                }
                auto analyzerIterator = _payload.FindMember(KEY_ANALYZERS);
                if (_payload.MemberEnd() != analyzerIterator) {
                    const Value& analyzersPayload = _payload[KEY_ANALYZERS];
                    if (!analyzersPayload.IsArray()) {
                        ACSDK_WARN(LX("audioAnalyzerParsingIncomplete").d("reason", "NotAnArray").d("field", "analyzers"));
                    } else {
                        vector<AudioAnalyzerState> analyzersData;
                        for (auto itr = analyzersPayload.Begin(); itr != analyzersPayload.End(); ++itr) {
                            const Value& analyzerStatePayload = *itr;
                            auto nameIterator = analyzerStatePayload.FindMember(KEY_ANALYZERS_INTERFACE);
                            auto stateIterator = analyzerStatePayload.FindMember(KEY_ANALYZERS_ENABLED);
                            if (analyzerStatePayload.MemberEnd() != nameIterator &&
                                analyzerStatePayload.MemberEnd() != stateIterator) {
                                AudioAnalyzerState state(nameIterator->value.GetString(), stateIterator->value.GetString());
                                analyzersData.push_back(state);
                            }
                        }
                        speakInfo->analyzersData = analyzersData;
                    }
                }
                if (!setSpeakDirectiveInfo(speakInfo->directive->getMessageId(), speakInfo)) {
                    ACSDK_ERROR(LX("executePreHandleFailed").d("reason", "prehandleCalledTwiceOnSameDirective")
                        .d("messageId", speakInfo->directive->getMessageId()));
                    return;
                }
                addToDirectiveQueue(speakInfo);
            }
            void SpeechSynthesizer::executeHandleAfterValidation(shared_ptr<SpeakDirectiveInfo> speakInfo) {
                speakInfo->isHandled = true;
                if (m_currentInfo) {
                    ACSDK_DEBUG3(LX(__func__).d("result", "skip").d("reason", "cancellationInProgress"));
                    return;
                }
                {
                    lock_guard<mutex> lock(m_speakInfoQueueMutex);
                    if (m_speakInfoQueue.empty() ||
                        (speakInfo->directive->getMessageId() != m_speakInfoQueue.front()->directive->getMessageId())) {
                        ACSDK_ERROR(LX("executeHandleFailed").d("reason", "unexpectedDirective").d("messageId", speakInfo->directive->getMessageId())
                            .d("expected", m_speakInfoQueue.empty() ? string{"empty"} : m_speakInfoQueue.front()->directive->getMessageId()));
                        sendExceptionEncounteredAndReportFailed(speakInfo,ExceptionErrorType::INTERNAL_ERROR,"UnexpectedId " + speakInfo->directive->getMessageId());
                        return;
                    }
                    m_speakInfoQueue.pop_front();
                }
                m_currentInfo = speakInfo;
                setDesiredState(SpeechSynthesizerState::PLAYING);
                auto activity = Activity::create(NAMESPACE, shared_from_this(), milliseconds::zero(),ContentType::MIXABLE);
                if (!m_focusManager->acquireChannel(CHANNEL_NAME, activity)) {
                    static const string message = std::string("Could not acquire ") + CHANNEL_NAME + " for " + NAMESPACE;
                    ACSDK_ERROR(LX("executeHandleFailed").d("reason", "CouldNotAcquireChannel").d("messageId", m_currentInfo->directive->getMessageId()));
                    sendExceptionEncounteredAndReportFailed(speakInfo, avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR, message);
                    lock_guard<mutex> lock{m_mutex};
                    m_desiredState = m_currentState;
                }
            }
            void SpeechSynthesizer::executePreHandle(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG(LX("executePreHandle").d("messageId", info->directive->getMessageId()));
                auto speakInfo = validateInfo("executePreHandle", info);
                if (!speakInfo) {
                    ACSDK_ERROR(LX("executePreHandleFailed").d("reason", "invalidDirectiveInfo"));
                    return;
                }
                executePreHandleAfterValidation(speakInfo);
            }
            void SpeechSynthesizer::executeHandle(shared_ptr<DirectiveInfo> info) {
                auto speakInfo = validateInfo("executeHandle", info);
                if (!speakInfo) {
                    ACSDK_ERROR(LX("executeHandleFailed").d("reason", "invalidDirectiveInfo"));
                    return;
                }
                ACSDK_DEBUG(LX("executeHandle").d("messageId", info->directive->getMessageId()));
                executeHandleAfterValidation(speakInfo);
            }
            void SpeechSynthesizer::executeCancel(std::shared_ptr<DirectiveInfo> info) {
                auto speakInfo = validateInfo("executeCancel", info);
                if (!speakInfo) {
                    ACSDK_ERROR(LX("executeCancel").d("reason", "invalidDirectiveInfo"));
                    return;
                }
                executeCancel(speakInfo);
            }
            void SpeechSynthesizer::executeCancel(shared_ptr<SpeakDirectiveInfo> speakInfo) {
                if (!speakInfo) {
                    ACSDK_ERROR(LX("executeCancelFailed").d("reason", "invalidDirectiveInfo"));
                    return;
                }
                ACSDK_DEBUG(LX(__func__).d("messageId", speakInfo->directive->getMessageId()));
                if (!m_currentInfo || (speakInfo->directive->getMessageId() != m_currentInfo->directive->getMessageId())) {
                    ACSDK_DEBUG3(LX(__func__).d("result", "cancelPendingDirective"));
                    speakInfo->clear();
                    removeSpeakDirectiveInfo(speakInfo->directive->getMessageId());
                    {
                        lock_guard<mutex> lock(m_speakInfoQueueMutex);
                        for (auto it = m_speakInfoQueue.begin(); it != m_speakInfoQueue.end(); it++) {
                            if (speakInfo->directive->getMessageId() == it->get()->directive->getMessageId()) {
                                it = m_speakInfoQueue.erase(it);
                                break;
                            }
                        }
                    }
                    removeDirective(speakInfo->directive->getMessageId());
                    return;
                }
                ACSDK_DEBUG3(LX(__func__).d("result", "cancelCurrentDirective").d("state", m_currentState).d("desiredState", m_desiredState)
                    .d("isPlaybackInitiated", m_currentInfo->isPlaybackInitiated));
                m_currentInfo->sendPlaybackStartedMessage = false;
                m_currentInfo->sendCompletedMessage = false;
                if (m_currentInfo->isPlaybackInitiated) stopPlaying();
                else {
                    {
                        lock_guard<mutex> lock{m_mutex};
                        if (SpeechSynthesizerState::FINISHED == m_currentState) m_desiredState = SpeechSynthesizerState::FINISHED;
                        else m_desiredState = SpeechSynthesizerState::INTERRUPTED;
                    }
                    executePlaybackFinished();
                    if (!m_currentInfo) releaseForegroundFocus();
                }
            }
            void SpeechSynthesizer::executeStateChange(SpeechSynthesizerState newState) {
                ACSDK_DEBUG(LX("executeStateChange").d("newState", newState));
                switch(newState) {
                    case SpeechSynthesizerState::PLAYING:
                        if (m_currentInfo) {
                            m_currentInfo->sendPlaybackStartedMessage = true;
                            m_currentInfo->sendPlaybackFinishedMessage = true;
                            m_currentInfo->sendCompletedMessage = true;
                            m_currentInfo->isPlaybackInitiated = true;
                            startPlaying();
                        }
                        break;
                    case SpeechSynthesizerState::INTERRUPTED:
                        if (m_currentInfo) {
                            m_currentInfo->sendCompletedMessage = false;
                            if (m_currentInfo->result) {
                                m_currentInfo->result->setFailed("Stopped due to SpeechSynthesizer going into INTERRUPTED state.");
                                m_currentInfo->isSetFailedCalled = true;
                            }

                            if (m_currentInfo->isPlaybackInitiated) stopPlaying();
                            else {
                                setDesiredState(SpeechSynthesizerState::INTERRUPTED);
                                executePlaybackFinished();
                            }
                        }
                        break;
                    case SpeechSynthesizerState::FINISHED: case SpeechSynthesizerState::GAINING_FOCUS:
                    case SpeechSynthesizerState::LOSING_FOCUS:
                        ACSDK_WARN(LX(__func__).d("unexpectedStateChange", newState));
                        break;
                }
            }
            void SpeechSynthesizer::executeProvideStateLocked(const unsigned int& stateRequestToken) {
                ACSDK_DEBUG(LX("executeProvideState").d("stateRequestToken", stateRequestToken).d("state", m_currentState));
                StateRefreshPolicy refreshPolicy = StateRefreshPolicy::NEVER;
                string speakDirectiveToken;
                if (m_currentInfo) speakDirectiveToken = m_currentInfo->token;
                if (SpeechSynthesizerState::PLAYING == m_currentState) {
                    m_offsetInMilliseconds = duration_cast<milliseconds>(m_speechPlayer->getOffset(m_mediaSourceId)).count();
                    refreshPolicy = StateRefreshPolicy::ALWAYS;
                }
                auto jsonState = buildState(speakDirectiveToken, m_offsetInMilliseconds);
                if (jsonState.empty()) {
                    ACSDK_ERROR(LX("executeProvideStateFailed").d("reason", "buildStateFailed").d("token", speakDirectiveToken));
                    return;
                }
                auto result = m_contextManager->setState(CONTEXT_MANAGER_SPEECH_STATE, jsonState, refreshPolicy, stateRequestToken);
                if (result != SetStateResult::SUCCESS) {
                    ACSDK_ERROR(LX("executeProvideStateFailed").d("reason", "contextManagerSetStateFailed").d("token", speakDirectiveToken));
                }
            }
            void SpeechSynthesizer::executePlaybackStarted() {
                ACSDK_DEBUG(LX("executePlaybackStarted"));
                if (!m_currentInfo) {
                    ACSDK_ERROR(LX("executePlaybackStartedIgnored").d("reason", "nullptrDirectiveInfo"));
                    return;
                }
                {
                    lock_guard<mutex> lock(m_mutex);
                    setCurrentStateLocked(SpeechSynthesizerState::PLAYING);
                }
                setDesiredState(SpeechSynthesizerState::FINISHED);
                m_waitOnStateChange.notify_one();
                if (m_currentInfo->sendPlaybackStartedMessage) {
                    sendEvent(SPEECH_STARTED_EVENT_NAME, buildPayload(m_currentInfo->token));
                }
            }
            void SpeechSynthesizer::executePlaybackFinished() {
                ACSDK_DEBUG(LX("executePlaybackFinished"));
                if (!m_currentInfo) {
                    ACSDK_ERROR(LX("executePlaybackFinishedIgnored").d("reason", "nullptrDirectiveInfo"));
                    return;
                }
                auto newState = SpeechSynthesizerState::FINISHED;
                auto eventName = SPEECH_FINISHED_EVENT_NAME;
                string payload;
                {
                    lock_guard<mutex> lock(m_mutex);
                    if (SpeechSynthesizerState::INTERRUPTED == m_desiredState) {
                        newState = SpeechSynthesizerState::INTERRUPTED;
                        eventName = SPEECH_INTERRUPTED_EVENT_NAME;
                        payload = buildPayload(m_currentInfo->token, m_offsetInMilliseconds);
                    } else {
                        payload = buildPayload(m_currentInfo->token);
                        m_offsetInMilliseconds = 0;
                    }
                    setCurrentStateLocked(newState);
                }
                ACSDK_DEBUG3(LX(__func__).d("reason", eventName));
                m_waitOnStateChange.notify_one();
                if (m_currentInfo->sendPlaybackFinishedMessage) sendEvent(eventName, payload);
                if (m_currentInfo->sendCompletedMessage) setHandlingCompleted();
                resetCurrentInfo();
                {
                    unique_lock<mutex> lock(m_speakInfoQueueMutex);
                    if (!m_isShuttingDown && !m_speakInfoQueue.empty()) {
                        if (m_speakInfoQueue.front()->isHandled) {
                            auto front = m_speakInfoQueue.front();
                            lock.unlock();
                            executeHandleAfterValidation(front);
                        }
                    }
                }
                resetMediaSourceId();
            }
            void SpeechSynthesizer::executePlaybackError(const ErrorType& type, string error) {
                ACSDK_DEBUG(LX("executePlaybackError").d("type", type).d("error", error));
                if (!m_currentInfo) return;
                setDesiredState(SpeechSynthesizerState::INTERRUPTED);
                {
                    lock_guard<mutex> lock(m_mutex);
                    setCurrentStateLocked(SpeechSynthesizerState::INTERRUPTED);
                }
                m_waitOnStateChange.notify_one();
                releaseForegroundFocus();
                {
                    unique_lock<mutex> lock(m_speakInfoQueueMutex);
                    m_speakInfoQueue.push_front(m_currentInfo);
                    while(!m_speakInfoQueue.empty()) {
                        auto speakInfo = m_speakInfoQueue.front();
                        m_speakInfoQueue.pop_front();
                        lock.unlock();
                        sendExceptionEncounteredAndReportFailed(speakInfo,ExceptionErrorType::INTERNAL_ERROR, error);
                        lock.lock();
                    }
                }
                resetCurrentInfo();
                resetMediaSourceId();
            }
            string SpeechSynthesizer::buildState(string& token, int64_t offsetInMilliseconds) const {
                Document state(kObjectType);
                Document::AllocatorType& alloc = state.GetAllocator();
                Value _KEY_TOKEN{KEY_TOKEN, strlen(KEY_TOKEN)};
                Value _KEY_OFFSET_IN_MILLISECONDS{KEY_OFFSET_IN_MILLISECONDS, strlen(KEY_OFFSET_IN_MILLISECONDS)};
                state.AddMember(_KEY_TOKEN, token, alloc);
                state.AddMember(_KEY_OFFSET_IN_MILLISECONDS, offsetInMilliseconds, alloc);
                switch (m_currentState) {
                    case SpeechSynthesizerState::PLAYING:
                        state.AddMember(KEY_PLAYER_ACTIVITY, PLAYER_STATE_PLAYING, alloc);
                        break;
                    case SpeechSynthesizerState::FINISHED: case SpeechSynthesizerState::GAINING_FOCUS:
                    case SpeechSynthesizerState::LOSING_FOCUS:
                        state.AddMember(KEY_PLAYER_ACTIVITY, PLAYER_STATE_FINISHED, alloc);
                        break;
                    case SpeechSynthesizerState::INTERRUPTED:
                        state.AddMember(KEY_PLAYER_ACTIVITY, PLAYER_STATE_INTERRUPTED, alloc);
                        break;
                }
                StringBuffer buffer;
                rapidjson::Writer<StringBuffer> writer(buffer);
                if (!state.Accept(writer)) {
                    ACSDK_ERROR(LX("buildStateFailed").d("reason", "writerRefusedJsonObject"));
                    return "";
                }
                return buffer.GetString();
            }
            string SpeechSynthesizer::buildPayload(string& token, int64_t offsetInMilliseconds) {
                JsonGenerator generator;
                generator.addMember(KEY_TOKEN, token);
                generator.addMember(KEY_OFFSET_IN_MILLISECONDS, offsetInMilliseconds);
                return generator.toString();
            }
            string SpeechSynthesizer::buildPayload(string& token) {
                JsonGenerator generator;
                generator.addMember(KEY_TOKEN, token);
                return generator.toString();
            }
            void SpeechSynthesizer::startPlaying() {
                ACSDK_DEBUG9(LX("startPlaying"));
                shared_ptr<AttachmentReader> attachmentReader = move(m_currentInfo->attachmentReader);
                m_mediaSourceId = m_speechPlayer->setSource(move(attachmentReader));
                if (m_captionManager && m_currentInfo->captionData.isValid()) {
                    m_captionManager->onCaption(m_mediaSourceId, m_currentInfo->captionData);
                }
                if (MediaPlayerInterface::ERROR == m_mediaSourceId) {
                    ACSDK_ERROR(LX("startPlayingFailed").d("reason", "setSourceFailed"));
                    executePlaybackError(ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "playFailed");
                } else if (!m_speechPlayer->play(m_mediaSourceId)) {
                    executePlaybackError(ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "playFailed");
                }
            }
            void SpeechSynthesizer::stopPlaying() {
                ACSDK_DEBUG9(LX("stopPlaying"));
                bool isAlreadyStopping = false;
                {
                    lock_guard<mutex> lock{m_mutex};
                    isAlreadyStopping = SpeechSynthesizerState::INTERRUPTED == m_desiredState;
                }
                if (MediaPlayerInterface::ERROR == m_mediaSourceId) {
                    ACSDK_ERROR(LX("stopPlayingFailed").d("reason", "invalidMediaSourceId").d("mediaSourceId", m_mediaSourceId));
                } else if (isAlreadyStopping) {
                    ACSDK_DEBUG9(LX("stopPlayingIgnored").d("reason", "isAlreadyStopping"));
                } else {
                    m_offsetInMilliseconds = duration_cast<milliseconds>(m_speechPlayer->getOffset(m_mediaSourceId)).count();
                    if (!m_speechPlayer->stop(m_mediaSourceId)) {
                        executePlaybackError(ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "stopFailed");
                    } else {
                        setDesiredState(SpeechSynthesizerState::INTERRUPTED);
                    }
                }
            }
            void SpeechSynthesizer::setCurrentStateLocked(SpeechSynthesizerState newState) {
                ACSDK_DEBUG9(LX("setCurrentStateLocked").d("state", newState));
                if (m_currentState == newState) {
                    return;
                }
                m_currentState = newState;
                managePowerResource(m_currentState);
                switch(newState) {
                    case SpeechSynthesizerState::PLAYING: case SpeechSynthesizerState::FINISHED:
                    case SpeechSynthesizerState::INTERRUPTED:
                        executeProvideStateLocked(0);
                        break;
                    case SpeechSynthesizerState::LOSING_FOCUS: case SpeechSynthesizerState::GAINING_FOCUS: break;
                }
                vector<AudioAnalyzerState> analyzersData;
                if (m_currentInfo) analyzersData = m_currentInfo->analyzersData;
                for (auto observer : m_observers) {
                    observer->onStateChanged(m_currentState, m_mediaSourceId, m_speechPlayer->getMediaPlayerState(m_mediaSourceId),
                                             analyzersData);
                }
            }
            void SpeechSynthesizer::setDesiredState(SpeechSynthesizerState desiredState) {
                lock_guard<mutex> lock{m_mutex};
                m_desiredState = desiredState;
            }
            void SpeechSynthesizer::resetCurrentInfo(shared_ptr<SpeakDirectiveInfo> speakInfo) {
                if (m_currentInfo != speakInfo) {
                    if (m_currentInfo) {
                        removeSpeakDirectiveInfo(m_currentInfo->directive->getMessageId());
                        removeDirective(m_currentInfo->directive->getMessageId());
                        m_currentInfo->clear();
                    }
                    m_currentInfo = speakInfo;
                }
            }
            void SpeechSynthesizer::setHandlingCompleted() {
                ACSDK_DEBUG9(LX("setHandlingCompleted"));
                if (m_currentInfo && m_currentInfo->result) m_currentInfo->result->setCompleted();
            }
            void SpeechSynthesizer::sendEvent(const string& eventName, const string& payload) const {
                if (payload.empty()) {
                    ACSDK_ERROR(LX("sendEventFailed").d("event", eventName).d("token", m_currentInfo->token));
                    return;
                }
                auto msgIdAndJsonEvent = buildJsonEventString(eventName, "", payload);
                auto request = make_shared<MessageRequest>(msgIdAndJsonEvent.second);
                m_messageSender->sendMessage(request);
            }
            void SpeechSynthesizer::sendExceptionEncounteredAndReportFailed(shared_ptr<SpeakDirectiveInfo> speakInfo,
                ExceptionErrorType type,
                const string& message) {
                if (speakInfo) {
                    if (speakInfo->directive) {
                        m_exceptionEncounteredSender->sendExceptionEncountered(speakInfo->directive->getUnparsedDirective(), type, message);
                        removeDirective(speakInfo->directive->getMessageId());
                    } else { ACSDK_ERROR(LX("sendExceptionEncounteredAndReportFailed").d("reason", "speakInfoHasNoDirective")); }
                    if (speakInfo->result) speakInfo->result->setFailed(message);
                    else { ACSDK_ERROR(LX("sendExceptionEncounteredAndReportFailed").d("reason", "speakInfoHasNoResult")); }
                    speakInfo->clear();
                } else { ACSDK_ERROR(LX("sendExceptionEncounteredAndReportFailed").d("reason", "speakInfoNotFound")); }
                unique_lock<mutex> lock(m_mutex);
                if (SpeechSynthesizerState::PLAYING == m_currentState || SpeechSynthesizerState::GAINING_FOCUS == m_currentState) {
                    lock.unlock();
                    if (speakInfo) speakInfo->isSetFailedCalled = true;
                    stopPlaying();
                }
            }
            void SpeechSynthesizer::sendExceptionEncounteredAndReportMissingProperty(shared_ptr<SpeakDirectiveInfo> speakInfo,
                const string& key) {
                ACSDK_ERROR(LX("executePreHandleFailed").d("reason", "missingProperty").d("key", key));
                sendExceptionEncounteredAndReportFailed(speakInfo, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                string("missing property: ") + key);
            }
            void SpeechSynthesizer::sendExceptionEncounteredAndReportUnexpectedPropertyType(shared_ptr<SpeakDirectiveInfo> speakInfo,
                                                                                            const string& key) {
                ACSDK_ERROR(LX("executePreHandleFailed").d("reason", "invalidProperty").d("key", key));
                sendExceptionEncounteredAndReportFailed(speakInfo, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                string("invalid property: ") + key);
            }
            void SpeechSynthesizer::releaseForegroundFocus() {
                ACSDK_DEBUG9(LX("releaseForegroundFocus"));
                {
                    lock_guard<mutex> lock(m_mutex);
                    m_currentFocus = FocusState::NONE;
                }
                m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
            }
            shared_ptr<SpeakDirectiveInfo> SpeechSynthesizer::validateInfo(const string& caller, shared_ptr<DirectiveInfo> info,
                bool checkResult) {
                if (!info) {
                    ACSDK_ERROR(LX(caller + "Failed").d("reason", "nullptrInfo"));
                    return nullptr;
                }
                if (!info->directive) {
                    ACSDK_ERROR(LX(caller + "Failed").d("reason", "nullptrDirective"));
                    return nullptr;
                }
                if (checkResult && !info->result) {
                    ACSDK_ERROR(LX(caller + "Failed").d("reason", "nullptrResult"));
                    return nullptr;
                }
                auto speakDirInfo = getSpeakDirectiveInfo(info->directive->getMessageId());
                if (speakDirInfo) return speakDirInfo;
                speakDirInfo = std::make_shared<SpeakDirectiveInfo>(info);
                return speakDirInfo;
            }
            shared_ptr<SpeakDirectiveInfo> SpeechSynthesizer::getSpeakDirectiveInfo(const string& messageId) {
                lock_guard<mutex> lock(m_speakDirectiveInfoMutex);
                auto it = m_speakDirectiveInfoMap.find(messageId);
                if (it != m_speakDirectiveInfoMap.end()) return it->second;
                return nullptr;
            }
            bool SpeechSynthesizer::setSpeakDirectiveInfo(const string& messageId, shared_ptr<SpeakDirectiveInfo> speakDirectiveInfo) {
                lock_guard<mutex> lock(m_speakDirectiveInfoMutex);
                auto it = m_speakDirectiveInfoMap.find(messageId);
                if (it != m_speakDirectiveInfoMap.end()) return false;
                m_speakDirectiveInfoMap[messageId] = speakDirectiveInfo;
                return true;
            }
            void SpeechSynthesizer::removeSpeakDirectiveInfo(const string& messageId) {
                lock_guard<std::mutex> lock(m_speakDirectiveInfoMutex);
                m_speakDirectiveInfoMap.erase(messageId);
            }
            void SpeechSynthesizer::clearPendingDirectivesLocked() {
                while (!m_speakInfoQueue.empty()) {
                    auto info = m_speakInfoQueue.front();
                    if (info->result) info->result->setCompleted();
                    removeSpeakDirectiveInfo(info->directive->getMessageId());
                    removeDirective(info->directive->getMessageId());
                    info->clear();
                    m_speakInfoQueue.pop_front();
                }
            }
            void SpeechSynthesizer::addToDirectiveQueue(shared_ptr<SpeakDirectiveInfo> speakInfo) {
                unique_lock<mutex> lock(m_speakInfoQueueMutex);
                ACSDK_DEBUG5(LX(__func__).d("queueSize", m_speakInfoQueue.size()).d("playBehavior", speakInfo->playBehavior));
                switch(speakInfo->playBehavior) {
                    case PlayBehavior::ENQUEUE: m_speakInfoQueue.push_back(speakInfo); break;
                    case PlayBehavior::REPLACE_ENQUEUED:
                        clearPendingDirectivesLocked();
                        m_speakInfoQueue.push_back(speakInfo);
                        break;
                    case PlayBehavior::REPLACE_ALL:
                        clearPendingDirectivesLocked();
                        m_speakInfoQueue.push_back(speakInfo);
                        if (m_currentInfo) {
                            lock.unlock();
                            executeCancel(m_currentInfo);
                            lock.lock();
                        }
                        break;
                }
            }
            void SpeechSynthesizer::resetMediaSourceId() {
                m_mediaSourceId = MediaPlayerInterface::ERROR;
            }
            void SpeechSynthesizer::onDialogUXStateChanged(DialogUXState newState) {
                m_executor.submit([this, newState]() { executeOnDialogUXStateChanged(newState); });
            }
            void SpeechSynthesizer::executeOnDialogUXStateChanged(DialogUXState newState) {
                if (!m_initialDialogUXStateReceived) {
                    m_initialDialogUXStateReceived = true;
                    return;
                }
                if (newState != DialogUXState::IDLE) return;
                if (m_currentFocus != FocusState::NONE &&
                    m_currentState != SpeechSynthesizerState::GAINING_FOCUS) {
                    m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                    m_currentFocus = FocusState::NONE;
                }
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> SpeechSynthesizer::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            void SpeechSynthesizer::submitMetric(MetricEventBuilder& metricEventBuilder) {
                if (!m_metricRecorder) return;
                if (m_currentInfo) {
                    auto metricEvent = metricEventBuilder.addDataPoint(DataPointStringBuilder{}.setName("HTTP2_STREAM")
                                           .setValue(m_currentInfo->directive->getAttachmentContextId()).build())
                                           .addDataPoint(DataPointStringBuilder{}.setName("DIRECTIVE_MESSAGE_ID")
                                           .setValue(m_currentInfo->directive->getMessageId()).build()).build();
                    if (metricEvent == nullptr) {
                        ACSDK_ERROR(LX("Error creating metric."));
                        return;
                    }
                    recordMetric(m_metricRecorder, metricEvent);
                }
            }
            void SpeechSynthesizer::managePowerResource(SpeechSynthesizerState newState) {
                if (!m_powerResourceManager) return;
                ACSDK_DEBUG5(LX(__func__).d("state", newState));
                switch (newState) {
                    case SpeechSynthesizerState::PLAYING:
                        m_powerResourceManager->acquirePowerResource(POWER_RESOURCE_COMPONENT_NAME);
                        break;
                    case SpeechSynthesizerState::FINISHED: case SpeechSynthesizerState::INTERRUPTED:
                        m_powerResourceManager->releasePowerResource(POWER_RESOURCE_COMPONENT_NAME);
                        break;
                    default: break;
                }
            }
        }
    }
}