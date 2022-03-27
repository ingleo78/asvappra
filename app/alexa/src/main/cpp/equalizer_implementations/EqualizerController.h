#ifndef ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERCONTROLLER_H_
#define ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERCONTROLLER_H_

#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <sdkinterfaces/Audio/EqualizerConfigurationInterface.h>
#include <sdkinterfaces/Audio/EqualizerControllerListenerInterface.h>
#include <sdkinterfaces/Audio/EqualizerInterface.h>
#include <sdkinterfaces/Audio/EqualizerModeControllerInterface.h>
#include <sdkinterfaces/Audio/EqualizerStorageInterface.h>
#include <util/error/SuccessResult.h>

namespace alexaClientSDK {
    namespace equalizer {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace audio;
        using namespace error;
        class EqualizerController {
        public:
            ~EqualizerController() = default;
            static shared_ptr<EqualizerController> create(shared_ptr<EqualizerModeControllerInterface> modeController,
                                                          shared_ptr<EqualizerConfigurationInterface> configuration,
                                                          shared_ptr<EqualizerStorageInterface> storage);
            SuccessResult<int> getBandLevel(EqualizerBand band);
            SuccessResult<EqualizerBandLevelMap> getBandLevels(set<EqualizerBand> bands);
            void setBandLevel(EqualizerBand band, int level);
            void setBandLevels(const EqualizerBandLevelMap& bandLevelMap);
            void adjustBandLevels(const EqualizerBandLevelMap& bandAdjustmentMap);
            void resetBands(const set<EqualizerBand>& bands);
            EqualizerMode getCurrentMode();
            void setCurrentMode(EqualizerMode mode);
            EqualizerState getCurrentState();
            shared_ptr<EqualizerConfigurationInterface> getConfiguration();
            void registerEqualizer(shared_ptr<EqualizerInterface> equalizer);
            void unregisterEqualizer(shared_ptr<EqualizerInterface> equalizer);
            void addListener(shared_ptr<EqualizerControllerListenerInterface> listener);
            void removeListener(shared_ptr<EqualizerControllerListenerInterface> listener);
        private:
            EqualizerController(shared_ptr<EqualizerModeControllerInterface> modeController, shared_ptr<EqualizerConfigurationInterface> configuration,
                                shared_ptr<EqualizerStorageInterface> storage);
            void updateState();
            void applyChangesToCurrentState(const EqualizerBandLevelMap& changesDataMap, function<int(int, int)> operation);
            EqualizerState loadState();
            int truncateBandLevel(int level);
            void notifyListenersOnStateChanged(bool changed);
            shared_ptr<EqualizerModeControllerInterface> m_modeController;
            shared_ptr<EqualizerConfigurationInterface> m_configuration;
            shared_ptr<EqualizerStorageInterface> m_storage;
            EqualizerState m_currentState;
            mutex m_stateMutex;
            mutex m_modeMutex;
            list<shared_ptr<EqualizerControllerListenerInterface>> m_listeners;
            list<shared_ptr<EqualizerInterface>> m_equalizers;
        };
    }
}
#endif