#ifndef ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTING_H_
#define ALEXA_CLIENT_SDK_SETTINGS_TEST_SETTINGS_MOCKSETTING_H_

#include <gmock/gmock.h>
#include <settings/SettingInterface.h>

namespace alexaClientSDK {
    namespace settings {
        namespace test {
            template <typename ValueT> class MockSetting : public SettingInterface<ValueT> {
            public:
                MOCK_METHOD1_T(setLocalChange, SetSettingResult(const ValueT& value));
                MOCK_METHOD1_T(setAvsChange, bool(const ValueT& value));
                MOCK_METHOD1_T(clearData, bool(const ValueT& value));
                MockSetting(const ValueT& value);
            };
            template <typename ValueT> MockSetting<ValueT>::MockSetting(const ValueT& value) : SettingInterface<ValueT>(value) {}
        }
    }
}
#endif