#ifndef ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGEVENTSENDER_H_
#define ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTINGEVENTSENDER_H_

#include <future>
#include <string>
#include <gmock/gmock.h>
#include <settings/SettingEventSenderInterface.h>

namespace alexaClientSDK {
    namespace settings {
        namespace test {
            class MockSettingEventSender : public SettingEventSenderInterface {
            public:
                MOCK_METHOD1(sendChangedEvent, std::shared_future<bool>(const std::string& value));
                MOCK_METHOD1(sendReportEvent, std::shared_future<bool>(const std::string& value));
                MOCK_METHOD1(sendStateReportEvent, std::shared_future<bool>(const std::string& payload));
                MOCK_METHOD0(cancel, void());
            };
        }
    }
}
#endif