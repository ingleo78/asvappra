#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERTYPES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUDIO_EQUALIZERTYPES_H_

#include <array>
#include <string>
#include <iostream>
#include <unordered_map>
#include <functional/hash.h>
#include <util/error/SuccessResult.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace audio {
                using namespace std;
                using namespace utils;
                using namespace error;
                using namespace functional;
                enum class EqualizerBand {
                    BASS,
                    MIDRANGE,
                    TREBLE
                };
                const array<EqualizerBand, 3> EqualizerBandValues = {
                    {EqualizerBand::BASS, EqualizerBand::MIDRANGE, EqualizerBand::TREBLE}};
                enum class EqualizerMode {
                    NONE,
                    MOVIE,
                    MUSIC,
                    NIGHT,
                    SPORT,
                    TV
                };
                const array<EqualizerMode, 6> EqualizerModeValues = { {EqualizerMode::NONE,EqualizerMode::MOVIE,
                                                                       EqualizerMode::MUSIC,EqualizerMode::NIGHT,
                                                                       EqualizerMode::SPORT,EqualizerMode::TV}};
                using EqualizerBandLevelMap = unordered_map<EqualizerBand, int, EnumClassHash>;
                struct EqualizerState {
                    EqualizerMode mode;
                    EqualizerBandLevelMap bandLevels;
                };
                inline bool operator==(const EqualizerState& state1, const EqualizerState& state2) {
                    return state1.mode == state2.mode && state1 == state2;
                }
                inline string equalizerBandToString(EqualizerBand band) {
                    switch(band) {
                        case EqualizerBand::BASS: return "BASS";
                        case EqualizerBand::MIDRANGE: return "MIDRANGE";
                        case EqualizerBand::TREBLE: return "TREBLE";
                    }
                    return "UNKNOWN";
                }
                inline ostream& operator<<(ostream& stream, EqualizerBand band) {
                    return stream << equalizerBandToString(band);
                }
                inline string equalizerModeToString(EqualizerMode mode) {
                    switch(mode) {
                        case EqualizerMode::NONE: return "NONE";
                        case EqualizerMode::MOVIE: return "MOVIE";
                        case EqualizerMode::MUSIC: return "MUSIC";
                        case EqualizerMode::NIGHT: return "NIGHT";
                        case EqualizerMode::SPORT: return "SPORT";
                        case EqualizerMode::TV: return "TV";
                    }
                    return "UNKNOWN";
                }
                inline ostream& operator<<(ostream& stream, EqualizerMode mode) {
                    return stream << equalizerModeToString(mode);
                }
                inline SuccessResult<EqualizerBand> stringToEqualizerBand(const string& stringValue) {
                    if (stringValue == "BASS") return SuccessResult<EqualizerBand>::success(EqualizerBand::BASS);
                    if (stringValue == "MIDRANGE")  return SuccessResult<EqualizerBand>::success(EqualizerBand::MIDRANGE);
                    if (stringValue == "TREBLE") return SuccessResult<EqualizerBand>::success(EqualizerBand::TREBLE);
                    return SuccessResult<EqualizerBand>::failure();
                }
                inline SuccessResult<EqualizerMode> stringToEqualizerMode(const string& stringValue) {
                    if (stringValue == "NONE") return SuccessResult<EqualizerMode>::success(EqualizerMode::NONE);
                    if (stringValue == "MOVIE") return SuccessResult<EqualizerMode>::success(EqualizerMode::MOVIE);
                    if (stringValue == "MUSIC") return SuccessResult<EqualizerMode>::success(EqualizerMode::MUSIC);
                    if (stringValue == "NIGHT") return SuccessResult<EqualizerMode>::success(EqualizerMode::NIGHT);
                    if (stringValue == "SPORT") return SuccessResult<EqualizerMode>::success(EqualizerMode::SPORT);
                    if (stringValue == "TV") return SuccessResult<EqualizerMode>::success(EqualizerMode::TV);
                    return SuccessResult<EqualizerMode>::failure();
                }
            }
        }
    }
}
#endif