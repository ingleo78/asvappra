#include <memory>
#include <string>
#include <gtest/gtest.h>
#include <registration_manager/CustomerDataManager.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Settings/MockSetting.h>
#include <settings/SettingsManager.h>
#include "StateReportGenerator.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace logger;
            using namespace registrationManager;
            using namespace testing;
            using namespace settings::test;
            using MockSettingManager = SettingsManager<SettingInterface<bool>, SettingInterface<int>, SettingInterface<string>>;
            enum Index { BOOL, INT, STRING };
            constexpr bool BOOL_SETTING_VALUE = true;
            constexpr int INT_SETTING_VALUE = 10;
            static const string BOOL_SETTING_STRING_VALUE = "true";
            static const string INT_SETTING_STRING_VALUE = "10";
            static const string STRING_SETTING_VALUE = "string";
            static const string STRING_SETTING_STRING_VALUE = R"("string")";
            class StateReportGeneratorTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
            protected:
                shared_ptr<MockSetting<bool>> m_mockBoolSetting;
                shared_ptr<MockSetting<int>> m_mockIntSetting;
                shared_ptr<MockSetting<string>> m_mockStringSetting;
                shared_ptr<MockSettingManager> m_mockSettingManager;
                StateReportGenerator::SettingConfigurations<MockSettingManager> m_configurations;
            };
            void StateReportGeneratorTest::SetUp() {
                m_mockBoolSetting = make_shared<MockSetting<bool>>(BOOL_SETTING_VALUE);
                m_mockIntSetting = make_shared<MockSetting<int>>(INT_SETTING_VALUE);
                m_mockStringSetting = make_shared<MockSetting<string>>(STRING_SETTING_VALUE);
                auto customerDataManager = make_shared<CustomerDataManager>();
                m_mockSettingManager = make_shared<MockSettingManager>(customerDataManager);
                SettingEventMetadata boolSettingMetadata{"test", "", "BoolSettingReport", "boolSetting"};
                SettingEventMetadata intSettingMetadata{"test", "", "IntSettingReport", "intSetting"};
                SettingEventMetadata stringSettingMetadata{"test", "", "StringSettingReport", "stringSetting"};
                get<Index::BOOL>(m_configurations).metadata.set(boolSettingMetadata);
                get<Index::INT>(m_configurations).metadata.set(intSettingMetadata);
                get<Index::STRING>(m_configurations).metadata.set(stringSettingMetadata);
            }
            void StateReportGeneratorTest::TearDown() {
                m_mockBoolSetting.reset();
                m_mockIntSetting.reset();
                m_mockStringSetting.reset();
                m_mockSettingManager.reset();
            }
            TEST_F(StateReportGeneratorTest, test_createWithoutSettingManagerShouldFail) {
                EXPECT_FALSE(StateReportGenerator::create<MockSettingManager>(nullptr, m_configurations).hasValue());
            }
            TEST_F(StateReportGeneratorTest, test_createWithEmptySettingManagerShouldSucceed) {
                EXPECT_TRUE(StateReportGenerator::create(m_mockSettingManager, m_configurations).hasValue());
            }
            TEST_F(StateReportGeneratorTest, test_createWithFullSettingManagerShouldSucceed) {
                m_mockSettingManager->addSetting<Index::BOOL>(m_mockBoolSetting);
                m_mockSettingManager->addSetting<Index::INT>(m_mockIntSetting);
                m_mockSettingManager->addSetting<Index::STRING>(m_mockStringSetting);
                EXPECT_TRUE(StateReportGenerator::create(m_mockSettingManager, m_configurations).hasValue());
            }
            TEST_F(StateReportGeneratorTest, test_generateReportWithFullSettingManager) {
                m_mockSettingManager->addSetting<Index::BOOL>(m_mockBoolSetting);
                m_mockSettingManager->addSetting<Index::INT>(m_mockIntSetting);
                m_mockSettingManager->addSetting<Index::STRING>(m_mockStringSetting);
                auto generator = StateReportGenerator::create(m_mockSettingManager, m_configurations);
                ASSERT_TRUE(generator.hasValue());
                auto report = generator.value().generateReport();
                ASSERT_EQ(report.size(), 3u);
                EXPECT_TRUE(report[0].find(STRING_SETTING_STRING_VALUE));
                EXPECT_TRUE(report[1].find(INT_SETTING_STRING_VALUE));
                EXPECT_TRUE(report[2].find(BOOL_SETTING_STRING_VALUE));
            }
            TEST_F(StateReportGeneratorTest, test_generateReportWithPartialSettingManager) {
                m_mockSettingManager->addSetting<Index::INT>(m_mockIntSetting);
                m_mockSettingManager->addSetting<Index::STRING>(m_mockStringSetting);
                auto generator = StateReportGenerator::create(m_mockSettingManager, m_configurations);
                ASSERT_TRUE(generator.hasValue());
                auto report = generator.value().generateReport();
                ASSERT_EQ(report.size(), 2u);
                EXPECT_TRUE(report[0].find(STRING_SETTING_STRING_VALUE));
                EXPECT_TRUE(report[1].find(INT_SETTING_STRING_VALUE));
            }
        }
    }
}