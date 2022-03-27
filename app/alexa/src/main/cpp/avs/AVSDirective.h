#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSDIRECTIVE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSDIRECTIVE_H_

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include "attachment/AttachmentManagerInterface.h"
#include "AVSMessage.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace avs;
            using namespace attachment;
            using namespace utils;
            using namespace sds;
            class AVSDirective : public AVSMessage {
            public:
                enum class ParseStatus {
                    SUCCESS,
                    ERROR_INVALID_JSON,
                    ERROR_MISSING_DIRECTIVE_KEY,
                    ERROR_MISSING_HEADER_KEY,
                    ERROR_MISSING_NAMESPACE_KEY,
                    ERROR_MISSING_NAME_KEY,
                    ERROR_MISSING_MESSAGE_ID_KEY,
                    ERROR_MISSING_PAYLOAD_KEY
                };
                static pair<unique_ptr<AVSDirective>, ParseStatus> create(const string& unparsedDirective, shared_ptr<AttachmentManagerInterface> attachmentManager,
                                                                          const string& attachmentContextId);
                static unique_ptr<AVSDirective> create(const string& unparsedDirective, shared_ptr<AVSMessageHeader> avsMessageHeader, const string& payload,
                                                       shared_ptr<AttachmentManagerInterface> attachmentManager, const string& attachmentContextId,
                                                       const Optional<AVSMessageEndpoint>& endpoint = Optional<AVSMessageEndpoint>());
                unique_ptr<AttachmentReader> getAttachmentReader(const string& contentId, ReaderPolicy readerPolicy) const;
                string getUnparsedDirective() const;
                string getAttachmentContextId() const;
                string messageId() { return getMessageId(); };
                shared_ptr<AVSMessageHeader> m_avsMessageHeader;
                string m_payload;
                Optional<AVSMessageEndpoint> m_endpoint;
            private:
                AVSDirective(const string& unparsedDirective, shared_ptr<AVSMessageHeader> avsMessageHeader, const string& payload,
                             shared_ptr<AttachmentManagerInterface> attachmentManager, const string& attachmentContextId, const Optional<AVSMessageEndpoint>& endpoint);
                const string m_unparsedDirective;
                shared_ptr<AttachmentManagerInterface> m_attachmentManager;
                string m_attachmentContextId;
            };
            inline string avsDirectiveParseStatusToString(AVSDirective::ParseStatus status) {
                switch(status) {
                    case AVSDirective::ParseStatus::SUCCESS: return "SUCCESS";
                    case AVSDirective::ParseStatus::ERROR_INVALID_JSON: return "ERROR_INVALID_JSON";
                    case AVSDirective::ParseStatus::ERROR_MISSING_DIRECTIVE_KEY: return "ERROR_MISSING_DIRECTIVE_KEY";
                    case AVSDirective::ParseStatus::ERROR_MISSING_HEADER_KEY: return "ERROR_MISSING_HEADER_KEY";
                    case AVSDirective::ParseStatus::ERROR_MISSING_NAMESPACE_KEY: return "ERROR_MISSING_NAMESPACE_KEY";
                    case AVSDirective::ParseStatus::ERROR_MISSING_NAME_KEY: return "ERROR_MISSING_NAME_KEY";
                    case AVSDirective::ParseStatus::ERROR_MISSING_MESSAGE_ID_KEY: return "ERROR_MISSING_MESSAGE_ID_KEY";
                    case AVSDirective::ParseStatus::ERROR_MISSING_PAYLOAD_KEY: return "ERROR_MISSING_PAYLOAD_KEY";
                }
                return "UNKNOWN_STATUS";
            }
            inline ostream& operator<<(ostream& stream, const AVSDirective::ParseStatus& status) {
                return stream << avsDirectiveParseStatusToString(status);
            }
        }
    }
}
#endif