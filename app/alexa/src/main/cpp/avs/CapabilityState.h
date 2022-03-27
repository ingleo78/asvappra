#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYSTATE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYSTATE_H_

#include <cstdint>
#include <string>
#include <timing/TimePoint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils::timing;
            struct CapabilityState {
                CapabilityState(const string& valuePayload, const TimePoint& timeOfSample = TimePoint::now(), const uint32_t uncertaintyInMilliseconds = 0);
                CapabilityState(const CapabilityState& other) = default;
                CapabilityState() = default;
                CapabilityState& operator=(const CapabilityState& other) = default;
                inline bool operator==(const CapabilityState& rhs) const;
                inline bool operator!=(const CapabilityState& rhs) const;
                string valuePayload;
                TimePoint timeOfSample;
                uint32_t uncertaintyInMilliseconds;
            };
            bool CapabilityState::operator==(const CapabilityState& rhs) const {
                auto thisTime = timeOfSample.getTime_Unix();
                auto rhsTime = rhs.timeOfSample.getTime_Unix();
                return tie(valuePayload, thisTime, uncertaintyInMilliseconds) == tie(rhs.valuePayload, rhsTime, rhs.uncertaintyInMilliseconds);
            }
            bool CapabilityState::operator!=(const CapabilityState& rhs) const {
                return !(rhs == *this);
            }
            inline CapabilityState::CapabilityState(const string& valuePayload, const TimePoint& timeOfSample, const uint32_t uncertaintyInMilliseconds) :
                                                    valuePayload(valuePayload), timeOfSample(timeOfSample), uncertaintyInMilliseconds(uncertaintyInMilliseconds) {}
        }
    }
}
#endif