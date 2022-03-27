#ifndef ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERBANDMAPPERINTERFACE_H_
#define ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERBANDMAPPERINTERFACE_H_

#include <functional>
#include <sdkinterfaces/Audio/EqualizerTypes.h>

namespace alexaClientSDK {
    namespace equalizer {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace audio;
        class EqualizerBandMapperInterface {
        public:
            virtual ~EqualizerBandMapperInterface() = default;
            virtual void mapEqualizerBands(const EqualizerBandLevelMap& bandLevelMap, function<void(int, int)> setBandCallback);
        };
    }
}
#endif