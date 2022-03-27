#include <functional>
#include <logger/Logger.h>
#include "DNDSettingProtocol.h"

static const std::string TAG("DNDSettingProtocol");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace doNotDisturb {
            static const std::string INVALID_VALUE = "";
            unique_ptr<DNDSettingProtocol> DNDSettingProtocol::create(
                const SettingEventMetadata& metadata,
                shared_ptr<SettingEventSenderInterface> eventSender,
                shared_ptr<DeviceSettingStorageInterface> settingStorage) {
                ACSDK_DEBUG5(LX(__func__).d("settingName", metadata.settingName));
                if (!eventSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullEventSender"));
                    return nullptr;
                }
                if (!settingStorage) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullSettingStorage"));
                    return nullptr;
                }
                string settingKey = metadata.eventNamespace + "::" + metadata.settingName;
                return unique_ptr<DNDSettingProtocol>(new DNDSettingProtocol(settingKey, eventSender, settingStorage));
            }
            SetSettingResult DNDSettingProtocol::localChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange,
                                                             SettingNotificationFunction notifyObservers) {
                ACSDK_DEBUG5(LX(__func__).d("setting", m_key));
                if (!applyChange || !revertChange || !notifyObservers) {
                    ACSDK_ERROR(LX("avsChangeFailed").d("reason", "invalidCallback").d("invalidApply", !applyChange).d("invalidRevert", !revertChange)
                        .d("invalidNotify", !notifyObservers));
                    return SetSettingResult::INTERNAL_ERROR;
                }
                m_executor.submit([this, applyChange, revertChange, notifyObservers]() {
                    notifyObservers(SettingNotifications::LOCAL_CHANGE_IN_PROGRESS);
                    bool ok = false;
                    string value;
                    tie(ok, value) = applyChange();
                    if (!ok) {
                        ACSDK_ERROR(LX("localChangeFailed").d("reason", "cannotApplyChange"));
                        notifyObservers(SettingNotifications::LOCAL_CHANGE_FAILED);
                        return;
                    }
                    if (!this->m_storage->storeSetting(m_key, value, SettingStatus::LOCAL_CHANGE_IN_PROGRESS)) {
                        ACSDK_ERROR(LX("localChangeFailed").d("reason", "cannotUpdateDatabase"));
                        revertChange();
                        notifyObservers(SettingNotifications::LOCAL_CHANGE_FAILED);
                        return;
                    }
                    notifyObservers(SettingNotifications::LOCAL_CHANGE);
                    this->m_eventSender->sendChangedEvent(value).get();
                    if (!this->m_storage->storeSetting(m_key, value, SettingStatus::SYNCHRONIZED)) {
                        ACSDK_ERROR(LX("localChangeFailed").d("reason", "cannotUpdateStatus"));
                    }
                });
                return SetSettingResult::ENQUEUED;
            }
            bool DNDSettingProtocol::avsChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) {
                ACSDK_DEBUG5(LX(__func__).d("setting", m_key));
                if (!applyChange || !revertChange || !notifyObservers) {
                    ACSDK_ERROR(LX("avsChangeFailed").d("reason", "invalidCallback").d("invalidApply", !applyChange).d("invalidRevert", !revertChange)
                        .d("invalidNotify", !notifyObservers));
                    return false;
                }
                promise<bool> requestSaved;
                auto future = requestSaved.get_future();
                m_executor.submit([this, applyChange, revertChange, notifyObservers, &requestSaved]() {
                    if (!m_storage->updateSettingStatus(m_key, SettingStatus::AVS_CHANGE_IN_PROGRESS)) {
                        requestSaved.set_value(false);
                        return;
                    }
                    requestSaved.set_value(true);
                    notifyObservers(SettingNotifications::AVS_CHANGE_IN_PROGRESS);
                    bool ok = false;
                    string value;
                    tie(ok, value) = applyChange();
                    if (!ok) {
                        ACSDK_ERROR(LX("avsChangeFailed").d("reason", "cannotApplyChange"));
                        notifyObservers(SettingNotifications::AVS_CHANGE_FAILED);
                    } else if (!this->m_storage->storeSetting(m_key, value, SettingStatus::AVS_CHANGE_IN_PROGRESS)) {
                        ACSDK_ERROR(LX("avsChangeFailed").d("reason", "cannotUpdateDatabaseValue"));
                        notifyObservers(SettingNotifications::AVS_CHANGE_FAILED);
                        value = revertChange();
                    } else notifyObservers(SettingNotifications::AVS_CHANGE);
                    this->m_eventSender->sendReportEvent(value);
                    if (!this->m_storage->updateSettingStatus(m_key, SettingStatus::SYNCHRONIZED)) {
                        ACSDK_ERROR(LX("avsChangeFailed").d("reason", "cannotUpdateStatus"));
                    }
                });
                return future.get();
            }
            bool DNDSettingProtocol::restoreValue(ApplyDbChangeFunction applyChange, SettingNotificationFunction notifyObservers) {
                ACSDK_DEBUG5(LX(__func__).d("setting", m_key));
                if (!applyChange || !notifyObservers) {
                    ACSDK_ERROR(LX("avsChangeFailed").d("reason", "invalidCallback").d("invalidApply", !applyChange).d("invalidNotify", !notifyObservers));
                    return false;
                }
                string valueOrErrorStr;
                string valueStr;
                SettingStatus status = SettingStatus::NOT_AVAILABLE;
                tie(status, valueOrErrorStr) = m_storage->loadSetting(m_key);
                if (SettingStatus::NOT_AVAILABLE != status) valueStr = valueOrErrorStr;
                auto applyStrChange = [valueStr, applyChange] { return applyChange(valueStr); };
                auto revertChange = [applyChange] { return applyChange(INVALID_VALUE).second; };
                switch (status) {
                    case SettingStatus::NOT_AVAILABLE: case SettingStatus::LOCAL_CHANGE_IN_PROGRESS:
                        return localChange(applyStrChange, revertChange, notifyObservers) == SetSettingResult::ENQUEUED;
                    case SettingStatus::AVS_CHANGE_IN_PROGRESS: return avsChange(applyStrChange, revertChange, notifyObservers);
                    case SettingStatus::SYNCHRONIZED: return applyChange(valueStr).first;
                }
                return false;
            }
            bool DNDSettingProtocol::clearData() {
                ACSDK_DEBUG5(LX(__func__).d("setting", m_key));
                return m_storage->deleteSetting(m_key);
            }
            DNDSettingProtocol::DNDSettingProtocol(const string& key, shared_ptr<SettingEventSenderInterface> eventSender,
                                                   shared_ptr<storage::DeviceSettingStorageInterface> settingStorage) : m_key{key}, m_eventSender{eventSender},
                                                   m_storage{settingStorage} {}
        }
    }
}