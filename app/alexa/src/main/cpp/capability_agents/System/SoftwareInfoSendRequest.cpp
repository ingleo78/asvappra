#include <string>
#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/EventBuilder.h>
#include <avs/MessageRequest.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include <util/RetryTimer.h>
#include <uuid_generation/UUIDGeneration.h>
#include "SoftwareInfoSender.h"
#include "SoftwareInfoSendRequest.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace logger;
            using namespace json;
            using namespace rapidjson;
            static const string TAG{"SoftwareInfoSendRequest"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE_SYSTEM = "System";
            static const string NAME_SOFTWARE_INFO = "SoftwareInfo";
            static const char FIRMWARE_VERSION_STRING[] = "firmwareVersion";
            static vector<int> RETRY_TABLE = { 1000, 5000, 25000, 1250000 };
            static RetryTimer RETRY_TIMER(RETRY_TABLE);
            shared_ptr<SoftwareInfoSendRequest> SoftwareInfoSendRequest::create(FirmwareVersion firmwareVersion,
                                                                                shared_ptr<MessageSenderInterface> messageSender,
                                                                                shared_ptr<SoftwareInfoSenderObserverInterface> observer) {
                ACSDK_DEBUG5(LX("create").d("firmwareVersion", firmwareVersion));
                if (!isValidFirmwareVersion(firmwareVersion)) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "invalidFirmwareVersion").d("firmwareVersion", firmwareVersion));
                    return nullptr;
                }
                if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "messageSenderNull"));
                    return nullptr;
                }
                return shared_ptr<SoftwareInfoSendRequest>(new SoftwareInfoSendRequest(firmwareVersion, messageSender, observer));
            }
            void SoftwareInfoSendRequest::onSendCompleted(Status status) {
                ACSDK_DEBUG5(LX("onSendCompleted").d("status", status));
                lock_guard<mutex> lock(m_mutex);
                if (Status::SUCCESS == status || Status::SUCCESS_NO_CONTENT == status) {
                    if (m_observer) {
                        m_observer->onFirmwareVersionAccepted(m_firmwareVersion);
                        m_observer.reset();
                    }
                } else {
                    auto& timer = m_retryTimers[m_retryCounter % (sizeof(m_retryTimers) / sizeof(m_retryTimers[0]))];
                    auto delay = RETRY_TIMER.calculateTimeToRetry(m_retryCounter++);
                    ACSDK_INFO(LX("retrySendingSoftwareInfoQueued").d("retry", m_retryCounter).d("delayInMilliseconds", delay.count()));
                    timer.stop();
                    timer.start(delay, [](shared_ptr<SoftwareInfoSendRequest> softwareInfoSendRequest) { softwareInfoSendRequest->send(); },
                                shared_from_this());
                }
            }
            void SoftwareInfoSendRequest::onExceptionReceived(const string& message) {
                ACSDK_DEBUG5(LX("onExceptionReceived").d("message", message));
            }
            void SoftwareInfoSendRequest::doShutdown() {
                ACSDK_DEBUG5(LX("doShutdown"));
                lock_guard<mutex> lock(m_mutex);
                for (auto& timer : m_retryTimers) {
                    timer.stop();
                }
                m_messageSender.reset();
                m_observer.reset();
            }
            SoftwareInfoSendRequest::SoftwareInfoSendRequest(FirmwareVersion firmwareVersion, shared_ptr<MessageSenderInterface> messageSender,
                                                             shared_ptr<SoftwareInfoSenderObserverInterface> observer) :
                    RequiresShutdown("SoftwareInfoSendRequest"),
                    m_firmwareVersion{firmwareVersion},
                    m_messageSender{messageSender},
                    m_observer{observer},
                    m_retryCounter{0} {
            }
            void SoftwareInfoSendRequest::send() {
                ACSDK_DEBUG5(LX("send").d("firmwareVersion", m_firmwareVersion));
                string jsonContent;
                if (!buildJsonForSoftwareInfo(&jsonContent, m_firmwareVersion)) {
                    ACSDK_ERROR(LX("sendFailed").d("reason", "buildJsonForSoftwareInfoFailed"));
                    onSendCompleted(Status::INTERNAL_ERROR);
                    return;
                }
                auto request = make_shared<MessageRequest>(jsonContent);
                request->addObserver(shared_from_this());
                m_messageSender->sendMessage(request);
            }
            bool SoftwareInfoSendRequest::buildJsonForSoftwareInfo(string* jsonContent, FirmwareVersion firmwareVersion) {
                string versionString = to_string(firmwareVersion);
                Document payloadDataDocument(kObjectType);
                Value _FIRMWARE_VERSION_STRING{FIRMWARE_VERSION_STRING, strlen(FIRMWARE_VERSION_STRING)};
                payloadDataDocument.AddMember(_FIRMWARE_VERSION_STRING, versionString, payloadDataDocument.GetAllocator());
                StringBuffer payloadJson;
                rapidjson::Writer<StringBuffer> payloadWriter(payloadJson);
                payloadDataDocument.Accept(payloadWriter);
                string payload = payloadJson.GetString();
                auto msgIdAndJsonEvent = buildJsonEventString(NAMESPACE_SYSTEM, NAME_SOFTWARE_INFO, "", payload, "");
                if (msgIdAndJsonEvent.first.empty()) {
                    ACSDK_ERROR(LX("buildJsonForSoftwareInfoFailed").d("reason", "msgIdEmpty"));
                    return false;
                }
                if (msgIdAndJsonEvent.second.empty()) {
                    ACSDK_ERROR(LX("buildJsonForSoftwareInfoFailed").d("reason", "JsonEventEmpty"));
                    return false;
                }
                *jsonContent = msgIdAndJsonEvent.second;
                return true;
            }
        }
    }
}