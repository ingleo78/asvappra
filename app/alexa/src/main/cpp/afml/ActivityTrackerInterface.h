#ifndef ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_ACTIVITYTRACKERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AFML_INCLUDE_AFML_ACTIVITYTRACKERINTERFACE_H_

#include <vector>
#include "Channel.h"

namespace alexaClientSDK {
    namespace afml {
        class ActivityTrackerInterface {
        public:
            virtual ~ActivityTrackerInterface() = default;
            virtual void notifyOfActivityUpdates(const std::vector<Channel::State>& channelStates) = 0;
        };
    }
}
#endif