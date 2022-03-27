#include "AlertScheduler.h"
#include <util/error/FinallyGuard.h>
#include <logger/Logger.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include <timing/TimeUtils.h>

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace acsdkAlertsInterfaces;
        using namespace avs;
        using namespace utils;
        using namespace logger;
        using namespace timing;
        using namespace error;
        using namespace metrics;
        static const string TAG("AlertScheduler");
        static const string ALERT_METRIC_SOURCE_PREFIX = "ALERT-";
        static const string ALERT_SCHEDULING_FAILED = "alertSchedulingFailed";
        static const string ALERT_PAST_DUE_DURING_SCHEDULING = "alertpastDueWhileScheduling";
        static const string ACTIVE_ALERT_RELOADED_DURING_SCHEDULING = "activeAlertReloadedDuringScheduling";
        #define LX(event) LogEntry(TAG, event)
        static void submitMetric(const shared_ptr<MetricRecorderInterface>& metricRecorder, const string& eventName, int count) {
            if (!metricRecorder) return;
            auto metricEvent = MetricEventBuilder{}.setActivityName(ALERT_METRIC_SOURCE_PREFIX + eventName)
                                   .addDataPoint(DataPointCounterBuilder{}.setName(eventName).increment(count).build()).build();
            if (metricEvent == nullptr) {
                ACSDK_ERROR(LX("Error creating metric."));
                return;
            }
            recordMetric(metricRecorder, metricEvent);
        }
        AlertScheduler::AlertScheduler(shared_ptr<AlertStorageInterface> alertStorage, shared_ptr<RendererInterface> alertRenderer, seconds alertPastDueTimeLimit,
                                       shared_ptr<MetricRecorderInterface> metricRecorder) : m_alertStorage{alertStorage}, m_alertRenderer{alertRenderer},
                                       m_alertPastDueTimeLimit{alertPastDueTimeLimit}, m_focusState{avsCommon::avs::FocusState::NONE}, m_shouldScheduleAlerts{false},
                                       m_metricRecorder{metricRecorder} {}
        void AlertScheduler::onAlertStateChange(const string& alertToken, const string& alertType, State state, const string& reason) {
            ACSDK_DEBUG9(LX("onAlertStateChange").d("alertToken", alertToken).d("alertType", alertType).d("state", state).d("reason", reason));
            m_executor.submit([this, alertToken, alertType, state, reason]() {
                executeOnAlertStateChange(alertToken, alertType, state, reason);
            });
        }
        bool AlertScheduler::initialize(const shared_ptr<AlertObserverInterface>& observer, const shared_ptr<DeviceSettingsManager>& settingsManager,
                                        bool startAlertSchedulingOnInitialization) {
            m_shouldScheduleAlerts = startAlertSchedulingOnInitialization;
            if (!observer) {
                ACSDK_ERROR(LX("initializeFailed").m("observer was nullptr."));
                return false;
            }
            m_observer = observer;
            if (!m_alertStorage->open()) {
                ACSDK_INFO(LX("initialize").m("Couldn't open database.  Creating."));
                if (!m_alertStorage->createDatabase()) {
                    ACSDK_ERROR(LX("initializeFailed").m("Could not create database."));
                    return false;
                }
            }
            if (!reloadAlertsFromDatabase(settingsManager, m_shouldScheduleAlerts)) {
                ACSDK_ERROR(LX("initializeFailed").m("Could not reload alerts from database."));
                return false;
            }
            return true;
        }
        bool AlertScheduler::scheduleAlert(std::shared_ptr<Alert> alert) {
            ACSDK_DEBUG9(LX("scheduleAlert").d("token", alert->getToken()));
            int64_t unixEpochNow = 0;
            if (!m_timeUtils.getCurrentUnixTime(&unixEpochNow)) {
                ACSDK_ERROR(LX("scheduleAlertFailed").d("reason", "could not get current unix time."));
                return false;
            }
            std::lock_guard<std::mutex> lock(m_mutex);
            if (alert->isPastDue(unixEpochNow, m_alertPastDueTimeLimit)) {
                ACSDK_ERROR(LX("scheduleAlertFailed").d("reason", "parsed alert is past-due.  Ignoring."));
                return false;
            }
            auto oldAlert = getAlertLocked(alert->getToken());
            if (oldAlert) {
                ACSDK_DEBUG9(LX("oldAlert").d("token", oldAlert->getToken()));
                bool alertIsCurrentlyActive = m_activeAlert && (m_activeAlert->getToken() == oldAlert->getToken());
                if (alertIsCurrentlyActive) return false;
                return updateAlert(oldAlert, alert->getScheduledTime_ISO_8601(), alert->getAssetConfiguration());
            }
            if (!m_alertStorage->store(alert)) {
                ACSDK_ERROR(LX("scheduleAlertFailed").d("reason", "could not store alert in database."));
                return false;
            }
            alert->setRenderer(m_alertRenderer);
            alert->setObserver(this);
            m_scheduledAlerts.insert(alert);
            if (!m_activeAlert) setTimerForNextAlertLocked();
            return true;
        }
        bool AlertScheduler::reloadAlertsFromDatabase(
            std::shared_ptr<settings::DeviceSettingsManager> settingsManager,
            bool shouldScheduleAlerts) {
            m_shouldScheduleAlerts = shouldScheduleAlerts;
            int64_t unixEpochNow = 0;
            if (!m_timeUtils.getCurrentUnixTime(&unixEpochNow)) {
                ACSDK_ERROR(LX("initializeFailed").d("reason", "could not get current unix time."));
                submitMetric(m_metricRecorder, ALERT_SCHEDULING_FAILED, 1);
                return false;
            } else submitMetric(m_metricRecorder, ALERT_SCHEDULING_FAILED, 0);
            vector<shared_ptr<Alert>> alerts;
            lock_guard<mutex> lock(m_mutex);
            if (m_scheduledAlertTimer.isActive()) m_scheduledAlertTimer.stop();
            m_scheduledAlerts.clear();
            m_alertStorage->load(&alerts, settingsManager);
            if (m_shouldScheduleAlerts) {
                int alertPastDueDuringSchedulingCount = 0;
                int activeAlertReloadedDuringSchedulingCount = 0;
                for (auto& alert : alerts) {
                    bool alertIsCurrentlyActive = m_activeAlert && (m_activeAlert->getToken() == alert->getToken());
                    if (!alertIsCurrentlyActive) {
                        if (alert->isPastDue(unixEpochNow, m_alertPastDueTimeLimit)) {
                            notifyObserver(alert->getToken(), alert->getTypeName(), AlertObserverInterface::State::PAST_DUE);
                            ACSDK_DEBUG5(LX(ALERT_PAST_DUE_DURING_SCHEDULING).d("alertId", alert->getToken()));
                            ++alertPastDueDuringSchedulingCount;
                            eraseAlert(alert);
                        } else {
                            if (Alert::State::ACTIVE == alert->getState()) {
                                alert->reset();
                                m_alertStorage->modify(alert);
                                ACSDK_DEBUG5(LX(ACTIVE_ALERT_RELOADED_DURING_SCHEDULING).d("alertId", alert->getToken()));
                                ++activeAlertReloadedDuringSchedulingCount;
                            }
                            alert->setRenderer(m_alertRenderer);
                            alert->setObserver(this);
                            m_scheduledAlerts.insert(alert);
                            notifyObserver(alert->getToken(), alert->getTypeName(),AlertObserverInterface::State::SCHEDULED_FOR_LATER,
                                           alert->getScheduledTime_ISO_8601());
                        }
                    }
                }
                if (!m_activeAlert) setTimerForNextAlertLocked();
                submitMetric(m_metricRecorder, ALERT_PAST_DUE_DURING_SCHEDULING, alertPastDueDuringSchedulingCount);
                submitMetric(m_metricRecorder, ACTIVE_ALERT_RELOADED_DURING_SCHEDULING, activeAlertReloadedDuringSchedulingCount);
            } else {
                for (auto& alert : alerts) {
                    alert->setRenderer(m_alertRenderer);
                    alert->setObserver(this);
                    m_scheduledAlerts.insert(alert);
                }
            }
            return true;
        }
        bool AlertScheduler::updateAlert(const shared_ptr<Alert>& alert, const string& newScheduledTime, const Alert::AssetConfiguration& newAssetConfiguration) {
            ACSDK_DEBUG5(LX(__func__).d("token", alert->getToken()).m("updateAlert"));
            m_scheduledAlerts.erase(alert);
            FinallyGuard guard{[this, &alert] {
                m_scheduledAlerts.insert(alert);
                if (!m_activeAlert) setTimerForNextAlertLocked();
            }};
            auto oldScheduledTime = alert->getScheduledTime_ISO_8601();
            auto oldAssetConfiguration = alert->getAssetConfiguration();
            if (!alert->updateScheduledTime(newScheduledTime)) {
                ACSDK_ERROR(LX("updateAlertFailed").m("Update alert time failed."));
                return false;
            }
            if (!alert->setAssetConfiguration(newAssetConfiguration)) {
                ACSDK_ERROR(LX("updateAlertFailed").m("Update asset configuration failed."));
                alert->updateScheduledTime(oldScheduledTime);
                return false;
            }
            if (!m_alertStorage->modify(alert)) {
                ACSDK_ERROR(LX("updateAlertFailed").d("reason", "could not update alert in database."));
                alert->updateScheduledTime(oldScheduledTime);
                alert->setAssetConfiguration(oldAssetConfiguration);
                return false;
            }
            return true;
        }
        bool AlertScheduler::snoozeAlert(const string& alertToken, const string& updatedTime_ISO_8601) {
            lock_guard<mutex> lock(m_mutex);
            if (!m_activeAlert || m_activeAlert->getToken() != alertToken) {
                ACSDK_ERROR(LX("snoozeAlertFailed").m("alert is not active.").d("token", alertToken));
                return false;
            }
            m_activeAlert->snooze(updatedTime_ISO_8601);
            return true;
        }
        bool AlertScheduler::deleteAlert(const string& alertToken) {
            ACSDK_DEBUG9(LX("deleteAlert").d("alertToken", alertToken));
            lock_guard<mutex> lock(m_mutex);
            if (m_activeAlert && m_activeAlert->getToken() == alertToken) {
                deactivateActiveAlertHelperLocked(Alert::StopReason::AVS_STOP);
                return true;
            }
            auto alert = getAlertLocked(alertToken);
            if (!alert) {
                ACSDK_WARN(LX(__func__).d("Alert does not exist", alertToken));
                return true;
            }
            eraseAlert(alert);
            m_scheduledAlerts.erase(alert);
            setTimerForNextAlertLocked();
            return true;
        }
        bool AlertScheduler::deleteAlerts(const list<string>& tokenList) {
            ACSDK_DEBUG5(LX(__func__));
            bool deleteActiveAlert = false;
            list<shared_ptr<Alert>> alertsToBeRemoved;
            lock_guard<std::mutex> lock(m_mutex);
            for (auto& alertToken : tokenList) {
                if (m_activeAlert && m_activeAlert->getToken() == alertToken) {
                    deleteActiveAlert = true;
                    alertsToBeRemoved.push_back(m_activeAlert);
                    ACSDK_DEBUG3(LX(__func__).m("Active alert is going to be deleted."));
                    continue;
                }
                auto alert = getAlertLocked(alertToken);
                if (!alert) {
                    ACSDK_WARN(LX(__func__).d("Alert is missing", alertToken));
                    continue;
                }
                alertsToBeRemoved.push_back(alert);
            }
            if (!m_alertStorage->bulkErase(alertsToBeRemoved)) {
                ACSDK_ERROR(LX("deleteAlertsFailed").d("reason", "Could not erase alerts from database"));
                return false;
            }
            if (deleteActiveAlert) {
                deactivateActiveAlertHelperLocked(Alert::StopReason::AVS_STOP);
                m_activeAlert.reset();
            }
            for (auto& alert : alertsToBeRemoved) {
                m_scheduledAlerts.erase(alert);
                notifyObserver(alert->getToken(), alert->getTypeName(), AlertObserverInterface::State::DELETED);
            }
            setTimerForNextAlertLocked();
            return true;
        }
        bool AlertScheduler::isAlertActive(std::shared_ptr<Alert> alert) {
            lock_guard<mutex> lock(m_mutex);
            return isAlertActiveLocked(alert);
        }
        void AlertScheduler::updateFocus(avsCommon::avs::FocusState focusState) {
            ACSDK_DEBUG9(LX("updateFocus").d("focusState", focusState));
            lock_guard<mutex> lock(m_mutex);
            if (m_focusState == focusState) return;
            m_focusState = focusState;
            switch(m_focusState) {
                case FocusState::FOREGROUND:
                    if (m_activeAlert) {
                        m_activeAlert->setFocusState(m_focusState);
                        auto token = m_activeAlert->getToken();
                        auto type = m_activeAlert->getTypeName();
                        notifyObserver(token, type, AlertObserverInterface::State::FOCUS_ENTERED_FOREGROUND);
                    } else activateNextAlertLocked();
                    return;
                case FocusState::BACKGROUND:
                    if (m_activeAlert) {
                        m_activeAlert->setFocusState(m_focusState);
                        auto token = m_activeAlert->getToken();
                        auto type = m_activeAlert->getTypeName();
                        notifyObserver(token, type, AlertObserverInterface::State::FOCUS_ENTERED_BACKGROUND);
                    } else activateNextAlertLocked();
                    return;
                case FocusState::NONE: deactivateActiveAlertHelperLocked(Alert::StopReason::LOCAL_STOP); return;
            }
            ACSDK_ERROR(LX("updateFocusFailed").d("unhandledState", focusState));
        }
        avsCommon::avs::FocusState AlertScheduler::getFocusState() {
            lock_guard<std::mutex> lock(m_mutex);
            return m_focusState;
        }
        AlertScheduler::AlertsContextInfo AlertScheduler::getContextInfo() {
            lock_guard<std::mutex> lock(m_mutex);
            AlertScheduler::AlertsContextInfo alertContexts;
            for (const auto& alert : m_scheduledAlerts) alertContexts.scheduledAlerts.push_back(alert->getContextInfo());
            if (m_activeAlert) {
                alertContexts.scheduledAlerts.push_back(m_activeAlert->getContextInfo());
                alertContexts.activeAlerts.push_back(m_activeAlert->getContextInfo());
            }
            return alertContexts;
        }
        void AlertScheduler::onLocalStop() {
            ACSDK_DEBUG9(LX("onLocalStop"));
            lock_guard<mutex> lock(m_mutex);
            deactivateActiveAlertHelperLocked(Alert::StopReason::LOCAL_STOP);
        }
        void AlertScheduler::clearData(Alert::StopReason reason) {
            ACSDK_INFO(LX("clearData").d("reason", Alert::stopReasonToString(reason)));
            lock_guard<std::mutex> lock(m_mutex);
            deactivateActiveAlertHelperLocked(reason);
            if (m_scheduledAlertTimer.isActive()) m_scheduledAlertTimer.stop();
            for (const auto& alert : m_scheduledAlerts) notifyObserver(alert->getToken(), alert->getTypeName(), AlertObserverInterface::State::DELETED);
            m_scheduledAlerts.clear();
            m_alertStorage->clearDatabase();
        }
        void AlertScheduler::shutdown() {
            m_executor.shutdown();
            m_scheduledAlertTimer.stop();
            m_observer.reset();
            lock_guard<mutex> lock(m_mutex);
            m_alertStorage.reset();
            m_alertRenderer.reset();
            m_activeAlert.reset();
            m_scheduledAlerts.clear();
        }
        void AlertScheduler::executeOnAlertStateChange(string alertToken, string alertType, State state, string reason) {
            ACSDK_DEBUG9(LX("executeOnAlertStateChange").d("alertToken", alertToken).d("state", state).d("reason", reason));
            lock_guard<mutex> lock(m_mutex);
            switch(state) {
                case State::READY: notifyObserver(alertToken, alertType, state, reason); break;
                case State::STARTED:
                    if (m_activeAlert && Alert::State::ACTIVATING == m_activeAlert->getState()) {
                        m_activeAlert->setStateActive();
                        m_alertStorage->modify(m_activeAlert);
                        notifyObserver(alertToken, alertType, state, reason);
                    }
                    break;
                case State::STOPPED:
                    notifyObserver(alertToken, alertType, state, reason);
                    if (m_activeAlert && m_activeAlert->getToken() == alertToken) {
                        eraseAlert(m_activeAlert);
                        m_activeAlert.reset();
                    } else {
                        auto alert = getAlertLocked(alertToken);
                        if (alert) {
                            ACSDK_DEBUG((LX("erasing a stopped Alert that is no longer active").d("alertToken", alertToken)));
                            eraseAlert(alert);
                        }
                    }
                    setTimerForNextAlertLocked();
                    break;
                case State::COMPLETED:
                    notifyObserver(alertToken, alertType, state, reason);
                    eraseAlert(m_activeAlert);
                    m_activeAlert.reset();
                    setTimerForNextAlertLocked();
                    break;
                case State::SNOOZED:
                    m_alertStorage->modify(m_activeAlert);
                    m_scheduledAlerts.insert(m_activeAlert);
                    m_activeAlert.reset();
                    notifyObserver(alertToken, alertType, state, reason);
                    setTimerForNextAlertLocked();
                    break;
                case State::PAST_DUE: break;
                case State::FOCUS_ENTERED_FOREGROUND: break;
                case State::FOCUS_ENTERED_BACKGROUND: break;
                case State::SCHEDULED_FOR_LATER: break;
                case State::DELETED: break;
                case State::ERROR:
                    if (m_activeAlert && m_activeAlert->getToken() == alertToken) {
                        eraseAlert(m_activeAlert);
                        m_activeAlert.reset();
                        setTimerForNextAlertLocked();
                    } else {
                        auto alert = getAlertLocked(alertToken);
                        if (alert) {
                            ACSDK_DEBUG((LX("erasing Alert with an error that is no longer active").d("alertToken", alertToken)));
                            eraseAlert(alert);
                            m_scheduledAlerts.erase(alert);
                            setTimerForNextAlertLocked();
                        }
                    }
                    notifyObserver(alertToken, alertType, state, reason);
                    break;
            }
        }
        void AlertScheduler::notifyObserver(
            const string& alertToken,
            const string& alertType,
            AlertObserverInterface::State state,
            const string& reason) {
            ACSDK_DEBUG9(LX("notifyObserver").d("alertToken", alertToken).d("alertType", alertType).d("state", state).d("reason", reason));
            m_executor.submit([this, alertToken, alertType, state, reason]() { executeNotifyObserver(alertToken, alertType, state, reason); });
        }
        void AlertScheduler::executeNotifyObserver(const string& alertToken, const string& alertType, AlertObserverInterface::State state, const string& reason) {
            m_observer->onAlertStateChange(alertToken, alertType, state, reason);
        }
        void AlertScheduler::deactivateActiveAlertHelperLocked(Alert::StopReason reason) {
            if (m_activeAlert) m_activeAlert->deactivate(reason);
        }
        void AlertScheduler::setTimerForNextAlert() {
            lock_guard<mutex> lock(m_mutex);
            setTimerForNextAlertLocked();
        }
        void AlertScheduler::setTimerForNextAlertLocked() {
            if (m_shouldScheduleAlerts) {
                ACSDK_DEBUG9(LX("setTimerForNextAlertLocked"));
                if (m_scheduledAlertTimer.isActive()) m_scheduledAlertTimer.stop();
                if (m_activeAlert) {
                    ACSDK_INFO(LX("executeScheduleNextAlertForRendering").m("An alert is already active."));
                    return;
                }
                if (m_scheduledAlerts.empty()) {
                    ACSDK_DEBUG9(LX("executeScheduleNextAlertForRendering").m("no work to do."));
                    return;
                }
                auto alert = (*m_scheduledAlerts.begin());
                int64_t timeNow;
                if (!m_timeUtils.getCurrentUnixTime(&timeNow)) {
                    ACSDK_ERROR(LX("executeScheduleNextAlertForRenderingFailed").d("reason", "could not get current unix time."));
                    return;
                }
                seconds secondsToWait{alert->getScheduledTime_Unix() - timeNow};
                if (secondsToWait < seconds::zero()) secondsToWait = seconds::zero();
                if (secondsToWait == seconds::zero()) {
                    auto token = alert->getToken();
                    auto type = alert->getTypeName();
                    notifyObserver(token, type, AlertObserverInterface::State::READY);
                } else {
                    if (!m_scheduledAlertTimer.start(secondsToWait,bind(&AlertScheduler::onAlertReady, this, alert->getToken(), alert->getTypeName())).valid()) {
                        ACSDK_ERROR(LX("executeScheduleNextAlertForRenderingFailed").d("reason", "startTimerFailed"));
                    }
                }
            } else { ACSDK_DEBUG5(LX("executeScheduleNextAlertForRenderingSkipped").d("reason", "m_shouldScheduleAlerts is false.")); }
        }
        void AlertScheduler::onAlertReady(const string& alertToken, const string& alertType) {
            ACSDK_DEBUG9(LX("onAlertReady").d("alertToken", alertToken).d("alertType", alertType));
            notifyObserver(alertToken, alertType, AlertObserverInterface::State::READY);
        }
        void AlertScheduler::activateNextAlertLocked() {
            ACSDK_DEBUG9(LX("activateNextAlertLocked"));
            if (m_activeAlert) {
                ACSDK_ERROR(LX("activateNextAlertLockedFailed").d("reason", "An alert is already active."));
                return;
            }
            if (m_scheduledAlerts.empty()) return;
            m_activeAlert = *(m_scheduledAlerts.begin());
            m_scheduledAlerts.erase(m_scheduledAlerts.begin());
            m_activeAlert->setFocusState(m_focusState);
            m_activeAlert->activate();
        }
        bool AlertScheduler::isAlertActiveLocked(shared_ptr<Alert> alert) const {
            if (!m_activeAlert) return false;
            if (m_activeAlert->getToken() != alert->getToken()) return false;
            auto state = m_activeAlert->getState();
            return (Alert::State::ACTIVATING == state || Alert::State::ACTIVE == state);
        }
        std::shared_ptr<Alert> AlertScheduler::getAlertLocked(const string& token) const {
            for (auto& alert : m_scheduledAlerts) {
                if (token == alert->getToken()) return alert;
            }
            return nullptr;
        }
        list<shared_ptr<Alert>> AlertScheduler::getAllAlerts() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            auto list = std::list<shared_ptr<Alert>>(m_scheduledAlerts.begin(), m_scheduledAlerts.end());
            if (m_activeAlert) list.push_back(m_activeAlert);
            return list;
        }
        void AlertScheduler::eraseAlert(shared_ptr<Alert> alert) {
            ACSDK_DEBUG9(LX(__func__));
            if (!alert) {
                ACSDK_ERROR(LX("eraseAlertFailed").m("alert was nullptr"));
                return;
            }
            auto alertToken = alert->getToken();
            if (!m_alertStorage->erase(alert)) {
                ACSDK_ERROR(LX(__func__).m("Could not erase alert from database").d("token", alertToken));
                return;
            }
            notifyObserver(alertToken, alert->getTypeName(), AlertObserverInterface::State::DELETED);
        }
    }
}