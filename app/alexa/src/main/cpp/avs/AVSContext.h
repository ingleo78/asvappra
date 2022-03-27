#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSCONTEXT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSCONTEXT_H_

#include <map>
#include <ostream>
#include <string>
#include <util/Optional.h>
#include "CapabilityTag.h"
#include "CapabilityState.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            class AVSContext {
            public:
                using States = std::map<CapabilityTag, CapabilityState>;
                AVSContext() = default;
                std::string toJson() const;
                States getStates() const;
                utils::Optional<CapabilityState> getState(const CapabilityTag& identifier) const;
                void addState(const CapabilityTag& identifier, const CapabilityState& state);
                void removeState(const CapabilityTag& identifier);
            private:
                States m_states;
            };
        }
    }
}
#endif