#include <logger/Logger.h>
#include "SDKConfigEqualizerConfiguration.h"

namespace alexaClientSDK {
    namespace equalizer {
        using namespace error;
        using namespace logger;
        static const string TAG{"SDKConfigEqualizerConfiguration"};
        #define LX(event) LogEntry(TAG, event)
        static const string ENABLED_CONFIG_KEY = "enabled";
        static const string BANDS_CONFIG_KEY = "bands";
        static const string MODES_CONFIG_KEY = "modes";
        static const string MODE_CONFIG_KEY = "mode";
        static const string DEFAULT_STATE_CONFIG_KEY = "defaultState";
        static const string MIN_LEVEL_CONFIG_KEY = "minLevel";
        static const string MAX_LEVEL_CONFIG_KEY = "maxLevel";
        static const string DEFAULT_DELTA_CONFIG_KEY = "defaultDelta";
        const bool SDKConfigEqualizerConfiguration::BAND_IS_SUPPORTED_IF_MISSING_IN_CONFIG;
        const bool SDKConfigEqualizerConfiguration::MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG;
        shared_ptr<SDKConfigEqualizerConfiguration> SDKConfigEqualizerConfiguration::create(ConfigurationNode& configRoot) {
            set<EqualizerBand> bandsSupported;
            set<EqualizerMode> modesSupported;
            EqualizerState defaultState;
            bool hasErrors = false;
            bool isEnabled;
            configRoot.getBool(ENABLED_CONFIG_KEY, &isEnabled, true);
            if (!isEnabled) {
                return shared_ptr<SDKConfigEqualizerConfiguration>(new SDKConfigEqualizerConfiguration());
            }
            auto defaultConfiguration = InMemoryEqualizerConfiguration::createDefault();
            int minLevel;
            int maxLevel;
            int defaultDelta;
            configRoot.getInt(MIN_LEVEL_CONFIG_KEY, &minLevel, defaultConfiguration->getMinBandLevel());
            configRoot.getInt(MAX_LEVEL_CONFIG_KEY, &maxLevel, defaultConfiguration->getMaxBandLevel());
            configRoot.getInt(DEFAULT_DELTA_CONFIG_KEY, &defaultDelta, defaultConfiguration->getDefaultBandDelta());
            auto supportedBandsBranch = configRoot[BANDS_CONFIG_KEY];
            if (supportedBandsBranch) {
                for (EqualizerBand band : EqualizerBandValues) {
                    string bandName = equalizerBandToString(band);
                    bool value;
                    supportedBandsBranch.getBool(bandName, &value, BAND_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                    if (value) bandsSupported.insert(band);
                }
            } else bandsSupported = defaultConfiguration->getSupportedBands();
            auto supportedModesBranch = configRoot[MODES_CONFIG_KEY];
            if (supportedModesBranch) {
                for (EqualizerMode mode : EqualizerModeValues) {
                    if (EqualizerMode::NONE == mode) continue;
                    string modeName = equalizerModeToString(mode);
                    bool value;
                    supportedModesBranch.getBool(modeName, &value, MODE_IS_SUPPORTED_IF_MISSING_IN_CONFIG);
                    if (value) modesSupported.insert(mode);
                }
            } else modesSupported = defaultConfiguration->getSupportedModes();
            bool hasDefaultStateDefined = false;
            EqualizerState defaultConfigDefaultState = defaultConfiguration->getDefaultState();
            defaultState.mode = defaultConfigDefaultState.mode;
            auto defaultStateBranch = configRoot[DEFAULT_STATE_CONFIG_KEY];
            if (defaultStateBranch) {
                string defaultModeStr;
                if (defaultStateBranch.getString(MODE_CONFIG_KEY, &defaultModeStr)) {
                    SuccessResult<EqualizerMode> modeResult = stringToEqualizerMode(defaultModeStr);
                    if (!modeResult.isSucceeded()) {
                        ACSDK_WARN(LX(__func__).m("Invalid value for default state mode, assuming none set").d("value", defaultModeStr));
                    } else {
                        EqualizerMode defaultMode = modeResult.value();
                        if (modesSupported.end() == modesSupported.find(defaultMode)) {
                            ACSDK_ERROR(LX("createFailed").d("reason", "Unsupported mode is set as default state mode").d("mode", defaultModeStr));
                            hasErrors = true;
                        } else defaultState.mode = defaultMode;
                    }
                }
                auto defaultBandsBranch = defaultStateBranch[BANDS_CONFIG_KEY];
                if (defaultBandsBranch) {
                    EqualizerBandLevelMap levelMap;
                    for (EqualizerBand band : bandsSupported) {
                        string bandName = equalizerBandToString(band);
                        int level;
                        if (!defaultBandsBranch.getInt(bandName, &level)) {
                            ACSDK_ERROR(LX("createFailed").d("reason", "Default state definition incomplete").d("missing band", bandName));
                            hasErrors = true;
                        }
                        levelMap[band] = level;
                    }
                    defaultState.bandLevels = levelMap;
                    hasDefaultStateDefined = true;
                }
            }
            if (!hasDefaultStateDefined) {
                for (auto band : bandsSupported) {
                    defaultState.bandLevels[band] = defaultConfigDefaultState.bandLevels[band];
                }
            }
            if (hasErrors) return nullptr;
            auto config = shared_ptr<SDKConfigEqualizerConfiguration>(new SDKConfigEqualizerConfiguration(minLevel, maxLevel, defaultDelta,
                                                                      bandsSupported, modesSupported, defaultState));
            if (!config->validateConfiguration()) return nullptr;
            if (bandsSupported.empty()) {
                ACSDK_WARN(LX(__func__).m("Configuration has no bands supported while Equalizer is enabled. Is it intended?"));
            }
            return config;
        }
        SDKConfigEqualizerConfiguration::SDKConfigEqualizerConfiguration(int minBandLevel, int maxBandLevel, int defaultDelta,
                                                                         set<EqualizerBand> bandsSupported, set<EqualizerMode> modesSupported,
                                                                         EqualizerState defaultState) : InMemoryEqualizerConfiguration(minBandLevel,
                                                                         maxBandLevel, defaultDelta, bandsSupported, modesSupported,
                                                                         defaultState) {}
        SDKConfigEqualizerConfiguration::SDKConfigEqualizerConfiguration() : InMemoryEqualizerConfiguration() {}
    }
}