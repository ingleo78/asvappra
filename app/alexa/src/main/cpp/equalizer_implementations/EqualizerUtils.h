#ifndef ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERUTILS_H_
#define ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERUTILS_H_

#include <sdkinterfaces/Audio/EqualizerTypes.h>
#include <util/error/SuccessResult.h>

namespace alexaClientSDK {
    namespace equalizer {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace audio;
        using namespace error;
        using namespace json;
        using namespace jsonUtils;
        using namespace logger;
        class EqualizerUtils {
        public:
            static string serializeEqualizerState(const EqualizerState& state);
            static SuccessResult<EqualizerState> deserializeEqualizerState(const std::string& serializedState);
        };
    }
}
#endif