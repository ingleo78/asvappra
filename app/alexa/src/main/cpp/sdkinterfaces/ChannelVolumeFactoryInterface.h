#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CHANNELVOLUMEFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CHANNELVOLUMEFACTORYINTERFACE_H_

#include <memory>
#include <functional>
#include <stdint.h>
#include "ChannelVolumeInterface.h"
#include "SpeakerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            class ChannelVolumeFactoryInterface {
            public:
                virtual shared_ptr<ChannelVolumeInterface> createChannelVolumeInterface(shared_ptr<SpeakerInterface> speaker, ChannelVolumeInterface::Type type,
                                                                                        function<int8_t(int8_t)>& volumeCurve) = 0;
                virtual ~ChannelVolumeFactoryInterface() = default;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CHANNELVOLUMEFACTORYINTERFACE_H_
