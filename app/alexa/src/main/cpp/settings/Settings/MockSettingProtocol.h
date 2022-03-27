#ifndef ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGPROTOCOL_H_
#define ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGPROTOCOL_H_

#include <functional>
#include <string>
#include <gmock/gmock.h>
#include "../SettingProtocolInterface.h"

namespace alexaClientSDK {
    namespace settings {
        namespace test {
            class MockSettingProtocol : public SettingProtocolInterface {
            public:
                MockSettingProtocol(const std::string& initialValue, bool applyChange, bool revertChange);
                SetSettingResult localChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange,
                                             SettingNotificationFunction notifyObservers) override;
                bool avsChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange, SettingNotificationFunction notifyObservers) override;
                bool restoreValue(ApplyDbChangeFunction applyChange, SettingNotificationFunction notifyObservers) override;
                bool clearData() override;
                bool isDataCleared();
            private:
                std::string m_initialValue;
                bool m_applyChange;
                bool m_revertChange;
                bool m_isClearedData;
            };
            MockSettingProtocol::MockSettingProtocol(const std::string& initialValue, bool applyChange, bool revertChange) : m_initialValue{initialValue},
                                                     m_applyChange{applyChange}, m_revertChange{revertChange}, m_isClearedData{false} {}
            SetSettingResult MockSettingProtocol::localChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange,
                                                              SettingNotificationFunction notifyObservers) {
                if (m_applyChange) applyChange();
                if (m_revertChange) revertChange();
                notifyObservers(SettingNotifications::LOCAL_CHANGE);
                return SetSettingResult::ENQUEUED;
            }
            bool MockSettingProtocol::avsChange(ApplyChangeFunction applyChange, RevertChangeFunction revertChange,
                                                SettingNotificationFunction notifyObservers) {
                if (m_applyChange) applyChange();
                if (m_revertChange) revertChange();
                notifyObservers(SettingNotifications::AVS_CHANGE);
                return true;
            }
            bool MockSettingProtocol::restoreValue(ApplyDbChangeFunction applyChange, SettingNotificationFunction notifyObservers) {
                if (m_applyChange) applyChange(m_initialValue);
                return true;
            }
            bool MockSettingProtocol::clearData() {
                m_isClearedData = true;
                return true;
            }
            bool MockSettingProtocol::isDataCleared() {
                return m_isClearedData;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGPROTOCOL_H_
