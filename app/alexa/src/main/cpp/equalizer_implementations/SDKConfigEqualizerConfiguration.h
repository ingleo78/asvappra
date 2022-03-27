#ifndef ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_SDKCONFIGEQUALIZERCONFIGURATION_H_
#define ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_SDKCONFIGEQUALIZERCONFIGURATION_H_

#include <memory>
#include <mutex>
#include <sdkinterfaces/Audio/EqualizerConfigurationInterface.h>
#include <configuration/ConfigurationNode.h>
#include "InMemoryEqualizerConfiguration.h"

namespace alexaClientSDK {
    namespace equalizer {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace audio;
        using namespace configuration;
        class SDKConfigEqualizerConfiguration : public InMemoryEqualizerConfiguration {
        public:
            static const bool BAND_IS_SUPPORTED_IF_MISSING_IN_CONFIG = false;
            static const bool MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG = false;
            static shared_ptr<SDKConfigEqualizerConfiguration> create(ConfigurationNode& configRoot);
        private:
            SDKConfigEqualizerConfiguration(int minBandLevel, int maxBandLevel, int defaultDelta, set<EqualizerBand> bandsSupported,
                                            set<EqualizerMode> modesSupported, EqualizerState defaultState);
            SDKConfigEqualizerConfiguration();
        };
    }
}
#endif