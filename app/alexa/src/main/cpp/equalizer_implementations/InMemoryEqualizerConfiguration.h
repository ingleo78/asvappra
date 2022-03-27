#ifndef ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_INMEMORYEQUALIZERCONFIGURATION_H_
#define ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_INMEMORYEQUALIZERCONFIGURATION_H_

#include <memory>
#include <mutex>
#include <sdkinterfaces/Audio/EqualizerConfigurationInterface.h>

namespace alexaClientSDK {
    namespace equalizer {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace audio;
        class InMemoryEqualizerConfiguration : public EqualizerConfigurationInterface {
        public:
            ~InMemoryEqualizerConfiguration() override = default;
            static shared_ptr<InMemoryEqualizerConfiguration> create(int minBandLevel, int maxBandLevel, int defaultDelta,
                                                                     const set<EqualizerBand>& bandsSupported, const set<EqualizerMode>& modesSupported,
                                                                     EqualizerState defaultState);
            static shared_ptr<InMemoryEqualizerConfiguration> createDisabled();
            static shared_ptr<InMemoryEqualizerConfiguration> createDefault();
            bool isEnabled() const override;
            set<EqualizerBand> getSupportedBands() const override;
            set<EqualizerMode> getSupportedModes() const override;
            int getMinBandLevel() const override;
            int getMaxBandLevel() const override;
            int getDefaultBandDelta() const override;
            EqualizerState getDefaultState() const override;
            bool isBandSupported(EqualizerBand band) const override;
            bool isModeSupported(EqualizerMode mode) const override;
        protected:
            InMemoryEqualizerConfiguration(int minBandLevel, int maxBandLevel, int defaultDelta, const set<EqualizerBand>& bandsSupported,
                                           const set<EqualizerMode>& modesSupported, EqualizerState defaultState);
            InMemoryEqualizerConfiguration();
            bool validateConfiguration();
            bool validateBandLevelMap(const EqualizerBandLevelMap& bandLevelMap, bool validateValues);
        private:
            int m_maxBandLevel;
            int m_minBandLevel;
            int m_defaultDelta;
            set<EqualizerBand> m_bandsSupported;
            set<EqualizerMode> m_modesSupported;
            EqualizerState m_defaultState;
            bool m_isEnabled;
        };
    }
}
#endif