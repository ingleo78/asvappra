#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTREGISTRATIONOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTREGISTRATIONOBSERVERINTERFACE_H_

#include <iostream>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include "EndpointIdentifier.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                using namespace std;
                using namespace avs;
                class EndpointRegistrationObserverInterface {
                public:
                    enum class RegistrationResult {
                        SUCCEEDED,
                        CONFIGURATION_ERROR,
                        INTERNAL_ERROR,
                        REGISTRATION_IN_PROGRESS,
                        PENDING_DEREGISTRATION
                    };
                    enum class DeregistrationResult {
                        SUCCEEDED,
                        NOT_REGISTERED,
                        INTERNAL_ERROR,
                        CONFIGURATION_ERROR,
                        REGISTRATION_IN_PROGRESS,
                        PENDING_DEREGISTRATION
                    };
                    using RegistrationResult = EndpointRegistrationObserverInterface::RegistrationResult;
                    using DeregistrationResult = EndpointRegistrationObserverInterface::DeregistrationResult;
                    virtual ~EndpointRegistrationObserverInterface() = default;
                    virtual void onEndpointRegistration(const EndpointIdentifier& endpointId, const AVSDiscoveryEndpointAttributes& attributes,
                                                        const RegistrationResult result);
                    virtual void onEndpointDeregistration(const EndpointIdentifier& endpointId, const DeregistrationResult result);
                };
                using RegistrationResult = EndpointRegistrationObserverInterface::RegistrationResult;
                using DeregistrationResult = EndpointRegistrationObserverInterface::DeregistrationResult;
                inline ostream& operator<<(ostream& stream, const RegistrationResult& registrationResult) {
                    switch(registrationResult) {
                        case RegistrationResult::SUCCEEDED: stream << "SUCCEEDED"; break;
                        case RegistrationResult::CONFIGURATION_ERROR: stream << "CONFIGURATION_ERROR"; break;
                        case RegistrationResult::INTERNAL_ERROR: stream << "INTERNAL_ERROR"; break;
                        case RegistrationResult::REGISTRATION_IN_PROGRESS: stream << "REGISTRATION_IN_PROGRESS"; break;
                        case RegistrationResult::PENDING_DEREGISTRATION: stream << "PENDING_DEREGISTRATION"; break;
                        default: stream << "UNKNOWN";
                    }
                    return stream;
                }
                inline ostream& operator<<(ostream& stream, const DeregistrationResult& deregistrationResult) {
                    switch(deregistrationResult) {
                        case DeregistrationResult::SUCCEEDED: stream << "SUCCEEDED"; break;
                        case DeregistrationResult::NOT_REGISTERED: stream << "NOT_REGISTERED"; break;
                        case DeregistrationResult::CONFIGURATION_ERROR: stream << "CONFIGURATION_ERROR"; break;
                        case DeregistrationResult::INTERNAL_ERROR: stream << "INTERNAL_ERROR"; break;
                        case DeregistrationResult::REGISTRATION_IN_PROGRESS: stream << "REGISTRATION_IN_PROGRESS"; break;
                        case DeregistrationResult::PENDING_DEREGISTRATION: stream << "PENDING_DEREGISTRATION"; break;
                        default: stream << "UNKNOWN";
                    }
                    return stream;
                }
            }
        }
    }
}
#endif