#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTUTILS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTUTILS_H_

#include <memory>
#include <vector>
#include "AttachmentReader.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                class AttachmentUtils {
                public:
                    static std::unique_ptr<AttachmentReader> createAttachmentReader(const std::vector<char>& srcBuffer);
                };
            }
        }
    }
}
#endif