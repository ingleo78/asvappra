#include "DefaultChannelVolumeFactory.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            shared_ptr<ChannelVolumeInterface> DefaultChannelVolumeFactory::createChannelVolumeInterface(shared_ptr<SpeakerInterface> speaker, Type type,
                                                                                                         function<int8_t(int8_t)> volumeCurve) {
                return ChannelVolumeManager::create(speaker, type, volumeCurve);
            }
        }
    }
}