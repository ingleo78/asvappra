#include <chrono>
#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <json/en.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/SpeakerConstants/SpeakerConstants.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <util/Metrics.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include "SpeakerManagerConstants.h"
#include "SpeakerManager.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            using namespace chrono;
            using namespace speakerConstants;
            using namespace json;
            using namespace configuration;
            using namespace logger;
            using SMI = SpeakerManagerInterface;
            static const string SPEAKER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
            static const string SPEAKER_CAPABILITY_INTERFACE_NAME = "Speaker";
            static const string SPEAKER_CAPABILITY_INTERFACE_VERSION = "1.0";
            static const vector<int> DEFAULT_RETRY_TABLE = {milliseconds(10).count(), milliseconds(20).count(), milliseconds(40).count()};
            static const string TAG{"SpeakerManager"};
            static const string SPEAKERMANAGER_CONFIGURATION_ROOT_KEY = "speakerManagerCapabilityAgent";
            static const string SPEAKERMANAGER_MIN_UNMUTE_VOLUME_KEY = "minUnmuteVolume";
            static const string SPEAKER_MANAGER_METRIC_PREFIX = "SPEAKER_MANAGER-";
            #define LX(event) LogEntry(TAG, event)
            template <class T> static bool withinBounds(T value, T min, T max) {
                if (value < min || value > max) {
                    ACSDK_ERROR(LX("checkBoundsFailed").d("value", value).d("min", min).d("max", max));
                    return false;
                }
                return true;
            }
            static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const string& eventName, int count) {
                auto metricEventBuilder = MetricEventBuilder{}.setActivityName(SPEAKER_MANAGER_METRIC_PREFIX + eventName)
                                              .addDataPoint(DataPointCounterBuilder{}.setName(eventName).increment(count).build());
                auto metricEvent = metricEventBuilder.build();
                if (metricEvent == nullptr) {
                    ACSDK_ERROR(LX("Error creating metric."));
                    return;
                }
                recordMetric(metricRecorder, metricEvent);
            }
            static shared_ptr<CapabilityConfiguration> getSpeakerCapabilityConfiguration();
            shared_ptr<SpeakerManager> SpeakerManager::create(const vector<shared_ptr<ChannelVolumeInterface>>& groupVolumeInterfaces,
                                                              shared_ptr<ContextManagerInterface> contextManager,
                                                              shared_ptr<MessageSenderInterface> messageSender,
                                                              shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                              shared_ptr<MetricRecorderInterface> metricRecorder) {
                if (!contextManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
                    return nullptr;
                } else if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                } else if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncounteredSender"));
                    return nullptr;
                }
                int minUnmuteVolume = MIN_UNMUTE_VOLUME;
                auto configurationRoot = ConfigurationNode::getRoot()[SPEAKERMANAGER_CONFIGURATION_ROOT_KEY];
                configurationRoot.getInt(SPEAKERMANAGER_MIN_UNMUTE_VOLUME_KEY, &minUnmuteVolume, MIN_UNMUTE_VOLUME);
                auto speakerManager = shared_ptr<SpeakerManager>(new SpeakerManager(groupVolumeInterfaces, contextManager, messageSender,
                                                                 exceptionEncounteredSender, minUnmuteVolume, metricRecorder));
                return speakerManager;
            }
            SpeakerManager::SpeakerManager(const vector<shared_ptr<ChannelVolumeInterface>>& groupVolumeInterfaces,
                                           shared_ptr<ContextManagerInterface> contextManager,
                                           shared_ptr<MessageSenderInterface> messageSender,
                                           shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender, const int minUnmuteVolume,
                                           shared_ptr<MetricRecorderInterface> metricRecorder) : CapabilityAgent{NAMESPACE, exceptionEncounteredSender},
                                           RequiresShutdown{"SpeakerManager"}, m_metricRecorder{metricRecorder}, m_contextManager{contextManager},
                                           m_messageSender{messageSender}, m_minUnmuteVolume{minUnmuteVolume}, m_retryTimer{DEFAULT_RETRY_TABLE},
                                           m_maxRetries{DEFAULT_RETRY_TABLE.size()}, m_maximumVolumeLimit{AVS_SET_VOLUME_MAX} {
                for (auto groupVolume : groupVolumeInterfaces) {
                    m_speakerMap.insert(pair<Type, shared_ptr<ChannelVolumeInterface>>(groupVolume->getSpeakerType(), groupVolume));
                }
                ACSDK_INFO(LX("mapCreated").d("numSpeakerVolume", m_speakerMap.count(Type::AVS_SPEAKER_VOLUME))
                    .d("numAlertsVolume", m_speakerMap.count(Type::AVS_ALERTS_VOLUME)));
                const auto type = Type::AVS_SPEAKER_VOLUME;
                if (m_speakerMap.count(type)) {
                    SpeakerSettings settings;
                    if (!validateSpeakerSettingsConsistency(type, &settings) || !updateContextManager(type, settings)) {
                        ACSDK_ERROR(LX("initialUpdateContextManagerFailed"));
                    }
                }
                m_capabilityConfigurations.insert(getSpeakerCapabilityConfiguration());
            }
            shared_ptr<CapabilityConfiguration> getSpeakerCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, SPEAKER_CAPABILITY_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, SPEAKER_CAPABILITY_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, SPEAKER_CAPABILITY_INTERFACE_VERSION});
                return std::make_shared<CapabilityConfiguration>(configMap);
            }
            DirectiveHandlerConfiguration SpeakerManager::getConfiguration() const {
                auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                DirectiveHandlerConfiguration configuration;
                configuration[SET_VOLUME] = audioNonBlockingPolicy;
                configuration[ADJUST_VOLUME] = audioNonBlockingPolicy;
                configuration[SET_MUTE] = audioNonBlockingPolicy;
                return configuration;
            }
            void SpeakerManager::doShutdown() {
                m_waitCancelEvent.wakeUp();
                m_executor.shutdown();
                m_messageSender.reset();
                m_contextManager.reset();
                m_observers.clear();
                m_speakerMap.clear();
            }
            void SpeakerManager::preHandleDirective(shared_ptr<DirectiveInfo> info) {}
            void SpeakerManager::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            };
            bool SpeakerManager::parseDirectivePayload(string payload, Document* document) {
                if (!document) {
                    ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", "nullDocument"));
                    return false;
                }
                ParseResult result = document->Parse(payload.data());
                if (!result) {
                    ACSDK_ERROR(LX("parseDirectivePayloadFailed").d("reason", "parseFailed").d("error", GetParseError_En(result.Code()))
                        .d("offset", result.Offset()));
                    return false;
                }
                return true;
            }
            void SpeakerManager::sendExceptionEncountered(shared_ptr<DirectiveInfo> info, const string& message, ExceptionErrorType type) {
                m_executor.submit([this, info, message, type] {
                    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);
                    if (info && info->result) info->result->setFailed(message);
                    removeDirective(info);
                });
            }
            void SpeakerManager::executeSetHandlingCompleted(shared_ptr<DirectiveInfo> info) {
                if (info && info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void SpeakerManager::removeDirective(std::shared_ptr<DirectiveInfo> info) {
                if (info->directive && info->result) {
                    CapabilityAgent::removeDirective(info->directive->getMessageId());
                    m_waitCancelEvent.wakeUp();
                }
            }
            void SpeakerManager::executeSendSpeakerSettingsChangedEvent(const string& eventName, SpeakerSettings settings) {
                ACSDK_DEBUG9(LX("executeSendSpeakerSettingsChangedEvent"));
                Document payload(kObjectType);
                Value _VOLUME_KEY{VOLUME_KEY, strlen(VOLUME_KEY)};
                Value _MUTED_KEY{MUTED_KEY, strlen(MUTED_KEY)};
                payload.AddMember(_VOLUME_KEY, settings.volume, payload.GetAllocator());
                payload.AddMember(_MUTED_KEY, settings.mute, payload.GetAllocator());
                StringBuffer buffer;
                rapidjson::Writer<StringBuffer> writer(buffer);
                if (!payload.Accept(writer)) {
                    ACSDK_ERROR(LX("executeSendSpeakerSettingsChangedEventFailed").d("reason", "writerRefusedJsonObject"));
                    return;
                }
                auto event = buildJsonEventString(eventName, "", buffer.GetString());
                auto request = make_shared<MessageRequest>(event.second);
                m_messageSender->sendMessage(request);
            }
            void SpeakerManager::handleDirective(shared_ptr<DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                const string directiveName = info->directive->getName();
                Type directiveType = Type::AVS_SPEAKER_VOLUME;
                Document payload(kObjectType);
                if (!parseDirectivePayload(info->directive->getPayload(), &payload)) {
                    sendExceptionEncountered(info, "Payload Parsing Failed", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return;
                }
                if (directiveName == SET_VOLUME.name) {
                    int64_t volume;
                    if (jsonUtils::retrieveValue(payload, VOLUME_KEY, &volume) &&
                        withinBounds(volume, static_cast<int64_t>(AVS_SET_VOLUME_MIN), static_cast<int64_t>(AVS_SET_VOLUME_MAX))) {
                        m_executor.submit([this, volume, directiveType, info] {
                            if (m_speakerMap.count(directiveType) == 0) {
                                ACSDK_INFO(LX("noSpeakersRegistered").d("type", directiveType).m("swallowingDirective"));
                                executeSetHandlingCompleted(info);
                                return;
                            }
                            if (executeSetMute(directiveType,false,SMI::NotificationProperties(Source::DIRECTIVE,
                                false, false)) && executeSetVolume(directiveType, static_cast<int8_t>(volume),
                                SMI::NotificationProperties(Source::DIRECTIVE))) {
                                executeSetHandlingCompleted(info);
                            } else sendExceptionEncountered(info, "SetVolumeFailed", ExceptionErrorType::INTERNAL_ERROR);
                        });
                    } else {
                        sendExceptionEncountered(info, "Parsing Valid Payload Value Failed", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    }
                } else if (directiveName == ADJUST_VOLUME.name) {
                    int64_t delta;
                    if (jsonUtils::retrieveValue(payload, VOLUME_KEY, &delta) &&
                        withinBounds(delta, static_cast<int64_t>(AVS_ADJUST_VOLUME_MIN), static_cast<int64_t>(AVS_ADJUST_VOLUME_MAX))) {
                        m_executor.submit([this, delta, directiveType, info] {
                            if (m_speakerMap.count(directiveType) == 0) {
                                ACSDK_INFO(LX("noSpeakersRegistered").d("type", directiveType).m("swallowingDirective"));
                                executeSetHandlingCompleted(info);
                                return;
                            }
                            if (executeSetMute(directiveType,false,SMI::NotificationProperties(Source::DIRECTIVE,
                                false, false)) && executeAdjustVolume(directiveType, static_cast<int8_t>(delta),
                                SMI::NotificationProperties(Source::DIRECTIVE))) {
                                executeSetHandlingCompleted(info);
                            } else {
                                sendExceptionEncountered(info, "SetVolumeFailed", ExceptionErrorType::INTERNAL_ERROR);
                            }
                        });
                    } else {
                        sendExceptionEncountered(info, "Parsing Valid Payload Value Failed", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    }
                } else if (directiveName == SET_MUTE.name) {
                    bool mute = false;
                    if (jsonUtils::retrieveValue(payload, MUTE_KEY, &mute)) {
                        m_executor.submit([this, mute, directiveType, info] {
                            if (m_speakerMap.count(directiveType) == 0) {
                                ACSDK_INFO(LX("noSpeakersRegistered").d("type", directiveType).m("swallowingDirective"));
                                executeSetHandlingCompleted(info);
                                return;
                            }
                            bool success = true;
                            if (!mute) success = executeRestoreVolume(directiveType,Source::DIRECTIVE);
                            success &= executeSetMute(directiveType, mute, SMI::NotificationProperties(Source::DIRECTIVE));
                            if (success) executeSetHandlingCompleted(info);
                            else sendExceptionEncountered(info, "SetMuteFailed", ExceptionErrorType::INTERNAL_ERROR);
                        });
                    } else {
                        sendExceptionEncountered(info, "Parsing Valid Payload Value Failed", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    }
                } else {
                    sendExceptionEncountered(info, "Unexpected Directive", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                }
            }
            void SpeakerManager::cancelDirective(shared_ptr<DirectiveInfo> info) {
                removeDirective(info);
            }
            void SpeakerManager::addSpeakerManagerObserver(shared_ptr<SpeakerManagerObserverInterface> observer) {
                ACSDK_DEBUG9(LX("addSpeakerManagerObserverCalled"));
                if (!observer) {
                    ACSDK_ERROR(LX("addSpeakerManagerObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                ACSDK_DEBUG9(LX("addSpeakerManagerObserver").d("observer", observer.get()));
                m_executor.submit([this, observer] {
                    if (!m_observers.insert(observer).second) {
                        ACSDK_ERROR(LX("addSpeakerManagerObserverFailed").d("reason", "duplicateObserver"));
                    }
                });
            }
            void SpeakerManager::removeSpeakerManagerObserver(shared_ptr<SpeakerManagerObserverInterface> observer) {
                ACSDK_DEBUG9(LX("removeSpeakerManagerObserverCalled"));
                if (!observer) {
                    ACSDK_ERROR(LX("removeSpeakerManagerObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                ACSDK_DEBUG9(LX("removeSpeakerManagerObserver").d("observer", observer.get()));
                m_executor.submit([this, observer] {
                    if (m_observers.erase(observer) == 0) {
                        ACSDK_WARN(LX("removeSpeakerManagerObserverFailed").d("reason", "nonExistentObserver"));
                    }
                });
            }
            bool SpeakerManager::validateSpeakerSettingsConsistency(Type type, SpeakerSettings* settings) {
                auto beginIteratorAndEndIterator = m_speakerMap.equal_range(type);
                const auto begin = beginIteratorAndEndIterator.first;
                const auto end = beginIteratorAndEndIterator.second;
                bool consistent = true;
                SpeakerSettings comparator;
                if (begin == end) {
                    ACSDK_ERROR(LX("validateSpeakerSettingsConsistencyFailed").d("reason", "noSpeakersWithTypeFound").d("type", type));
                    submitMetric(m_metricRecorder, "noSpeakersWithType", 1);
                    return false;
                }
                submitMetric(m_metricRecorder, "noSpeakersWithType", 0);
                if (!begin->second->getSpeakerSettings(&comparator)) {
                    ACSDK_ERROR(LX("validateSpeakerSettingsConsistencyFailed").d("reason", "gettingSpeakerSettingsFailed"));
                    submitMetric(m_metricRecorder, "speakersCannotGetSetting", 1);
                    return false;
                }
                multimap<Type, shared_ptr<ChannelVolumeInterface>>::iterator typeAndSpeakerIterator = begin;
                while(++typeAndSpeakerIterator != end) {
                    auto volumeGroup = typeAndSpeakerIterator->second;
                    SpeakerInterface::SpeakerSettings temp;
                    if (!volumeGroup->getSpeakerSettings(&temp)) {
                        ACSDK_ERROR(LX("validateSpeakerSettingsConsistencyFailed").d("reason", "gettingSpeakerSettingsFailed"));
                        submitMetric(m_metricRecorder, "speakersCannotGetSetting", 1);
                        return false;
                    }
                    if (comparator.volume != temp.volume || comparator.mute != temp.mute) {
                        submitMetric(m_metricRecorder, "speakersInconsistent", 1);
                        ACSDK_ERROR(LX("validateSpeakerSettingsConsistencyFailed").d("reason", "inconsistentSpeakerSettings")
                            .d("comparatorVolume", static_cast<int>(comparator.volume)).d("comparatorMute", comparator.mute)
                            .d("volume", static_cast<int>(temp.volume)).d("mute", temp.mute));
                        consistent = false;
                        break;
                    }
                }
                ACSDK_DEBUG9(LX("validateSpeakerSettingsConsistencyResult").d("consistent", consistent));
                submitMetric(m_metricRecorder, "speakersCannotGetSetting", 0);
                if (consistent && settings) {
                    submitMetric(m_metricRecorder, "speakersInconsistent", 0);
                    settings->volume = comparator.volume;
                    settings->mute = comparator.mute;
                    ACSDK_DEBUG9(LX("validateSpeakerSettings").d("volume", static_cast<int>(settings->volume)).d("mute", settings->mute));
                }
                return consistent;
            }
            bool SpeakerManager::updateContextManager(const Type& type, const SpeakerSettings& settings) {
                ACSDK_DEBUG9(LX("updateContextManagerCalled").d("speakerType", type));
                if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME != type) {
                    ACSDK_DEBUG9(LX("updateContextManagerSkipped").d("reason", "typeMismatch").d("expected", Type::AVS_SPEAKER_VOLUME)
                        .d("actual", type));
                    return false;
                }
                Document state(kObjectType);
                Value _VOLUME_KEY{VOLUME_KEY, strlen(VOLUME_KEY)};
                Value _MUTED_KEY{MUTED_KEY, strlen(MUTED_KEY)};
                state.AddMember(_VOLUME_KEY, settings.volume, state.GetAllocator());
                state.AddMember(_MUTED_KEY, settings.mute, state.GetAllocator());
                StringBuffer buffer;
                rapidjson::Writer<StringBuffer> writer(buffer);
                if (!state.Accept(writer)) {
                    ACSDK_ERROR(LX("updateContextManagerFailed").d("reason", "writeToBufferFailed"));
                    return false;
                }
                if (SetStateResult::SUCCESS != m_contextManager->setState(VOLUME_STATE, buffer.GetString(), StateRefreshPolicy::NEVER)) {
                    ACSDK_ERROR(LX("updateContextManagerFailed").d("reason", "contextManagerSetStateFailed"));
                    return false;
                }
                return true;
            }
            future<bool> SpeakerManager::setVolume(Type type, int8_t volume, const SMI::NotificationProperties& properties) {
                ACSDK_DEBUG9(LX(__func__).d("volume", static_cast<int>(volume)));
                return m_executor.submit([this, type, volume, properties] {
                    return withinBounds(volume, AVS_SET_VOLUME_MIN, AVS_SET_VOLUME_MAX) && executeSetVolume(type, volume, properties);
                });
            }
            future<bool> SpeakerManager::setVolume(Type type, int8_t volume, bool forceNoNotifications, Source source) {
                ACSDK_DEBUG9(LX("setVolumeCalled").d("volume", static_cast<int>(volume)));
                SMI::NotificationProperties properties;
                properties.source = source;
                if (forceNoNotifications) {
                    properties.notifyObservers = false;
                    properties.notifyAVS = false;
                }
                return setVolume(type, volume, properties);
            }
            bool SpeakerManager::executeSetVolume(Type type, int8_t volume, const SMI::NotificationProperties& properties) {
                ACSDK_DEBUG9(LX("executeSetVolumeCalled").d("volume", static_cast<int>(volume)));
                if (m_speakerMap.count(type) == 0) {
                    ACSDK_ERROR(LX("executeSetVolumeFailed").d("reason", "noSpeakersWithType").d("type", type));
                    submitMetric(m_metricRecorder, "setVolumeFailedZeroSpeakers", 1);
                    return false;
                }
                submitMetric(m_metricRecorder, "setVolumeFailedZeroSpeakers", 0);
                submitMetric(m_metricRecorder, "setVolume", 1);
                if (volume == 0) submitMetric(m_metricRecorder, "setVolumeZero", 1);
                auto adjustedVolume = volume;
                auto maximumVolumeLimit = getMaximumVolumeLimit();
                if (volume > maximumVolumeLimit) {
                    ACSDK_DEBUG0(LX("adjustingSetVolumeValue").d("reason", "valueHigherThanLimit").d("value", (int)volume)
                        .d("maximumVolumeLimitSetting", (int)maximumVolumeLimit));
                    adjustedVolume = maximumVolumeLimit;
                }
                SpeakerSettings settings;
                if (!executeGetSpeakerSettings(type, &settings)) {
                    ACSDK_ERROR(LX("executeSetVolumeFailed").d("reason", "speakerSettingsInconsistent"));
                    return false;
                }
                const int8_t previousVolume = settings.volume;
                retryAndApplySettings([this, type, adjustedVolume]() -> bool {
                    auto beginIteratorAndEndIterator = m_speakerMap.equal_range(type);
                    auto begin = beginIteratorAndEndIterator.first;
                    auto end = beginIteratorAndEndIterator.second;
                    for (auto typeAndSpeakerIterator = begin; typeAndSpeakerIterator != end; typeAndSpeakerIterator++) {
                        auto speaker = typeAndSpeakerIterator->second;
                        if (!speaker->setUnduckedVolume(adjustedVolume)) {
                            submitMetric(m_metricRecorder, "setSpeakerVolumeFailed", 1);
                            return false;
                        }
                    }
                    submitMetric(m_metricRecorder, "setSpeakerVolumeFailed", 0);
                    return true;
                });
                if (!validateSpeakerSettingsConsistency(type, &settings)) {
                    ACSDK_ERROR(LX("executeSetVolumeFailed").d("reason", "speakerSettingsInconsistent"));
                    return false;
                }
                updateContextManager(type, settings);
                if (properties.notifyObservers) executeNotifyObserver(properties.source, type, settings);
                if (properties.notifyAVS && !(previousVolume == settings.volume && Source::LOCAL_API == properties.source)) {
                    executeNotifySettingsChanged(settings, VOLUME_CHANGED, properties.source, type);
                }
                return true;
            }
            bool SpeakerManager::executeRestoreVolume(Type type, Source source) {
                SpeakerSettings settings;
                if (!validateSpeakerSettingsConsistency(type, &settings)) {
                    ACSDK_ERROR(LX("executeAdjustVolumeFailed").d("reason", "initialSpeakerSettingsInconsistent"));
                    return false;
                }
                if (settings.volume > 0) return true;
                return executeSetVolume(type, m_minUnmuteVolume, SpeakerManagerInterface::NotificationProperties(source));
            }
            future<bool> SpeakerManager::adjustVolume(Type type, int8_t delta, const NotificationProperties& properties) {
                ACSDK_DEBUG9(LX(__func__).d("delta", static_cast<int>(delta)));
                return m_executor.submit([this, type, delta, properties] {
                    return withinBounds(delta, AVS_ADJUST_VOLUME_MIN, getMaximumVolumeLimit()) && executeAdjustVolume(type, delta, properties);
                });
            }
            future<bool> SpeakerManager::adjustVolume(Type type, int8_t delta, bool forceNoNotifications, Source source) {
                ACSDK_DEBUG9(LX("adjustVolumeCalled").d("delta", static_cast<int>(delta)));
                SMI::NotificationProperties properties;
                properties.source = source;
                if (forceNoNotifications) {
                    ACSDK_INFO(LX(__func__).d("reason", "forceNoNotifications").m("Skipping sending notifications"));
                    properties.notifyObservers = false;
                    properties.notifyAVS = false;
                }
                return adjustVolume(type, delta, properties);
            }
            bool SpeakerManager::executeAdjustVolume(Type type, int8_t delta, const SMI::NotificationProperties& properties) {
                ACSDK_DEBUG9(LX("executeAdjustVolumeCalled").d("delta", (int)delta));
                if (m_speakerMap.count(type) == 0) {
                    ACSDK_ERROR(LX("executeAdjustVolumeFailed").d("reason", "noSpeakersWithType").d("type", type));
                    return false;
                }
                submitMetric(m_metricRecorder, "adjustVolume", 1);
                SpeakerSettings settings;
                if (!executeGetSpeakerSettings(type, &settings)) {
                    ACSDK_ERROR(LX("executeAdjustVolumeFailed").d("reason", "speakerSettingsInconsistent"));
                    return false;
                }
                auto maxVolumeLimit = getMaximumVolumeLimit();
                if (settings.volume > maxVolumeLimit) {
                    ACSDK_DEBUG0(LX("adjustingSettingsVolumeValue").d("reason", "valueHigherThanLimit").d("value", (int)settings.volume)
                        .d("maximumVolumeLimitSetting", (int)maxVolumeLimit));
                    settings.volume = maxVolumeLimit;
                }
                const int8_t previousVolume = settings.volume;
                if (delta > 0) settings.volume = min((int)settings.volume + delta, (int)maxVolumeLimit);
                else settings.volume = max((int)settings.volume + delta, (int)AVS_SET_VOLUME_MIN);
                retryAndApplySettings([this, type, settings] {
                    auto beginIteratorAndEndIterator = m_speakerMap.equal_range(type);
                    auto begin = beginIteratorAndEndIterator.first;
                    auto end = beginIteratorAndEndIterator.second;
                    for (auto typeAndSpeakerIterator = begin; typeAndSpeakerIterator != end; typeAndSpeakerIterator++) {
                        auto speaker = typeAndSpeakerIterator->second;
                        if (!speaker->setUnduckedVolume(settings.volume)) return false;
                    }
                    return true;
                });
                if (!validateSpeakerSettingsConsistency(type, &settings)) {
                    ACSDK_ERROR(LX("executeAdjustVolumeFailed").d("reason", "speakerSettingsInconsistent"));
                    return false;
                }
                ACSDK_DEBUG(LX("executeAdjustVolumeSuccess").d("newVolume", (int)settings.volume));
                updateContextManager(type, settings);
                if (properties.notifyObservers) executeNotifyObserver(properties.source, type, settings);
                if (properties.notifyAVS && !(previousVolume == settings.volume && Source::LOCAL_API == properties.source)) {
                    executeNotifySettingsChanged(settings, VOLUME_CHANGED, properties.source, type);
                }
                return true;
            }
            future<bool> SpeakerManager::setMute(Type type, bool mute, const SMI::NotificationProperties& properties) {
                ACSDK_DEBUG9(LX(__func__).d("mute", mute));
                return m_executor.submit([this, type, mute, properties] { return executeSetMute(type, mute, properties); });
            }
            future<bool> SpeakerManager::setMute(Type type, bool mute, bool forceNoNotifications, Source source) {
                ACSDK_DEBUG9(LX("setMuteCalled").d("mute", mute));
                SMI::NotificationProperties properties;
                properties.source = source;
                if (forceNoNotifications) {
                    ACSDK_INFO(LX(__func__).d("reason", "forceNoNotifications").m("Skipping sending notifications"));
                    properties.notifyObservers = false;
                    properties.notifyAVS = false;
                }
                return setMute(type, mute, properties);
            }
            bool SpeakerManager::executeSetMute(Type type, bool mute, const SMI::NotificationProperties& properties) {
                ACSDK_DEBUG9(LX("executeSetMuteCalled").d("mute", mute));
                if (!mute) {
                    SpeakerInterface::SpeakerSettings settings;
                    if (!validateSpeakerSettingsConsistency(type, &settings)) {
                        ACSDK_WARN(LX("executeSetMuteWarn").m("cannot check if device is muted").d("reason", "speakerSettingsInconsistent"));
                    } else if (!settings.mute) return true;
                }
                if (m_speakerMap.count(type) == 0) {
                    ACSDK_ERROR(LX("executeSetMuteFailed").d("reason", "noSpeakersWithType").d("type", type));
                    return false;
                }
                retryAndApplySettings([this, type, mute] {
                    auto beginIteratorAndEndIterator = m_speakerMap.equal_range(type);
                    auto begin = beginIteratorAndEndIterator.first;
                    auto end = beginIteratorAndEndIterator.second;
                    for (auto typeAndSpeakerIterator = begin; typeAndSpeakerIterator != end; typeAndSpeakerIterator++) {
                        auto speaker = typeAndSpeakerIterator->second;
                        if (!speaker->setMute(mute)) return false;
                    }
                    return true;
                });
                submitMetric(m_metricRecorder, mute ? "setMute" : "setUnMute", 1);
                SpeakerInterface::SpeakerSettings settings;
                if (!validateSpeakerSettingsConsistency(type, &settings)) {
                    ACSDK_ERROR(LX("executeSetMute").d("reason", "speakerSettingsInconsistent"));
                    return false;
                }
                updateContextManager(type, settings);
                if (properties.notifyObservers) executeNotifyObserver(properties.source, type, settings);
                if (properties.notifyAVS) executeNotifySettingsChanged(settings, MUTE_CHANGED, properties.source, type);
                return true;
            }
        #ifndef ENABLE_MAXVOLUME_SETTING
            future<bool> SpeakerManager::setMaximumVolumeLimit(const int8_t maximumVolumeLimit) {
                return m_executor.submit([this, maximumVolumeLimit] {
                    return withinBounds(maximumVolumeLimit, AVS_ADJUST_VOLUME_MIN, AVS_ADJUST_VOLUME_MAX) &&
                                        executeSetMaximumVolumeLimit(maximumVolumeLimit);
                });
            }
            bool SpeakerManager::executeSetMaximumVolumeLimit(const int8_t maximumVolumeLimit) {
                ACSDK_DEBUG3(LX(__func__).d("maximumVolumeLimit", static_cast<int>(maximumVolumeLimit)));
                for (auto it = m_speakerMap.begin(); it != m_speakerMap.end(); it = m_speakerMap.upper_bound(it->first)) {
                    SpeakerSettings speakerSettings;
                    auto speakerType = it->first;
                    ACSDK_DEBUG3(LX(__func__).d("type", speakerType));
                    if (!executeGetSpeakerSettings(speakerType, &speakerSettings)) {
                        ACSDK_ERROR(LX("executeSetMaximumVolumeLimitFailed").d("reason", "getSettingsFailed"));
                        return false;
                    }
                    if (speakerSettings.volume > maximumVolumeLimit) {
                        ACSDK_DEBUG1(LX("reducingVolume").d("reason", "volumeIsHigherThanNewLimit").d("type", it->first)
                            .d("volume", (int)speakerSettings.volume).d("limit", (int)maximumVolumeLimit));
                        if (!executeSetVolume(speakerType, maximumVolumeLimit, SMI::NotificationProperties(Source::DIRECTIVE))) {
                            ACSDK_ERROR(LX("executeSetMaximumVolumeLimitFailed").d("reason", "setVolumeFailed"));
                            return false;
                        }
                    }
                }
                m_maximumVolumeLimit = maximumVolumeLimit;
                return true;
            }
        #endif
            void SpeakerManager::executeNotifySettingsChanged(const SpeakerSettings& settings, const string& eventName, const Source& source,
                                                              const Type& type) {
                if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == type) executeSendSpeakerSettingsChangedEvent(eventName, settings);
                else { ACSDK_INFO(LX("eventNotSent").d("reason", "typeMismatch").d("speakerType", type)); }
            }
            void SpeakerManager::executeNotifyObserver(const Source& source, const Type& type, const SpeakerSettings& settings) {
                ACSDK_DEBUG9(LX("executeNotifyObserverCalled"));
                for (auto observer : m_observers) {
                    observer->onSpeakerSettingsChanged(source, type, settings);
                }
            }
            future<bool> SpeakerManager::getSpeakerSettings(Type type, SpeakerSettings* settings) {
                ACSDK_DEBUG9(LX("getSpeakerSettingsCalled"));
                return m_executor.submit([this, type, settings] { return executeGetSpeakerSettings(type, settings); });
            }
            bool SpeakerManager::executeGetSpeakerSettings(Type type, SpeakerSettings* settings) {
                ACSDK_DEBUG9(LX("executeGetSpeakerSettingsCalled"));
                if (m_speakerMap.count(type) == 0) {
                    ACSDK_ERROR(LX("executeGetSpeakerSettingsFailed").d("reason", "noSpeakersWithType").d("type", type));
                    return false;
                }
                if (!validateSpeakerSettingsConsistency(type, settings)) {
                    ACSDK_ERROR(LX("executeGetSpeakerSettingsCalled").d("reason", "speakerSettingsInconsistent"));
                    return false;
                }
                return true;
            }
            void SpeakerManager::addChannelVolumeInterface(shared_ptr<ChannelVolumeInterface> channelVolumeInterface) {
                if (!channelVolumeInterface) {
                    ACSDK_ERROR(LX("addChannelVolumeInterfaceFailed").d("reason", "channelVolumeInterface cannot be nullptr"));
                    return;
                }
                m_executor.submit([this, channelVolumeInterface] {
                    m_speakerMap.insert(pair<Type, shared_ptr<ChannelVolumeInterface>>(
                        channelVolumeInterface->getSpeakerType(), channelVolumeInterface));
                });
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> SpeakerManager::getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            int8_t SpeakerManager::getMaximumVolumeLimit() {
                return m_maximumVolumeLimit;
            }
            template <typename Task, typename... Args> void SpeakerManager::retryAndApplySettings(Task task, Args&&... args) {
                auto boundTask = bind(std::forward<Task>(task), forward<Args>(args)...);
                size_t attempt = 0;
                m_waitCancelEvent.reset();
                while(attempt < m_maxRetries) {
                    if (boundTask()) break;
                    if (m_waitCancelEvent.wait(m_retryTimer.calculateTimeToRetry(static_cast<int>(attempt)))) break;
                    attempt++;
                }
            }
        }
    }
}