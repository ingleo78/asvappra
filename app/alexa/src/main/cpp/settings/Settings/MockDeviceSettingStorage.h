#ifndef ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKDEVICESETTINGSTORAGE_H_
#define ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKDEVICESETTINGSTORAGE_H_

#include <gmock/gmock.h>
#include <settings/Storage/DeviceSettingStorageInterface.h>

namespace alexaClientSDK {
    namespace settings {
        namespace storage {
            namespace test {
                class MockDeviceSettingStorage : public DeviceSettingStorageInterface {
                public:
                    MOCK_METHOD0(open, bool());
                    MOCK_METHOD0(close, void());
                    MOCK_METHOD3(storeSetting, bool(const std::string& key, const std::string& value, SettingStatus status));
                    MOCK_METHOD1(storeSettings, bool(const std::vector<std::tuple<std::string, std::string, SettingStatus>>& data));
                    MOCK_METHOD1(loadSetting, SettingStatusAndValue(const std::string& key));
                    MOCK_METHOD1(deleteSetting, bool(const std::string& key));
                    //MOCK_METHOD2(updateSettingStatus, bool(const std::string& key, SettingStatus status));
                };
            }
        }
    }
}
#endif