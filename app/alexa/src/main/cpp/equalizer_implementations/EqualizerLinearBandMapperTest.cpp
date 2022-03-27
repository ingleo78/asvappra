#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <functional>
#include <memory>
#include <sdkinterfaces/Audio/EqualizerTypes.h>
#include "EqualizerLinearBandMapper.h"

namespace alexaClientSDK {
    namespace equalizer {
        namespace test {
            using namespace testing;
            static constexpr int VALID_NUMBER_OF_BANDS = 3;
            static constexpr int INVALID_NUMBER_OF_BANDS_BELOW = 0;
            static constexpr int INVALID_NUMBER_OF_BANDS_ABOVE = 999999;
            static constexpr int BAND_LEVEL_TOP = 10;
            static constexpr int BAND_LEVEL_BOTTOM = -10;
            static constexpr int BAND_LEVEL_ZERO = 0;
            static constexpr int BAND_LEVEL_AVERAGE = (BAND_LEVEL_TOP + BAND_LEVEL_BOTTOM + BAND_LEVEL_ZERO) / 3;
            static constexpr int CURRENT_NUMBER_OF_AVS_BANDS = 3;
            class EqualizerLinearBandMapperTest : public Test {
            protected:
                void testAllValuesEqual(int numberOfBands, int value);
            };
            void EqualizerLinearBandMapperTest::testAllValuesEqual(int numberOfBands, int value) {
                auto bandMapper = EqualizerLinearBandMapper::create(numberOfBands);
                ASSERT_THAT(bandMapper, NotNull());
                EqualizerBandLevelMap bandLevelMap;
                for (auto band : EqualizerBandValues) {
                    bandLevelMap[band] = value;
                }
                function<void(int, int)> callback = [value](int index, int level) { EXPECT_EQ(level, value); };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenValidParameters_create_shouldSucceed) {
                auto bandMapper = EqualizerLinearBandMapper::create(VALID_NUMBER_OF_BANDS);
                EXPECT_THAT(bandMapper, NotNull());
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenInvalidParameters_create_shouldFail) {
                auto bandMapper = EqualizerLinearBandMapper::create(INVALID_NUMBER_OF_BANDS_BELOW);
                EXPECT_THAT(bandMapper, IsNull());
                bandMapper = EqualizerLinearBandMapper::create(INVALID_NUMBER_OF_BANDS_ABOVE);
                EXPECT_THAT(bandMapper, IsNull());
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenSameAsAVSBands_map_shouldNotModifyLevels) {
                auto numberOfAVSBands = EqualizerBandValues.size();
                auto bandMapper = EqualizerLinearBandMapper::create(numberOfAVSBands);
                ASSERT_THAT(bandMapper, NotNull());
                EqualizerBandLevelMap bandLevelMap;
                int index = 0;
                for (auto band : EqualizerBandValues) {
                    bandLevelMap[band] = index++;
                }
                function<void(int, int)> callback = [](int index, int level) { EXPECT_EQ(level, index); };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenMoreBandsToMapTo_mapAllSameNonZero_shouldMapToSame) {
                int numberOfAVSBands = EqualizerBandValues.size();
                testAllValuesEqual(numberOfAVSBands + 1, BAND_LEVEL_TOP);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenMoreBandsToMapTo_mapAllSameZero_shouldMapToSame) {
                int numberOfAVSBands = EqualizerBandValues.size();
                testAllValuesEqual(numberOfAVSBands + 1, BAND_LEVEL_ZERO);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenDoubleBandsToMapTo_mapAllSameNonZero_shouldMapToSame) {
                int numberOfAVSBands = EqualizerBandValues.size();
                testAllValuesEqual(numberOfAVSBands * 2, BAND_LEVEL_TOP);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenDoubleBandsToMapTo_mapAllSameZero_shouldMapToSame) {
                int numberOfAVSBands = EqualizerBandValues.size();
                testAllValuesEqual(numberOfAVSBands * 2, BAND_LEVEL_ZERO);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenLessBandsToMapTo_mapAllSameNonZero_shouldMapToSame) {
                int numberOfAVSBands = EqualizerBandValues.size();
                testAllValuesEqual(numberOfAVSBands - 1, BAND_LEVEL_TOP);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenLessBandsToMapTo_mapAllSameZero_shouldMapToSame) {
                int numberOfAVSBands = EqualizerBandValues.size();
                testAllValuesEqual(numberOfAVSBands - 1, BAND_LEVEL_ZERO);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenOneBandToMapTo_mapNonZero_shouldMapToSame) {
                testAllValuesEqual(1, BAND_LEVEL_TOP);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenOneBandToMapTo_mapZero_shouldMapToSame) {
                testAllValuesEqual(1, BAND_LEVEL_ZERO);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenAnyNumberOfBands_map_shouldKeepAverageLevel) {
                int numberOfAVSBands = EqualizerBandValues.size();
                for (int targetBandsCount = 1; targetBandsCount < numberOfAVSBands * 10; targetBandsCount++) {
                    auto bandMapper = EqualizerLinearBandMapper::create(targetBandsCount);
                    ASSERT_THAT(bandMapper, NotNull());
                    EqualizerBandLevelMap bandLevelMap = {
                        {EqualizerBand::BASS, BAND_LEVEL_BOTTOM},
                        {EqualizerBand::MIDRANGE, BAND_LEVEL_ZERO},
                        {EqualizerBand::TREBLE, BAND_LEVEL_TOP},
                    };
                    int targetLevelAccumulator = 0;
                    function<void(int, int)> callback = [&targetLevelAccumulator](int index, int level) {
                        targetLevelAccumulator += level;
                    };
                    bandMapper->mapEqualizerBands(bandLevelMap, callback);
                    EXPECT_EQ(targetLevelAccumulator / targetBandsCount, BAND_LEVEL_AVERAGE);
                }
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenMoreBandsToMapTo_mapSpecificValues_shouldMapToSpecificOutput) {
                int numberOfAVSBands = EqualizerBandValues.size();
                ASSERT_EQ(numberOfAVSBands, CURRENT_NUMBER_OF_AVS_BANDS);
                EqualizerBandLevelMap bandLevelMap = {
                    {EqualizerBand::BASS, BAND_LEVEL_BOTTOM},
                    {EqualizerBand::MIDRANGE, BAND_LEVEL_ZERO},
                    {EqualizerBand::TREBLE, BAND_LEVEL_TOP},
                };
                array<int, CURRENT_NUMBER_OF_AVS_BANDS + 1> mappedValues{};
                auto bandMapper = EqualizerLinearBandMapper::create(CURRENT_NUMBER_OF_AVS_BANDS + 1);
                function<void(int, int)> callback = [&mappedValues](int index, int level) { mappedValues[index] = level; };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
                EXPECT_EQ(mappedValues[0], BAND_LEVEL_BOTTOM);
                EXPECT_EQ(mappedValues[1], (BAND_LEVEL_BOTTOM + BAND_LEVEL_ZERO) / 2);
                EXPECT_EQ(mappedValues[2], (BAND_LEVEL_ZERO + BAND_LEVEL_TOP) / 2);
                EXPECT_EQ(mappedValues[3], BAND_LEVEL_TOP);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenEvenMoreBandsToMapTo_mapSpecificValues_shouldMapToSpecificOutput) {
                int numberOfAVSBands = EqualizerBandValues.size();
                ASSERT_EQ(numberOfAVSBands, CURRENT_NUMBER_OF_AVS_BANDS);
                EqualizerBandLevelMap bandLevelMap = {
                    {EqualizerBand::BASS, BAND_LEVEL_BOTTOM},
                    {EqualizerBand::MIDRANGE, BAND_LEVEL_ZERO},
                    {EqualizerBand::TREBLE, BAND_LEVEL_TOP},
                };
                array<int, CURRENT_NUMBER_OF_AVS_BANDS + 2> mappedValues{};
                auto bandMapper = EqualizerLinearBandMapper::create(CURRENT_NUMBER_OF_AVS_BANDS + 2);
                function<void(int, int)> callback = [&mappedValues](int index, int level) { mappedValues[index] = level; };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
                EXPECT_EQ(mappedValues[0], BAND_LEVEL_BOTTOM);
                EXPECT_EQ(mappedValues[1], (BAND_LEVEL_BOTTOM + BAND_LEVEL_ZERO) / 2);
                EXPECT_EQ(mappedValues[2], BAND_LEVEL_ZERO);
                EXPECT_EQ(mappedValues[3], (BAND_LEVEL_ZERO + BAND_LEVEL_TOP) / 2);
                EXPECT_EQ(mappedValues[4], BAND_LEVEL_TOP);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenLessBandsToMapTo_mapSpecificValues_shouldMapToSpecificOutput) {
                int numberOfAVSBands = EqualizerBandValues.size();
                ASSERT_EQ(numberOfAVSBands, CURRENT_NUMBER_OF_AVS_BANDS);
                EqualizerBandLevelMap bandLevelMap = {
                    {EqualizerBand::BASS, BAND_LEVEL_BOTTOM},
                    {EqualizerBand::MIDRANGE, BAND_LEVEL_ZERO},
                    {EqualizerBand::TREBLE, BAND_LEVEL_TOP},
                };
                array<int, CURRENT_NUMBER_OF_AVS_BANDS - 1> mappedValues{};
                auto bandMapper = EqualizerLinearBandMapper::create(CURRENT_NUMBER_OF_AVS_BANDS - 1);
                function<void(int, int)> callback = [&mappedValues](int index, int level) { mappedValues[index] = level; };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
                EXPECT_EQ(mappedValues[0], (BAND_LEVEL_BOTTOM + BAND_LEVEL_ZERO) / 2);
                EXPECT_EQ(mappedValues[1], (BAND_LEVEL_TOP + BAND_LEVEL_ZERO) / 2);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenEvenLessBandsToMapTo_mapSpecificValues_shouldMapToSpecificOutput) {
                int numberOfAVSBands = EqualizerBandValues.size();
                ASSERT_EQ(numberOfAVSBands, CURRENT_NUMBER_OF_AVS_BANDS);
                EqualizerBandLevelMap bandLevelMap = {
                    {EqualizerBand::BASS, BAND_LEVEL_BOTTOM},
                    {EqualizerBand::MIDRANGE, BAND_LEVEL_ZERO},
                    {EqualizerBand::TREBLE, BAND_LEVEL_TOP},
                };
                array<int, CURRENT_NUMBER_OF_AVS_BANDS - 2> mappedValues{};
                auto bandMapper = EqualizerLinearBandMapper::create(CURRENT_NUMBER_OF_AVS_BANDS - 2);
                function<void(int, int)> callback = [&mappedValues](int index, int level) { mappedValues[index] = level; };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
                EXPECT_EQ(mappedValues[0], (BAND_LEVEL_BOTTOM + BAND_LEVEL_ZERO + BAND_LEVEL_TOP) / 3);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenOneBandSupported_mapToOneBand_shouldMapSameValue) {
                EqualizerBandLevelMap bandLevelMap = {{EqualizerBand::BASS, BAND_LEVEL_BOTTOM}};
                std::array<int, 1> mappedValues{};
                auto bandMapper = EqualizerLinearBandMapper::create(1);
                std::function<void(int, int)> callback = [&mappedValues](int index, int level) { mappedValues[index] = level; };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
                EXPECT_EQ(mappedValues[0], BAND_LEVEL_BOTTOM);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenOneBandSupported_mapToTwoBands_shouldMapToBothSame) {
                EqualizerBandLevelMap bandLevelMap = {{EqualizerBand::BASS, BAND_LEVEL_BOTTOM}};
                array<int, 2> mappedValues{};
                auto bandMapper = EqualizerLinearBandMapper::create(2);
                function<void(int, int)> callback = [&mappedValues](int index, int level) { mappedValues[index] = level; };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
                EXPECT_EQ(mappedValues[0], BAND_LEVEL_BOTTOM);
                EXPECT_EQ(mappedValues[1], BAND_LEVEL_BOTTOM);
            }
            TEST_F(EqualizerLinearBandMapperTest, test_givenTwoBandsSupported_mapToOneBands_shouldMapToAverage) {
                EqualizerBandLevelMap bandLevelMap = {{EqualizerBand::BASS, BAND_LEVEL_BOTTOM}, {EqualizerBand::MIDRANGE, BAND_LEVEL_ZERO}};
                array<int, 1> mappedValues{};
                auto bandMapper = EqualizerLinearBandMapper::create(1);
                function<void(int, int)> callback = [&mappedValues](int index, int level) { mappedValues[index] = level; };
                bandMapper->mapEqualizerBands(bandLevelMap, callback);
                EXPECT_EQ(mappedValues[0], (BAND_LEVEL_BOTTOM + BAND_LEVEL_ZERO) / 2);
            }
        }
    }
}