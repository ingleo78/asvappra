#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_AUDIOINPUTPROCESSOR_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_AIP_INCLUDE_AIP_AUDIOINPUTPROCESSOR_H_

#include <chrono>
#include <map>
#include <memory>
#include <unordered_set>
#include <vector>
#include <avs/attachment/InProcessAttachmentReader.h>
#include <avs/CapabilityAgent.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/DialogUXStateAggregator.h>
#include <avs/DirectiveHandlerConfiguration.h>
#include <avs/ExceptionErrorType.h>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/AudioInputProcessorObserverInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/ChannelObserverInterface.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/InternetConnectionObserverInterface.h>
#include <sdkinterfaces/LocaleAssetsManagerInterface.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/PowerResourceManagerInterface.h>
#include <sdkinterfaces/SystemSoundPlayerInterface.h>
#include <sdkinterfaces/UserInactivityMonitorInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>
#include <timing/Timer.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/SettingEventMetadata.h>
#include <settings/WakeWordConfirmationSettingType.h>
#include <speech_enconder/SpeechEncoder.h>
#include "AudioProvider.h"
#include "Initiator.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace aip {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace settings;
            using namespace speechencoder;
            using namespace utils;
            using namespace attachment;
            using namespace metrics;
            using namespace threading;
            using namespace timing;
            class AudioInputProcessor : public CapabilityAgent, public CapabilityConfigurationInterface, public DialogUXStateObserverInterface,
                                        public MessageRequestObserverInterface, public InternetConnectionObserverInterface, public RequiresShutdown,
                                        public ChannelObserverInterface, public enable_shared_from_this<AudioInputProcessor> {
            public:
                using ObserverInterface = AudioInputProcessorObserverInterface;
                static constexpr const char* KEYWORD_TEXT_STOP = "STOP";
                static const auto INVALID_INDEX = numeric_limits<AudioInputStream::Index>::max();
                static shared_ptr<AudioInputProcessor> create(shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                              shared_ptr<MessageSenderInterface> messageSender, shared_ptr<ContextManagerInterface> contextManager,
                                                              shared_ptr<FocusManagerInterface> focusManager,
                                                              shared_ptr<DialogUXStateAggregator> dialogUXStateAggregator,
                                                              shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                              shared_ptr<UserInactivityMonitorInterface> userInactivityNotifier,
                                                              shared_ptr<SystemSoundPlayerInterface> systemSoundPlayer,
                                                              const shared_ptr<LocaleAssetsManagerInterface>& assetsManager,
                                                              shared_ptr<WakeWordConfirmationSetting> wakeWordConfirmation,
                                                              shared_ptr<SpeechConfirmationSetting> speechConfirmation,
                                                              shared_ptr<WakeWordsSetting> wakeWordsSetting = nullptr,
                                                              shared_ptr<SpeechEncoder> speechEncoder = nullptr,
                                                              AudioProvider defaultAudioProvider = AudioProvider::null(),
                                                              shared_ptr<PowerResourceManagerInterface> powerResourceManager = nullptr,
                                                              shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
                void addObserver(shared_ptr<ObserverInterface> observer);
                void removeObserver(shared_ptr<ObserverInterface> observer);
                future<bool> recognize(AudioProvider audioProvider, Initiator initiator, steady_clock::time_point startOfSpeechTimestamp = steady_clock::now(),
                                       AudioInputStream::Index begin = INVALID_INDEX, AudioInputStream::Index keywordEnd = INVALID_INDEX, std::string keyword = "",
                                       shared_ptr<const vector<char>> KWDMetadata = nullptr, const std::string& initiatorToken = "");
                future<bool> stopCapture();
                future<void> resetState();
                void onContextAvailable(const std::string& jsonContext);
                void onContextFailure(const ContextRequestError error) override;
                void onSendCompleted(MessageRequestObserverInterface::Status status) override;
                void onExceptionReceived(const std::string& exceptionMessage) override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<DirectiveInfo> info) override;
                void handleDirective(shared_ptr<DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<DirectiveInfo> info) override;
                void onDeregistered() override;
                DirectiveHandlerConfiguration getConfiguration() const override;
                void onFocusChanged(FocusState newFocus, MixingBehavior behavior) override;
                void onDialogUXStateChanged(DialogUXStateObserverInterface::DialogUXState newState) override;
                unordered_set<shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
                void onConnectionStatusChanged(bool connected) override;
                static SettingEventMetadata getWakeWordsEventsMetadata();
                static SettingEventMetadata getWakeWordConfirmationMetadata();
                static SettingEventMetadata getSpeechConfirmationMetadata();
            private:
                AudioInputProcessor(shared_ptr<DirectiveSequencerInterface> directiveSequencer, shared_ptr<MessageSenderInterface> messageSender,
                                    shared_ptr<ContextManagerInterface> contextManager, shared_ptr<FocusManagerInterface> focusManager,
                                    shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                    shared_ptr<UserInactivityMonitorInterface> userInactivityMonitor, shared_ptr<SystemSoundPlayerInterface> systemSoundPlayer,
                                    shared_ptr<SpeechEncoder> speechEncoder, AudioProvider defaultAudioProvider,
                                    shared_ptr<WakeWordConfirmationSetting> wakeWordConfirmation, shared_ptr<SpeechConfirmationSetting> speechConfirmation,
                                    shared_ptr<WakeWordsSetting> wakeWordsSetting, shared_ptr<CapabilityConfiguration> capabilitiesConfiguration,
                                    shared_ptr<PowerResourceManagerInterface> powerResourceManager, shared_ptr<MetricRecorderInterface> metricRecorder);
                void doShutdown() override;
                future<bool> expectSpeechTimedOut();
                void handleStopCaptureDirective(shared_ptr<DirectiveInfo> info);
                void handleExpectSpeechDirective(shared_ptr<DirectiveInfo> info);
                void handleSetEndOfSpeechOffsetDirective(shared_ptr<DirectiveInfo> info);
                void handleDirectiveFailure(const std::string& errorMessage, shared_ptr<DirectiveInfo> info, ExceptionErrorType errorType);
                bool executeRecognize(AudioProvider provider, Initiator initiator, steady_clock::time_point startOfSpeechTimestamp, AudioInputStream::Index begin,
                                      AudioInputStream::Index keywordEnd, const std::string& keyword, shared_ptr<const vector<char>> KWDMetadata,
                                      const std::string& initiatorToken);
                bool executeRecognize(AudioProvider provider, const std::string& initiatorJson, steady_clock::time_point startOfSpeechTimestamp = steady_clock::now(),
                                      AudioInputStream::Index begin = INVALID_INDEX, AudioInputStream::Index end = INVALID_INDEX, const std::string& keyword = "",
                                      shared_ptr<const vector<char>> KWDMetadata = nullptr, bool initiatedByWakeword = false, bool falseWakewordDetection = false);
                void executeOnContextAvailable(const std::string jsonContext);
                void executeOnContextFailure(const ContextRequestError error);
                void executeOnFocusChanged(FocusState newFocus);
                bool executeStopCapture(bool stopImmediately = false, shared_ptr<DirectiveInfo> info = nullptr);
                void executeResetState();
                bool executeExpectSpeech(milliseconds timeout, shared_ptr<DirectiveInfo> info);
                bool executeExpectSpeechTimedOut();
                void executeOnDialogUXStateChanged(DialogUXStateObserverInterface::DialogUXState newState);
                void executeDisconnected();
                void setState(ObserverInterface::State state);
                void removeDirective(shared_ptr<DirectiveInfo> info);
                void sendRequestNow();
                bool handleSetWakeWordConfirmation(shared_ptr<DirectiveInfo> info);
                bool handleSetSpeechConfirmation(shared_ptr<DirectiveInfo> info);
                bool handleSetWakeWords(shared_ptr<DirectiveInfo> info);
                void managePowerResource(ObserverInterface::State newState);
                shared_ptr<DialogUXStateObserverInterface> m_dialogUXStateObserverInterface;
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                shared_ptr<DirectiveSequencerInterface> m_directiveSequencer;
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<ContextManagerInterface> m_contextManager;
                shared_ptr<FocusManagerInterface> m_focusManager;
                shared_ptr<UserInactivityMonitorInterface> m_userInactivityMonitor;
                Timer m_expectingSpeechTimer;
                shared_ptr<SpeechEncoder> m_encoder;
                unordered_set<shared_ptr<ObserverInterface>> m_observers;
                AudioProvider m_defaultAudioProvider;
                AudioProvider m_lastAudioProvider;
                shared_ptr<AttachmentReader> m_reader;
                shared_ptr<AttachmentReader> m_KWDMetadataReader;
                std::string m_recognizePayload;
                shared_ptr<MessageRequest> m_recognizeRequest;
                shared_ptr<MessageRequest> m_recognizeRequestSent;
                ObserverInterface::State m_state;
                FocusState m_focusState;
                bool m_preparingToSend;
                function<void()> m_deferredStopCapture;
                bool m_initialDialogUXStateReceived;
                bool m_initialStateReceived;
                bool m_localStopCapturePerformed;
                shared_ptr<SystemSoundPlayerInterface> m_systemSoundPlayer;
                unique_ptr<std::string> m_precedingExpectSpeechInitiator;
                unordered_set<shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
                shared_ptr<WakeWordConfirmationSetting> m_wakeWordConfirmation;
                shared_ptr<SpeechConfirmationSetting> m_speechConfirmation;
                shared_ptr<WakeWordsSetting> m_wakeWordsSetting;
                shared_ptr<PowerResourceManagerInterface> m_powerResourceManager;
                steady_clock::time_point m_stopCaptureReceivedTime;
                Executor m_executor;
                std::string m_preCachedDialogRequestId;
            };
        }
    }
}
#endif