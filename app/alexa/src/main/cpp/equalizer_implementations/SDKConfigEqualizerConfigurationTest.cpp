#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <list>
#include <memory>
#include <configuration/ConfigurationNode.h>
#include "SDKConfigEqualizerConfiguration.h"

namespace alexaClientSDK {
    namespace equalizer {
        namespace test {
            using namespace testing;
            using namespace configuration;
            using JSONStream = vector<shared_ptr<istream>>;
            const string JSON_LIMITED_BANDS = R"({"bands":{"BASS":false, "MIDRANGE":false, "TREBLE":true}})";
            const string JSON_LIMITED_BANDS_ONE_MISSING = R"({"bands":{"BASS":false, "MIDRANGE":false}})";
            const string JSON_NO_BANDS_PROVIDED = R"({"bands":{}})";
            const string JSON_INVALID_BAND = R"({"bands":{"DEEPBASS":true}})";
            const string JSON_ONE_MODE_MENTIONED_ENABLED = R"({"modes":{"NIGHT": true}})";
            const string JSON_ONE_MODE_MENTIONED_DISABLED = R"({"modes":{"NIGHT": false}})";
            const string JSON_EMPTY_DEFAULT_STATE_BRANCH = R"({"defaultState":{}})";
            const string JSON_DEFAULT_MODE_SUPPORTED = R"({"modes": {"NIGHT":true}, "defaultState":{"mode":"NIGHT"}})";
            const string JSON_DEFAULT_MODE_UNSUPPORTED = R"({"defaultState":{"mode":"NIGHT"}})";
            const string JSON_DEFAULT_STATE_MISSING_BANDS = R"({"defaultState":{"bands":{"BASS": 1}}})";
            const string JSON_DEFAULT_STATE_BANDS_EMPTY_NO_BANDS_SUPPORTED = R"({"bands":{"BASS":false, "MIDRANGE":false, "TREBLE":false},
                                                                             "defaultState":{"bands":{}}})";
            class SDKConfigEqualizerConfigurationTest : public Test {
            public:
            protected:
                void TearDown() override;
            public:
                SDKConfigEqualizerConfigurationTest();
            };
            SDKConfigEqualizerConfigurationTest::SDKConfigEqualizerConfigurationTest() {}
            void SDKConfigEqualizerConfigurationTest::TearDown() {
                ConfigurationNode::uninitialize();
            }
            static ConfigurationNode generateConfigFromJSON(string json) {
                auto stream = shared_ptr<stringstream>(new stringstream(json));
                JSONStream jsonStream({stream});
                ConfigurationNode::initialize(jsonStream);
                return ConfigurationNode::getRoot();
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_providedEmptyConfig_shouldSucceed) {
                auto defaultConfig = InMemoryEqualizerConfiguration::createDefault();
                ConfigurationNode rootNode;
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_providedEmptyConfig_shouldUseDefaultConfig) {
                auto defaultConfig = InMemoryEqualizerConfiguration::createDefault();
                ConfigurationNode rootNode;
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                EXPECT_EQ(defaultConfig->getMinBandLevel(), config->getMinBandLevel());
                EXPECT_EQ(defaultConfig->getMaxBandLevel(), config->getMaxBandLevel());
                EXPECT_EQ(defaultConfig->getSupportedBands(), config->getSupportedBands());
                EXPECT_EQ(defaultConfig->getSupportedModes(), config->getSupportedModes());
                EXPECT_EQ(defaultConfig->getDefaultState().mode, config->getDefaultState().mode);
                EXPECT_EQ(defaultConfig->getDefaultState().bandLevels, config->getDefaultState().bandLevels);
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_providedLimitedBandsDefined_shouldSucceed) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_LIMITED_BANDS);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                EXPECT_FALSE(config->isBandSupported(EqualizerBand::BASS));
                EXPECT_FALSE(config->isBandSupported(EqualizerBand::MIDRANGE));
                EXPECT_TRUE(config->isBandSupported(EqualizerBand::TREBLE));
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_providedLimitedBandsOneMissing_shouldSucceed) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_LIMITED_BANDS_ONE_MISSING);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                EXPECT_FALSE(config->isBandSupported(EqualizerBand::BASS));
                EXPECT_FALSE(config->isBandSupported(EqualizerBand::MIDRANGE));
                EXPECT_EQ(config->isBandSupported(EqualizerBand::TREBLE),SDKConfigEqualizerConfiguration::BAND_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_havingEmptyBandList_shouldUseHardDefaults) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_NO_BANDS_PROVIDED);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
                EXPECT_EQ(config->isBandSupported(EqualizerBand::BASS),SDKConfigEqualizerConfiguration::BAND_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isBandSupported(EqualizerBand::MIDRANGE),SDKConfigEqualizerConfiguration::BAND_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isBandSupported(EqualizerBand::TREBLE),SDKConfigEqualizerConfiguration::BAND_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_havingOnlyInvalidBand_shouldSucceedAndSupportNone) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_INVALID_BAND);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
                EXPECT_FALSE(config->isBandSupported(EqualizerBand::BASS));
                EXPECT_FALSE(config->isBandSupported(EqualizerBand::MIDRANGE));
                EXPECT_FALSE(config->isBandSupported(EqualizerBand::TREBLE));
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_oneModeDefinedAndEnabled_shouldPutOthersToDefaults) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_ONE_MODE_MENTIONED_ENABLED);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
                EXPECT_TRUE(config->isModeSupported(EqualizerMode::NIGHT));
                EXPECT_EQ(config->isModeSupported(EqualizerMode::TV),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isModeSupported(EqualizerMode::SPORT),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isModeSupported(EqualizerMode::MUSIC),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isModeSupported(EqualizerMode::MOVIE),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_oneModeDefinedAndDisabled_shouldPutOthersToDefaults) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_ONE_MODE_MENTIONED_DISABLED);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
                EXPECT_FALSE(config->isModeSupported(EqualizerMode::NIGHT));
                EXPECT_EQ(config->isModeSupported(EqualizerMode::TV),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isModeSupported(EqualizerMode::SPORT),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isModeSupported(EqualizerMode::MUSIC),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                EXPECT_EQ(config->isModeSupported(EqualizerMode::MOVIE),SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_givenEmptyDefaultStateBranchEmpty_shouldUseHardDefaults) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_EMPTY_DEFAULT_STATE_BRANCH);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
                auto defaultConfig = InMemoryEqualizerConfiguration::createDefault();
                EXPECT_EQ(config->getDefaultState().mode, defaultConfig->getDefaultState().mode);
                EXPECT_EQ(defaultConfig->getDefaultState().bandLevels, config->getDefaultState().bandLevels);
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_givenSupportedDefaultMode_shouldSucceed) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_DEFAULT_MODE_SUPPORTED);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
                EXPECT_EQ(config->getDefaultState().mode, EqualizerMode::NIGHT);
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_givenUnsupportedDefaultMode_shouldFail) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_DEFAULT_MODE_UNSUPPORTED);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, IsNull());
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_havingNotAllBandsInDefaultState_shouldFail) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_DEFAULT_STATE_MISSING_BANDS);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, IsNull());
            }
            TEST_F(SDKConfigEqualizerConfigurationTest, test_havingNoBandsDefinedInDefaultStateWithNoBandsSupported_shouldSucceed) {
                ConfigurationNode rootNode = generateConfigFromJSON(JSON_DEFAULT_STATE_BANDS_EMPTY_NO_BANDS_SUPPORTED);
                auto config = SDKConfigEqualizerConfiguration::create(rootNode);
                ASSERT_THAT(config, NotNull());
            }
        }
    }
}