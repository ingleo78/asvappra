#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONTEXTREQUESTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONTEXTREQUESTERINTERFACE_H_

#include <cstdint>
#include <string>
#include <ostream>
#include <avs/AVSContext.h>
#include <logger/Logger.h>
#include <logger/LogEntry.h>
#include "ContextRequestToken.h"
#include "Endpoints/EndpointIdentifier.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            enum class ContextRequestError {
                STATE_PROVIDER_TIMEDOUT,
                BUILD_CONTEXT_ERROR,
                ENDPOINT_UNREACHABLE
            };
            using namespace avs;
            using namespace endpoints;
            using namespace utils;
            using namespace logger;
            class ContextRequesterInterface {
            public:
                virtual ~ContextRequesterInterface() = default;
                virtual void onContextAvailable(const string& jsonContext);
                virtual void onContextAvailable(const EndpointIdentifier& endpointId, const AVSContext& endpointContext,
                                                ContextRequestToken requestToken);
                virtual void onContextFailure(const ContextRequestError error);
                virtual void onContextFailure(const ContextRequestError error, ContextRequestToken token);
            };
            inline ostream& operator<<(ostream& stream, const ContextRequestError& error) {
                switch(error) {
                    case ContextRequestError::STATE_PROVIDER_TIMEDOUT: stream << "STATE_PROVIDER_TIMEDOUT"; break;
                    case ContextRequestError::BUILD_CONTEXT_ERROR: stream << "BUILD_CONTEXT_ERROR"; break;
                    case ContextRequestError::ENDPOINT_UNREACHABLE: stream << "ENDPOINT_UNREACHABLE"; break;
                }
                return stream;
            }
            inline void ContextRequesterInterface::onContextAvailable(const string& jsonContext) {
                acsdkError(LogEntry("ContextRequesterInterface", __func__).d("reason", "methodDeprecated"));
            }
            inline void ContextRequesterInterface::onContextAvailable(const EndpointIdentifier& endpointId, const AVSContext& endpointContext,
                                                                      ContextRequestToken requestToken) {
                onContextAvailable(endpointContext.toJson());
            }
            inline void ContextRequesterInterface::onContextFailure(const ContextRequestError error) {
                acsdkError(utils::logger::LogEntry("ContextRequesterInterface", __func__).d("reason", "methodDeprecated"));
            }
            inline void ContextRequesterInterface::onContextFailure(const ContextRequestError error, ContextRequestToken token) {
                onContextFailure(error);
            }
        }
    }
}
#endif