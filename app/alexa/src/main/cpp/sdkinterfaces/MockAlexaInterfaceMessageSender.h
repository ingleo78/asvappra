#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKALEXAINTERFACEMESSAGESENDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKALEXAINTERFACEMESSAGESENDER_H_

#include <gmock/gmock.h>
#include <avs/AVSMessageEndpoint.h>
#include "AlexaInterfaceMessageSenderInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                class MockAlexaInterfaceMessageSender : public AlexaInterfaceMessageSenderInterface {
                public:
                    MOCK_METHOD4(sendResponseEvent, bool(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                 const string& jsonPayload));
                    MOCK_METHOD5(sendErrorResponseEvent, bool(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint,
                                 const ErrorResponseType errorType, const string& errorMessage));
                    MOCK_METHOD3(sendDeferredResponseEvent, bool(const string& instance, const string& correlationToken, const int estimatedDeferralInSeconds));
                    MOCK_METHOD1(alexaResponseTypeToErrorType, ErrorResponseType(const AlexaResponseType& responseType));
                };
            }
        }
    }
}
#endif