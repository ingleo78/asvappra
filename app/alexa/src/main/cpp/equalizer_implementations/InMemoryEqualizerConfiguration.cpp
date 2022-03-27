#include <unordered_set>
#include <logger/Logger.h>
#include "InMemoryEqualizerConfiguration.h"

namespace alexaClientSDK {
    namespace equalizer {
        using namespace logger;
        static const string TAG{"InMemoryEqualizerConfiguration"};
        #define LX(event) LogEntry(TAG, event)
        static constexpr int DEFAULT_LEVEL = 0;
        static constexpr int DEFAULT_MIN_LEVEL = -6;
        static constexpr int DEFAULT_MAX_LEVEL = 6;
        static constexpr int DEFAULT_ADJUST_DELTA = 1;
        set<EqualizerBand> InMemoryEqualizerConfiguration::getSupportedBands() const {
            return m_bandsSupported;
        }
        set<EqualizerMode> InMemoryEqualizerConfiguration::getSupportedModes() const {
            return m_modesSupported;
        }
        int InMemoryEqualizerConfiguration::getMinBandLevel() const {
            return m_minBandLevel;
        }
        int InMemoryEqualizerConfiguration::getMaxBandLevel() const {
            return m_maxBandLevel;
        }
        int InMemoryEqualizerConfiguration::getDefaultBandDelta() const {
            return m_defaultDelta;
        }
        EqualizerState InMemoryEqualizerConfiguration::getDefaultState() const {
            return m_defaultState;
        }
        bool InMemoryEqualizerConfiguration::isBandSupported(EqualizerBand band) const {
            if (!m_isEnabled) return false;
            return m_bandsSupported.end() != m_bandsSupported.find(band);
        }
        bool InMemoryEqualizerConfiguration::isModeSupported(EqualizerMode mode) const {
            if (!m_isEnabled) return false;
            return m_modesSupported.end() != m_modesSupported.find(mode);
        }
        InMemoryEqualizerConfiguration::InMemoryEqualizerConfiguration(int minBandLevel, int maxBandLevel, int defaultDelta,
                                                                       const set<EqualizerBand>& bandsSupported, const set<EqualizerMode>& modesSupported,
                                                                       EqualizerState defaultState) : m_maxBandLevel{maxBandLevel},
                                                                       m_minBandLevel{minBandLevel}, m_defaultDelta{defaultDelta},
                                                                       m_bandsSupported{bandsSupported}, m_modesSupported{modesSupported},
                                                                       m_defaultState(defaultState), m_isEnabled{true} {
            m_modesSupported.erase(EqualizerMode::NONE);
        }
        bool InMemoryEqualizerConfiguration::validateConfiguration() {
            ACSDK_DEBUG1(LX(__func__).m("Validating Equalizer configuration"));
            if (!m_isEnabled) return false;
            bool isValid = true;
            bool areBandExtremumsValid = true;
            if (m_maxBandLevel < m_minBandLevel) {
                ACSDK_ERROR(LX("validateConfigurationFailed").d("reason", "Maximum band level must be greater than minimum band level")
                    .d("maxLevel", m_maxBandLevel).d("minLevel", m_minBandLevel));
                areBandExtremumsValid = false;
                isValid = false;
            }
            if (m_defaultDelta == 0) {
                ACSDK_ERROR(LX("validateConfigurationFailed").d("reason", "delta value for adjusting the band cannot be zero"));
                isValid = false;
            }
            ACSDK_DEBUG1(LX(__func__).m("Validating default Equalizer state"));
            isValid = validateBandLevelMap(m_defaultState.bandLevels, areBandExtremumsValid) && isValid;
            if (EqualizerMode::NONE != m_defaultState.mode) {
                if (!isModeSupported(m_defaultState.mode)) {
                    ACSDK_ERROR(LX("validateConfigurationFailed").d("reason", "Default mode is unsupported")
                        .d("mode", equalizerModeToString(m_defaultState.mode)));
                    isValid = false;
                }
            }
            return isValid;
        }
        bool InMemoryEqualizerConfiguration::validateBandLevelMap(const EqualizerBandLevelMap& bandLevelMap, bool validateValues) {
            if (!m_isEnabled) return false;
            bool isValid = true;
            for (auto bandMapIter : bandLevelMap) {
                EqualizerBand band = bandMapIter.first;
                if (!isBandSupported(band)) {
                    ACSDK_ERROR(LX("validateBandLevelMapFailed").d("reason", "Band unsupported").d("band", equalizerBandToString(band)));
                    isValid = false;
                } else {
                    if (validateValues) {
                        int bandLevel = bandMapIter.second;
                        if (bandLevel < m_minBandLevel || bandLevel > m_maxBandLevel) {
                            ACSDK_ERROR(LX("validateBandLevelMapFailed").d("reason", "Invalid level value").d("level", bandLevel)
                                .d("minimum", m_minBandLevel).d("maximum", m_maxBandLevel).d("band", equalizerBandToString(band)));
                            isValid = false;
                        }
                    }
                }
            }
            return isValid;
        }
        shared_ptr<InMemoryEqualizerConfiguration> InMemoryEqualizerConfiguration::create(int minBandLevel, int maxBandLevel, int defaultDelta,
                                                                                          const set<EqualizerBand>& bandsSupported,
                                                                                          const set<EqualizerMode>& modesSupported,
                                                                                          EqualizerState defaultState) {
            auto configuration = shared_ptr<InMemoryEqualizerConfiguration>(new InMemoryEqualizerConfiguration(minBandLevel, maxBandLevel,
                                                                            defaultDelta, bandsSupported, modesSupported, defaultState));
            if (!configuration->validateConfiguration()) return nullptr;
            return configuration;
        }
        shared_ptr<InMemoryEqualizerConfiguration> InMemoryEqualizerConfiguration::createDefault() {
            return create(DEFAULT_MIN_LEVEL, DEFAULT_MAX_LEVEL, DEFAULT_ADJUST_DELTA,set<EqualizerBand>({EqualizerBand::BASS,
                          EqualizerBand::MIDRANGE, EqualizerBand::TREBLE}),set<EqualizerMode>(),EqualizerState{EqualizerMode::NONE,
                          EqualizerBandLevelMap({{EqualizerBand::BASS, DEFAULT_LEVEL},{EqualizerBand::MIDRANGE, DEFAULT_LEVEL},
                          {EqualizerBand::TREBLE, DEFAULT_LEVEL}})});
        }
        bool InMemoryEqualizerConfiguration::isEnabled() const {
            return m_isEnabled;
        }
        shared_ptr<InMemoryEqualizerConfiguration> InMemoryEqualizerConfiguration::createDisabled() {
            return shared_ptr<InMemoryEqualizerConfiguration>(new InMemoryEqualizerConfiguration());
        }
        InMemoryEqualizerConfiguration::InMemoryEqualizerConfiguration() : m_maxBandLevel{DEFAULT_LEVEL}, m_minBandLevel{DEFAULT_LEVEL},
                                                                           m_defaultDelta{DEFAULT_ADJUST_DELTA},
                                                                           m_defaultState{EqualizerMode::NONE, {{}}},
                                                                           m_isEnabled{false} {}
    }
}