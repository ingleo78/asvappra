#include <vector>
#include <util/sds/ReaderPolicy.h>
#include <util/sds/WriterPolicy.h>
#include <logger/Logger.h>
#include <memory/Memory.h>
#include "InProcessAttachment.h"
#include "AttachmentManager.h"
#include "AttachmentReader.h"
#include "AttachmentWriter.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                using namespace std;
                using namespace chrono;
                using namespace avsCommon;
                using namespace utils;
                using namespace logger;
                using namespace sds;
                static const string TAG("AttachmentManager");
                #define LX(event) LogEntry(TAG, event)
                constexpr minutes AttachmentManager::ATTACHMENT_MANAGER_TIMOUT_MINUTES_DEFAULT;
                constexpr minutes AttachmentManager::ATTACHMENT_MANAGER_TIMOUT_MINUTES_MINIMUM;
                static const string ATTACHMENT_ID_COMBINING_SUBSTRING = ":";
                AttachmentManager::AttachmentManagementDetails::AttachmentManagementDetails() : creationTime{steady_clock::now()} { }
                AttachmentManager::AttachmentManager(AttachmentType attachmentType) : m_attachmentType{attachmentType},
                                                     m_attachmentExpirationMinutes{ATTACHMENT_MANAGER_TIMOUT_MINUTES_DEFAULT} { }
                string AttachmentManager::generateAttachmentId(const string& contextId, const string& contentId) const {
                    if (contextId.empty() && contentId.empty()) {
                        ACSDK_ERROR(LX("generateAttachmentIdFailed").d("reason", "contextId and contentId are empty").d("result", "empty string"));
                        return "";
                    }
                    if (contextId.empty()) {
                        ACSDK_WARN(LX("generateAttachmentIdWarning").d("reason", "contextId is empty").d("result", "contentId"));
                        return contentId;
                    }
                    if (contentId.empty()) {
                        ACSDK_WARN(LX("generateAttachmentIdWarning").d("reason", "contentId is empty").d("result", "contextId"));
                        return contextId;
                    }
                    return contextId + ATTACHMENT_ID_COMBINING_SUBSTRING + contentId;
                }
                bool AttachmentManager::setAttachmentTimeoutMinutes(minutes minutes) {
                    if (minutes < ATTACHMENT_MANAGER_TIMOUT_MINUTES_MINIMUM) {
                        int minimumMinutes = ATTACHMENT_MANAGER_TIMOUT_MINUTES_MINIMUM.count();
                        string minutePrintString = (1 == minimumMinutes) ? " minute" : " minutes";
                        ACSDK_ERROR(LX("setAttachmentTimeoutError").d("reason", "timeout parameter less than minimum value")
                                        .d("attemptedSetting", to_string(minimumMinutes) + minutePrintString));
                        return false;
                    }
                    lock_guard<mutex> lock(m_mutex);
                    m_attachmentExpirationMinutes = minutes;
                    return true;
                }
                AttachmentManager::AttachmentManagementDetails& AttachmentManager::getDetailsLocked(const string& attachmentId) {
                    auto& details = m_attachmentDetailsMap[attachmentId];
                    if (!details.attachment) {
                        switch (m_attachmentType) {
                            case AttachmentType::IN_PROCESS:
                                details.attachment = memory::make_unique<InProcessAttachment>(attachmentId);
                                break;
                        }
                        if (!details.attachment) ACSDK_ERROR(LX("getDetailsLockedError").d("reason", "Unsupported attachment type"));
                    }
                    return details;
                }
                unique_ptr<AttachmentWriter> AttachmentManager::createWriter(const string& attachmentId, WriterPolicy policy) {
                    lock_guard<mutex> lock(m_mutex);
                    auto& details = getDetailsLocked(attachmentId);
                    if (!details.attachment) {
                        ACSDK_ERROR(LX("createWriterFailed").d("reason", "Could not access attachment"));
                        return nullptr;
                    }
                    auto writer = details.attachment->createWriter(policy);
                    removeExpiredAttachmentsLocked();
                    return writer;
                }
                unique_ptr<AttachmentReader> AttachmentManager::createReader(const string& attachmentId, ReaderPolicy policy) {
                    lock_guard<mutex> lock(m_mutex);
                    auto& details = getDetailsLocked(attachmentId);
                    if (!details.attachment) {
                        ACSDK_ERROR(LX("createWriterFailed").d("reason", "Could not access attachment"));
                        return nullptr;
                    }
                    auto reader = details.attachment->createReader(policy);
                    removeExpiredAttachmentsLocked();
                    return reader;
                }
                void AttachmentManager::removeExpiredAttachmentsLocked() {
                    vector<string> idsToErase;
                    auto now = steady_clock::now();
                    for (auto& iter : m_attachmentDetailsMap) {
                        auto& details = iter.second;
                        auto attachmentLifetime = duration_cast<minutes>(now - details.creationTime);
                        if ((details.attachment->hasCreatedReader() && details.attachment->hasCreatedWriter()) ||
                            attachmentLifetime > m_attachmentExpirationMinutes) {
                            idsToErase.push_back(iter.first);
                        }
                    }
                    for (auto id : idsToErase) {
                        m_attachmentDetailsMap.erase(id);
                    }
                }
            }
        }
    }
}