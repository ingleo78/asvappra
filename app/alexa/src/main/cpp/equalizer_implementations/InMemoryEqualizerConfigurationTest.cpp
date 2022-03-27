#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <list>
#include <memory>
#include "InMemoryEqualizerConfiguration.h"

namespace alexaClientSDK {
    namespace equalizer {
        namespace test {
            using namespace testing;
            static constexpr int MIN_LEVEL = -6;
            static constexpr int MAX_LEVEL = 6;
            static constexpr int DEFAULT_ADJUST_DELTA = 1;
            static constexpr int BELOW_MIN_LEVEL = MIN_LEVEL - 100;
            static constexpr int ABOVE_MAX_LEVEL = MAX_LEVEL + 100;
            static constexpr int DEFAULT_LEVEL = 0;
            class InMemoryEqualizerConfigurationTest : public Test {
            public:
            protected:
                shared_ptr<InMemoryEqualizerConfiguration> m_configuration;
                set<EqualizerBand> getDefaultBands();
                set<EqualizerMode> getDefaultModes();
                EqualizerState getDefaultState();
            };
            set<EqualizerBand> InMemoryEqualizerConfigurationTest::getDefaultBands() {
                return set<EqualizerBand>({EqualizerBand::BASS, EqualizerBand::MIDRANGE, EqualizerBand::TREBLE});
            }
            set<EqualizerMode> InMemoryEqualizerConfigurationTest::getDefaultModes() {
                return set<EqualizerMode>({EqualizerMode::MOVIE, EqualizerMode::MUSIC, EqualizerMode::NIGHT, EqualizerMode::SPORT, EqualizerMode::TV});
            }
            EqualizerState InMemoryEqualizerConfigurationTest::getDefaultState() {
                return EqualizerState{EqualizerMode::NONE,EqualizerBandLevelMap({{EqualizerBand::BASS, DEFAULT_LEVEL},
                                      {EqualizerBand::MIDRANGE, DEFAULT_LEVEL}, {EqualizerBand::TREBLE, DEFAULT_LEVEL}})};
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedValidParameters_createInstance_shouldSucceed) {
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA,
                                                                         getDefaultBands(), getDefaultModes(),
                                                                         getDefaultState());
                ASSERT_THAT(m_configuration, NotNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedInvalidLevelRange_createInstance_shouldFail) {
                m_configuration = InMemoryEqualizerConfiguration::create(MAX_LEVEL, MIN_LEVEL, DEFAULT_ADJUST_DELTA,
                                                                         getDefaultBands(), getDefaultModes(),
                                                                         getDefaultState());
                ASSERT_THAT(m_configuration, IsNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedInvalidBandDelta_createInstance_shouldFail) {
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, 0, getDefaultBands(),
                                                                         getDefaultModes(), getDefaultState());
                ASSERT_THAT(m_configuration, IsNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedMixMaxLevelSetToDefault_createInstance_shouldSucceed) {
                m_configuration = InMemoryEqualizerConfiguration::create(DEFAULT_LEVEL, DEFAULT_LEVEL, DEFAULT_ADJUST_DELTA,
                                                                         getDefaultBands(), getDefaultModes(),
                                                                         getDefaultState());
                ASSERT_THAT(m_configuration, NotNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedSameNonDefaultMixMaxLevel_createInstance_shouldSucceed) {
                m_configuration = InMemoryEqualizerConfiguration::create(MAX_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA,
                                                                         getDefaultBands(), getDefaultModes(),
                                                                         getDefaultState());
                ASSERT_THAT(m_configuration, IsNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedDefaultStateLevelAboveMax_createInstance_shouldFail) {
                auto state = getDefaultState();
                state.bandLevels[EqualizerBand::TREBLE] = ABOVE_MAX_LEVEL;
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA,
                                                                         getDefaultBands(), getDefaultModes(),
                                                                         state);
                ASSERT_THAT(m_configuration, IsNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedDefaultStateLevelBelowMin_createInstance_shouldFail) {
                auto state = getDefaultState();
                state.bandLevels[EqualizerBand::TREBLE] = BELOW_MIN_LEVEL;
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA,
                                                                         getDefaultBands(), getDefaultModes(),
                                                                         state);
                ASSERT_THAT(m_configuration, IsNull());
            }
            TEST_F(
                InMemoryEqualizerConfigurationTest,
                test_providedDefaultStateLevelBelowMinDifferentBand_createInstance_shouldFail) {
                auto state = getDefaultState();
                state.bandLevels[EqualizerBand::BASS] = BELOW_MIN_LEVEL;
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA,
                                                                         getDefaultBands(), getDefaultModes(),
                                                                         state);
                ASSERT_THAT(m_configuration, IsNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedNoModes_createInstance_shouldSucceed) {
                auto bands = set<EqualizerBand>({EqualizerBand::MIDRANGE});
                auto state = EqualizerState{EqualizerMode::NONE, EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, DEFAULT_LEVEL}})};
                auto modes = set<EqualizerMode>({});
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA, bands, modes, state);
                ASSERT_THAT(m_configuration, NotNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedSupportedModeInDefaultState_createInstance_shouldSucceed) {
                auto bands = std::set<EqualizerBand>({EqualizerBand::MIDRANGE});
                auto state = EqualizerState{EqualizerMode::NIGHT, EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, DEFAULT_LEVEL}})};
                auto modes = std::set<EqualizerMode>({EqualizerMode::NIGHT});
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA, bands, modes, state);
                ASSERT_THAT(m_configuration, NotNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedUnsupportedModeInDefaultState_createInstance_shouldFail) {
                auto bands = set<EqualizerBand>({EqualizerBand::MIDRANGE});
                auto state = EqualizerState{EqualizerMode::TV, EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, DEFAULT_LEVEL}})};
                auto modes = set<EqualizerMode>({EqualizerMode::NIGHT});
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA, bands, modes, state);
                ASSERT_THAT(m_configuration, IsNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedNONEModeAsSupported_createInstance_shouldSucceed) {
                auto bands = set<EqualizerBand>({EqualizerBand::MIDRANGE});
                auto state = EqualizerState{EqualizerMode::NONE, EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, DEFAULT_LEVEL}})};
                auto modes = set<EqualizerMode>({EqualizerMode::NIGHT, EqualizerMode::NONE});
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA, bands, modes, state);
                ASSERT_THAT(m_configuration, NotNull());
            }
            TEST_F(InMemoryEqualizerConfigurationTest, test_providedValidConfiguration_isSupportedMethods_shouldSucceed) {
                auto bands = set<EqualizerBand>({EqualizerBand::MIDRANGE});
                auto state = EqualizerState{EqualizerMode::NONE, EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, DEFAULT_LEVEL}})};
                auto modes = set<EqualizerMode>({EqualizerMode::NIGHT, EqualizerMode::TV});
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA, bands, modes, state);
                ASSERT_THAT(m_configuration, NotNull());
                EXPECT_TRUE(m_configuration->isBandSupported(EqualizerBand::MIDRANGE));
                EXPECT_FALSE(m_configuration->isBandSupported(EqualizerBand::TREBLE));
                EXPECT_FALSE(m_configuration->isModeSupported(EqualizerMode::NONE));
                EXPECT_TRUE(m_configuration->isModeSupported(EqualizerMode::NIGHT));
                EXPECT_TRUE(m_configuration->isModeSupported(EqualizerMode::TV));
                EXPECT_FALSE(m_configuration->isModeSupported(EqualizerMode::SPORT));
            }
        }
    }
}