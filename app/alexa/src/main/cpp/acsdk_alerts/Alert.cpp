#include <limits>
#include <string>
#include <time.h>
#include <avs/FocusState.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <timing/TimeUtils.h>
#include <util/string/StringUtils.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "Alarm.h"
#include "Alert.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace chrono;
        using namespace acsdkAlertsInterfaces;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace json;
        using namespace jsonUtils;
        using namespace logger;
        using namespace timing;
        using namespace renderer;
        using namespace rapidjson;
        using namespace settings;
        using namespace types;
        using namespace avsCommon::utils::string;
        static const std::string KEY_TOKEN = "token";
        static const std::string KEY_SCHEDULED_TIME = "scheduledTime";
        static const std::string KEY_ASSETS = "assets";
        static const std::string KEY_ASSET_ID = "assetId";
        static const std::string KEY_ASSET_URL = "url";
        static const std::string KEY_ASSET_PLAY_ORDER = "assetPlayOrder";
        static const std::string KEY_LOOP_COUNT = "loopCount";
        static const std::string KEY_LOOP_PAUSE_IN_MILLISECONDS = "loopPauseInMilliSeconds";
        static const std::string KEY_BACKGROUND_ASSET_ID = "backgroundAlertAsset";
        const std::chrono::seconds MAXIMUM_ALERT_RENDERING_TIME = std::chrono::hours(1);
        const auto BACKGROUND_ALERT_SOUND_PAUSE_TIME = std::chrono::seconds(10);
        static const std::string TAG("Alert");
        #define LX(event) LogEntry(TAG, event)
        Alert::Alert(function<pair<unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                     shared_ptr<DeviceSettingsManager> settingsManager) : m_stopReason{StopReason::UNSET}, m_focusState{FocusState::NONE}, m_hasTimerExpired{false},
                     m_observer{nullptr}, m_defaultAudioFactory{defaultAudioFactory}, m_shortAudioFactory{shortAudioFactory}, m_settingsManager{settingsManager},
                     m_focusChangedDuringAlertActivation{false}, m_startRendererAgainAfterFullStop{false} {}
        static Alert::ParseFromJsonStatus parseAlertAssetConfigurationFromJson(const Value& payload, Alert::AssetConfiguration* assetConfiguration,
                                                                               Alert::DynamicData* dynamicData) {
            if (!assetConfiguration) {
                ACSDK_ERROR(LX("parseAlertAssetConfigurationFromJson : assetConfiguration is nullptr."));
                return Alert::ParseFromJsonStatus::OK;
            }
            if (!dynamicData) {
                ACSDK_ERROR(LX("parseAlertAssetConfigurationFromJson : dynamicData is nullptr."));
                return Alert::ParseFromJsonStatus::OK;
            }
            Alert::AssetConfiguration localAssetConfig;
            bool assetInfoFoundOk = true;
            int64_t loopCount = std::numeric_limits<int>::max();
            int64_t loopPauseInMilliseconds = 0;
            std::string backgroundAssetId;
            if (!jsonArrayExists(payload, KEY_ASSETS)) {
                ACSDK_DEBUG0(LX("parseAlertAssetConfigurationFromJson : assets are not present."));
                assetInfoFoundOk = false;
            }
            if (!jsonArrayExists(payload, KEY_ASSET_PLAY_ORDER)) {
                ACSDK_DEBUG0(LX("parseAlertAssetConfigurationFromJson : asset play order is not present."));
                assetInfoFoundOk = false;
            }
            if (!retrieveValue(payload, KEY_LOOP_COUNT, &loopCount)) {
                ACSDK_DEBUG0(LX("parseAlertAssetConfigurationFromJson : loop count is not present."));
            }
            if (!retrieveValue(payload, KEY_LOOP_PAUSE_IN_MILLISECONDS, &loopPauseInMilliseconds)) {
                ACSDK_DEBUG0(LX("parseAlertAssetConfigurationFromJson : loop pause in milliseconds is not present."));
            }
            if (!retrieveValue(payload, KEY_BACKGROUND_ASSET_ID, &backgroundAssetId)) {
                ACSDK_DEBUG0(LX("parseAlertAssetConfigurationFromJson : backgroundAssetId is not present."));
            }
            if (!assetInfoFoundOk) return Alert::ParseFromJsonStatus::OK;
            auto assetJsonArray = payload.FindMember(KEY_ASSETS.c_str())->value.GetArray();
            for (SizeType i = 0; i < assetJsonArray.Size(); i++) {
                Alert::Asset asset;
                if (!retrieveValue(assetJsonArray[i], KEY_ASSET_ID, &asset.id)) {
                    ACSDK_WARN(LX("parseAlertAssetConfigurationFromJson : assetId is not present."));
                    return Alert::ParseFromJsonStatus::OK;
                }
                if (!retrieveValue(assetJsonArray[i], KEY_ASSET_URL, &asset.url)) {
                    ACSDK_WARN(LX("parseAlertAssetConfigurationFromJson : assetUrl is not present."));
                    return Alert::ParseFromJsonStatus::OK;
                }
                if (asset.id.empty() || asset.url.empty()) {
                    ACSDK_WARN(LX("parseAlertAssetConfigurationFromJson : invalid asset data."));
                    return Alert::ParseFromJsonStatus::OK;
                }
                if (assetConfiguration->assets.find(asset.id) != assetConfiguration->assets.end()) {
                    ACSDK_WARN(LX("parseAlertAssetConfigurationFromJson : duplicate assetId detected."));
                    return Alert::ParseFromJsonStatus::OK;
                }
                localAssetConfig.assets[asset.id] = asset;
            }
            auto playOrderItemsJsonArray = payload.FindMember(KEY_ASSET_PLAY_ORDER.c_str())->value.GetArray();
            for (SizeType i = 0; i < playOrderItemsJsonArray.Size(); i++) {
                if (!playOrderItemsJsonArray[i].IsString()) {
                    ACSDK_WARN(LX("parseAlertAssetConfigurationFromJson : invalid play order item type detected."));
                    return Alert::ParseFromJsonStatus::OK;
                }
                auto assetId = playOrderItemsJsonArray[i].GetString();
                if (localAssetConfig.assets.find(assetId) == localAssetConfig.assets.end()) {
                    ACSDK_WARN(LX("parseAlertAssetConfigurationFromJson : invalid play order item - asset does not exist."));
                    return Alert::ParseFromJsonStatus::OK;
                }
                localAssetConfig.assetPlayOrderItems.push_back(assetId);
            }
            if (loopCount > numeric_limits<int>::max()) {
                ACSDK_WARN(LX("parseAlertAssetConfigurationFromJson").d("loopCountValue", loopCount).m("loopCount cannot be converted to integer."));
                return Alert::ParseFromJsonStatus::OK;
            }
            localAssetConfig.loopPause = std::chrono::milliseconds{loopPauseInMilliseconds};
            localAssetConfig.backgroundAssetId = backgroundAssetId;
            *assetConfiguration = localAssetConfig;
            dynamicData->loopCount = static_cast<int>(loopCount);
            return Alert::ParseFromJsonStatus::OK;
        }
        Alert::ParseFromJsonStatus Alert::parseFromJson(const rapidjson::Value& payload, std::string* errorMessage) {
            lock_guard<mutex> lock(m_mutex);
            if (!retrieveValue(payload, KEY_TOKEN, &m_staticData.token)) {
                ACSDK_ERROR(LX("parseFromJsonFailed").m("could not parse token."));
                *errorMessage = "missing property: " + KEY_TOKEN;
                return ParseFromJsonStatus::MISSING_REQUIRED_PROPERTY;
            }
            std::string parsedScheduledTime_ISO_8601;
            if (!retrieveValue(payload, KEY_SCHEDULED_TIME, &parsedScheduledTime_ISO_8601)) {
                ACSDK_ERROR(LX("parseFromJsonFailed").m("could not parse scheduled time."));
                *errorMessage = "missing property: " + KEY_SCHEDULED_TIME;
                return ParseFromJsonStatus::MISSING_REQUIRED_PROPERTY;
            }
            if (!m_dynamicData.timePoint.setTime_ISO_8601(parsedScheduledTime_ISO_8601)) {
                ACSDK_ERROR(LX("parseFromJsonFailed").m("could not convert time to unix.").d("parsed time string", parsedScheduledTime_ISO_8601));
                return ParseFromJsonStatus::INVALID_VALUE;
            }
            return parseAlertAssetConfigurationFromJson(payload, &m_dynamicData.assetConfiguration, &m_dynamicData);
        }
        Alert::ParseFromJsonStatus parseFromJson(const Document& payload, std::string* errorMessage) {
            parseFromJson(payload, errorMessage);
        }
        void Alert::setRenderer(shared_ptr<RendererInterface> renderer) {
            lock_guard<mutex> lock(m_mutex);
            if (m_renderer) {
                ACSDK_ERROR(LX("setRendererFailed").m("Renderer is already set."));
                return;
            }
            m_renderer = renderer;
        }
        void Alert::setObserver(AlertObserverInterface* observer) {
            lock_guard<mutex> lock(m_mutex);
            m_observer = observer;
        }
        void Alert::setFocusState(FocusState focusState) {
            lock_guard<mutex> lock(m_mutex);
            auto alertState = m_dynamicData.state;
            if (focusState == m_focusState) return;
            m_focusState = focusState;
            if (State::ACTIVATING == alertState) {
                m_focusChangedDuringAlertActivation = true;
                return;
            }
            if (State::ACTIVE == alertState) {
                m_startRendererAgainAfterFullStop = true;
                m_renderer->stop();
            }
        }
        bool Alert::setStateActive() {
            lock_guard<mutex> lock(m_mutex);
            if (State::ACTIVATING != m_dynamicData.state) {
                ACSDK_ERROR(LX("setStateActiveFailed").d("current state", stateToString(m_dynamicData.state)));
                return false;
            }
            m_dynamicData.state = State::ACTIVE;
            return true;
        }
        void Alert::reset() {
            lock_guard<mutex> lock(m_mutex);
            m_dynamicData.state = Alert::State::SET;
        }
        void Alert::activate() {
            ACSDK_DEBUG9(LX("activate"));
            unique_lock<mutex> lock(m_mutex);
            if (Alert::State::ACTIVATING == m_dynamicData.state || Alert::State::ACTIVE == m_dynamicData.state) {
                ACSDK_ERROR(LX("activateFailed").m("Alert is already active."));
                return;
            }
            m_dynamicData.state = Alert::State::ACTIVATING;
            if (!m_maxLengthTimer.isActive()) {
                if (!m_maxLengthTimer.start(MAXIMUM_ALERT_RENDERING_TIME, std::bind(&Alert::onMaxTimerExpiration, this)).valid()) {
                    ACSDK_ERROR(LX("executeStartFailed").d("reason", "startTimerFailed"));
                }
            }
            startRendererLocked();
        }
        void Alert::deactivate(StopReason reason) {
            ACSDK_DEBUG9(LX("deactivate").d("reason", reason));
            lock_guard<mutex> lock(m_mutex);
            m_dynamicData.state = Alert::State::STOPPING;
            m_stopReason = reason;
            m_maxLengthTimer.stop();
            m_renderer->stop();
        }
        void Alert::getAlertData(StaticData* staticData, DynamicData* dynamicData) const {
            lock_guard<mutex> lock(m_mutex);
            if (!dynamicData && !staticData) return;
            if (staticData) *staticData = m_staticData;
            if (dynamicData) *dynamicData = m_dynamicData;
        }
        static bool hasAsset(const std::string& key, const std::unordered_map<std::string, Alert::Asset>& assets) {
            auto search = assets.find(key);
            return (search != assets.end()) && (search->second.id == key) && (!search->second.url.empty());
        }
        static bool validateAssetConfiguration(const Alert::AssetConfiguration& assetConfiguration) {
            if (!assetConfiguration.backgroundAssetId.empty() &&
                !hasAsset(assetConfiguration.backgroundAssetId, assetConfiguration.assets)) {
                ACSDK_ERROR(LX("validateAssetConfigurationFailed").d("reason", "invalidAssetConfiguration")
                    .d("assetId", assetConfiguration.backgroundAssetId).m("backgroundAssetId is not represented in the list of assets"));
                return false;
            }
            for (std::string const& a : assetConfiguration.assetPlayOrderItems) {
                if (!hasAsset(a, assetConfiguration.assets)) {
                    ACSDK_ERROR(LX("validateAssetConfigurationFailed").d("reason", "invalidAssetConfiguration").d("assetId", a)
                        .m("Asset ID on assetPlayOrderItems is not represented in the list of assets"));
                    return false;
                }
            }
            return true;
        }
        bool Alert::setAlertData(StaticData* staticData, DynamicData* dynamicData) {
            lock_guard<mutex> lock(m_mutex);
            if (!dynamicData && !staticData) return false;
            if (dynamicData) {
                if (!validateAssetConfiguration(dynamicData->assetConfiguration)) {
                    ACSDK_ERROR(LX("setAlertDataFailed").d("reason", "validateAssetConfigurationFailed"));
                    return false;
                }
                if (dynamicData->loopCount < 0) {
                    ACSDK_ERROR(
                        LX("setAlertDataFailed").d("loopCountValue", dynamicData->loopCount).m("loopCount less than zero."));
                    return false;
                }
                m_dynamicData = *dynamicData;
            }
            if (staticData) m_staticData = *staticData;
            return true;
        }
        void Alert::onRendererStateChange(RendererObserverInterface::State state, const std::string& reason) {
            ACSDK_DEBUG1(LX("onRendererStateChange").d("state", state).d("reason", reason).d("m_hasTimerExpired", m_hasTimerExpired)
                .d("m_dynamicData.state", m_dynamicData.state));
            bool shouldNotifyObserver = false;
            bool shouldRetryRendering = false;
            AlertObserverInterface::State notifyState = AlertObserverInterface::State::ERROR;
            std::string notifyReason;
            unique_lock<std::mutex> lock(m_mutex);
            auto observerCopy = m_observer;
            switch(state) {
                case RendererObserverInterface::State::UNSET: break;
                case RendererObserverInterface::State::STARTED:
                    if (State::STOPPED == m_dynamicData.state) {
                        ACSDK_ERROR(LX("onRendererStateChange").m("Renderer started but alert is STOPPED. Stop the Renderer"));
                        m_renderer->stop();
                    } else if (State::ACTIVATING == m_dynamicData.state) {
                        if (m_focusChangedDuringAlertActivation) {
                            m_focusChangedDuringAlertActivation = false;
                            shouldRetryRendering = true;
                            m_renderer->stop();
                        } else {
                            shouldNotifyObserver = true;
                            notifyState = AlertObserverInterface::State::STARTED;
                        }
                    }
                    break;
                case RendererObserverInterface::State::STOPPED:
                    if (m_hasTimerExpired) {
                        m_dynamicData.state = State::COMPLETED;
                        shouldNotifyObserver = true;
                        notifyState = AlertObserverInterface::State::COMPLETED;
                    } else {
                        if (Alert::State::STOPPING == m_dynamicData.state) {
                            m_dynamicData.state = State::STOPPED;
                            shouldNotifyObserver = true;
                            notifyState = AlertObserverInterface::State::STOPPED;
                            notifyReason = stopReasonToString(m_stopReason);
                        } else if (Alert::State::SNOOZING == m_dynamicData.state) {
                            m_dynamicData.state = State::SNOOZED;
                            shouldNotifyObserver = true;
                            notifyState = AlertObserverInterface::State::SNOOZED;
                        } else if (m_startRendererAgainAfterFullStop) {
                            m_startRendererAgainAfterFullStop = false;
                            shouldRetryRendering = true;
                        }
                    }
                    break;
                case RendererObserverInterface::State::COMPLETED:
                    m_dynamicData.state = State::COMPLETED;
                    shouldNotifyObserver = true;
                    notifyState = AlertObserverInterface::State::COMPLETED;
                    break;
                case RendererObserverInterface::State::ERROR:
                    if (State::STOPPING == m_dynamicData.state || State::STOPPED == m_dynamicData.state) {
                        ACSDK_INFO(LX("onRendererStateChangeFailed").d("reason", reason).m("Renderer failed while alert is being stopped. We do nothing here."));
                    } else if (
                        !m_dynamicData.assetConfiguration.assetPlayOrderItems.empty() && !m_dynamicData.hasRenderingFailed) {
                        ACSDK_ERROR(LX("onRendererStateChangeFailed").d("reason", reason).m("Renderer failed to handle a url. Retrying with local background audio sound."));
                        m_dynamicData.hasRenderingFailed = true;
                        shouldRetryRendering = true;
                    } else {
                        shouldNotifyObserver = true;
                        notifyState = AlertObserverInterface::State::ERROR;
                        notifyReason = reason;
                    }
                    break;
            }
            lock.unlock();
            if (shouldNotifyObserver && observerCopy) observerCopy->onAlertStateChange(m_staticData.token, getTypeName(), notifyState, notifyReason);
            if (shouldRetryRendering) {
                this_thread::sleep_for(milliseconds(75));
                startRenderer();
            }
        }
        std::string Alert::getToken() const {
            lock_guard<mutex> lock(m_mutex);
            return m_staticData.token;
        }
        int64_t Alert::getScheduledTime_Unix() const {
            lock_guard<mutex> lock(m_mutex);
            return m_dynamicData.timePoint.getTime_Unix();
        }
        std::string Alert::getScheduledTime_ISO_8601() const {
            lock_guard<mutex> lock(m_mutex);
            return getScheduledTime_ISO_8601Locked();
        }
        std::string Alert::getScheduledTime_ISO_8601Locked() const {
            return m_dynamicData.timePoint.getTime_ISO_8601();
        }
        Alert::State Alert::getState() const {
            lock_guard<mutex> lock(m_mutex);
            return m_dynamicData.state;
        }
        int Alert::getId() const {
            lock_guard<mutex> lock(m_mutex);
            return m_staticData.dbId;
        }
        bool Alert::updateScheduledTime(const std::string& newScheduledTime) {
            lock_guard<mutex> lock(m_mutex);
            const auto state = m_dynamicData.state;
            if (State::ACTIVE == state || State::ACTIVATING == state || State::STOPPING == state) {
                ACSDK_ERROR(LX("updateScheduledTimeFailed").d("reason", "unexpectedState").d("state", m_dynamicData.state));
                return false;
            }
            if (!m_dynamicData.timePoint.setTime_ISO_8601(newScheduledTime)) {
                ACSDK_ERROR(LX("updateScheduledTimeFailed").d("reason", "setTimeFailed").d("newTime", newScheduledTime));
                return false;
            }
            m_dynamicData.state = State::SET;
            return true;
        }
        bool Alert::snooze(const std::string& updatedScheduledTime) {
            lock_guard<std::mutex> lock(m_mutex);
            if (!m_dynamicData.timePoint.setTime_ISO_8601(updatedScheduledTime)) {
                ACSDK_ERROR(LX("snoozeFailed").d("reason", "setTimeFailed").d("updatedScheduledTime", updatedScheduledTime));
                return false;
            }
            m_dynamicData.state = State::SNOOZING;
            m_renderer->stop();
            return true;
        }
        Alert::StopReason Alert::getStopReason() const {
            lock_guard<mutex> lock(m_mutex);
            return m_stopReason;
        }
        int Alert::getLoopCount() const {
            lock_guard<mutex> lock(m_mutex);
            return m_dynamicData.loopCount;
        }
        milliseconds Alert::getLoopPause() const {
            lock_guard<mutex> lock(m_mutex);
            return m_dynamicData.assetConfiguration.loopPause;
        }
        std::string Alert::getBackgroundAssetId() const {
            lock_guard<mutex> lock(m_mutex);
            return m_dynamicData.assetConfiguration.backgroundAssetId;
        }
        Alert::AssetConfiguration Alert::getAssetConfiguration() const {
            lock_guard<mutex> lock(m_mutex);
            return m_dynamicData.assetConfiguration;
        }
        bool Alert::setAssetConfiguration(const Alert::AssetConfiguration& assetConfiguration) {
            lock_guard<mutex> lock(m_mutex);
            if (!validateAssetConfiguration(assetConfiguration)) {
                return false;
            }
            m_dynamicData.assetConfiguration = assetConfiguration;
            return true;
        }
        void Alert::startRenderer() {
            lock_guard<mutex> lock(m_mutex);
            startRendererLocked();
        }
        void Alert::startRendererLocked() {
            ACSDK_DEBUG9(LX("startRenderer"));
            vector<std::string> urls;
            auto loopCount = m_dynamicData.loopCount;
            auto loopPause = m_dynamicData.assetConfiguration.loopPause;
            bool startWithPause = false;
            auto audioFactory = getDefaultAudioFactory();
            if (FocusState::BACKGROUND == m_focusState) {
                audioFactory = getShortAudioFactory();
                if (!m_dynamicData.assetConfiguration.backgroundAssetId.empty() && !m_dynamicData.hasRenderingFailed) {
                    urls.push_back(
                        m_dynamicData.assetConfiguration.assets[m_dynamicData.assetConfiguration.backgroundAssetId].url);
                }
                loopPause = BACKGROUND_ALERT_SOUND_PAUSE_TIME;
                if (State::ACTIVATING != m_dynamicData.state) startWithPause = true;
            } else if (!m_dynamicData.assetConfiguration.assets.empty() && !m_dynamicData.hasRenderingFailed) {
                for (auto item : m_dynamicData.assetConfiguration.assetPlayOrderItems) urls.push_back(m_dynamicData.assetConfiguration.assets[item].url);
            }
            auto alarmVolumeRampEnabled = false;
            if (m_settingsManager) {
                auto alarmVolumeRampSetting = m_settingsManager->getValue<DeviceSettingsIndex::ALARM_VOLUME_RAMP>(types::getAlarmVolumeRampDefault()).second;
                alarmVolumeRampEnabled = isEnabled(alarmVolumeRampSetting) && (getTypeName() == Alarm::getTypeNameStatic());
            }
            m_renderer->start(shared_from_this(), audioFactory, alarmVolumeRampEnabled, urls, loopCount, loopPause, startWithPause);
        }
        void Alert::onMaxTimerExpiration() {
            ACSDK_DEBUG1(LX("onMaxTimerExpiration"));
            lock_guard<mutex> lock(m_mutex);
            m_dynamicData.state = Alert::State::STOPPING;
            m_hasTimerExpired = true;
            m_renderer->stop();
        }
        bool Alert::isPastDue(int64_t currentUnixTime, std::chrono::seconds timeLimit) {
            lock_guard<mutex> lock(m_mutex);
            int64_t cutoffTime = currentUnixTime - timeLimit.count();
            return (m_dynamicData.timePoint.getTime_Unix() < cutoffTime);
        }
        function<pair<unique_ptr<istream>, const MediaType>()> Alert::getDefaultAudioFactory() const {
            return m_defaultAudioFactory;
        }
        function<pair<unique_ptr<istream>, const MediaType>()> Alert::getShortAudioFactory() const {
            return m_shortAudioFactory;
        }
        Alert::ContextInfo Alert::getContextInfo() const {
            lock_guard<mutex> lock(m_mutex);
            return ContextInfo(m_staticData.token, getTypeName(), getScheduledTime_ISO_8601Locked());
        }
        std::string Alert::stateToString(Alert::State state) {
            switch(state) {
                case Alert::State::UNSET: return "UNSET";
                case Alert::State::SET: return "SET";
                case Alert::State::READY: return "READY";
                case Alert::State::ACTIVATING: return "ACTIVATING";
                case Alert::State::ACTIVE: return "ACTIVE";
                case Alert::State::SNOOZING: return "SNOOZING";
                case Alert::State::SNOOZED: return "SNOOZED";
                case Alert::State::STOPPING: return "STOPPING";
                case Alert::State::STOPPED: return "STOPPED";
                case Alert::State::COMPLETED: return "COMPLETED";
            }
            ACSDK_ERROR(LX("stateToStringFailed").d("unhandledCase", state));
            return "UNKNOWN_STATE";
        }
        std::string Alert::stopReasonToString(Alert::StopReason stopReason) {
            switch(stopReason) {
                case Alert::StopReason::UNSET: return "UNSET";
                case Alert::StopReason::AVS_STOP: return "AVS_STOP";
                case Alert::StopReason::LOCAL_STOP: return "LOCAL_STOP";
                case Alert::StopReason::SHUTDOWN: return "SHUTDOWN";
                case Alert::StopReason::LOG_OUT: return "LOG_OUT";
            }
            ACSDK_ERROR(LX("stopReasonToStringFailed").d("unhandledCase", stopReason));
            return "UNKNOWN_STOP_REASON";
        }
        std::string Alert::parseFromJsonStatusToString(Alert::ParseFromJsonStatus parseFromJsonStatus) {
            switch(parseFromJsonStatus) {
                case Alert::ParseFromJsonStatus::OK: return "OK";
                case Alert::ParseFromJsonStatus::MISSING_REQUIRED_PROPERTY: return "MISSING_REQUIRED_PROPERTY";
                case Alert::ParseFromJsonStatus::INVALID_VALUE: return "INVALID_VALUE";
            }
            ACSDK_ERROR(LX("parseFromJsonStatusToStringFailed").d("unhandledCase", parseFromJsonStatus));
            return "UNKNOWN_PARSE_FROM_JSON_STATUS";
        }
        void Alert::printDiagnostic() {
            lock_guard<mutex> lock(m_mutex);
            std::string assetInfoString;
            for (auto asset : m_dynamicData.assetConfiguration.assets) assetInfoString += "\nid:" + asset.second.id + ", url:" + asset.second.url;
            std::string assetPlayOrderItemsInfoString;
            for (auto assetOrderItem : m_dynamicData.assetConfiguration.assetPlayOrderItems) assetPlayOrderItemsInfoString += "id:" + assetOrderItem + ", ";
            std::stringstream ss;
            ss << std::endl
               << " ** Alert | id:" << std::to_string(m_staticData.dbId) << std::endl
               << "          | type:" << getTypeName() << std::endl
               << "          | token:" << m_staticData.token << std::endl
               << "          | scheduled time (8601):" << getScheduledTime_ISO_8601() << std::endl
               << "          | scheduled time (Unix):" << getScheduledTime_Unix() << std::endl
               << "          | state:" << stateToString(m_dynamicData.state) << std::endl
               << "          | number assets:" << m_dynamicData.assetConfiguration.assets.size() << std::endl
               << "          | number assets play order items:" << m_dynamicData.assetConfiguration.assetPlayOrderItems.size()
               << std::endl
               << "          | asset info:" << assetInfoString << std::endl
               << "          | asset order info:" << assetPlayOrderItemsInfoString << std::endl
               << "          | background asset id:" << m_dynamicData.assetConfiguration.backgroundAssetId << std::endl
               << "          | loop count:" << m_dynamicData.loopCount << std::endl
               << "          | loop pause in milliseconds:" << m_dynamicData.assetConfiguration.loopPause.count() << std::endl;
            ACSDK_INFO(LX(ss.str()));
        }
    }
}