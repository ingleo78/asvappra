#ifndef ACSDKALERTS_ALERT_H_
#define ACSDKALERTS_ALERT_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <acsdk_alerts/Renderer/Renderer.h>
#include <acsdk_alerts/Renderer/RendererObserverInterface.h>
#include <avs/FocusState.h>
#include <timing/Timer.h>
#include <timing/TimePoint.h>
#include <acsdk_alerts_interfaces/AlertObserverInterface.h>
#include <settings/DeviceSettingsManager.h>
#include <json/document.h>

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace rapidjson;
        using namespace renderer;
        using namespace utils;
        using namespace settings;
        using namespace timing;
        class Alert : public RendererObserverInterface, public enable_shared_from_this<Alert> {
        public:
            enum class State {
                UNSET,
                SET,
                READY,
                ACTIVATING,
                ACTIVE,
                SNOOZING,
                SNOOZED,
                STOPPING,
                STOPPED,
                COMPLETED
            };
            enum class StopReason {
                UNSET,
                AVS_STOP,
                LOCAL_STOP,
                SHUTDOWN,
                LOG_OUT
            };
            struct Asset {
                Asset() = default;
                Asset(const std::string& id, const std::string& url) : id{id}, url{url} {}
                std::string id;
                std::string url;
            };
            struct AssetConfiguration {
                AssetConfiguration() : loopPause{std::chrono::milliseconds{0}} {}
                unordered_map<std::string, Asset> assets;
                vector<std::string> assetPlayOrderItems;
                std::string backgroundAssetId;
                milliseconds loopPause;
            };
            struct StaticData {
                StaticData() : dbId{0} {}
                std::string token;
                int dbId;
            };
            struct DynamicData {
                DynamicData() : state{State::SET}, loopCount{0}, hasRenderingFailed{false} {}
                State state;
                avsCommon::utils::timing::TimePoint timePoint;
                int loopCount;
                bool hasRenderingFailed;
                AssetConfiguration assetConfiguration;
            };
            enum class ParseFromJsonStatus {
                OK,
                MISSING_REQUIRED_PROPERTY,
                INVALID_VALUE
            };
            struct ContextInfo {
                ContextInfo(const std::string& token, const std::string& type, const std::string& scheduledTime_ISO_8601) : token{token}, type{type},
                            scheduledTime_ISO_8601{scheduledTime_ISO_8601} {}
                std::string token;
                std::string type;
                std::string scheduledTime_ISO_8601;
            };
            static std::string stateToString(Alert::State state);
            static std::string stopReasonToString(Alert::StopReason stopReason);
            static std::string parseFromJsonStatusToString(Alert::ParseFromJsonStatus parseFromJsonStatus);
            Alert(function<pair<unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                  shared_ptr<DeviceSettingsManager> settingsManager);
            virtual std::string getTypeName() const = 0;
            function<std::pair<unique_ptr<istream>, const MediaType>()> getDefaultAudioFactory() const;
            function<pair<unique_ptr<istream>, const MediaType>()> getShortAudioFactory() const;
            Alert::ContextInfo getContextInfo() const;
            void onRendererStateChange(RendererObserverInterface::State state, const std::string& reason) override;
            ParseFromJsonStatus parseFromJson(const Value& payload, std::string* errorMessage);
            ParseFromJsonStatus parseFromJson(const Document& payload, std::string* errorMessage);
            void setFocusState(FocusState focusState);
            void setRenderer(shared_ptr<RendererInterface> renderer);
            void setObserver(acsdkAlertsInterfaces::AlertObserverInterface* observer);
            void activate();
            void deactivate(StopReason reason);
            bool updateScheduledTime(const std::string& newScheduledTime);
            bool snooze(const std::string& updatedScheduledTime);
            bool setStateActive();
            void reset();
            std::string getToken() const;
            int64_t getScheduledTime_Unix() const;
            std::string getScheduledTime_ISO_8601() const;
            Alert::State getState() const;
            StopReason getStopReason() const;
            void getAlertData(StaticData* staticData, DynamicData* dynamicData) const;
            bool setAlertData(StaticData* staticData, DynamicData* dynamicData);
            int getId() const;
            bool isPastDue(int64_t currentUnixTime, std::chrono::seconds timeLimit);
            int getLoopCount() const;
            milliseconds getLoopPause() const;
            std::string getBackgroundAssetId() const;
            AssetConfiguration getAssetConfiguration() const;
            bool setAssetConfiguration(const AssetConfiguration& assetConfiguration);
            void printDiagnostic();
        private:
            std::string getScheduledTime_ISO_8601Locked() const;
            void startRenderer();
            void startRendererLocked();
            void onMaxTimerExpiration();
            mutable mutex m_mutex;
            StaticData m_staticData;
            DynamicData m_dynamicData;
            StopReason m_stopReason;
            avsCommon::avs::FocusState m_focusState;
            bool m_hasTimerExpired;
            avsCommon::utils::timing::Timer m_maxLengthTimer;
            acsdkAlertsInterfaces::AlertObserverInterface* m_observer;
            std::shared_ptr<renderer::RendererInterface> m_renderer;
            const std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> m_defaultAudioFactory;
            const std::function<std::pair<std::unique_ptr<std::istream>, const avsCommon::utils::MediaType>()> m_shortAudioFactory;
            std::shared_ptr<settings::DeviceSettingsManager> m_settingsManager;
            bool m_focusChangedDuringAlertActivation;
            bool m_startRendererAgainAfterFullStop;
        };
        struct TimeComparator {
            bool operator()(const shared_ptr<Alert>& lhs, const shared_ptr<Alert>& rhs) const {
                if (lhs->getScheduledTime_Unix() == rhs->getScheduledTime_Unix()) return (lhs->getToken() < rhs->getToken());
                return (lhs->getScheduledTime_Unix() < rhs->getScheduledTime_Unix());
            }
        };
        inline std::ostream& operator<<(ostream& stream, const Alert::State& state) {
            return stream << Alert::stateToString(state);
        }
        inline std::ostream& operator<<(ostream& stream, const Alert::StopReason& reason) {
            return stream << Alert::stopReasonToString(reason);
        }
        inline ostream& operator<<(ostream& stream, const Alert::ParseFromJsonStatus& status) {
            return stream << Alert::parseFromJsonStatusToString(status);
        }
    }
}
#endif