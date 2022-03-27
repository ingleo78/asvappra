#include <algorithm>
#include <cctype>
#include <list>
#include <sstream>
#include <string.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/FocusState.h>
#include <avs/MessageRequest.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <memory/Memory.h>
#include <util/Metrics.h>
#include <metrics/DataPointDurationBuilder.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/DataPointStringBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include <util/string/StringUtils.h>
#include <uuid_generation/UUIDGeneration.h>
#include <avs/attachment/AttachmentUtils.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSender.h>
#include <settings/SharedAVSSettingProtocol.h>
#include <speech_enconder/SpeechEncoder.h>
#include <avs/attachment/DefaultAttachmentReader.h>
#include "AudioInputProcessor.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace aip {
            static const std::string SPEECHRECOGNIZER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const std::string SPEECHRECOGNIZER_CAPABILITY_INTERFACE_NAME = "SpeechRecognizer";
            static const std::string SPEECHRECOGNIZER_CAPABILITY_INTERFACE_VERSION = "2.3";
            static const std::string CAPABILITY_INTERFACE_CONFIGURATIONS_KEY = "configurations";
            static const std::string CAPABILITY_INTERFACE_WAKE_WORDS_KEY = "wakeWords";
            static const std::string CAPABILITY_MAXIMUM_CONCURRENT_WAKEWORDS_KEY = "maximumConcurrentWakeWords";
            static const std::string CAPABILITY_INTERFACE_SCOPES_KEY = "scopes";
            static const std::string CAPABILITY_INTERFACE_VALUES_KEY = "values";
            static const std::string CAPABILITY_INTERFACE_DEFAULT_LOCALE = "DEFAULT";
            static const std::string TAG("AudioInputProcessor");
            #define LX(event) LogEntry(TAG, event)
            static const std::string CHANNEL_NAME = FocusManagerInterface::DIALOG_CHANNEL_NAME;
            static const std::string NAMESPACE = "SpeechRecognizer";
            static const NamespaceAndName STOP_CAPTURE{NAMESPACE, "StopCapture"};
            static const NamespaceAndName EXPECT_SPEECH{NAMESPACE, "ExpectSpeech"};
            static const NamespaceAndName SET_END_OF_SPEECH_OFFSET{NAMESPACE, "SetEndOfSpeechOffset"};
            static const NamespaceAndName SET_WAKE_WORD_CONFIRMATION{NAMESPACE, "SetWakeWordConfirmation"};
            static const NamespaceAndName SET_SPEECH_CONFIRMATION{NAMESPACE, "SetSpeechConfirmation"};
            static const NamespaceAndName SET_WAKE_WORDS{NAMESPACE, "SetWakeWords"};
            static const std::string INITIATOR_KEY = "initiator";
            static const std::string PROFILE_KEY = "profile";
            static const std::string FORMAT_KEY = "format";
            static const std::string TYPE_KEY = "type";
            static const std::string TOKEN_KEY = "token";
            static const std::string PAYLOAD_KEY = "payload";
            static const std::string WAKEWORD_INDICES_KEY = "wakeWordIndices";
            static const std::string START_INDEX_KEY = "startIndexInSamples";
            static const std::string END_INDEX_KEY = "endIndexInSamples";
            static const std::string WAKE_WORD_KEY = "wakeWord";
            static const std::string AUDIO_ATTACHMENT_FIELD_NAME = "audio";
            static const std::string KWD_METADATA_FIELD_NAME = "wakewordEngineMetadata";
            static const std::string START_OF_SPEECH_TIMESTAMP_FIELD_NAME = "startOfSpeechTimestamp";
            static const std::string END_OF_SPEECH_OFFSET_FIELD_NAME = "endOfSpeechOffsetInMilliseconds";
            static const std::string WAKE_WORD_CONFIRMATION_CHANGED_EVENT_NAME = "WakeWordConfirmationChanged";
            static const std::string WAKE_WORD_CONFIRMATION_REPORT_EVENT_NAME = "WakeWordConfirmationReport";
            static const std::string WAKE_WORD_CONFIRMATION_PAYLOAD_KEY = "wakeWordConfirmation";
            static const std::string SPEECH_CONFIRMATION_CHANGED_EVENT_NAME = "SpeechConfirmationChanged";
            static const std::string SPEECH_CONFIRMATION_REPORT_EVENT_NAME = "SpeechConfirmationReport";
            static const std::string SPEECH_CONFIRMATION_PAYLOAD_KEY = "speechConfirmation";
            static const std::string WAKE_WORDS_CHANGED_EVENT_NAME = "WakeWordsChanged";
            static const std::string WAKE_WORDS_REPORT_EVENT_NAME = "WakeWordsReport";
            static const std::string WAKE_WORDS_PAYLOAD_KEY = "wakeWords";
            static const std::string POWER_RESOURCE_COMPONENT_NAME = "AudioInputProcessor";
            static const std::string METRIC_ACTIVITY_NAME_PREFIX_AIP = "AIP-";
            static const std::string START_OF_UTTERANCE = "START_OF_UTTERANCE";
            static const std::string START_OF_UTTERANCE_ACTIVITY_NAME = METRIC_ACTIVITY_NAME_PREFIX_AIP + START_OF_UTTERANCE;
            static const std::string START_OF_STREAM_TIMESTAMP = "START_OF_STREAM_TIMESTAMP";
            static const std::string WW_DURATION = "WW_DURATION";
            static const std::string WW_DURATION_ACTIVITY_NAME = METRIC_ACTIVITY_NAME_PREFIX_AIP + WW_DURATION;
            static const std::string STOP_CAPTURE_RECEIVED = "STOP_CAPTURE";
            static const std::string STOP_CAPTURE_RECEIVED_ACTIVITY_NAME = METRIC_ACTIVITY_NAME_PREFIX_AIP + STOP_CAPTURE_RECEIVED;
            static const std::string END_OF_SPEECH_OFFSET_RECEIVED = "END_OF_SPEECH_OFFSET";
            static const std::string END_OF_SPEECH_OFFSET_RECEIVED_ACTIVITY_NAME = METRIC_ACTIVITY_NAME_PREFIX_AIP + END_OF_SPEECH_OFFSET_RECEIVED;
            static const std::string STOP_CAPTURE_TO_END_OF_SPEECH_METRIC_NAME = "STOP_CAPTURE_TO_END_OF_SPEECH";
            static const std::string STOP_CAPTURE_TO_END_OF_SPEECH_ACTIVITY_NAME = METRIC_ACTIVITY_NAME_PREFIX_AIP + STOP_CAPTURE_TO_END_OF_SPEECH_METRIC_NAME;
            static const milliseconds PREROLL_DURATION = milliseconds(500);
            static const int MILLISECONDS_PER_SECOND = 1000;
            static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const std::string& activityName, const DataPoint& dataPoint,
                                     const std::string& dialogRequestId = "") {
                auto metricEventBuilder = MetricEventBuilder{}.setActivityName(activityName).addDataPoint(dataPoint).addDataPoint(DataPointStringBuilder{}
                                              .setName("DIALOG_REQUEST_ID").setValue(dialogRequestId).build());
                auto metricEvent = metricEventBuilder.build();
                if (metricEvent == nullptr) {
                    ACSDK_ERROR(LX("Error creating metric with explicit dialogRequestId"));
                    return;
                }
                recordMetric(metricRecorder, metricEvent);
            }
            static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, MetricEventBuilder& metricEventBuilder,
                                     const std::string& dialogRequestId = "") {
                metricEventBuilder.addDataPoint(DataPointStringBuilder{}.setName("DIALOG_REQUEST_ID").setValue(dialogRequestId).build());
                auto metricEvent = metricEventBuilder.build();
                if (metricEvent == nullptr) {
                    ACSDK_ERROR(LX("Error creating metric with explicit dialogRequestId"));
                    return;
                }
                recordMetric(metricRecorder, metricEvent);
            }
            static void submitMetric(const shared_ptr<MetricRecorderInterface> metricRecorder, const std::string& activityName,
                                     const DataPoint& dataPoint, const shared_ptr<AVSDirective> directive = nullptr) {
                auto metricEventBuilder = MetricEventBuilder{}.setActivityName(activityName).addDataPoint(dataPoint);
                if (directive != nullptr) {
                    AVSMessage *message = new AVSMessage(directive->m_avsMessageHeader, (char*)directive->m_payload.data(), directive->m_endpoint);
                    std::string contextId = (char*)directive->getAttachmentContextId().data();
                    std::string messageId = message->getMessageId();
                    std::string requestId = message->getDialogRequestId();
                    metricEventBuilder.addDataPoint(DataPointStringBuilder{}.setName("HTTP2_STREAM").setValue(contextId).build());
                    metricEventBuilder.addDataPoint(DataPointStringBuilder{}.setName("DIRECTIVE_MESSAGE_ID").setValue(messageId).build());
                    metricEventBuilder.addDataPoint(DataPointStringBuilder{}.setName("DIALOG_REQUEST_ID").setValue(requestId).build());
                }
                auto metricEvent = metricEventBuilder.build();
                if (metricEvent == nullptr) {
                    ACSDK_ERROR(LX("Error creating metric from directive"));
                    return;
                }
                recordMetric(metricRecorder, metricEvent);
            }
            static shared_ptr<CapabilityConfiguration> getSpeechRecognizerCapabilityConfiguration(const LocaleAssetsManagerInterface& assetsManager);
            shared_ptr<AudioInputProcessor> AudioInputProcessor::create(shared_ptr<DirectiveSequencerInterface> directiveSequencer,
                                                                        shared_ptr<MessageSenderInterface> messageSender,
                                                                        shared_ptr<ContextManagerInterface> contextManager,
                                                                        shared_ptr<FocusManagerInterface> focusManager,
                                                                        shared_ptr<DialogUXStateAggregator> dialogUXStateAggregator,
                                                                        shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                        shared_ptr<UserInactivityMonitorInterface> userInactivityMonitor,
                                                                        shared_ptr<SystemSoundPlayerInterface> systemSoundPlayer,
                                                                        const shared_ptr<LocaleAssetsManagerInterface>& assetsManager,
                                                                        shared_ptr<WakeWordConfirmationSetting> wakeWordConfirmation,
                                                                        shared_ptr<SpeechConfirmationSetting> speechConfirmation,
                                                                        shared_ptr<WakeWordsSetting> wakeWordsSetting, shared_ptr<SpeechEncoder> speechEncoder,
                                                                        AudioProvider defaultAudioProvider, shared_ptr<PowerResourceManagerInterface> powerResourceManager,
                                                                        shared_ptr<MetricRecorderInterface> metricRecorder) {
                if (!directiveSequencer) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullDirectiveSequencer"));
                    return nullptr;
                } else if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                } else if (!contextManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                    return nullptr;
                } else if (!focusManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullFocusManager"));
                    return nullptr;
                } else if (!dialogUXStateAggregator) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullDialogUXStateAggregator"));
                    return nullptr;
                } else if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncounteredSender"));
                    return nullptr;
                } else if (!userInactivityMonitor) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullUserInactivityMonitor"));
                    return nullptr;
                } else if (!systemSoundPlayer) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullSystemSoundPlayer"));
                    return nullptr;
                } else if (!wakeWordConfirmation) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullWakeWordsConfirmation"));
                    return nullptr;
                } else if (!speechConfirmation) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullSpeechConfirmation"));
                    return nullptr;
                } else if (!assetsManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullAssetsManager"));
                    return nullptr;
                } else if (!assetsManager->getDefaultSupportedWakeWords().empty() && !wakeWordsSetting) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullWakeWordsSetting"));
                    return nullptr;
                }
                auto capabilitiesConfiguration = getSpeechRecognizerCapabilityConfiguration(*assetsManager);
                if (!capabilitiesConfiguration) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "unableToCreateCapabilitiesConfiguration"));
                    return nullptr;
                }
                auto aip = shared_ptr<AudioInputProcessor>(new AudioInputProcessor(directiveSequencer, messageSender, contextManager, focusManager,
                                                           exceptionEncounteredSender, userInactivityMonitor, systemSoundPlayer, speechEncoder, move(defaultAudioProvider),
                                                           wakeWordConfirmation, speechConfirmation, wakeWordsSetting, capabilitiesConfiguration,
                                                           powerResourceManager, move(metricRecorder)));
                if (aip) {
                    DialogUXStateObserverInterface doi = reinterpret_cast<DialogUXStateObserverInterface&>(aip);
                    shared_ptr<DialogUXStateObserverInterface> sptrdoi = make_shared<DialogUXStateObserverInterface>(doi);
                    dialogUXStateAggregator->addObserver(sptrdoi);
                }
                return aip;
            }
            DirectiveHandlerConfiguration AudioInputProcessor::getConfiguration() const {
                DirectiveHandlerConfiguration configuration;
                configuration[STOP_CAPTURE] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[EXPECT_SPEECH] = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, true);
                configuration[SET_END_OF_SPEECH_OFFSET] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[SET_WAKE_WORD_CONFIRMATION] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[SET_SPEECH_CONFIRMATION] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[SET_WAKE_WORDS] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                return configuration;
            }
            void AudioInputProcessor::addObserver(shared_ptr<ObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                m_executor.submit([this, observer]() { m_observers.insert(observer); });
            }
            void AudioInputProcessor::removeObserver(shared_ptr<ObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                m_executor.submit([this, observer]() { m_observers.erase(observer); }).wait();
            }
            future<bool> AudioInputProcessor::recognize(AudioProvider audioProvider, Initiator initiator, steady_clock::time_point startOfSpeechTimestamp,
                                                        AudioInputStream::Index begin, AudioInputStream::Index keywordEnd, std::string keyword,
                                                        shared_ptr<const vector<char>> KWDMetadata, const std::string& initiatorToken) {
                ACSDK_METRIC_IDS(TAG, "Recognize", "", "", Metrics::Location::AIP_RECEIVE);
                ACSDK_DEBUG5(LX(__func__));
                std::string upperCaseKeyword = keyword;
                transform(upperCaseKeyword.begin(), upperCaseKeyword.end(), upperCaseKeyword.begin(), ::toupper);
                if (KEYWORD_TEXT_STOP == upperCaseKeyword) {
                    ACSDK_DEBUG(LX("skippingRecognizeEvent").d("reason", "invalidKeyword").d("keyword", keyword));
                    promise<bool> ret;
                    ret.set_value(false);
                    return ret.get_future();
                }
                if (audioProvider.stream && INVALID_INDEX == begin) {
                    static const bool startWithNewData = true;
                    auto reader = audioProvider.stream->createReader(AudioInputStream::Reader::Policy::NONBLOCKING, startWithNewData);
                    if (!reader) {
                        ACSDK_ERROR(LX("recognizeFailed").d("reason", "createReaderFailed"));
                        promise<bool> ret;
                        ret.set_value(false);
                        return ret.get_future();
                    }
                    begin = reader->tell();
                }
                return m_executor.submit([this, audioProvider, initiator, startOfSpeechTimestamp, begin, keywordEnd, keyword, KWDMetadata, initiatorToken]() {
                    return executeRecognize(audioProvider, initiator, startOfSpeechTimestamp, begin, keywordEnd, keyword, KWDMetadata, initiatorToken);
                });
            }
            future<bool> AudioInputProcessor::stopCapture() {
                return m_executor.submit([this]() { return executeStopCapture(); });
            }
            future<void> AudioInputProcessor::resetState() {
                return m_executor.submit([this]() { executeResetState(); });
            }
            void AudioInputProcessor::onContextAvailable(const std::string& jsonContext) {
                m_executor.submit([this, jsonContext]() { executeOnContextAvailable(jsonContext); });
            }
            void AudioInputProcessor::onContextFailure(const ContextRequestError error) {
                m_executor.submit([this, error]() { executeOnContextFailure(error); });
            }
            void AudioInputProcessor::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
            }
            void AudioInputProcessor::preHandleDirective(shared_ptr<DirectiveInfo> info) {
            }
            void AudioInputProcessor::handleDirective(shared_ptr<DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                if (!info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirective"));
                    return;
                }
                AVSMessage *avsMessage = new AVSMessage(info->directive->m_avsMessageHeader, (char*)info->directive->m_payload.data(), info->directive->m_endpoint);
                char *name = (char*)avsMessage->getName().data();
                if (strcmp((char*)avsMessage->getName().data(), (char*)STOP_CAPTURE.name.data()) == 0) {
                    ACSDK_METRIC_MSG(TAG, info->directive, Metrics::Location::AIP_RECEIVE);
                    submitMetric(m_metricRecorder, STOP_CAPTURE_RECEIVED_ACTIVITY_NAME,DataPointCounterBuilder{}.setName(STOP_CAPTURE_RECEIVED)
                        .increment(1).build(), info->directive);
                    handleStopCaptureDirective(info);
                } else if (strcmp(name, (char*)EXPECT_SPEECH.name.data()) == 0) handleExpectSpeechDirective(info);
                else if (strcmp(name, (char*)SET_END_OF_SPEECH_OFFSET.name.data()) == 0) handleSetEndOfSpeechOffsetDirective(info);
                else if (strcmp(name, (char*)SET_WAKE_WORD_CONFIRMATION.name.data()) == 0) handleSetWakeWordConfirmation(info);
                else if (strcmp(name, (char*)SET_SPEECH_CONFIRMATION.name.data()) == 0) handleSetSpeechConfirmation(info);
                else if (strcmp(name, (char*)SET_WAKE_WORDS.name.data()) == 0) handleSetWakeWords(info);
                else {
                    std::string nameSpace = avsMessage->getNamespace();
                    std::string errorMessage = "unexpected directive " + nameSpace + ":" + name;
                    char *_errorMessage = (char*)errorMessage.data();
                    char *unparsedDirective = (char*)info->directive->getUnparsedDirective().data();
                    m_exceptionEncounteredSender->sendExceptionEncountered(unparsedDirective,ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, _errorMessage);
                    if (info->result) info->result->setFailed(errorMessage);
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "unknownDirective").d("namespace", nameSpace).d("name", name));
                    removeDirective(info);
                }
            }
            void AudioInputProcessor::cancelDirective(shared_ptr<DirectiveInfo> info) {
                removeDirective(info);
            }
            void AudioInputProcessor::onDeregistered() {
                resetState();
            }
            void AudioInputProcessor::onFocusChanged(FocusState newFocus, MixingBehavior behavior) {
                ACSDK_DEBUG9(LX("onFocusChanged").d("newFocus", newFocus).d("MixingBehavior", behavior));
                m_executor.submit([this, newFocus]() { executeOnFocusChanged(newFocus); });
            }
            void AudioInputProcessor::onDialogUXStateChanged(DialogUXStateObserverInterface::DialogUXState newState) {
                m_executor.submit([this, newState]() { executeOnDialogUXStateChanged(newState); });
            }
            AudioInputProcessor::AudioInputProcessor(shared_ptr<DirectiveSequencerInterface> directiveSequencer, shared_ptr<MessageSenderInterface> messageSender,
                                                     shared_ptr<ContextManagerInterface> contextManager, shared_ptr<FocusManagerInterface> focusManager,
                                                     shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                     shared_ptr<UserInactivityMonitorInterface> userInactivityMonitor, shared_ptr<SystemSoundPlayerInterface> systemSoundPlayer,
                                                     shared_ptr<speechencoder::SpeechEncoder> speechEncoder, AudioProvider defaultAudioProvider,
                                                     shared_ptr<WakeWordConfirmationSetting> wakeWordConfirmation, shared_ptr<SpeechConfirmationSetting> speechConfirmation,
                                                     shared_ptr<WakeWordsSetting> wakeWordsSetting, shared_ptr<CapabilityConfiguration> capabilitiesConfiguration,
                                                     shared_ptr<PowerResourceManagerInterface> powerResourceManager, shared_ptr<MetricRecorderInterface> metricRecorder) :
                                                     CapabilityAgent{(char*)NAMESPACE.data(), exceptionEncounteredSender}, RequiresShutdown{"AudioInputProcessor"},
                                                     m_metricRecorder{metricRecorder}, m_directiveSequencer{directiveSequencer}, m_messageSender{messageSender},
                                                     m_contextManager{contextManager}, m_focusManager{focusManager}, m_userInactivityMonitor{userInactivityMonitor},
                                                     m_encoder{speechEncoder}, m_defaultAudioProvider{defaultAudioProvider}, m_lastAudioProvider{AudioProvider::null()},
                                                     m_KWDMetadataReader{nullptr}, m_state{ObserverInterface::State::IDLE}, m_focusState{FocusState::NONE},
                                                     m_preparingToSend{false}, m_initialDialogUXStateReceived{false}, m_localStopCapturePerformed{false},
                                                     m_systemSoundPlayer{systemSoundPlayer}, m_precedingExpectSpeechInitiator{nullptr},
                                                     m_wakeWordConfirmation{wakeWordConfirmation}, m_speechConfirmation{speechConfirmation},
                                                     m_wakeWordsSetting{wakeWordsSetting}, m_powerResourceManager{powerResourceManager} {
                m_capabilityConfigurations.insert(capabilitiesConfiguration);
                m_dialogUXStateObserverInterface = make_shared<DialogUXStateObserverInterface>();
            }
            std::string generateSupportedWakeWordsJson(const std::string& scope, const LocaleAssetsManagerInterface::WakeWordsSets& wakeWordsCombination) {
                JsonGenerator generator;
                generator.addStringArray(CAPABILITY_INTERFACE_SCOPES_KEY, list<std::string>({scope}));
                generator.addCollectionOfStringArray(CAPABILITY_INTERFACE_VALUES_KEY, wakeWordsCombination);
                return generator.toString();
            }
            shared_ptr<CapabilityConfiguration> getSpeechRecognizerCapabilityConfiguration(const LocaleAssetsManagerInterface& assetsManager) {
                unordered_map<std::string, std::string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, SPEECHRECOGNIZER_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, SPEECHRECOGNIZER_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, SPEECHRECOGNIZER_CAPABILITY_INTERFACE_VERSION});
                auto defaultWakeWords = assetsManager.getDefaultSupportedWakeWords();
                if (!defaultWakeWords.empty()) {
                    set<std::string> wakeWords;
                    wakeWords.insert(generateSupportedWakeWordsJson(CAPABILITY_INTERFACE_DEFAULT_LOCALE, defaultWakeWords));
                    for (const auto& entry : assetsManager.getLanguageSpecificWakeWords()) {
                        wakeWords.insert(generateSupportedWakeWordsJson(entry.first, entry.second));
                    }
                    for (const auto& entry : assetsManager.getLocaleSpecificWakeWords()) {
                        wakeWords.insert(generateSupportedWakeWordsJson(entry.first, entry.second));
                    }
                    JsonGenerator generator;
                    generator.addMembersArray(CAPABILITY_INTERFACE_WAKE_WORDS_KEY, wakeWords);
                    configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, generator.toString()});
                    ACSDK_DEBUG7(LX(__func__).d("wakeWords", generator.toString()));
                }
                return make_shared<CapabilityConfiguration>(configMap);
            }
            SettingEventMetadata AudioInputProcessor::getWakeWordsEventsMetadata() {
                return SettingEventMetadata{NAMESPACE, WAKE_WORDS_CHANGED_EVENT_NAME, WAKE_WORDS_REPORT_EVENT_NAME, WAKE_WORDS_PAYLOAD_KEY};
            }
            void AudioInputProcessor::doShutdown() {
                m_executor.shutdown();
                executeResetState();
                m_directiveSequencer.reset();
                m_messageSender.reset();
                m_contextManager.reset();
                m_focusManager.reset();
                m_userInactivityMonitor.reset();
                m_observers.clear();
            }
            future<bool> AudioInputProcessor::expectSpeechTimedOut() {
                return m_executor.submit([this]() { return executeExpectSpeechTimedOut(); });
            }
            void AudioInputProcessor::handleStopCaptureDirective(shared_ptr<DirectiveInfo> info) {
                m_stopCaptureReceivedTime = steady_clock::now();
                m_executor.submit([this, info]() {
                    bool stopImmediately = true;
                    executeStopCapture(stopImmediately, info);
                });
            }
            void AudioInputProcessor::handleExpectSpeechDirective(shared_ptr<DirectiveInfo> info) {
                int64_t timeout;
                std::string payload = (char*)info->directive->m_payload.data();
                bool found = jsonUtils::retrieveValue(payload, "timeoutInMilliseconds", &timeout);
                if (!found) {
                    static const char* errorMessage = "missing/invalid timeoutInMilliseconds";
                    char *unparsedDirective = (char*)info->directive->getUnparsedDirective().data();
                    m_exceptionEncounteredSender->sendExceptionEncountered(unparsedDirective, ExceptionErrorType::UNSUPPORTED_OPERATION, errorMessage);
                    if (info->result) info->result->setFailed(errorMessage);
                    ACSDK_ERROR(LX("handleExpectSpeechDirectiveFailed").d("reason", "missingJsonField").d("field", "timeoutInMilliseconds"));
                    removeDirective(info);
                    return;
                }
                m_executor.submit([this, timeout, info]() { executeExpectSpeech(milliseconds{timeout}, info); });
            }
            void AudioInputProcessor::handleSetEndOfSpeechOffsetDirective(shared_ptr<DirectiveInfo> info) {
                submitMetric(m_metricRecorder, STOP_CAPTURE_TO_END_OF_SPEECH_ACTIVITY_NAME,
                    DataPointDurationBuilder{duration_cast<milliseconds>(steady_clock::now() - m_stopCaptureReceivedTime)}
                             .setName(STOP_CAPTURE_TO_END_OF_SPEECH_METRIC_NAME).build(), info->directive);
                std::string payload = (char*)info->directive->m_payload.data();
                int64_t endOfSpeechOffset = 0;
                int64_t startOfSpeechTimestamp = 0;
                std::string startOfSpeechTimeStampInString;
                bool foundEnd = jsonUtils::retrieveValue(payload, END_OF_SPEECH_OFFSET_FIELD_NAME, &endOfSpeechOffset);
                bool foundStart = jsonUtils::retrieveValue(payload, START_OF_SPEECH_TIMESTAMP_FIELD_NAME, &startOfSpeechTimeStampInString);
                if (foundEnd && foundStart) {
                    auto offset = milliseconds(endOfSpeechOffset);
                    submitMetric(m_metricRecorder, END_OF_SPEECH_OFFSET_RECEIVED_ACTIVITY_NAME,DataPointDurationBuilder{offset}
                        .setName(END_OF_SPEECH_OFFSET_RECEIVED).build(), info->directive);
                    istringstream iss{startOfSpeechTimeStampInString};
                    iss >> startOfSpeechTimestamp;
                    ACSDK_DEBUG0(LX("handleSetEndOfSpeechOffsetDirective").d("startTimeSpeech(ms)", startOfSpeechTimestamp).d("endTimeSpeech(ms)",
                                 startOfSpeechTimestamp + endOfSpeechOffset));
                    info->result->setCompleted();
                } else {
                    std::string missing;
                    if (!foundEnd && !foundStart) missing = END_OF_SPEECH_OFFSET_FIELD_NAME + " and " + START_OF_SPEECH_TIMESTAMP_FIELD_NAME;
                    else if (!foundEnd) missing = END_OF_SPEECH_OFFSET_FIELD_NAME;
                    else missing = START_OF_SPEECH_TIMESTAMP_FIELD_NAME;
                    ACSDK_ERROR(LX("handleSetEndOfSpeechOffsetDirective").d("missing", missing));
                    info->result->setFailed("Missing parameter(s): " + missing);
                }
                removeDirective(info);
            }
            bool AudioInputProcessor::executeRecognize(AudioProvider provider, Initiator initiator, steady_clock::time_point startOfSpeechTimestamp,
                                                       AudioInputStream::Index begin, AudioInputStream::Index end, const std::string& keyword,
                                                       shared_ptr<const vector<char>> KWDMetadata, const std::string& initiatorToken) {
                if (Initiator::WAKEWORD == initiator && keyword.empty()) {
                    ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "emptyKeywordWithWakewordInitiator"));
                    return false;
                }
                AudioInputStream::Index preroll = provider.format.sampleRateHz / 2;
                bool falseWakewordDetection = Initiator::WAKEWORD == initiator && begin != INVALID_INDEX && begin >= preroll && end != INVALID_INDEX;
                JsonGenerator generator;
                generator.addMember(TYPE_KEY, initiatorToString(initiator));
                generator.startObject(PAYLOAD_KEY);
                if (falseWakewordDetection) {
                    generator.startObject(WAKEWORD_INDICES_KEY);
                    generator.addMember(START_INDEX_KEY, preroll);
                    generator.addMember(END_INDEX_KEY, preroll + (end - begin));
                    generator.finishObject();
                    begin -= preroll;
                }
                if (!keyword.empty()) generator.addMember(WAKE_WORD_KEY, string::stringToUpperCase(keyword));
                if (!initiatorToken.empty()) generator.addMember(TOKEN_KEY, initiatorToken);
                bool initiatedByWakeword = (Initiator::WAKEWORD == initiator) ? true : false;
                return executeRecognize(provider,generator.toString(), startOfSpeechTimestamp, begin, end, keyword, KWDMetadata, initiatedByWakeword,
                                        falseWakewordDetection);
            }
            bool AudioInputProcessor::executeRecognize(AudioProvider provider, const std::string& initiatorJson, steady_clock::time_point startOfSpeechTimestamp,
                                                       AudioInputStream::Index begin, AudioInputStream::Index end, const std::string& keyword,
                                                       shared_ptr<const vector<char>> KWDMetadata, bool initiatedByWakeword, bool falseWakewordDetection) {
                if (!provider.stream) {
                    ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "nullAudioInputStream"));
                    return false;
                }
                unordered_map<int, std::string> mapSampleRatesAVSEncoding = {{32000, "OPUS"}};
                std::string avsEncodingFormat;
                unordered_map<int, std::string>::iterator itSampleRateAVSEncoding;
                switch(provider.format.encoding) {
                    case AudioFormat::Encoding::LPCM:
                        if (provider.format.sampleRateHz != 16000) {
                            ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "unsupportedSampleRateForPCM").d("sampleRate", provider.format.sampleRateHz));
                            return false;
                        } else if (provider.format.sampleSizeInBits != 16) {
                            ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "unsupportedSampleSize").d("sampleSize", provider.format.sampleSizeInBits));
                            return false;
                        }
                        avsEncodingFormat = "AUDIO_L16_RATE_16000_CHANNELS_1";
                        break;
                    case AudioFormat::Encoding::OPUS:
                        itSampleRateAVSEncoding = mapSampleRatesAVSEncoding.find(provider.format.sampleRateHz);
                        if (itSampleRateAVSEncoding == mapSampleRatesAVSEncoding.end()) {
                            ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "unsupportedSampleRateForOPUS").d("sampleRate", provider.format.sampleRateHz));
                            return false;
                        }
                        avsEncodingFormat = itSampleRateAVSEncoding->second;
                        break;
                    default:
                        ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "unsupportedEncoding").d("encoding", provider.format.encoding));
                        return false;
                }
                if (provider.format.endianness != AudioFormat::Endianness::LITTLE) {
                    ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "unsupportedEndianness").d("endianness", provider.format.endianness));
                    return false;
                } else if (provider.format.numChannels != 1) {
                    ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "unsupportedNumChannels").d("channels", provider.format.numChannels));
                    return false;
                }
                switch(m_state) {
                    case ObserverInterface::State::IDLE: case ObserverInterface::State::EXPECTING_SPEECH: break;
                    case ObserverInterface::State::RECOGNIZING:
                        if (!m_lastAudioProvider.canBeOverridden) {
                            ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "Active audio provider can not be overridden"));
                            return false;
                        }
                        if (!provider.canOverride) {
                            ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "New audio provider can not override"));
                            return false;
                        }
                        if (m_reader) {
                            m_reader->close(AttachmentReader::ClosePoint::AFTER_DRAINING_CURRENT_BUFFER);
                            m_reader.reset();
                        }
                        break;
                    case ObserverInterface::State::BUSY:
                        ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "Barge-in is not permitted while busy"));
                        return false;
                }
                if (settings::WakeWordConfirmationSettingType::TONE == m_wakeWordConfirmation->get()) {
                    m_executor.submit([this]() { m_systemSoundPlayer->playTone(SystemSoundPlayerInterface::Tone::WAKEWORD_NOTIFICATION); });
                }
                const bool shouldBeEncoded = (m_encoder != nullptr);
                if (shouldBeEncoded && m_encoder->getContext()) avsEncodingFormat = m_encoder->getContext()->getAVSFormatName();
                JsonGenerator payloadGenerator;
                payloadGenerator.addMember(PROFILE_KEY, asrProfileToString(provider.profile));
                payloadGenerator.addMember(FORMAT_KEY, avsEncodingFormat);
                if (m_precedingExpectSpeechInitiator) {
                    if (!m_precedingExpectSpeechInitiator->empty()) payloadGenerator.addRawJsonMember(INITIATOR_KEY, *m_precedingExpectSpeechInitiator);
                    m_precedingExpectSpeechInitiator.reset();
                } else if (!initiatorJson.empty()) payloadGenerator.addRawJsonMember(INITIATOR_KEY, initiatorJson);
                payloadGenerator.addMember(START_OF_SPEECH_TIMESTAMP_FIELD_NAME, std::to_string(startOfSpeechTimestamp.time_since_epoch().count()));
                InProcessAttachmentReader::SDSTypeIndex offset = 0;
                AudioInputStream::Reader::Reference reference = AudioInputStream::Reader::Reference::BEFORE_WRITER;
                if (INVALID_INDEX != begin) {
                    offset = begin;
                    reference = AudioInputStream::Reader::Reference::ABSOLUTE;
                }
                if (shouldBeEncoded) {
                    ACSDK_DEBUG(LX("encodingAudio").d("format", avsEncodingFormat));
                    if (!m_encoder->startEncoding(provider.stream, provider.format, offset, reference)) {
                        ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "Failed to start encoder"));
                        return false;
                    }
                    offset = 0;
                    reference = AudioInputStream::Reader::Reference::BEFORE_WRITER;
                } else { ACSDK_DEBUG(LX("notEncodingAudio")); }
                m_reader = DefaultAttachmentReader<AudioInputStream>::create(ReaderPolicy::NONBLOCKING, shouldBeEncoded ? m_encoder->getEncodedStream() :
                                                                             provider.stream, offset, reference);
                if (!m_reader) {
                    ACSDK_ERROR(LX("executeRecognizeFailed").d("reason", "Failed to create attachment reader"));
                    return false;
                }
                if (KWDMetadata) {
                    m_KWDMetadataReader = avsCommon::avs::attachment::AttachmentUtils::createAttachmentReader(*KWDMetadata);
                    if (!m_KWDMetadataReader) {
                        ACSDK_ERROR(LX("sendingKWDMetadataFailed").d("reason", "Failed to create attachment reader"));
                    }
                }
                m_preCachedDialogRequestId = uuidGeneration::generateUUID();
                setState(ObserverInterface::State::RECOGNIZING);
                m_preparingToSend = true;
                m_localStopCapturePerformed = false;
                m_contextManager->getContextWithoutReportableStateProperties(std::make_shared<ContextRequesterInterface>());
                m_expectingSpeechTimer.stop();
                m_lastAudioProvider = provider;
                m_recognizePayload = payloadGenerator.toString();
                m_recognizeRequest.reset();
                submitMetric(m_metricRecorder, START_OF_UTTERANCE_ACTIVITY_NAME,DataPointCounterBuilder{}.setName(START_OF_UTTERANCE).increment(1)
                             .build(), m_preCachedDialogRequestId);
                if (initiatedByWakeword) {
                    auto duration = milliseconds((end - begin) * MILLISECONDS_PER_SECOND / provider.format.sampleRateHz);
                    auto startOfStreamTimestamp = startOfSpeechTimestamp;
                    if (falseWakewordDetection) startOfStreamTimestamp -= PREROLL_DURATION;
                    submitMetric(m_metricRecorder,MetricEventBuilder{}.setActivityName(WW_DURATION_ACTIVITY_NAME).addDataPoint(DataPointDurationBuilder{duration}.
                                 setName(WW_DURATION).build()).addDataPoint(DataPointDurationBuilder{duration_cast<milliseconds>(startOfStreamTimestamp.time_since_epoch())}
                                 .setName(START_OF_STREAM_TIMESTAMP).build()), m_preCachedDialogRequestId);
                    ACSDK_DEBUG(LX(__func__).d("WW_DURATION(ms)", duration.count()));
                }
                return true;
            }
            void AudioInputProcessor::executeOnContextAvailable(const std::string jsonContext) {
                ACSDK_DEBUG(LX("executeOnContextAvailable").sensitive("jsonContext", jsonContext));
                if (m_state != ObserverInterface::State::RECOGNIZING) {
                    ACSDK_ERROR(LX("executeOnContextAvailableFailed").d("reason", "Not permitted in current state").d("state", m_state));
                    return;
                }
                if (!m_reader) {
                    ACSDK_ERROR(LX("executeOnContextAvailableFailed").d("reason", "nullReader"));
                    executeResetState();
                    return;
                }
                if (m_recognizePayload.empty()) {
                    ACSDK_ERROR(LX("executeOnContextAvailableFailed").d("reason", "payloadEmpty"));
                    executeResetState();
                    return;
                }
                if (m_focusState != FocusState::FOREGROUND) {
                    ChannelObserverInterface coi = reinterpret_cast<ChannelObserverInterface&>(*this);
                    shared_ptr<ChannelObserverInterface> channelObserverInterface =  make_shared<ChannelObserverInterface>(coi);
                    auto activity = FocusManagerInterface::Activity::create(NAMESPACE, channelObserverInterface);
                    if (!m_focusManager->acquireChannel(CHANNEL_NAME, activity)) {
                        ACSDK_ERROR(LX("executeOnContextAvailableFailed").d("reason", "Unable to acquire channel"));
                        executeResetState();
                        return;
                    }
                }
                if (!m_preCachedDialogRequestId.empty()) {
                    m_directiveSequencer->setDialogRequestId(m_preCachedDialogRequestId);
                    m_preCachedDialogRequestId.clear();
                }
                auto msgIdAndJsonEvent = buildJsonEventString("Recognize", m_directiveSequencer->getDialogRequestId(), m_recognizePayload, jsonContext);
                m_recognizeRequest = std::make_shared<avsCommon::avs::MessageRequest>(msgIdAndJsonEvent.second);
                if (m_KWDMetadataReader) m_recognizeRequest->addAttachmentReader(KWD_METADATA_FIELD_NAME, m_KWDMetadataReader);
                m_recognizeRequest->addAttachmentReader(AUDIO_ATTACHMENT_FIELD_NAME, m_reader);
                m_KWDMetadataReader.reset();
                if (avsCommon::avs::FocusState::FOREGROUND == m_focusState) sendRequestNow();
            }
            void AudioInputProcessor::executeOnContextFailure(const ContextRequestError error) {
                ACSDK_ERROR(LX("executeOnContextFailure").d("error", error));
                executeResetState();
            }
            void AudioInputProcessor::executeOnFocusChanged(avsCommon::avs::FocusState newFocus) {
                ACSDK_DEBUG(LX("executeOnFocusChanged").d("newFocus", newFocus));
                m_focusState = newFocus;
                if (newFocus != FocusState::FOREGROUND) {
                    ACSDK_DEBUG(LX("executeOnFocusChanged").d("reason", "Lost focus"));
                    executeResetState();
                    return;
                }
                if (m_state != ObserverInterface::State::RECOGNIZING) return;
                sendRequestNow();
            }
            bool AudioInputProcessor::executeStopCapture(bool stopImmediately, shared_ptr<DirectiveInfo> info) {
                if (info && info->isCancelled) {
                    ACSDK_DEBUG(LX("stopCaptureIgnored").d("reason", "isCancelled"));
                    return true;
                }
                if (m_state != ObserverInterface::State::RECOGNIZING) {
                    static const char* errorMessage = "StopCapture only allowed in RECOGNIZING state.";
                    auto returnValue = false;
                    if (info) {
                        if (info->result) {
                            if (m_localStopCapturePerformed) {
                                m_localStopCapturePerformed = false;
                                returnValue = true;
                                info->result->setCompleted();
                                ACSDK_INFO(LX("executeStopCapture").m("StopCapture directive ignored because local StopCapture was performed."));
                            } else {
                                info->result->setFailed(errorMessage);
                                ACSDK_ERROR(LX("executeStopCaptureFailed").d("reason", "invalidState").d("expectedState", "RECOGNIZING").d("state", m_state));
                            }
                        }
                        removeDirective(info);
                    }
                    return returnValue;
                }
                if (nullptr == info) m_localStopCapturePerformed = true;
                function<void()> stopCapture = [=] {
                    ACSDK_DEBUG(LX("stopCapture").d("stopImmediately", stopImmediately));
                    if (m_encoder) m_encoder->stopEncoding(stopImmediately);
                    else {
                        if (stopImmediately) m_reader->close(AttachmentReader::ClosePoint::IMMEDIATELY);
                        else m_reader->close(AttachmentReader::ClosePoint::AFTER_DRAINING_CURRENT_BUFFER);
                    }
                    m_reader.reset();
                    setState(ObserverInterface::State::BUSY);
                    if (info) {
                        if (info->result) info->result->setCompleted();
                        removeDirective(info);
                    }
                    if (m_speechConfirmation->get() == SpeechConfirmationSettingType::TONE) {
                        m_systemSoundPlayer->playTone(SystemSoundPlayerInterface::Tone::END_SPEECH);
                    }
                };
                if (m_preparingToSend) m_deferredStopCapture = stopCapture;
                else stopCapture();
                return true;
            }
            void AudioInputProcessor::executeResetState() {
                ACSDK_DEBUG(LX(__func__));
                m_expectingSpeechTimer.stop();
                m_precedingExpectSpeechInitiator.reset();
                if (m_reader) m_reader->close();
                if (m_encoder) m_encoder->stopEncoding(true);
                m_reader.reset();
                m_KWDMetadataReader.reset();
                if (m_recognizeRequestSent) {
                    MessageRequestObserverInterface mroi = reinterpret_cast<MessageRequestObserverInterface&>(*this);
                    shared_ptr<MessageRequestObserverInterface> messageObserverInterface = make_shared<MessageRequestObserverInterface>(mroi);
                    m_recognizeRequestSent->removeObserver(messageObserverInterface);
                    m_recognizeRequestSent.reset();
                }
                m_recognizeRequest.reset();
                m_preparingToSend = false;
                m_deferredStopCapture = nullptr;
                if (m_focusState != avsCommon::avs::FocusState::NONE) {
                    shared_ptr<ChannelObserverInterface> channelObserverInterface = make_shared<ChannelObserverInterface>();
                    m_focusManager->releaseChannel(CHANNEL_NAME, channelObserverInterface);
                }
                m_focusState = FocusState::NONE;
                setState(ObserverInterface::State::IDLE);
            }
            bool AudioInputProcessor::executeExpectSpeech(milliseconds timeout, shared_ptr<DirectiveInfo> info) {
                if (info && info->isCancelled) {
                    ACSDK_DEBUG(LX("expectSpeechIgnored").d("reason", "isCancelled"));
                    return true;
                }
                if (m_state != ObserverInterface::State::IDLE && m_state != ObserverInterface::State::BUSY) {
                    static const char* errorMessage = "ExpectSpeech only allowed in IDLE or BUSY state.";
                    if (info->result) info->result->setFailed(errorMessage);
                    removeDirective(info);
                    ACSDK_ERROR(LX("executeExpectSpeechFailed").d("reason", "invalidState").d("expectedState", "IDLE/BUSY").d("state", m_state));
                    return false;
                }
                if (m_precedingExpectSpeechInitiator) {
                    ACSDK_ERROR(LX(__func__).d("reason", "precedingExpectSpeechInitiatorUnconsumed").d(INITIATOR_KEY.c_str(), *m_precedingExpectSpeechInitiator));
                }
                m_precedingExpectSpeechInitiator = memory::make_unique<std::string>("");
                Value _payload{(char*)info->directive->m_payload.data(), info->directive->m_payload.length()};
                bool found = jsonUtils::retrieveValue1(_payload, INITIATOR_KEY.data(), m_precedingExpectSpeechInitiator.get());
                if (found) { ACSDK_DEBUG(LX(__func__).d("initiatorFound", *m_precedingExpectSpeechInitiator)); }
                else *m_precedingExpectSpeechInitiator = "";
                if (!m_expectingSpeechTimer.start(timeout, bind(&AudioInputProcessor::expectSpeechTimedOut, this)).valid()) {
                    ACSDK_ERROR(LX("executeExpectSpeechFailed").d("reason", "startTimerFailed"));
                }
                setState(ObserverInterface::State::EXPECTING_SPEECH);
                if (info->result) info->result->setCompleted();
                removeDirective(info);
                if (m_lastAudioProvider && m_lastAudioProvider.alwaysReadable) return executeRecognize(m_lastAudioProvider, "");
                else if (m_defaultAudioProvider && m_defaultAudioProvider.alwaysReadable) return executeRecognize(m_defaultAudioProvider, "");
                return true;
            }
            bool AudioInputProcessor::executeExpectSpeechTimedOut() {
                if (m_state != ObserverInterface::State::EXPECTING_SPEECH) {
                    ACSDK_ERROR(LX("executeExpectSpeechTimedOutFailure").d("reason", "invalidState").d("expectedState", "EXPECTING_SPEECH").d("state", m_state));
                    return false;
                }
                m_precedingExpectSpeechInitiator.reset();
                auto msgIdAndJsonEvent = buildJsonEventString("ExpectSpeechTimedOut");
                auto request = make_shared<MessageRequest>(msgIdAndJsonEvent.second);
                shared_ptr<MessageRequestObserverInterface> messageRequestObserverInterface = make_shared<MessageRequestObserverInterface>();
                request->addObserver(messageRequestObserverInterface);
                m_messageSender->sendMessage(request);
                setState(ObserverInterface::State::IDLE);
                ACSDK_ERROR(LX("executeExpectSpeechFailed").d("reason", "Timed Out"));
                return true;
            }
            void AudioInputProcessor::executeOnDialogUXStateChanged(DialogUXStateObserverInterface::DialogUXState newState) {
                if (!m_initialDialogUXStateReceived) {
                    m_initialDialogUXStateReceived = true;
                    return;
                }
                if (newState != DialogUXStateObserverInterface::DialogUXState::IDLE) return;
                executeResetState();
            }
            void AudioInputProcessor::setState(ObserverInterface::State state) {
                if (m_state == state) return;
                if (ObserverInterface::State::RECOGNIZING == m_state || ObserverInterface::State::RECOGNIZING == state) m_userInactivityMonitor->onUserActive();
                ACSDK_DEBUG(LX("setState").d("from", m_state).d("to", state));
                m_state = state;
                managePowerResource(m_state);
                for (auto observer : m_observers) observer->onStateChanged(m_state);
            }
            void AudioInputProcessor::removeDirective(shared_ptr<DirectiveInfo> info) {
                if (info->directive && info->result) CapabilityAgent::removeDirective(info->directive->messageId());
            }
            void AudioInputProcessor::sendRequestNow() {
                ACSDK_DEBUG(LX(__func__));
                if (m_recognizeRequest) {
                    ACSDK_METRIC_IDS(TAG, "Recognize", "", "", Metrics::Location::AIP_SEND);
                    std::shared_ptr<MessageRequestObserverInterface> messageRequestObserverInterface = make_shared<MessageRequestObserverInterface>();
                    if (m_recognizeRequestSent && (m_recognizeRequestSent != m_recognizeRequest)) {
                        m_recognizeRequestSent->removeObserver(messageRequestObserverInterface);
                    }
                    m_recognizeRequestSent = m_recognizeRequest;
                    m_recognizeRequestSent->addObserver(messageRequestObserverInterface);
                    m_messageSender->sendMessage(m_recognizeRequestSent);
                    m_recognizeRequest.reset();
                    m_preparingToSend = false;
                    if (m_deferredStopCapture) {
                        m_deferredStopCapture();
                        m_deferredStopCapture = nullptr;
                    }
                }
            }
            void AudioInputProcessor::onExceptionReceived(const std::string& exceptionMessage) {
                ACSDK_ERROR(LX("onExceptionReceived").d("exception", exceptionMessage));
                resetState();
            }
            void AudioInputProcessor::onSendCompleted(MessageRequestObserverInterface::Status status) {
                ACSDK_DEBUG(LX("onSendCompleted").d("status", status));
                if (status == MessageRequestObserverInterface::Status::SUCCESS || status == MessageRequestObserverInterface::Status::PENDING) {
                    ACSDK_DEBUG5(LX("stopCapture").d("reason", "streamClosed"));
                    stopCapture();
                    return;
                }
                ACSDK_DEBUG(LX("resetState").d("dueToStatus", status));
                resetState();
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> AudioInputProcessor::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            void AudioInputProcessor::handleDirectiveFailure(const std::string& errorMessage, shared_ptr<DirectiveInfo> info, ExceptionErrorType errorType) {
                m_exceptionEncounteredSender->sendExceptionEncountered((char*)info->directive->getUnparsedDirective().data(), errorType, errorMessage);
                if (info->result) info->result->setFailed(errorMessage);
                removeDirective(info);
                ACSDK_ERROR(LX("handleDirectiveFailed").d("error", errorMessage));
            }
            SettingEventMetadata AudioInputProcessor::getWakeWordConfirmationMetadata() {
                return settings::SettingEventMetadata{NAMESPACE, WAKE_WORD_CONFIRMATION_CHANGED_EVENT_NAME, WAKE_WORD_CONFIRMATION_REPORT_EVENT_NAME,
                                                      WAKE_WORD_CONFIRMATION_PAYLOAD_KEY};
            }
            SettingEventMetadata AudioInputProcessor::getSpeechConfirmationMetadata() {
                return SettingEventMetadata{NAMESPACE, SPEECH_CONFIRMATION_CHANGED_EVENT_NAME, SPEECH_CONFIRMATION_REPORT_EVENT_NAME, SPEECH_CONFIRMATION_PAYLOAD_KEY};
            }
            bool AudioInputProcessor::handleSetWakeWordConfirmation(shared_ptr<DirectiveInfo> info) {
                std::string jsonValue;
                char *payload = (char*)info->directive->m_payload.data();
                bool found = jsonUtils::retrieveValue(payload, WAKE_WORD_CONFIRMATION_PAYLOAD_KEY, &jsonValue);
                if (!found) {
                    std::string errorMessage = "missing " + WAKE_WORD_CONFIRMATION_PAYLOAD_KEY;
                    sendExceptionEncounteredAndReportFailed(info, errorMessage, avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                WakeWordConfirmationSettingType value = settings::getWakeWordConfirmationDefault();
                stringstream ss{jsonValue};
                ss >> value;
                if (ss.fail()) {
                    std::string errorMessage = "invalid " + WAKE_WORD_CONFIRMATION_PAYLOAD_KEY;
                    sendExceptionEncounteredAndReportFailed(info, errorMessage, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                auto executeChange = [this, value]() { m_wakeWordConfirmation->setAvsChange(value); };
                m_executor.submit(executeChange);
                if (info->result) info->result->setCompleted();
                removeDirective(info);
                return true;
            }
            bool AudioInputProcessor::handleSetSpeechConfirmation(shared_ptr<DirectiveInfo> info) {
                std::string jsonValue;
                char *payload = (char*)info->directive->m_payload.data();
                bool found = jsonUtils::retrieveValue(payload, SPEECH_CONFIRMATION_PAYLOAD_KEY, &jsonValue);
                if (!found) {
                    std::string errorMessage = "missing/invalid " + SPEECH_CONFIRMATION_PAYLOAD_KEY;
                    sendExceptionEncounteredAndReportFailed(info, errorMessage, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                SpeechConfirmationSettingType value;
                stringstream ss{jsonValue};
                ss >> value;
                if (ss.fail()) {
                    std::string errorMessage = "invalid " + SPEECH_CONFIRMATION_PAYLOAD_KEY;
                    sendExceptionEncounteredAndReportFailed(info, errorMessage, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                auto executeChange = [this, value]() { m_speechConfirmation->setAvsChange(value); };
                m_executor.submit(executeChange);
                if (info->result) info->result->setCompleted();
                removeDirective(info);
                return true;
            }
            bool AudioInputProcessor::handleSetWakeWords(shared_ptr<DirectiveInfo> info) {
                char *payload = (char*)info->directive->m_payload.data();
                auto wakeWords = jsonUtils::retrieveStringArray<WakeWords>(payload, WAKE_WORDS_PAYLOAD_KEY);
                if (wakeWords.empty()) {
                    std::string errorMessage = "missing/invalid " + WAKE_WORDS_PAYLOAD_KEY;
                    sendExceptionEncounteredAndReportFailed(info, errorMessage, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                if (!m_wakeWordsSetting) {
                    std::string errorMessage = "Wake words are not supported in this device";
                    sendExceptionEncounteredAndReportFailed(info, errorMessage, ExceptionErrorType::UNSUPPORTED_OPERATION);
                    return false;
                }
                m_executor.submit([this, wakeWords, info]() { m_wakeWordsSetting->setAvsChange(wakeWords); });
                if (info->result) info->result->setCompleted();
                removeDirective(info);
                return true;
            }
            void AudioInputProcessor::managePowerResource(ObserverInterface::State newState) {
                if (!m_powerResourceManager) return;
                ACSDK_DEBUG5(LX(__func__).d("state", newState));
                switch(newState) {
                    case ObserverInterface::State::RECOGNIZING: case ObserverInterface::State::EXPECTING_SPEECH:
                        m_powerResourceManager->acquirePowerResource(POWER_RESOURCE_COMPONENT_NAME);
                        break;
                    case ObserverInterface::State::BUSY: case ObserverInterface::State::IDLE:
                        m_powerResourceManager->releasePowerResource(POWER_RESOURCE_COMPONENT_NAME);
                        break;
                }
            }
            void AudioInputProcessor::onConnectionStatusChanged(bool connected) {
                if (!connected) m_executor.submit([this]() { return executeDisconnected(); });
            }
            void AudioInputProcessor::executeDisconnected() {
                if (ObserverInterface::State::IDLE != m_state) {
                    ACSDK_WARN(LX(__func__).d("state", AudioInputProcessorObserverInterface::stateToString(m_state)));
                    executeResetState();
                }
            }
        }
    }
}