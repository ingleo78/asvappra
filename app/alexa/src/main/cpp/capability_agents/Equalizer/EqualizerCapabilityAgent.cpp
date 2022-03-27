#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/CapabilityConfiguration.h>
#include <avs/MessageRequest.h>
#include <json/JSONUtils.h>
#include "EqualizerCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace equalizer {
            using namespace configuration;
            using namespace error;
            using namespace json::jsonUtils;
            static const string TAG{"EqualizerController"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE = "EqualizerController";
            static const NamespaceAndName EQUALIZER_STATE{NAMESPACE, "EqualizerState"};
            static const NamespaceAndName DIRECTIVE_SETBANDS{NAMESPACE, "SetBands"};
            static const NamespaceAndName DIRECTIVE_ADJUSTBANDS{NAMESPACE, "AdjustBands"};
            static const NamespaceAndName DIRECTIVE_RESETBANDS{NAMESPACE, "ResetBands"};
            static const NamespaceAndName DIRECTIVE_SETMODE{NAMESPACE, "SetMode"};
            static const NamespaceAndName EVENT_EQUALIZERCHANGED{NAMESPACE, "EqualizerChanged"};
            static const string EQUALIZER_JSON_INTERFACE_TYPE = "AlexaInterface";
            static const string EQUALIZER_JSON_INTERFACE_NAME = "EqualizerController";
            static const string EQUALIZER_JSON_INTERFACE_VERSION = "1.0";
            static constexpr char JSON_KEY_BANDS[] = "bands";
            static constexpr char JSON_KEY_SUPPORTED[] = "supported";
            static constexpr char JSON_KEY_NAME[] = "name";
            static constexpr char JSON_KEY_LEVEL[] = "level";
            static constexpr char JSON_KEY_RANGE[] = "range";
            static constexpr char JSON_KEY_MINIMUM[] = "minimum";
            static constexpr char JSON_KEY_MAXIMUM[] = "maximum";
            static constexpr char JSON_KEY_MODES[] = "modes";
            static constexpr char JSON_KEY_MODE[] = "mode";
            static constexpr char JSON_KEY_LEVELDELTA[] = "levelDelta";
            static constexpr char JSON_KEY_LEVELDIRECTION[] = "levelDirection";
            static constexpr char LEVEL_DIRECTION_UP[] = "UP";
            static constexpr char LEVEL_DIRECTION_DOWN[] = "DOWN";
            shared_ptr<EqualizerCapabilityAgent> EqualizerCapabilityAgent::create(shared_ptr<EqualizerController> equalizerController,
                                                                                  shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                                                                  shared_ptr<EqualizerStorageInterface> equalizerStorage,
                                                                                  shared_ptr<CustomerDataManager> customerDataManager,
                                                                                  shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                                  shared_ptr<ContextManagerInterface> contextManager,
                                                                                  shared_ptr<MessageSenderInterface> messageSender) {
                if (nullptr == equalizerController) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "equalizerControllerNull"));
                    return nullptr;
                }
                if (nullptr == capabilitiesDelegate) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "capabilitiesDelegateNull"));
                    return nullptr;
                }
                if (nullptr == equalizerStorage) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "equalizerStoragelerNull"));
                    return nullptr;
                }
                if (nullptr == contextManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "contextManagerNull"));
                    return nullptr;
                }
                if (nullptr == messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "messageSenderNull"));
                    return nullptr;
                }
                if (nullptr == customerDataManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "customerDataManagerNull"));
                    return nullptr;
                }
                if (nullptr == exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "exceptionEncounteredSenderNull"));
                    return nullptr;
                }
                auto equalizerCA = shared_ptr<EqualizerCapabilityAgent>(new EqualizerCapabilityAgent(equalizerController, capabilitiesDelegate,
                                                                        equalizerStorage, customerDataManager, exceptionEncounteredSender,
                                                                        contextManager, messageSender));
                equalizerController->addListener(equalizerCA);
                return equalizerCA;
            }
            static string buildEQStateJson(const EqualizerState& state) {
                Document payload(kObjectType);
                auto& allocator = payload.GetAllocator();
                Value bandsBranch(rapidjson::kArrayType);
                for (const auto& bandIter : state.bandLevels) {
                    EqualizerBand band = bandIter.first;
                    int bandLevel = bandIter.second;
                    Value bandDesc(kObjectType);
                    Value json_key_name{JSON_KEY_NAME, strlen(JSON_KEY_NAME)};
                    string _band = equalizerBandToString(band).data();
                    Value __band{_band.data(), _band.length()};
                    Value json_key_level{JSON_KEY_LEVEL, strlen(JSON_KEY_LEVEL)};
                    Value _bandLevel{bandLevel};
                    bandDesc.AddMember(json_key_name, __band);
                    bandDesc.AddMember(json_key_level, _bandLevel);
                    bandsBranch.PushBack(bandDesc);
                }
                payload.AddMember(JSON_KEY_BANDS, bandsBranch, allocator);
                if (EqualizerMode::NONE != state.mode) {
                    Value json_key_mode{JSON_KEY_MODE, strlen(JSON_KEY_MODE)};
                    string mode = equalizerModeToString(state.mode).data();
                    Value _mode{mode.data(), mode.length()};
                    payload.AddMember(json_key_mode, _mode);
                }
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                if (!payload.Accept(writer)) {
                    ACSDK_ERROR(LX("buildEQStateJsonFailed").d("reason", "writerRefusedJsonObject"));
                    return "";
                }
                return buffer.GetString();
            }
            EqualizerCapabilityAgent::EqualizerCapabilityAgent(shared_ptr<alexaClientSDK::equalizer::EqualizerController> equalizerController,
                                                               shared_ptr<CapabilitiesDelegateInterface> capabilitiesDelegate,
                                                               shared_ptr<EqualizerStorageInterface> equalizerStorage,
                                                               shared_ptr<registrationManager::CustomerDataManager> customerDataManager,
                                                               shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                               shared_ptr<ContextManagerInterface> contextManager,
                                                               shared_ptr<MessageSenderInterface> messageSender) :
                                                               CapabilityAgent{NAMESPACE, exceptionEncounteredSender},
                                                               RequiresShutdown{"Equalizer"}, CustomerDataHandler{customerDataManager},
                                                               m_equalizerController{equalizerController}, m_capabilitiesDelegate{capabilitiesDelegate},
                                                               m_equalizerStorage{equalizerStorage}, m_messageSender{messageSender},
                                                               m_contextManager{contextManager} {
                generateCapabilityConfiguration();
                auto payload = buildEQStateJson(equalizerController->getCurrentState());
                if (0 == payload.length()) {
                    ACSDK_ERROR(LX("EqualizerCapabilityAgentFailed").d("reason", "Failed to serialize equalizer state."));
                    return;
                }
                m_contextManager->setState(EQUALIZER_STATE, payload, avsCommon::avs::StateRefreshPolicy::NEVER);
            }
            void EqualizerCapabilityAgent::generateCapabilityConfiguration() {
                unordered_map<string, string> configMap;
                configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, EQUALIZER_JSON_INTERFACE_TYPE});
                configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, EQUALIZER_JSON_INTERFACE_NAME});
                configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, EQUALIZER_JSON_INTERFACE_VERSION});
                auto equalizerConfiguration = m_equalizerController->getConfiguration();
                Document payload(kObjectType);
                auto& allocator = payload.GetAllocator();
                Value bandsBranch(rapidjson::kObjectType);
                Value modesBranch(rapidjson::kObjectType);
                Value bandsSupportedBranch(rapidjson::kArrayType);
                Value bandsRangeBranch(rapidjson::kObjectType);
                Value modesSupportedBranch(rapidjson::kArrayType);
                for (auto band : equalizerConfiguration->getSupportedBands()) {
                    Value bandDesc(kObjectType);
                    Value json_key_name{JSON_KEY_NAME, strlen(JSON_KEY_NAME)};
                    string _band = equalizerBandToString(band).data();
                    Value __band{_band.data(), _band.length()};
                    bandDesc.AddMember(json_key_name, __band);
                    bandsSupportedBranch.PushBack(bandDesc);
                }
                Value json_key_minimum{JSON_KEY_MINIMUM, strlen(JSON_KEY_MINIMUM)};
                Value json_key_maximum{JSON_KEY_MAXIMUM, strlen(JSON_KEY_MAXIMUM)};
                Value json_key_supported{JSON_KEY_SUPPORTED, strlen(JSON_KEY_SUPPORTED)};
                Value json_key_range{JSON_KEY_RANGE, strlen(JSON_KEY_RANGE)};
                Value minBandLevel{equalizerConfiguration->getMinBandLevel()};
                Value maxBandLevel{equalizerConfiguration->getMaxBandLevel()};
                bandsRangeBranch.AddMember(json_key_minimum, minBandLevel);
                bandsRangeBranch.AddMember(json_key_maximum, maxBandLevel);
                bandsBranch.AddMember(json_key_supported, bandsSupportedBranch);
                bandsBranch.AddMember(json_key_range, bandsRangeBranch);
                if (!equalizerConfiguration->getSupportedModes().empty()) {
                    for (auto mode : equalizerConfiguration->getSupportedModes()) {
                        Value modeDesc(rapidjson::kObjectType);
                        Value json_key_name{JSON_KEY_NAME, strlen(JSON_KEY_NAME)};
                        string _mode = equalizerModeToString(mode).data();
                        Value __mode{_mode.data(), _mode.length()};
                        modeDesc.AddMember(json_key_name, __mode);
                        modesSupportedBranch.PushBack(modeDesc);
                    }
                    Value json_key_supported{JSON_KEY_SUPPORTED, strlen(JSON_KEY_SUPPORTED)};
                    modesBranch.AddMember(json_key_supported, modesSupportedBranch);
                    payload.AddMember(JSON_KEY_MODES, modesBranch, allocator);
                }
                payload.AddMember(JSON_KEY_BANDS, bandsBranch, allocator);
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                if (!payload.Accept(writer)) {
                    ACSDK_ERROR(LX("generateCapabilityConfigurationFailed").d("reason", "writerRefusedJsonObject"));
                    return;
                }
                configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, buffer.GetString()});
                m_capabilityConfigurations.insert(make_shared<CapabilityConfiguration>(configMap));
            }
            DirectiveHandlerConfiguration EqualizerCapabilityAgent::getConfiguration() const {
                ACSDK_DEBUG5(LX(__func__));
                DirectiveHandlerConfiguration configuration;
                auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                configuration[DIRECTIVE_SETBANDS] = neitherNonBlockingPolicy;
                configuration[DIRECTIVE_ADJUSTBANDS] = neitherNonBlockingPolicy;
                configuration[DIRECTIVE_RESETBANDS] = neitherNonBlockingPolicy;
                configuration[DIRECTIVE_SETMODE] = neitherNonBlockingPolicy;
                return configuration;
            }
            void EqualizerCapabilityAgent::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                ACSDK_DEBUG5(LX(__func__));
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void EqualizerCapabilityAgent::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            bool parseDirectivePayload(const string& payload, Document* document) {
                ACSDK_DEBUG5(LX(__func__));
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
            void EqualizerCapabilityAgent::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                ACSDK_DEBUG5(LX(__func__));
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                m_executor.submit([this, info] {
                    const string directiveName = info->directive->getName();
                    Document payload(kObjectType);
                    if (!parseDirectivePayload(info->directive->getPayload(), &payload)) {
                        sendExceptionEncounteredAndReportFailed(info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    if (directiveName == DIRECTIVE_SETBANDS.name) {
                        if (!handleSetBandsDirective(info, payload)) return;
                    } else if (directiveName == DIRECTIVE_ADJUSTBANDS.name) {
                        if (!handleAdjustBandsDirective(info, payload)) return;
                    } else if (directiveName == DIRECTIVE_RESETBANDS.name) {
                        if (!handleResetBandsDirective(info, payload)) return;
                    } else if (directiveName == DIRECTIVE_SETMODE.name) {
                        if (!handleSetModeDirective(info, payload)) return;
                    } else {
                        sendExceptionEncounteredAndReportFailed(info, "Unexpected Directive", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return;
                    }
                    if (info->result) info->result->setCompleted();
                    removeDirective(info->directive->getMessageId());
                });
            }
            void EqualizerCapabilityAgent::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                removeDirective(info->directive->getMessageId());
            }
            unordered_set<shared_ptr<CapabilityConfiguration>> EqualizerCapabilityAgent::
                getCapabilityConfigurations() {
                return m_capabilityConfigurations;
            }
            void EqualizerCapabilityAgent::doShutdown() {
                m_executor.shutdown();
                m_equalizerController->removeListener(shared_from_this());
                m_equalizerController.reset();
                lock_guard<mutex> lock(m_storageMutex);
                m_equalizerStorage.reset();
            }
            void EqualizerCapabilityAgent::clearData() {
                ACSDK_DEBUG5(LX(__func__));
                unique_lock<mutex> lock(m_storageMutex);
                auto equalizerStorage = m_equalizerStorage;
                if (equalizerStorage) {
                    lock.unlock();
                    equalizerStorage->clear();
                }
            }
            void EqualizerCapabilityAgent::onEqualizerStateChanged(const EqualizerState& state) {
                auto payload = buildEQStateJson(state);
                if (0 == payload.length()) {
                    ACSDK_ERROR(LX("onEqualizerStateChangedFailed").d("reason", "Failed to serialize equalizer state."));
                    return;
                }
                m_contextManager->setState(EQUALIZER_STATE, payload,StateRefreshPolicy::NEVER);
                auto eventJson = buildJsonEventString(EVENT_EQUALIZERCHANGED.name, "", payload);
                auto request = make_shared<MessageRequest>(eventJson.second);
                m_messageSender->sendMessage(request);
            }
            void EqualizerCapabilityAgent::onEqualizerSameStateChanged(const EqualizerState& state) {
                onEqualizerStateChanged(state);
            }
            bool EqualizerCapabilityAgent::handleSetBandsDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& payload) {
                Value::ConstMemberIterator it;
                Value _payload{payload.GetString(), strlen(payload.GetString())};
                if (!findNode(_payload, JSON_KEY_BANDS, &it) || !it->value.IsArray()) {
                    sendExceptionEncounteredAndReportFailed(info, "'bands' node not found or is not an array.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                bool shouldFixConfiguration = false;
                auto eqConfig = m_equalizerController->getConfiguration();
                EqualizerBandLevelMap bandLevelMap;
                for (const auto& bandDesc : it->value.GetArray()) {
                    string bandName;
                    if (!retrieveValue(bandDesc, JSON_KEY_NAME, &bandName)) {
                        sendExceptionEncounteredAndReportFailed(info, "Invalid 'bands[].name' value.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return false;
                    }
                    SuccessResult<EqualizerBand> bandResult = stringToEqualizerBand(bandName);
                    if (!bandResult.isSucceeded() || !eqConfig->isBandSupported(bandResult.value())) {
                        ACSDK_WARN(LX(__func__).d("band", bandName).m("Unsupported band"));
                        shouldFixConfiguration = true;
                        continue;
                    }
                    EqualizerBand band = bandResult.value();
                    int64_t bandLevel = 0;
                    if (!retrieveValue(bandDesc, JSON_KEY_LEVEL, &bandLevel)) {
                        sendExceptionEncounteredAndReportFailed(info, "Invalid 'bands[].level' value.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return false;
                    }
                    int minValue = eqConfig->getMinBandLevel();
                    int maxValue = eqConfig->getMaxBandLevel();
                    if (bandLevel < minValue) {
                        ACSDK_WARN(LX(__func__).d("level", bandLevel).d("minimum", minValue).m("Band level below minimum"));
                        shouldFixConfiguration = true;
                        bandLevel = minValue;
                    } else if (bandLevel > maxValue) {
                        ACSDK_WARN(LX(__func__).d("level", bandLevel).d("maximum", maxValue).m("Band level above maximum"));
                        shouldFixConfiguration = true;
                        bandLevel = maxValue;
                    }
                    bandLevelMap[band] = bandLevel;
                }
                m_equalizerController->setBandLevels(bandLevelMap);
                if (shouldFixConfiguration) {
                    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(),ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                             "Unsupported EQ band or level values recieved.");
                    fixConfigurationDesynchronization();
                }
                return true;
            }
            bool EqualizerCapabilityAgent::handleAdjustBandsDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& payload) {
                Value::ConstMemberIterator it;
                Value _payload{payload.GetString(), strlen(payload.GetString())};
                if (!findNode(_payload, JSON_KEY_BANDS, &it) || !it->value.IsArray()) {
                    sendExceptionEncounteredAndReportFailed(info, "'bands' node not found or is not an array.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                bool shouldFixConfiguration = false;
                auto eqConfig = m_equalizerController->getConfiguration();
                EqualizerBandLevelMap bandLevelMap;
                for (const auto& bandDesc : it->value.GetArray()) {
                    string bandName;
                    if (!retrieveValue(bandDesc, JSON_KEY_NAME, &bandName)) {
                        sendExceptionEncounteredAndReportFailed(info, "Invalid 'bands[].name' value.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return false;
                    }
                    SuccessResult<EqualizerBand> bandResult = stringToEqualizerBand(bandName);
                    if (!bandResult.isSucceeded() || !eqConfig->isBandSupported(bandResult.value())) {
                        ACSDK_WARN(LX(__func__).d("band", bandName).m("Unsupported band"));
                        shouldFixConfiguration = true;
                        continue;
                    }
                    EqualizerBand band = bandResult.value();
                    int64_t bandLevelDelta = eqConfig->getDefaultBandDelta();
                    retrieveValue(bandDesc, JSON_KEY_LEVELDELTA, &bandLevelDelta);
                    ACSDK_DEBUG5(LX(__func__).d("modifying band with delta", bandLevelDelta));
                    string direction;
                    if (!retrieveValue(bandDesc, JSON_KEY_LEVELDIRECTION, &direction)) {
                        sendExceptionEncounteredAndReportFailed(info, "Invalid 'bands[].levelDelta' value.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return false;
                    }
                    bool isDirectionUp;
                    if (direction == LEVEL_DIRECTION_UP) isDirectionUp = true;
                    else if (direction == LEVEL_DIRECTION_DOWN) isDirectionUp = false;
                    else {
                        sendExceptionEncounteredAndReportFailed(info,"Invalid 'bands[].levelDirection', expected 'UP' or 'DOWN'.",
                                                           ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return false;
                    }
                    int minValue = eqConfig->getMinBandLevel();
                    int maxValue = eqConfig->getMaxBandLevel();
                    auto bandLevelResult = m_equalizerController->getBandLevel(band);
                    int bandLevel = bandLevelResult.value();
                    bandLevel += isDirectionUp ? bandLevelDelta : -bandLevelDelta;
                    if (bandLevel < minValue) {
                        ACSDK_WARN(LX(__func__).d("level", bandLevel).d("minimum", minValue).m("Adjusted band level below minimum"));
                        shouldFixConfiguration = true;
                        bandLevel = minValue;
                    } else if (bandLevel > maxValue) {
                        ACSDK_WARN(LX(__func__).d("level", bandLevel).d("maximum", maxValue).m("Adjusted band level above maximum"));
                        shouldFixConfiguration = true;
                        bandLevel = maxValue;
                    }
                    bandLevelMap[band] = bandLevel;
                }
                m_equalizerController->setBandLevels(bandLevelMap);
                if (shouldFixConfiguration) {
                    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(),ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                             "Unsupported EQ band or level values recieved.");
                    fixConfigurationDesynchronization();
                }
                return true;
            }
            bool EqualizerCapabilityAgent::handleResetBandsDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& payload) {
                Value::ConstMemberIterator it;
                Value _payload{payload.GetString(), strlen(payload.GetString())};
                if (!findNode(_payload, JSON_KEY_BANDS, &it) || !it->value.IsArray()) {
                    sendExceptionEncounteredAndReportFailed(info, "'bands' node not found or is not an array.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                bool shouldFixConfiguration = false;
                auto eqConfig = m_equalizerController->getConfiguration();
                set<EqualizerBand> bandsToReset;
                for (const auto& bandDesc : it->value.GetArray()) {
                    string bandName;
                    if (!retrieveValue(bandDesc, JSON_KEY_NAME, &bandName)) {
                        sendExceptionEncounteredAndReportFailed(info, "Invalid 'bands[].name' value.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                        return false;
                    }
                    SuccessResult<EqualizerBand> bandResult = stringToEqualizerBand(bandName);
                    if (!bandResult.isSucceeded() || !eqConfig->isBandSupported(bandResult.value())) {
                        ACSDK_WARN(LX(__func__).d("band", bandName).m("Unsupported band"));
                        shouldFixConfiguration = true;
                        continue;
                    }
                    bandsToReset.insert(bandResult.value());
                }
                if (!bandsToReset.empty()) m_equalizerController->resetBands(bandsToReset);
                if (shouldFixConfiguration) {
                    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(),ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                             "Unsupported EQ band recieved.");
                    fixConfigurationDesynchronization();
                }
                return true;
            }
            bool EqualizerCapabilityAgent::handleSetModeDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info, Document& payload) {
                auto eqConfig = m_equalizerController->getConfiguration();
                string modeName;
                if (!retrieveValue(payload, JSON_KEY_MODE, &modeName)) {
                    sendExceptionEncounteredAndReportFailed(info, "Invalid or missing 'mode' value.", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
                    return false;
                }
                SuccessResult<EqualizerMode> modeResult = stringToEqualizerMode(modeName);
                if (!modeResult.isSucceeded() || !eqConfig->isModeSupported(modeResult.value())) {
                    ACSDK_WARN(LX(__func__).d("mode", modeName).m("Unsupported mode"));
                    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(),ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED,
                                                             "Unsupported EQ band recieved.");
                    fixConfigurationDesynchronization();
                } else m_equalizerController->setCurrentMode(modeResult.value());
                return true;
            }
            void EqualizerCapabilityAgent::fixConfigurationDesynchronization() {
                m_capabilitiesDelegate->invalidateCapabilities();
            }
        }
    }
}