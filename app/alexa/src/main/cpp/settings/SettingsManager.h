#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSMANAGER_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSMANAGER_H_

#include <cstddef>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <logger/LogEntry.h>
#include <logger/LoggerUtils.h>
#include <registration_manager/CustomerDataHandler.h>
#include "SettingInterface.h"
#include "SettingStringConversion.h"

namespace alexaClientSDK {
    namespace settings {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace logger;
        template <typename... SettingsT> class SettingsManager : public registrationManager::CustomerDataHandler {
        public:
            template <size_t index> using SettingType = typename tuple_element<index, tuple<SettingsT...>>::type;
            template <size_t index> using ValueType = typename SettingType<index>::ValueType;
            template <size_t index> using ObserverType = typename SettingType<index>::ObserverType;
            template <typename SettingT> using SettingPointerType = shared_ptr<SettingT>;
            static constexpr size_t NUMBER_OF_SETTINGS{sizeof...(SettingsT)};
            SettingsManager(shared_ptr<registrationManager::CustomerDataManager> dataManager);
            virtual ~SettingsManager() = default;
            template <size_t index> SetSettingResult setValue(const ValueType<index>& value);
            template <size_t index> pair<bool, ValueType<index>> getValue(const ValueType<index>& defaultValue = ValueType<index>()) const;
            template <size_t index> string getJsonValue() const;
            template <size_t index> bool addObserver(shared_ptr<ObserverType<index>> observer);
            template <size_t index> void removeObserver(shared_ptr<ObserverType<index>> observer);
            template <size_t index> bool addSetting(shared_ptr<SettingType<index>> setting);
            template <size_t index> void removeSetting(shared_ptr<SettingType<index>> setting);
            template <size_t index> bool hasSetting();
            void clearData() override;
        private:
            template <size_t index> typename enable_if<index >= NUMBER_OF_SETTINGS, void>::type doClearData() {}
            template <size_t index> typename enable_if<index <= NUMBER_OF_SETTINGS - 1, void>::type doClearData() {
                auto& setting = get<index>(m_settings);
                if (setting) {
                    if (!setting->clearData(setting->getDefault())) {
                        acsdkError(LogEntry("SettingManager", "clearDataFailed").d("reason", "invalidSetting").d("settingIndex", index));
                    }
                } else {
                    acsdkDebug0(LogEntry("SettingManager", __func__).d("reason", "invalidSetting").d("settingIndex", index));
                }
                doClearData<index + 1>();
            }
            SettingsManager(const SettingsManager&) = delete;
            SettingsManager& operator=(const SettingsManager&) = delete;
            mutable mutex m_mutex;
            tuple<SettingPointerType<SettingsT>...> m_settings;
        };
        template <typename... SettingsT> SettingsManager<SettingsT...>::SettingsManager(shared_ptr<registrationManager::CustomerDataManager> dataManager) :
                                                                      CustomerDataHandler{dataManager} {}
        template <typename... SettingsT>template <size_t index>
        pair<bool, typename SettingsManager<SettingsT...>::template ValueType<index>> SettingsManager<SettingsT...>::getValue(const ValueType<index>& defaultValue) const {
            lock_guard<mutex> lock{m_mutex};
            auto& setting = get<index>(m_settings);
            if (setting) return {true, setting->get()};
            acsdkError(LogEntry("SettingManager", "getValueFailed").d("reason", "invalidSetting").d("settingIndex", index));
            return {false, defaultValue};
        }
        template <typename... SettingsT>template <size_t index> string SettingsManager<SettingsT...>::getJsonValue() const {
            lock_guard<mutex> lock{m_mutex};
            auto& setting = get<index>(m_settings);
            if (setting) {
                auto result = toSettingString<ValueType<index>>(setting->get());
                if (!result.first) {
                    acsdkError(LogEntry("SettingManager", "getStringValueFailed").d("reason", "toSettingStringFailed").d("settingIndex", index));
                    return std::string();
                }
                return result.second;
            }
            acsdkDebug0(LogEntry("SettingManager", __func__).d("result", "noSettingAvailable").d("settingIndex", index));
            return std::string();
        }
        template <typename... SettingsT>template <size_t index> SetSettingResult SettingsManager<SettingsT...>::setValue(const ValueType<index>& value) {
            lock_guard<mutex> lock{m_mutex};
            auto& setting = get<index>(m_settings);
            if (setting) return setting->setLocalChange(value);
            acsdkError(LogEntry("SettingManager", "setValueFailed").d("reason", "invalidSetting").d("settingIndex", index));
            return SetSettingResult::UNAVAILABLE_SETTING;
        }
        template <typename... SettingsT>template <size_t index> bool SettingsManager<SettingsT...>::addObserver(std::shared_ptr<ObserverType<index>> observer) {
            lock_guard<mutex> lock{m_mutex};
            auto& setting = get<index>(m_settings);
            if (setting && observer) return setting->addObserver(observer);
            acsdkError(LogEntry("SettingManager", "addObserverFailed").d("reason", "invalidSetting").d("settingIndex", index));
            return false;
        }
        template <typename... SettingsT>template <size_t index> void SettingsManager<SettingsT...>::removeObserver(std::shared_ptr<ObserverType<index>> observer) {
            lock_guard<mutex> lock{m_mutex};
            auto& setting = get<index>(m_settings);
            if (setting && observer) {
                setting->removeObserver(observer);
                return;
            }
            acsdkError(LogEntry("SettingManager", "removeObserverFailed").d("reason", "invalidSetting").d("settingIndex", index));
        }
        template <typename... SettingsT>template <size_t index> bool SettingsManager<SettingsT...>::addSetting(std::shared_ptr<SettingType<index>> newSetting) {
            lock_guard<mutex> lock{m_mutex};
            auto& setting = get<index>(m_settings);
            if (!setting && newSetting) {
                setting.swap(newSetting);
                return true;
            }
            acsdkError(LogEntry("SettingManager", "addSettingFailed").d("reason", "invalidSetting").d("settingIndex", index));
            return false;
        }
        template <typename... SettingsT>template <size_t index> void SettingsManager<SettingsT...>::removeSetting(shared_ptr<SettingType<index>> oldSetting) {
            lock_guard<mutex> lock{m_mutex};
            auto& setting = get<index>(m_settings);
            if (setting == oldSetting) setting.reset();
            else {
                acsdkError(LogEntry("SettingManager", "removeSettingFailed").d("reason", "invalidSetting").d("settingIndex", index));
            }
        }
        template <typename... SettingsT>template <size_t index> bool SettingsManager<SettingsT...>::hasSetting() {
            lock_guard<mutex> lock{m_mutex};
            return std::get<index>(m_settings);
        }
        template <typename... SettingsT> void SettingsManager<SettingsT...>::clearData() {
            lock_guard<mutex> lock{m_mutex};
            doClearData<0>();
        }
    }
}
#endif