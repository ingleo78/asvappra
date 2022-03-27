#ifndef ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGOBSERVER_H_
#define ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGOBSERVER_H_

#include <functional>
#include <memory>
#include <gmock/gmock.h>
#include "../SettingObserverInterface.h"

namespace alexaClientSDK {
    namespace settings {
        namespace test {
            template <typename SettingT> class MockSettingObserver : public SettingObserverInterface<SettingT> {
            public:
                MOCK_METHOD2_T(onSettingNotification, void(const typename SettingT::ValueType& value, SettingNotifications notification));
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGOBSERVER_H_
