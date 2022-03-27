#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_DEFAULTCHANNELVOLUMEFACTORY_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_DEFAULTCHANNELVOLUMEFACTORY_H_

#include <sdkinterfaces/ChannelVolumeFactoryInterface.h>
#include <capability_agents/SpeakerManager/ChannelVolumeManager.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using Type = ChannelVolumeInterface::Type;
            class DefaultChannelVolumeFactory : public ChannelVolumeFactoryInterface {
            public:
                virtual shared_ptr<ChannelVolumeInterface> createChannelVolumeInterface(shared_ptr<SpeakerInterface> speaker, Type type,
                                                                                        function<int8_t(int8_t)> volumeCurve);
            };
        }
    }
}
#endif