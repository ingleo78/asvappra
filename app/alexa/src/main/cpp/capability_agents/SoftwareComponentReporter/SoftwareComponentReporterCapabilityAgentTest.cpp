#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/rapidjson.h>
#include <avs/ComponentConfiguration.h>
#include <json/JSONUtils.h>
#include "SoftwareComponentReporterCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace softwareComponentReporter {
            namespace test {
                using namespace rapidjson;
                using namespace testing;
                using namespace configuration;
                class SoftwareComponentReporterCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    shared_ptr<SoftwareComponentReporterCapabilityAgent> m_componentReporter;
                };
                void SoftwareComponentReporterCapabilityAgentTest::SetUp() {
                    m_componentReporter = SoftwareComponentReporterCapabilityAgent::create();
                }
                void SoftwareComponentReporterCapabilityAgentTest::TearDown() {
                    m_componentReporter.reset();
                }
                TEST_F(SoftwareComponentReporterCapabilityAgentTest, test_addComponentConfiguration) {
                    shared_ptr<ComponentConfiguration> config = ComponentConfiguration::createComponentConfiguration("component", "1.0");
                    ASSERT_NE(config, nullptr);
                    EXPECT_TRUE(m_componentReporter->addConfiguration(config));
                }
                TEST_F(SoftwareComponentReporterCapabilityAgentTest, test_addMultipleComponentConfiguration) {
                    shared_ptr<ComponentConfiguration> config = ComponentConfiguration::createComponentConfiguration("component", "1.0");
                    shared_ptr<ComponentConfiguration> config2 = ComponentConfiguration::createComponentConfiguration("component2", "2.0");
                    ASSERT_NE(config, nullptr);
                    ASSERT_NE(config2, nullptr);
                    EXPECT_TRUE(m_componentReporter->addConfiguration(config));
                    EXPECT_TRUE(m_componentReporter->addConfiguration(config2));
                }
                TEST_F(SoftwareComponentReporterCapabilityAgentTest, test_addDupeComponentConfiguration) {
                    shared_ptr<ComponentConfiguration> config = ComponentConfiguration::createComponentConfiguration("component", "1.0");
                    shared_ptr<ComponentConfiguration> configDupe = ComponentConfiguration::createComponentConfiguration("component", "2.0");
                    ASSERT_NE(config, nullptr);
                    ASSERT_NE(configDupe, nullptr);
                    EXPECT_TRUE(m_componentReporter->addConfiguration(config));
                    EXPECT_FALSE(m_componentReporter->addConfiguration(configDupe));
                }
                TEST_F(SoftwareComponentReporterCapabilityAgentTest, test_configurationJsonBuilds) {
                    shared_ptr<ComponentConfiguration> config = ComponentConfiguration::createComponentConfiguration("component", "1.0");
                    ASSERT_NE(config, nullptr);
                    EXPECT_TRUE(m_componentReporter->addConfiguration(config));
                    auto configurationsSet = m_componentReporter->getCapabilityConfigurations();
                    EXPECT_EQ(configurationsSet.size(), 1u);
                    auto configuration = *configurationsSet.begin();
                    ASSERT_TRUE(configuration->additionalConfigurations.find(CAPABILITY_INTERFACE_CONFIGURATIONS_KEY) != configuration->additionalConfigurations.end());
                    string configurationJson = configuration->additionalConfigurations[CAPABILITY_INTERFACE_CONFIGURATIONS_KEY];
                    Document document;
                    ASSERT_TRUE(jsonUtils::parseJSON(configurationJson, &document));
                    vector<map<string, string>> softwareComponents;
                    rapidjson::Value _document{document.GetString(), strlen(document.GetString())};
                    EXPECT_TRUE(jsonUtils::retrieveArrayOfStringMapFromArray(_document, "softwareComponents", softwareComponents));
                    EXPECT_EQ(softwareComponents.size(), 1u);
                    auto component = softwareComponents.front();
                    EXPECT_EQ(component["name"], config->name);
                    EXPECT_EQ(component["version"], config->version);
                }
                TEST_F(SoftwareComponentReporterCapabilityAgentTest, test_configurationJsonEmpty) {
                    auto configurationsSet = m_componentReporter->getCapabilityConfigurations();
                    EXPECT_EQ(configurationsSet.size(), 1u);
                    auto configuration = *configurationsSet.begin();
                    ASSERT_TRUE(configuration->additionalConfigurations.find(CAPABILITY_INTERFACE_CONFIGURATIONS_KEY) == configuration->additionalConfigurations.end());
                }
            }
        }
    }
}