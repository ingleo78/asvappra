#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_SOURCECONFIG_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_SOURCECONFIG_H_

#include <algorithm>
#include <chrono>
#include <ostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                using namespace std;
                using namespace chrono;
                static constexpr short MAX_GAIN = 100;
                static constexpr short MIN_GAIN = 0;
                struct SourceConfig {
                    struct FadeInConfig {
                        short startGain;
                        short endGain;
                        milliseconds duration;
                        bool enabled;
                    };
                    struct AudioNormalizationConfig {
                        bool enabled;
                    };
                    FadeInConfig fadeInConfig;
                    AudioNormalizationConfig audioNormalizationConfig;
                    milliseconds endOffset;
                    static inline SourceConfig createWithFadeIn(short startGain, short endGain, const milliseconds& duration);
                };
                inline SourceConfig emptySourceConfig() {
                    return SourceConfig{{100, 100, milliseconds::zero(), false},
                                        {false}, milliseconds::zero()};
                }
                inline SourceConfig SourceConfig::createWithFadeIn(short startGain, short endGain, const milliseconds& duration) {
                    short validStartGain = max(MIN_GAIN, min(MAX_GAIN, startGain));
                    short validEndGain = max(MIN_GAIN, min(MAX_GAIN, endGain));
                    return SourceConfig{{validStartGain, validEndGain, duration, true}, {false},
                                        milliseconds::zero()};
                }
                inline ostream& operator<<(ostream& stream, const SourceConfig& config) {
                    return stream << "fadeIn{"
                                  << " enabled:" << config.fadeInConfig.enabled << ", start:" << config.fadeInConfig.startGain
                                  << ", end:" << config.fadeInConfig.endGain
                                  << ", duration(ms):" << config.fadeInConfig.duration.count() << "}"
                                  << ", normalization{"
                                  << " enabled: " << config.audioNormalizationConfig.enabled << "}"
                                  << ", endOffset(ms): " << config.endOffset.count();
                }
            }
        }
    }
}
#endif