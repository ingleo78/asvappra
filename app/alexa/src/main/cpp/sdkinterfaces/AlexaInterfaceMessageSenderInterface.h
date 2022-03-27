#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ALEXAINTERFACEMESSAGESENDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ALEXAINTERFACEMESSAGESENDERINTERFACE_H_

#include <memory>
#include <string>
#include <avs/AlexaResponseType.h>
#include <avs/AVSMessageEndpoint.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace avs;
            class AlexaInterfaceMessageSenderInterface {
            public:
                enum class ErrorResponseType {
                    ALREADY_IN_OPERATION,
                    BRIDGE_UNREACHABLE,
                    ENDPOINT_BUSY,
                    ENDPOINT_LOW_POWER,
                    ENDPOINT_UNREACHABLE,
                    EXPIRED_AUTHORIZATION_CREDENTIAL,
                    FIRMWARE_OUT_OF_DATE,
                    HARDWARE_MALFUNCTION,
                    INSUFFICIENT_PERMISSIONS,
                    INTERNAL_ERROR,
                    INVALID_AUTHORIZATION_CREDENTIAL,
                    INVALID_DIRECTIVE,
                    INVALID_VALUE,
                    NO_SUCH_ENDPOINT,
                    NOT_CALIBRATED,
                    NOT_SUPPORTED_IN_CURRENT_MODE,
                    NOT_IN_OPERATION,
                    POWER_LEVEL_NOT_SUPPORTED,
                    RATE_LIMIT_EXCEEDED,
                    TEMPERATURE_VALUE_OUT_OF_RANGE,
                    VALUE_OUT_OF_RANGE
                };
                virtual ~AlexaInterfaceMessageSenderInterface() = default;
                virtual bool sendResponseEvent(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                               const string_view& jsonPayload = "{}");
                virtual bool sendErrorResponseEvent(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                                    const ErrorResponseType errorType, const string_view& errorMessage = "");
                virtual bool sendDeferredResponseEvent(const string& instance, const string& correlationToken, const int estimatedDeferralInSeconds = 0);
                static ErrorResponseType alexaResponseTypeToErrorType(const avsCommon::avs::AlexaResponseType responseType);
            };
            using ErrorResponseType = AlexaInterfaceMessageSenderInterface::ErrorResponseType;
            inline ErrorResponseType AlexaInterfaceMessageSenderInterface::alexaResponseTypeToErrorType(const AlexaResponseType responseType) {
                switch(responseType) {
                    case AlexaResponseType::SUCCESS: return ErrorResponseType::INTERNAL_ERROR;
                    case AlexaResponseType::ALREADY_IN_OPERATION: return ErrorResponseType::ALREADY_IN_OPERATION;
                    case AlexaResponseType::BRIDGE_UNREACHABLE: return ErrorResponseType::BRIDGE_UNREACHABLE;
                    case AlexaResponseType::ENDPOINT_BUSY: return ErrorResponseType::ENDPOINT_BUSY;
                    case AlexaResponseType::ENDPOINT_LOW_POWER: return ErrorResponseType::ENDPOINT_LOW_POWER;
                    case AlexaResponseType::ENDPOINT_UNREACHABLE: return ErrorResponseType::ENDPOINT_UNREACHABLE;
                    case AlexaResponseType::FIRMWARE_OUT_OF_DATE: return ErrorResponseType::FIRMWARE_OUT_OF_DATE;
                    case AlexaResponseType::HARDWARE_MALFUNCTION: return ErrorResponseType::HARDWARE_MALFUNCTION;
                    case AlexaResponseType::INSUFFICIENT_PERMISSIONS: return ErrorResponseType::INSUFFICIENT_PERMISSIONS;
                    case AlexaResponseType::INTERNAL_ERROR: return ErrorResponseType::INTERNAL_ERROR;
                    case AlexaResponseType::INVALID_VALUE: return ErrorResponseType::INVALID_VALUE;
                    case AlexaResponseType::NOT_CALIBRATED: return ErrorResponseType::NOT_CALIBRATED;
                    case AlexaResponseType::NOT_SUPPORTED_IN_CURRENT_MODE: return ErrorResponseType::NOT_SUPPORTED_IN_CURRENT_MODE;
                    case AlexaResponseType::NOT_IN_OPERATION: return ErrorResponseType::NOT_IN_OPERATION;
                    case AlexaResponseType::POWER_LEVEL_NOT_SUPPORTED: return ErrorResponseType::POWER_LEVEL_NOT_SUPPORTED;
                    case AlexaResponseType::RATE_LIMIT_EXCEEDED: return ErrorResponseType::RATE_LIMIT_EXCEEDED;
                    case AlexaResponseType::TEMPERATURE_VALUE_OUT_OF_RANGE: return ErrorResponseType::TEMPERATURE_VALUE_OUT_OF_RANGE;
                    case AlexaResponseType::VALUE_OUT_OF_RANGE: return ErrorResponseType::VALUE_OUT_OF_RANGE;
                }
                return ErrorResponseType::INTERNAL_ERROR;
            }
        }
    }
}
#endif