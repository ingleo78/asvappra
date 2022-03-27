#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTEXCEPTIONENCOUNTEREDSENDER_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTEXCEPTIONENCOUNTEREDSENDER_H_

#include <condition_variable>
#include <string>
#include <future>
#include <fstream>
#include <chrono>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <avs/attachment/AttachmentManager.h>
#include <json/JSONUtils.h>

using namespace alexaClientSDK::avsCommon;
namespace alexaClientSDK {
    namespace integration {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace sdkInterfaces;
            using namespace avs;
            using namespace attachment;
            class TestExceptionEncounteredSender : public ExceptionEncounteredSenderInterface {
            public:
                void sendExceptionEncountered(const string& unparsedDirective, ExceptionErrorType error, const string& message) override;
                shared_ptr<AVSDirective> parseDirective(const string& rawJSON, shared_ptr<AttachmentManager> attachmentManager);
                class ExceptionParams {
                public:
                    ExceptionParams();
                    enum class Type {
                        UNSET,
                        EXCEPTION,
                        TIMEOUT
                    };
                    Type type;
                    shared_ptr<AVSDirective> directive;
                    string exceptionUnparsedDirective;
                    ExceptionErrorType exceptionError;
                    string exceptionMessage;
                };
                ExceptionParams waitForNext(const seconds duration);
            private:
                mutex m_mutex;
                condition_variable m_wakeTrigger;
                deque<ExceptionParams> m_queue;
            };
        }
    }
}
#endif