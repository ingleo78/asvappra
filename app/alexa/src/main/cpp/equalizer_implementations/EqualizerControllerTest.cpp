#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <sdkinterfaces/Audio/EqualizerConfigurationInterface.h>
#include <sdkinterfaces/Audio/MockEqualizerControllerListenerInterface.h>
#include <sdkinterfaces/Audio/MockEqualizerInterface.h>
#include <sdkinterfaces/Audio/MockEqualizerModeControllerInterface.h>
#include <sdkinterfaces/Audio/MockEqualizerStorageInterface.h>
#include <util/error/SuccessResult.h>
#include "EqualizerController.h"
#include "InMemoryEqualizerConfiguration.h"

namespace alexaClientSDK {
    namespace equalizer {
        namespace test {
            using namespace testing;
            using namespace utils;
            using namespace audio::test;
            using namespace utils::error;
            static constexpr int MIN_LEVEL = -10;
            static constexpr int MAX_LEVEL = 10;
            static constexpr int BELOW_MIN_LEVEL = -11;
            static constexpr int ABOVE_MAX_LEVEL = 11;
            static constexpr int DEFAULT_LEVEL = 0;
            static constexpr int DEFAULT_ADJUST_DELTA = 1;
            static constexpr int DEFAULT_MIDRANGE = DEFAULT_LEVEL;
            static constexpr int NON_DEFAULT_MIDRANGE = 4;
            static constexpr int DEFAULT_TREBLE = 5;
            static constexpr int NON_DEFAULT_TREBLE = -5;
            static constexpr EqualizerMode DEFAULT_MODE = EqualizerMode::NONE;
            static constexpr int STABILITY_CHECK_ITERATIONS = 100;
            class EqualizerControllerTest : public Test {
            protected:
                void SetUp() override;
                shared_ptr<MockEqualizerStorageInterface> m_storage;
                shared_ptr<EqualizerConfigurationInterface> m_configuration;
                shared_ptr<MockEqualizerModeControllerInterface> m_modeController;
            };
            void EqualizerControllerTest::SetUp() {
                auto bands = set<EqualizerBand>({EqualizerBand::MIDRANGE, EqualizerBand::TREBLE});
                auto defaultState = EqualizerState{DEFAULT_MODE,EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, DEFAULT_MIDRANGE},
                                                   {EqualizerBand::TREBLE, DEFAULT_TREBLE}})};
                auto modes = set<EqualizerMode>({EqualizerMode::NIGHT, EqualizerMode::TV});
                m_configuration = InMemoryEqualizerConfiguration::create(MIN_LEVEL, MAX_LEVEL, DEFAULT_ADJUST_DELTA, bands, modes, defaultState);
                m_storage = make_shared<NiceMock<MockEqualizerStorageInterface>>();
                m_modeController = make_shared<NiceMock<MockEqualizerModeControllerInterface>>();
                ON_CALL(*(m_storage.get()), loadState()).WillByDefault(Invoke([]() {
                    return SuccessResult<EqualizerState>::failure();
                }));
                ON_CALL(*(m_modeController.get()), setEqualizerMode(_)).WillByDefault(Return(true));
            }
            TEST_F(EqualizerControllerTest, test_providedEmptyConfig_shouldUseDefaults) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto configuration = controller->getConfiguration();
                ASSERT_NE(configuration, nullptr);
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::TREBLE).value(), DEFAULT_TREBLE);
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::MIDRANGE).value(), DEFAULT_MIDRANGE);
                auto bandLevelsResult = controller->getBandLevels(set<EqualizerBand>({EqualizerBand::TREBLE, EqualizerBand::MIDRANGE}));
                EXPECT_TRUE(bandLevelsResult.isSucceeded());
                auto bandLevels = bandLevelsResult.value();
                EXPECT_TRUE(bandLevels.end() != bandLevels.find(EqualizerBand::MIDRANGE));
                EXPECT_TRUE(bandLevels.end() != bandLevels.find(EqualizerBand::TREBLE));
                EXPECT_FALSE(bandLevels.end() != bandLevels.find(EqualizerBand::BASS));
                EXPECT_EQ(bandLevels[EqualizerBand::TREBLE], DEFAULT_TREBLE);
                EXPECT_EQ(bandLevels[EqualizerBand::MIDRANGE], DEFAULT_MIDRANGE);
                EXPECT_EQ(controller->getCurrentMode(), DEFAULT_MODE);
            }
            TEST_F(EqualizerControllerTest, test_changeBandLevels_shouldSucceed) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                controller->setBandLevel(EqualizerBand::TREBLE, NON_DEFAULT_TREBLE);
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::TREBLE).value(), NON_DEFAULT_TREBLE);
                controller->setBandLevel(EqualizerBand::MIDRANGE, NON_DEFAULT_MIDRANGE);
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::MIDRANGE).value(), NON_DEFAULT_MIDRANGE);
                auto bandLevelsResult = controller->getBandLevels(set<EqualizerBand>({EqualizerBand::TREBLE, EqualizerBand::MIDRANGE}));
                EXPECT_TRUE(bandLevelsResult.isSucceeded());
                auto bandLevels = bandLevelsResult.value();
                EXPECT_EQ(bandLevels[EqualizerBand::TREBLE], NON_DEFAULT_TREBLE);
                EXPECT_EQ(bandLevels[EqualizerBand::MIDRANGE], NON_DEFAULT_MIDRANGE);
                controller->setBandLevel(EqualizerBand::MIDRANGE, DEFAULT_LEVEL);
                controller->adjustBandLevels(EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, -1}}));
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::MIDRANGE).value(), DEFAULT_LEVEL - 1);
            }
            TEST_F(EqualizerControllerTest, test_setInvalidBandLevels_shouldClampToSupportedRange) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                controller->setBandLevel(EqualizerBand::TREBLE, BELOW_MIN_LEVEL);
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::TREBLE).value(), MIN_LEVEL);
                controller->setBandLevel(EqualizerBand::MIDRANGE, ABOVE_MAX_LEVEL);
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::MIDRANGE).value(), MAX_LEVEL);
                controller->adjustBandLevels(EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, -(MAX_LEVEL - MIN_LEVEL + 1)}}));
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::MIDRANGE).value(), MIN_LEVEL);
                controller->adjustBandLevels(EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, (MAX_LEVEL - MIN_LEVEL + 1)}}));
                EXPECT_EQ(controller->getBandLevel(EqualizerBand::MIDRANGE).value(), MAX_LEVEL);
            }
            TEST_F(EqualizerControllerTest, test_setMode_shouldSucceed) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                controller->setCurrentMode(EqualizerMode::NIGHT);
                ASSERT_EQ(controller->getCurrentMode(), EqualizerMode::NIGHT);
            }
            TEST_F(EqualizerControllerTest, test_setInvalidMode_shouldNotChangeMode) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                controller->setCurrentMode(EqualizerMode::MOVIE);
                ASSERT_EQ(controller->getCurrentMode(), DEFAULT_MODE);
            }
            TEST_F(EqualizerControllerTest, test_providedBandLevelChanges_addRemoveListener_shouldFollowSubscriptionStatus) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto listener = make_shared<NiceMock<MockEqualizerControllerListenerInterface>>();
                controller->addListener(listener);
                EqualizerState reportedState;
                //EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(1).WillOnce(SaveArg<0>(&reportedState));
                controller->setBandLevel(EqualizerBand::TREBLE, NON_DEFAULT_TREBLE);
                EXPECT_EQ(reportedState.bandLevels[EqualizerBand::TREBLE], NON_DEFAULT_TREBLE);
                EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(0);
                EXPECT_CALL(*(listener.get()), onEqualizerSameStateChanged(_)).Times(AtLeast(1));
                controller->setBandLevel(EqualizerBand::TREBLE, NON_DEFAULT_TREBLE);
                EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(AtLeast(0));
                controller->setBandLevel(EqualizerBand::MIDRANGE, DEFAULT_LEVEL);
                //EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(1).WillOnce(SaveArg<0>(&reportedState));
                controller->adjustBandLevels(EqualizerBandLevelMap({{EqualizerBand::MIDRANGE, 1}}));
                ASSERT_EQ(reportedState.bandLevels[EqualizerBand::MIDRANGE], DEFAULT_LEVEL + 1);
                controller->removeListener(listener);
                EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(0);
                controller->setBandLevel(EqualizerBand::TREBLE, DEFAULT_TREBLE);
            }
            TEST_F(EqualizerControllerTest, test_providedModeChanges_addRemoveListener_shouldFollowSubscriptionStatus) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto listener = make_shared<NiceMock<MockEqualizerControllerListenerInterface>>();
                controller->addListener(listener);
                EqualizerState reportedState;
                //EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(1).WillOnce(SaveArg<0>(&reportedState));
                controller->setCurrentMode(EqualizerMode::NIGHT);
                ASSERT_EQ(reportedState.mode, EqualizerMode::NIGHT);
                EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(0);
                controller->setCurrentMode(EqualizerMode::NIGHT);
                //EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(1).WillOnce(SaveArg<0>(&reportedState));
                controller->setCurrentMode(EqualizerMode::TV);
                ASSERT_EQ(reportedState.mode, EqualizerMode::TV);
                EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(0);
                controller->setCurrentMode(EqualizerMode::MUSIC);
            }
            TEST_F(
                EqualizerControllerTest,
                test_providedBandLevelChanges_addRemoveMultipleListeners_shouldFollowSubscriptionStatus) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                EqualizerState reportedState1;
                EqualizerState reportedState2;
                auto listener1 = make_shared<NiceMock<MockEqualizerControllerListenerInterface>>();
                //EXPECT_CALL(*(listener1.get()), onEqualizerStateChanged(_)).Times(1).WillOnce(SaveArg<0>(&reportedState1));
                controller->addListener(listener1);
                auto listener2 = make_shared<NiceMock<MockEqualizerControllerListenerInterface>>();
                //EXPECT_CALL(*(listener2.get()), onEqualizerStateChanged(_)).Times(1).WillOnce(SaveArg<0>(&reportedState2));
                controller->addListener(listener2);
                controller->setBandLevel(EqualizerBand::MIDRANGE, NON_DEFAULT_MIDRANGE);
                ASSERT_EQ(reportedState1.bandLevels[EqualizerBand::MIDRANGE], NON_DEFAULT_MIDRANGE);
                ASSERT_EQ(reportedState2.bandLevels[EqualizerBand::MIDRANGE], NON_DEFAULT_MIDRANGE);
                controller->removeListener(listener1);
                EXPECT_CALL(*(listener1.get()), onEqualizerStateChanged(_)).Times(0);
                //EXPECT_CALL(*(listener2.get()), onEqualizerStateChanged(_)).Times(1).WillOnce(SaveArg<0>(&reportedState2));
                controller->setBandLevel(EqualizerBand::MIDRANGE, DEFAULT_MIDRANGE);
                ASSERT_EQ(reportedState2.bandLevels[EqualizerBand::MIDRANGE], DEFAULT_MIDRANGE);
            }
            TEST_F(EqualizerControllerTest, test_triggerChangesMultipleTimes_ExpectListenersNotifiedSameTimes) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto listener = std::make_shared<NiceMock<MockEqualizerControllerListenerInterface>>();
                controller->addListener(listener);
                EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(STABILITY_CHECK_ITERATIONS);
                for (int i = 0; i < STABILITY_CHECK_ITERATIONS; i++) {
                    controller->setBandLevel(EqualizerBand::TREBLE, DEFAULT_TREBLE + 1 + i % 2);
                }
                EXPECT_CALL(*(listener.get()), onEqualizerStateChanged(_)).Times(0);
            }
            TEST_F(EqualizerControllerTest, test_providedBandLevelChanges_addRemoveSingleEqualizer_shouldFollowRegistrationStatus) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto equalizer = std::make_shared<NiceMock<MockEqualizerInterface>>();
                ON_CALL(*(equalizer.get()), getMinimumBandLevel()).WillByDefault(Return(MIN_LEVEL));
                ON_CALL(*(equalizer.get()), getMaximumBandLevel()).WillByDefault(Return(MAX_LEVEL));
                controller->registerEqualizer(equalizer);
                EqualizerBandLevelMap reportedBandLevels;
                //EXPECT_CALL(*(equalizer.get()), setEqualizerBandLevels(_)).Times(1).WillOnce(SaveArg<0>(&reportedBandLevels));
                controller->setBandLevel(EqualizerBand::TREBLE, NON_DEFAULT_TREBLE);
                EXPECT_EQ(reportedBandLevels[EqualizerBand::TREBLE], NON_DEFAULT_TREBLE);
                EXPECT_CALL(*(equalizer.get()), setEqualizerBandLevels(_)).Times(0);
                controller->unregisterEqualizer(equalizer);
                controller->setBandLevel(EqualizerBand::TREBLE, DEFAULT_TREBLE);
            }
            TEST_F(
                EqualizerControllerTest,
                test_providedBandLevelChanges_addRemoveMultipleEqualizers_shouldFollowRegistrationStatus) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto equalizer1 = make_shared<NiceMock<MockEqualizerInterface>>();
                ON_CALL(*(equalizer1.get()), getMinimumBandLevel()).WillByDefault(Return(MIN_LEVEL));
                ON_CALL(*(equalizer1.get()), getMaximumBandLevel()).WillByDefault(Return(MAX_LEVEL));
                controller->registerEqualizer(equalizer1);
                auto equalizer2 = std::make_shared<NiceMock<MockEqualizerInterface>>();
                ON_CALL(*(equalizer2.get()), getMinimumBandLevel()).WillByDefault(Return(MIN_LEVEL));
                ON_CALL(*(equalizer2.get()), getMaximumBandLevel()).WillByDefault(Return(MAX_LEVEL));
                controller->registerEqualizer(equalizer2);
                EqualizerBandLevelMap reportedBandLevels1;
                EqualizerBandLevelMap reportedBandLevels2;
                //EXPECT_CALL(*(equalizer1.get()), setEqualizerBandLevels(_)).Times(1).WillOnce(SaveArg<0>(&reportedBandLevels1));
                //EXPECT_CALL(*(equalizer2.get()), setEqualizerBandLevels(_)).Times(1).WillOnce(SaveArg<0>(&reportedBandLevels2));
                controller->setBandLevel(EqualizerBand::TREBLE, NON_DEFAULT_TREBLE);
                EXPECT_EQ(reportedBandLevels1[EqualizerBand::TREBLE], NON_DEFAULT_TREBLE);
                EXPECT_EQ(reportedBandLevels2[EqualizerBand::TREBLE], NON_DEFAULT_TREBLE);
                EXPECT_CALL(*(equalizer1.get()), setEqualizerBandLevels(_)).Times(0);
                //EXPECT_CALL(*(equalizer2.get()), setEqualizerBandLevels(_)).Times(1).WillOnce(SaveArg<0>(&reportedBandLevels2));
                controller->unregisterEqualizer(equalizer1);
                controller->setBandLevel(EqualizerBand::TREBLE, DEFAULT_TREBLE);
                EXPECT_EQ(reportedBandLevels2[EqualizerBand::TREBLE], DEFAULT_TREBLE);
            }
            TEST_F(EqualizerControllerTest, test_triggerChangesMultipleTimes_ExpectEqualizersCalledSameTimes) {
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto equalizer = make_shared<NiceMock<MockEqualizerInterface>>();
                controller->registerEqualizer(equalizer);
                EXPECT_CALL(*(equalizer.get()), setEqualizerBandLevels(_)).Times(STABILITY_CHECK_ITERATIONS);
                for (int i = 0; i < STABILITY_CHECK_ITERATIONS; i++) {
                    controller->setBandLevel(EqualizerBand::TREBLE, DEFAULT_TREBLE + 1 + i % 2);
                }
                EXPECT_CALL(*(equalizer.get()), setEqualizerBandLevels(_)).Times(0);
            }
            TEST_F(EqualizerControllerTest, test_saveLoadStateWithPersistentStorage_shouldSucceed) {
                EXPECT_CALL(*(m_storage.get()), loadState()).Times(1);
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                EXPECT_CALL(*(m_storage.get()), saveState(_)).Times(1);
                controller->setBandLevel(EqualizerBand::TREBLE, DEFAULT_TREBLE + 1);
                EXPECT_CALL(*(m_storage.get()), saveState(_)).Times(1);
                controller->setBandLevels(EqualizerBandLevelMap({{EqualizerBand::TREBLE, DEFAULT_TREBLE + 1}, {EqualizerBand::MIDRANGE, DEFAULT_MIDRANGE + 1}}));
                EXPECT_CALL(*(m_storage.get()), saveState(_)).Times(1);
                controller->adjustBandLevels(EqualizerBandLevelMap({{EqualizerBand::TREBLE, -1}}));
                EXPECT_CALL(*(m_storage.get()), saveState(_)).Times(1);
                controller->setCurrentMode(EqualizerMode::TV);
            }
            TEST_F(EqualizerControllerTest, test_setLevelBelowEqualizerMinimum_shouldClamp) {
                EXPECT_CALL(*(m_storage.get()), loadState()).Times(1);
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto equalizer = std::make_shared<NiceMock<MockEqualizerInterface>>();
                ON_CALL(*(equalizer.get()), getMinimumBandLevel()).WillByDefault(Return(MAX_LEVEL));
                ON_CALL(*(equalizer.get()), getMaximumBandLevel()).WillByDefault(Return(MAX_LEVEL));
                controller->registerEqualizer(equalizer);
                EqualizerBandLevelMap bandLevelMap;
                //EXPECT_CALL(*(equalizer.get()), setEqualizerBandLevels(_)).Times(1).WillOnce(SaveArg<0>(&bandLevelMap));
                controller->setBandLevel(EqualizerBand::MIDRANGE, MIN_LEVEL);
                EXPECT_EQ(bandLevelMap[EqualizerBand::MIDRANGE], MAX_LEVEL);
            }
            TEST_F(EqualizerControllerTest, test_setLevelAboveEqualizerMaximum_shouldClamp) {
                EXPECT_CALL(*(m_storage.get()), loadState()).Times(1);
                auto controller = EqualizerController::create(m_modeController, m_configuration, m_storage);
                auto equalizer = make_shared<NiceMock<MockEqualizerInterface>>();
                ON_CALL(*(equalizer.get()), getMinimumBandLevel()).WillByDefault(Return(MIN_LEVEL));
                ON_CALL(*(equalizer.get()), getMaximumBandLevel()).WillByDefault(Return(MIN_LEVEL));
                controller->registerEqualizer(equalizer);
                EqualizerBandLevelMap bandLevelMap;
                //EXPECT_CALL(*(equalizer.get()), setEqualizerBandLevels(_)).Times(1).WillOnce(SaveArg<0>(&bandLevelMap));
                controller->setBandLevel(EqualizerBand::MIDRANGE, MAX_LEVEL);
                EXPECT_EQ(bandLevelMap[EqualizerBand::MIDRANGE], MIN_LEVEL);
            }
        }
    }
}