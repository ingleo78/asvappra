#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ATTACHMENT_ATTACHMENTMANAGER_H_

#include <mutex>
#include <unordered_map>
#include "AttachmentManagerInterface.h"
#include "AttachmentReader.h"
#include "AttachmentWriter.h"
#include "Attachment.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                using namespace avsCommon;
                using namespace utils;
                using namespace sds;
                class AttachmentManager : public AttachmentManagerInterface {
                public:
                    static constexpr minutes ATTACHMENT_MANAGER_TIMOUT_MINUTES_DEFAULT = hours(12);
                    static constexpr minutes ATTACHMENT_MANAGER_TIMOUT_MINUTES_MINIMUM = minutes(1);
                    enum class AttachmentType {
                        IN_PROCESS
                    };
                    AttachmentManager(AttachmentType attachmentType);
                    string generateAttachmentId(const string& contextId, const string& contentId) const override;
                    bool setAttachmentTimeoutMinutes(minutes timeoutMinutes) override;
                    unique_ptr<AttachmentWriter> createWriter(const string& attachmentId, WriterPolicy policy = WriterPolicy::ALL_OR_NOTHING);
                    unique_ptr<AttachmentReader> createReader(const string& attachmentId, ReaderPolicy policy);
                private:
                    struct AttachmentManagementDetails {
                        AttachmentManagementDetails();
                        steady_clock::time_point creationTime;
                        unique_ptr<Attachment> attachment;
                    };
                    AttachmentManagementDetails& getDetailsLocked(const string& attachmentId);
                    void removeExpiredAttachmentsLocked();
                    AttachmentType m_attachmentType;
                    minutes m_attachmentExpirationMinutes;
                    mutex m_mutex;
                    unordered_map<string, AttachmentManagementDetails> m_attachmentDetailsMap;
                };
            }
        }
    }
}
#endif