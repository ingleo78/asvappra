#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CAPABILITIESOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CAPABILITIESOBSERVERINTERFACE_H_

#include <ostream>
#include <vector>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            class CapabilitiesObserverInterface {
            public:
                enum class State {
                    UNINITIALIZED,
                    SUCCESS,
                    FATAL_ERROR,
                    RETRIABLE_ERROR,
                };
                enum class Error {
                    UNINITIALIZED,
                    SUCCESS,
                    UNKNOWN_ERROR,
                    CANCELED,
                    FORBIDDEN,
                    SERVER_INTERNAL_ERROR,
                    BAD_REQUEST
                };
                virtual ~CapabilitiesObserverInterface() = default;
                virtual void onCapabilitiesStateChange(State newState, Error newError, const vector<string>& addedOrUpdatedEndpointIds,
                                                       const vector<string>& deletedEndpointIds) = 0;
            };
            inline std::ostream& operator<<(std::ostream& stream, const CapabilitiesObserverInterface::State& state) {
                switch (state) {
                    case CapabilitiesObserverInterface::State::UNINITIALIZED: return stream << "UNINITIALIZED";
                    case CapabilitiesObserverInterface::State::SUCCESS: return stream << "SUCCESS";
                    case CapabilitiesObserverInterface::State::FATAL_ERROR: return stream << "FATAL_ERROR";
                    case CapabilitiesObserverInterface::State::RETRIABLE_ERROR: return stream << "RETRIABLE_ERROR";
                }
                return stream << "Unknown CapabilitiesObserverInterface::State!: " << state;
            }
            inline std::ostream& operator<<(std::ostream& stream, const CapabilitiesObserverInterface::Error& error) {
                switch (error) {
                    case CapabilitiesObserverInterface::Error::UNINITIALIZED: return stream << "UNINITIALIZED";
                    case CapabilitiesObserverInterface::Error::SUCCESS: return stream << "SUCCESS";
                    case CapabilitiesObserverInterface::Error::UNKNOWN_ERROR: return stream << "UNKNOWN_ERROR";
                    case CapabilitiesObserverInterface::Error::FORBIDDEN: return stream << "FORBIDDEN";
                    case CapabilitiesObserverInterface::Error::SERVER_INTERNAL_ERROR: return stream << "SERVER_INTERNAL_ERROR";
                    case CapabilitiesObserverInterface::Error::BAD_REQUEST: return stream << "CLIENT_ERROR_BAD_REQUEST";
                    case CapabilitiesObserverInterface::Error::CANCELED: return stream << "CANCELED";
                }
                return stream << "Unknown CapabilitiesObserverInterface::Error!: " << error;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CAPABILITIESOBSERVERINTERFACE_H_
