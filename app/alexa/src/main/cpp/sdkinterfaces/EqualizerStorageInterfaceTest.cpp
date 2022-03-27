#include <gtest/gtest.h>
#include <memory>
#include "Audio/EqualizerStorageInterfaceTest.h"
#include "Audio/EqualizerInterface.h"
#include "Audio/MockAlertsAudioFactory.h"
#include "Audio/MockEqualizerConfigurationInterface.h"
#include "Audio/MockEqualizerControllerListenerInterface.h"
#include "Audio/MockEqualizerInterface.h"
#include "Audio/MockEqualizerModeControllerInterface.h"
#include "Audio/MockEqualizerStorageInterface.h"
#include "Audio/MockSystemSoundAudioFactory.h"
#include "Bluetooth/BluetoothDeviceInterface.h"
#include "Bluetooth/MockBluetoothDeviceManager.h"
#include "Bluetooth/MockBluetoothDevice.h"
#include "Bluetooth/BluetoothDeviceInterface.h"
#include "Bluetooth/MockBluetoothDeviceConnectionRule.h"
#include "Bluetooth/MockBluetoothHostController.h"
#include "Endpoints/MockEndpoint.h"
#include "Endpoints/MockEndpointRegistrationObserver.h"
#include "Endpoints/MockEndpointBuilder.h"
#include "Endpoints/MockEndpointRegistrationManager.h"
#include "ToggleController/ToggleControllerObserverInterface.h"
#include "AlexaInterfaceMessageSenderInterface.h"
#include "AudioInputProcessorObserverInterface.h"
#include "CallManagerInterface.h"
#include "CapabilitiesDelegateInterface.h"
#include "CapabilitiesObserverInterface.h"
#include "ChannelVolumeFactoryInterface.h"
#include "ContextManagerObserverInterface.h"
#include "DirectiveSequencerInterface.h"
#include "ExternalMediaAdapterInterface.h"
#include "GlobalSettingsObserverInterface.h"
#include "KeyWordObserverInterface.h"
#include "LocalPlaybackHandlerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                namespace test {
                    using namespace testing;
                    using namespace sdkInterfaces::audio::test;
                    using namespace sdkInterfaces::audio;
                    void EqualizerStorageInterfaceTest::SetUp() {
                        auto factory = GetParam();
                        m_storage = factory();
                    }
                    TEST_P(EqualizerStorageInterfaceTest, test_saveState_ExpectLoadReturnSame) {
                        EqualizerState defaultState = {EqualizerMode::MOVIE,
                                                       EqualizerBandLevelMap({{EqualizerBand::TREBLE, 0}, {EqualizerBand::MIDRANGE, 1}})};
                        m_storage->saveState(defaultState);
                        auto loadedStateResult = m_storage->loadState();
                        EXPECT_EQ(loadedStateResult.value(), defaultState);
                        EqualizerState state = {EqualizerMode::TV,
                                                EqualizerBandLevelMap({{EqualizerBand::TREBLE, 10}, {EqualizerBand::MIDRANGE, 11}})};
                        m_storage->saveState(state);
                        loadedStateResult = m_storage->loadState();
                        EXPECT_EQ(loadedStateResult.value(), state);
                    }
                    TEST_P(EqualizerStorageInterfaceTest, test_clearSavedData_ExpectAllDefaultsOnLoad) {
                        EqualizerState defaultState = {EqualizerMode::MOVIE,
                                                       EqualizerBandLevelMap({{EqualizerBand::TREBLE, 0}, {EqualizerBand::MIDRANGE, 1}})};
                        EqualizerState state = {EqualizerMode::MOVIE,
                                                EqualizerBandLevelMap({{EqualizerBand::TREBLE, 10}, {EqualizerBand::BASS, 11}})};
                        m_storage->saveState(state);
                        m_storage->clear();
                        auto loadedStateResult = m_storage->loadState();
                        EXPECT_FALSE(loadedStateResult.isSucceeded());
                    }
                }
            }
        }
    }
}
