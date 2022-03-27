#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXCEPTIONENCOUNTEREDSENDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXCEPTIONENCOUNTEREDSENDERINTERFACE_H_

#include <avs/ExceptionErrorType.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace avs;
            class ExceptionEncounteredSenderInterface {
            public:
                virtual ~ExceptionEncounteredSenderInterface() = default;
                virtual void sendExceptionEncountered(const string& unparsedDirective, ExceptionErrorType error, const string& errorDescription);
            };
        }
    }
}
#endif