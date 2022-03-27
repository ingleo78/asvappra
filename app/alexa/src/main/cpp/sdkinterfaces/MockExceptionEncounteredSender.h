#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKEXCEPTIONENCOUNTEREDSENDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKEXCEPTIONENCOUNTEREDSENDER_H_

#include <gmock/gmock.h>
#include "ExceptionEncounteredSenderInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                class MockExceptionEncounteredSender : public ExceptionEncounteredSenderInterface {
                public:
                    MOCK_METHOD3(sendExceptionEncountered, void(const string& unparsedDirective, ExceptionErrorType error, const string& errorDescription));
                };
            }
        }
    }
}
#endif