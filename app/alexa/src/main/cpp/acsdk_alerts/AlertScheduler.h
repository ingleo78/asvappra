#ifndef ACSDKALERTS_ALERTSCHEDULER_H_
#define ACSDKALERTS_ALERTSCHEDULER_H_

#include <list>
#include <set>
#include <string>
#include <acsdk_alerts/Storage/AlertStorageInterface.h>
#include <avs/FocusState.h>
#include <metrics/MetricRecorderInterface.h>
#include <settings/DeviceSettingsManager.h>
#include <acsdk_alerts_interfaces/AlertObserverInterface.h>

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace acsdkAlerts;
        using namespace acsdkAlertsInterfaces;
        using namespace renderer;
        using namespace storage;
        using namespace utils;
        using namespace metrics;
        using namespace threading;
        using namespace timing;
        class AlertScheduler : public AlertObserverInterface {
        public:
            struct AlertsContextInfo {
                vector<Alert::ContextInfo> scheduledAlerts;
                vector<Alert::ContextInfo> activeAlerts;
            };
            AlertScheduler(shared_ptr<AlertStorageInterface> alertStorage, shared_ptr<RendererInterface> alertRenderer, seconds alertPastDueTimeLimitSeconds,
                           shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
            void onAlertStateChange(const string& alertToken, const string& alertType, State state, const string& reason = "");
            bool initialize(const shared_ptr<AlertObserverInterface>& observer, const shared_ptr<DeviceSettingsManager>& settingsManager,
                            bool startAlertSchedulingOnInitialization = true);
            bool scheduleAlert(shared_ptr<Alert> alert);
            bool reloadAlertsFromDatabase(shared_ptr<DeviceSettingsManager> settingsManager, bool shouldScheduleAlerts);
            bool snoozeAlert(const string& alertToken, const std::string& updatedTime_ISO_8601);
            bool deleteAlert(const string& alertToken);
            bool deleteAlerts(const list<string>& tokenList);
            bool isAlertActive(shared_ptr<Alert> alert);
            void updateFocus(FocusState focusState);
            FocusState getFocusState();
            AlertScheduler::AlertsContextInfo getContextInfo();
            void onLocalStop();
            void clearData(Alert::StopReason reason = Alert::StopReason::SHUTDOWN);
            void shutdown();
            list<shared_ptr<Alert>> getAllAlerts();
        private:
            void executeOnAlertStateChange(string alertToken, string alertType, State state, string reason);
            bool updateAlert(const shared_ptr<Alert>& alert, const string& newScheduledTime, const Alert::AssetConfiguration& newAssetConfiguration);
            void notifyObserver(const string& alertToken, const string& alertType, AlertObserverInterface::State state, const string& reason = "");
            void executeNotifyObserver(const string& alertToken, const string& alertType, AlertObserverInterface::State state, const string& reason = "");
            void setTimerForNextAlertLocked();
            void setTimerForNextAlert();
            void activateNextAlertLocked();
            void onAlertReady(const string& alertToken, const string& alertType);
            bool isAlertActiveLocked(shared_ptr<Alert> alert) const;
            shared_ptr<Alert> getAlertLocked(const string& token) const;
            void deactivateActiveAlertHelperLocked(Alert::StopReason reason);
            void eraseAlert(shared_ptr<Alert> alert);
            TimeUtils m_timeUtils;
            shared_ptr<AlertObserverInterface> m_observer;
            shared_ptr<DeviceSettingsManager> m_settingsManager;
            mutex m_mutex;
            shared_ptr<AlertStorageInterface> m_alertStorage;
            shared_ptr<RendererInterface> m_alertRenderer;
            seconds m_alertPastDueTimeLimit;
            FocusState m_focusState;
            shared_ptr<Alert> m_activeAlert;
            set<shared_ptr<Alert>, TimeComparator> m_scheduledAlerts;
            Timer m_scheduledAlertTimer;
            bool m_shouldScheduleAlerts;
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            Executor m_executor;
        };
    }
}
#endif