#ifndef ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERLINEARBANDMAPPER_H_
#define ALEXA_CLIENT_SDK_EQUALIZERIMPLEMENTATIONS_INCLUDE_EQUALIZERIMPLEMENTATIONS_EQUALIZERLINEARBANDMAPPER_H_

#include <functional>
#include <memory>
#include <sdkinterfaces/Audio/EqualizerTypes.h>
#include "EqualizerBandMapperInterface.h"

namespace alexaClientSDK {
    namespace equalizer {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace audio;
        class EqualizerLinearBandMapper : public EqualizerBandMapperInterface {
        public:
            static shared_ptr<EqualizerLinearBandMapper> create(int numberOfTargetBands);
            void mapEqualizerBands(const EqualizerBandLevelMap& bandLevelMap, function<void(int, int)> setBandCallback) override;
        private:
            explicit EqualizerLinearBandMapper(int numberOfTargetBands);
            int m_numberOfTargetBands;
        };
    }
}
#endif