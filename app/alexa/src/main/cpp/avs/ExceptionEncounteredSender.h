#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EXCEPTIONENCOUNTEREDSENDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EXCEPTIONENCOUNTEREDSENDER_H_

#include <memory>
#include <string>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <threading/Executor.h>
#include "ExceptionErrorType.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace sdkInterfaces;
            class ExceptionEncounteredSender : public ExceptionEncounteredSenderInterface {
            public:
                static unique_ptr<ExceptionEncounteredSender> create(shared_ptr<MessageSenderInterface> messageSender);
                void sendExceptionEncountered(const string& unparsedDirective, ExceptionErrorType error, const string& errorDescription) override;
            private:
                ExceptionEncounteredSender(shared_ptr<MessageSenderInterface> messageSender);
                shared_ptr<MessageSenderInterface> m_messageSender;
            };
        }
    }
}
#endif