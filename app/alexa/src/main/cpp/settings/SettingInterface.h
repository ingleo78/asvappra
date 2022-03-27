#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGINTERFACE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGINTERFACE_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <util/GuardedValue.h>
#include <logger/LogEntry.h>
#include <logger/LoggerUtils.h>
#include "SetSettingResult.h"
#include "SettingObserverInterface.h"

namespace alexaClientSDK {
    namespace settings {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace logger;
        template <typename ValueT> class SettingInterface {
        public:
            using ValueType = ValueT;
            using ObserverType = SettingObserverInterface<SettingInterface<ValueType>>;
            virtual ~SettingInterface() = default;
            virtual SetSettingResult setLocalChange(const ValueType& value);
            virtual bool setAvsChange(const ValueType& value);
            virtual bool clearData(const ValueType& value);
            inline ValueType get() const;
            inline ValueType getDefault() const;
            inline bool addObserver(shared_ptr<ObserverType> observer);
            inline void removeObserver(shared_ptr<ObserverType>& observer);
        protected:
            SettingInterface(const ValueType& value);
            void notifyObservers(SettingNotifications notification);
            mutex m_observerMutex;
            unordered_set<shared_ptr<ObserverType>> m_observers;
            using GuardedValue = GuardedValue<ValueType>;
            typename conditional<is_scalar<ValueType>::value, atomic<ValueType>, GuardedValue>::type m_value;
            const ValueType m_defaultValue;
        };
        template <typename ValueT> ValueT SettingInterface<ValueT>::get() const {
            return m_value;
        }
        template <typename ValueT> ValueT SettingInterface<ValueT>::getDefault() const {
            return m_defaultValue;
        }
        template <typename ValueT> bool SettingInterface<ValueT>::addObserver(shared_ptr<ObserverType> observer) {
            if (observer) {
                lock_guard<std::mutex>{m_observerMutex};
                m_observers.insert(observer);
                return true;
            }
            acsdkError(LogEntry("SettingInterface", "addObserverFailed").d("reason", "nullObserver"));
            return false;
        }
        template <typename ValueT> void SettingInterface<ValueT>::notifyObservers(SettingNotifications notification) {
            lock_guard<mutex>{m_observerMutex};
            for (auto& observer : m_observers) {
                ValueT value = m_value;
                observer->onSettingNotification(value, notification);
            }
        }
        template <typename ValueT> void SettingInterface<ValueT>::removeObserver(shared_ptr<ObserverType>& observer) {
            lock_guard<mutex>{m_observerMutex};
            m_observers.erase(observer);
        }
        template <typename ValueT> SettingInterface<ValueT>::SettingInterface(const ValueT& value) : m_value(value), m_defaultValue{value} {}
    }
}
#endif