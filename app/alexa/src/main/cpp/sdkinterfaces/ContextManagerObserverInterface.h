#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONTEXTMANAGEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONTEXTMANAGEROBSERVERINTERFACE_H_

#include <memory>
#include <avs/CapabilityTag.h>
#include <avs/CapabilityState.h>
#include <util/Optional.h>
#include "AlexaStateChangeCauseType.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace avs;
            class ContextManagerObserverInterface {
            public:
                virtual ~ContextManagerObserverInterface() = default;
                virtual void onStateChanged(const CapabilityTag& identifier, const CapabilityState& state, AlexaStateChangeCauseType cause);
            };
        }
    }
}
#endif