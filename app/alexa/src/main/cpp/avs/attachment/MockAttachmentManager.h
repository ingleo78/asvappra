#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_TEST_AVSCOMMON_AVS_ATTACHMENT_MOCKATTACHMENTMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_TEST_AVSCOMMON_AVS_ATTACHMENT_MOCKATTACHMENTMANAGER_H_

#include <chrono>
#include <string>
#include <memory>
#include <gmock/gmock.h>
#include <avs/attachment/AttachmentManagerInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                namespace test {
                    using namespace std;
                    using namespace chrono;
                    using namespace utils;
                    using namespace sds;
                    class MockAttachmentManager : public AttachmentManagerInterface {
                    public:
                        //MOCK_CONST_METHOD2(generateAttachmentId, string(const string& contextId, const string& contentId));
                        MOCK_METHOD1(setAttachmentTimeoutMinutes, bool(minutes timeoutMinutes));
                        //MOCK_METHOD2(createWriter, unique_ptr<AttachmentWriter>(const string& attachmentId, WriterPolicy policy));
                        //MOCK_METHOD2(createReader, unique_ptr<AttachmentReader>(const string& attachmentId, ReaderPolicy policy));
                    };
                }
            }
        }
    }
}
#endif