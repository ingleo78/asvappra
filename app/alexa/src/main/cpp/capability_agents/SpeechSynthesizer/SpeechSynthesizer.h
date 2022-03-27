#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEECHSYNTHESIZER_INCLUDE_SPEECHSYNTHESIZER_SPEECHSYNTHESIZER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEECHSYNTHESIZER_INCLUDE_SPEECHSYNTHESIZER_SPEECHSYNTHESIZER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <deque>
#include <avs/AVSDirective.h>
#include <avs/PlayBehavior.h>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <avs/DialogUXStateAggregator.h>
#include <sdkinterfaces/SpeechSynthesizerObserverInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/PowerResourceManagerInterface.h>
#include <audio_analyzer/AudioAnalyzerState.h>
#include <media_player/MediaPlayerInterface.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <metrics/MetricEventBuilder.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <captions/CaptionData.h>
#include <captions/CaptionManagerInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speechSynthesizer {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace attachment;
            using namespace audioAnalyzer;
            using namespace captions;
            using namespace json;
            using namespace mediaPlayer;
            using namespace metrics;
            using namespace threading;
            class SpeechSynthesizer : public CapabilityAgent, public DialogUXStateObserverInterface, public CapabilityConfigurationInterface,
                                      public MediaPlayerObserverInterface, public RequiresShutdown, public enable_shared_from_this<SpeechSynthesizer> {
            public:
                using SpeechSynthesizerState = SpeechSynthesizerObserverInterface::SpeechSynthesizerState;
                using DialogUXState = DialogUXStateObserverInterface::DialogUXState;
                static shared_ptr<SpeechSynthesizer> create(shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                            shared_ptr<MessageSenderInterface> messageSender,
                                                            shared_ptr<FocusManagerInterface> focusManager,
                                                            shared_ptr<ContextManagerInterface> contextManager,
                                                            shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                            shared_ptr<MetricRecorderInterface> metricRecorder,
                                                            shared_ptr<DialogUXStateAggregator> dialogUXStateAggregator,
                                                            shared_ptr<CaptionManagerInterface> captionManager = nullptr,
                                                            shared_ptr<PowerResourceManagerInterface> powerResourceManager = nullptr);
                void onDialogUXStateChanged(DialogUXState newState) override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                void addObserver(shared_ptr<SpeechSynthesizerObserverInterface> observer);
                void removeObserver(shared_ptr<SpeechSynthesizerObserverInterface> observer);
                void onDeregistered() override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                void onFocusChanged(FocusState newFocus, MixingBehavior behavior) override;
                void provideState(const NamespaceAndName& stateProviderName, const unsigned int stateRequestToken) override;
                void onContextAvailable(const string& jsonContext) override;
                void onContextFailure(const ContextRequestError error) override;
                void onFirstByteRead(SourceId id, const MediaPlayerState& state) override;
                void onPlaybackStarted(SourceId id, const MediaPlayerState& state) override;
                void onPlaybackFinished(SourceId id, const MediaPlayerState& state) override;
                void onPlaybackError(SourceId id, const ErrorType& type, string error, const MediaPlayerState& state) override;
                void onPlaybackStopped(SourceId id, const MediaPlayerState& state) override;
                void onBufferUnderrun(SourceId id, const MediaPlayerState& state) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
            private:
                struct SpeakDirectiveInfo {
                    SpeakDirectiveInfo(shared_ptr<DirectiveInfo> directiveInfo);
                    void clear();
                    shared_ptr<AVSDirective> directive;
                    shared_ptr<DirectiveHandlerResultInterface> result;
                    string token;
                    unique_ptr<AttachmentReader> attachmentReader;
                    bool sendPlaybackStartedMessage;
                    bool sendPlaybackFinishedMessage;
                    bool sendCompletedMessage;
                    bool isSetFailedCalled;
                    bool isPlaybackInitiated;
                    bool isHandled;
                    PlayBehavior playBehavior;
                    CaptionData captionData;
                    vector<AudioAnalyzerState> analyzersData;
                };
            public:
                using SpeakDirectiveInfo = SpeechSynthesizer::SpeakDirectiveInfo;
            private:
                SpeechSynthesizer(shared_ptr<MediaPlayerInterface> mediaPlayer, shared_ptr<MessageSenderInterface> messageSender,
                                  shared_ptr<FocusManagerInterface> focusManager, shared_ptr<ContextManagerInterface> contextManager,
                                  shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                  shared_ptr<CaptionManagerInterface> captionManager = nullptr,
                                  shared_ptr<PowerResourceManagerInterface> powerResourceManager = nullptr);
                void doShutdown() override;
                void init();
                void executeHandleImmediately(shared_ptr<DirectiveInfo> info);
                void executePreHandleAfterValidation(shared_ptr<SpeakDirectiveInfo> speakInfo);
                void executeHandleAfterValidation(shared_ptr<SpeakDirectiveInfo> speakInfo);
                void executePreHandle(shared_ptr<DirectiveInfo> info);
                void executeHandle(shared_ptr<DirectiveInfo> info);
                void executeCancel(shared_ptr<DirectiveInfo> info);
                void executeCancel(shared_ptr<SpeakDirectiveInfo> speakInfo);
                void executeStateChange(SpeechSynthesizerState newState);
                void executeProvideStateLocked(const unsigned int& stateRequestToken);
                void executePlaybackStarted();
                void executePlaybackFinished();
                void executePlaybackError(const ErrorType& type, std::string error);
                void submitMetric(MetricEventBuilder& metricEventBuilder);
                void executeOnDialogUXStateChanged(DialogUXState newState);
                string buildState(string& token, int64_t offsetInMilliseconds) const;
                void sendEvent(const string& eventName, const string& payload) const;
                static string buildPayload(string& token);
                static string buildPayload(string& token, int64_t offsetInMilliseconds);
                void startPlaying();
                void stopPlaying();
                void setCurrentStateLocked(SpeechSynthesizerState newState);
                void setDesiredState(SpeechSynthesizerState desiredState);
                void resetCurrentInfo(shared_ptr<SpeakDirectiveInfo> info = nullptr);
                void setHandlingCompleted();
                void setHandlingFailed(const string& description);
                void sendExceptionEncounteredAndReportFailed(shared_ptr<SpeakDirectiveInfo> info, ExceptionErrorType type, const string& message);
                void sendExceptionEncounteredAndReportMissingProperty(shared_ptr<SpeakDirectiveInfo> info, const string& missingProperty);
                void sendExceptionEncounteredAndReportUnexpectedPropertyType(shared_ptr<SpeakDirectiveInfo> info, const string& unexpectedProperty);
                void releaseForegroundFocus();
                shared_ptr<SpeakDirectiveInfo> validateInfo(const string& caller, shared_ptr<DirectiveInfo> info, bool checkResult = true);
                shared_ptr<SpeakDirectiveInfo> getSpeakDirectiveInfo(const string& messageId);
                bool setSpeakDirectiveInfo(const string& messageId, shared_ptr<SpeakDirectiveInfo> speakDirectiveInfo);
                void addToDirectiveQueue(shared_ptr<SpeakDirectiveInfo> speakInfo);
                void removeSpeakDirectiveInfo(const string& messageId);
                void resetMediaSourceId();
                void clearPendingDirectivesLocked();
                void managePowerResource(SpeechSynthesizerState newState);
                SourceId m_mediaSourceId;
                int64_t m_offsetInMilliseconds;
                shared_ptr<MediaPlayerInterface> m_speechPlayer;
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<FocusManagerInterface> m_focusManager;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<CaptionManagerInterface> m_captionManager;
                unordered_set<shared_ptr<SpeechSynthesizerObserverInterface>> m_observers;
                SpeechSynthesizerState m_currentState;
                SpeechSynthesizerState m_desiredState;
                FocusState m_currentFocus;
                shared_ptr<SpeakDirectiveInfo> m_currentInfo;
                mutex m_mutex;
                condition_variable m_waitOnStateChange;
                unordered_map<string, shared_ptr<SpeakDirectiveInfo>> m_speakDirectiveInfoMap;
                mutex m_speakDirectiveInfoMutex;
                deque<shared_ptr<SpeakDirectiveInfo>> m_speakInfoQueue;
                bool m_isShuttingDown;
                mutex m_speakInfoQueueMutex;
                bool m_initialDialogUXStateReceived;
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                shared_ptr<PowerResourceManagerInterface> m_powerResourceManager;
                Executor m_executor;
            };
        }
    }
}
#endif