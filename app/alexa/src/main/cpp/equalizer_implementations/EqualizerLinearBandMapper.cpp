#include <array>
#include <logger/Logger.h>
#include "EqualizerLinearBandMapper.h"

namespace alexaClientSDK {
    namespace equalizer {
        using namespace utils;
        using namespace logger;
        static const string TAG{"EqualizerLinearBandMapper"};
        #define LX(event) LogEntry(TAG, event)
        static const int MIN_TARGET_BANDS = 1;
        static const int MAX_TARGET_BANDS = 1000;
        void EqualizerLinearBandMapper::mapEqualizerBands(const EqualizerBandLevelMap& bandLevelMap, function<void(int, int)> setBandCallback) {
            if (bandLevelMap.empty()) {
                ACSDK_ERROR(LX("mapEqualizerBandsFailed").d("reason", "no source bands provided"));
                return;
            }
            vector<int> levels(bandLevelMap.size());
            int outBandIndex = 0;
            for (EqualizerBand band : EqualizerBandValues) {
                auto it = bandLevelMap.find(band);
                if (bandLevelMap.end() != it) levels[outBandIndex++] = it->second;
            }
            int numberOfBandsToMapFrom = outBandIndex;
            if (numberOfBandsToMapFrom <= m_numberOfTargetBands) {
                outBandIndex = 0;
                int accumulator = m_numberOfTargetBands;
                for (int i = 0; i < m_numberOfTargetBands; i++) {
                    int level = levels[outBandIndex];
                    accumulator -= numberOfBandsToMapFrom;
                    if (accumulator < 1) {
                        outBandIndex++;
                        if (accumulator < 0) {
                            level += levels[outBandIndex];
                            level /= 2;
                        }
                        accumulator += m_numberOfTargetBands;
                    }
                    setBandCallback(i, level);
                }
            } else {
                outBandIndex = 0;
                int accumulator = numberOfBandsToMapFrom;
                int level = 0;
                int bandsGrouped = 0;
                for (int i = 0; i < numberOfBandsToMapFrom; i++) {
                    level += levels[i];
                    bandsGrouped++;
                    accumulator -= m_numberOfTargetBands;
                    if (accumulator < 1) {
                        level /= bandsGrouped;
                        setBandCallback(outBandIndex, level);
                        outBandIndex++;
                        accumulator += numberOfBandsToMapFrom;
                        if (accumulator < numberOfBandsToMapFrom) {
                            level = levels[i];
                            bandsGrouped = 1;
                        } else {
                            level = 0;
                            bandsGrouped = 0;
                        }
                    }
                }
            }
        }
        EqualizerLinearBandMapper::EqualizerLinearBandMapper(int numberOfTargetBands) : m_numberOfTargetBands{numberOfTargetBands} {}
        shared_ptr<EqualizerLinearBandMapper> EqualizerLinearBandMapper::create(int numberOfTargetBands) {
            if (numberOfTargetBands < MIN_TARGET_BANDS || numberOfTargetBands > MAX_TARGET_BANDS) {
                ACSDK_ERROR(LX("createFailed").d("reason", "invalid number of target bands").d("target bands", numberOfTargetBands)
                    .d("min", MIN_TARGET_BANDS).d("max", MAX_TARGET_BANDS));
                return nullptr;
            }
            return shared_ptr<EqualizerLinearBandMapper>(new EqualizerLinearBandMapper(numberOfTargetBands));
        }
    }
}