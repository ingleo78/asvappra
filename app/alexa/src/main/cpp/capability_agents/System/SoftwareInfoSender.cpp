#include <string>
#include <json/document.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "SoftwareInfoSender.h"
#include "SoftwareInfoSendRequest.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace acl;
            using namespace json;
            using namespace logger;
            using namespace rapidjson;
            static const string TAG{"SoftwareInfoSender"};
            #define LX(event) LogEntry(TAG, event)
            static const string NAMESPACE_SYSTEM = "System";
            static const NamespaceAndName REPORT_SOFTWARE_INFO(NAMESPACE_SYSTEM, "ReportSoftwareInfo");
            shared_ptr<SoftwareInfoSender> SoftwareInfoSender::create(FirmwareVersion firmwareVersion, bool sendSoftwareInfoUponConnect,
                                                                      unordered_set<shared_ptr<SoftwareInfoSenderObserverInterface>> observers,
                                                                      shared_ptr<AVSConnectionManagerInterface> connection,
                                                                      shared_ptr<MessageSenderInterface> messageSender,
                                                                      shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) {
                ACSDK_DEBUG5(LX("create"));
                if (firmwareVersion <= 0) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "invalidFirmwareVersion").d("firmwareVersion", firmwareVersion));
                    return nullptr;
                }
                for (auto observer : observers) {
                    if (!observer) {
                        ACSDK_ERROR(LX("createFailed").d("reason", "nullSoftwareInfoSenderObserver").d("return", "nullptr"));
                        return nullptr;
                    }
                }
                if (!connection) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullConnection"));
                    return nullptr;
                }
                if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                }
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncounteredSender"));
                    return nullptr;
                }
                auto result = shared_ptr<SoftwareInfoSender>(new SoftwareInfoSender(firmwareVersion, sendSoftwareInfoUponConnect, observers,
                                                             connection, messageSender, exceptionEncounteredSender));
                connection->addConnectionStatusObserver(result);
                return result;
            }
            bool SoftwareInfoSender::setFirmwareVersion(FirmwareVersion firmwareVersion) {
                ACSDK_DEBUG5(LX("setFirmwareVersion").d("firmwareVersion", firmwareVersion));
                if (!isValidFirmwareVersion(firmwareVersion)) {
                    ACSDK_ERROR(LX("setFirmwareVersion").d("reason", "invalidFirmwareVersion").d("firmwareVersion", firmwareVersion));
                    return false;
                }
                shared_ptr<SoftwareInfoSendRequest> previousSendRequest;
                shared_ptr<SoftwareInfoSendRequest> newSendRequest;
                bool result = true;
                {
                    lock_guard<mutex> lock(m_mutex);
                    if (firmwareVersion == m_firmwareVersion) return result;
                    m_firmwareVersion = firmwareVersion;
                    if (Status::CONNECTED == m_connectionStatus) {
                        newSendRequest = SoftwareInfoSendRequest::create(m_firmwareVersion, m_messageSender, shared_from_this());
                        if (newSendRequest) {
                            previousSendRequest = m_clientInitiatedSendRequest;
                            m_clientInitiatedSendRequest = newSendRequest;
                        } else result = false;
                    } else {
                        m_sendSoftwareInfoUponConnect = true;
                        previousSendRequest = m_clientInitiatedSendRequest;
                        m_clientInitiatedSendRequest.reset();
                    }
                }
                if (previousSendRequest) {
                    ACSDK_INFO(LX("cancellingPreviousClientInitiatedSendRequest"));
                    previousSendRequest->shutdown();
                }
                if (newSendRequest) newSendRequest->send();
                return result;
            }
            DirectiveHandlerConfiguration SoftwareInfoSender::getConfiguration() const {
                ACSDK_DEBUG5(LX("getConfiguration"));
                static DirectiveHandlerConfiguration configuration = {{REPORT_SOFTWARE_INFO,BlockingPolicy(BlockingPolicy::MEDIUMS_NONE,false)}};
                return configuration;
            }
            void SoftwareInfoSender::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                handleDirective(make_shared<CapabilityAgent::DirectiveInfo>(directive, nullptr));
            }
            void SoftwareInfoSender::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            void SoftwareInfoSender::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullInfo"));
                    return;
                }
                ACSDK_DEBUG5(LX("handleDirective").d("messageId", info->directive->getMessageId()));
                if (info->directive->getNamespace() != REPORT_SOFTWARE_INFO.nameSpace ||
                    info->directive->getName() != REPORT_SOFTWARE_INFO.name) {
                    sendExceptionEncounteredAndReportFailed(info, "Unsupported operation", ExceptionErrorType::UNSUPPORTED_OPERATION);
                    return;
                }
                shared_ptr<SoftwareInfoSendRequest> previousSendRequest;
                shared_ptr<SoftwareInfoSendRequest> newSendRequest;
                {
                    lock_guard<mutex> lock(m_mutex);
                    if (m_firmwareVersion == INVALID_FIRMWARE_VERSION) {
                        sendExceptionEncounteredAndReportFailed(info, "NoFirmwareVersion", ExceptionErrorType::INTERNAL_ERROR);
                        return;
                    }
                    newSendRequest = SoftwareInfoSendRequest::create(m_firmwareVersion, m_messageSender, shared_from_this());
                    if (newSendRequest) {
                        previousSendRequest = m_handleDirectiveSendRequest;
                        m_handleDirectiveSendRequest = newSendRequest;
                    } else {
                        sendExceptionEncounteredAndReportFailed(info, "sendFirmwareVersionFailed", ExceptionErrorType::INTERNAL_ERROR);
                    }
                }
                if (previousSendRequest) {
                    ACSDK_INFO(LX("cancellingPreviousHandleDirectiveSendRequest"));
                    previousSendRequest->shutdown();
                }
                if (newSendRequest) newSendRequest->send();
                if (info->result) info->result->setCompleted();
                removeDirective(info);
            }
            void SoftwareInfoSender::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "nullInfo"));
                    return;
                }
                ACSDK_DEBUG5(LX("cancelDirective").d("messageId", info->directive->getMessageId()));
                shared_ptr<SoftwareInfoSendRequest> outstandingRequest;
                {
                    lock_guard<mutex> lock(m_mutex);
                    if (!m_handleDirectiveSendRequest) return;
                    outstandingRequest = m_handleDirectiveSendRequest;
                    m_handleDirectiveSendRequest.reset();
                }
                if (outstandingRequest) outstandingRequest->shutdown();
            }
            void SoftwareInfoSender::onConnectionStatusChanged(Status status, ChangedReason reason) {
                ACSDK_DEBUG5(LX("onConnectionStatusChanged").d("status", status).d("reason", reason));
                shared_ptr<SoftwareInfoSendRequest> newSendRequest;
                {
                    lock_guard<mutex> lock(m_mutex);
                    if (status == m_connectionStatus) return;
                    m_connectionStatus = status;
                    if (!m_sendSoftwareInfoUponConnect || status != ConnectionStatusObserverInterface::Status::CONNECTED) return;
                    newSendRequest = SoftwareInfoSendRequest::create(m_firmwareVersion, m_messageSender, shared_from_this());
                    m_sendSoftwareInfoUponConnect = false;
                    if (newSendRequest) m_clientInitiatedSendRequest = newSendRequest;
                    else { ACSDK_ERROR(LX("onConnectionStatusChangedFailed").d("reason", "failedToCreateOnConnectSendRequest")); }
                }
                if (newSendRequest) newSendRequest->send();
            }
            void SoftwareInfoSender::doShutdown() {
                ACSDK_DEBUG5(LX("shutdown"));
                unordered_set<std::shared_ptr<SoftwareInfoSenderObserverInterface>> localObservers;
                shared_ptr<AVSConnectionManagerInterface> localConnection;
                shared_ptr<MessageSenderInterface> localMessageSender;
                shared_ptr<ExceptionEncounteredSenderInterface> localExceptionEncounteredSender;
                shared_ptr<SoftwareInfoSendRequest> localClientInitiatedSendRequest;
                shared_ptr<SoftwareInfoSendRequest> localHandleDirectiveSendRequest;
                {
                    lock_guard<mutex> lock(m_mutex);
                    m_sendSoftwareInfoUponConnect = false;
                    swap(m_observers, localObservers);
                    swap(m_connection, localConnection);
                    swap(m_messageSender, localMessageSender);
                    swap(m_exceptionEncounteredSender, localExceptionEncounteredSender);
                    swap(m_clientInitiatedSendRequest, localClientInitiatedSendRequest);
                    swap(m_handleDirectiveSendRequest, localHandleDirectiveSendRequest);
                }
                localConnection->removeConnectionStatusObserver(shared_from_this());
                if (localClientInitiatedSendRequest) localClientInitiatedSendRequest->shutdown();
                if (localHandleDirectiveSendRequest) localHandleDirectiveSendRequest->shutdown();
            }
            void SoftwareInfoSender::onFirmwareVersionAccepted(FirmwareVersion firmwareVersion) {
                unique_lock<mutex> lock(m_mutex);
                auto localObservers = m_observers;
                lock.unlock();
                for (const auto& localObserver : localObservers) {
                    localObserver->onFirmwareVersionAccepted(firmwareVersion);
                }
            }
            SoftwareInfoSender::SoftwareInfoSender(FirmwareVersion firmwareVersion, bool sendSoftwareInfoUponConnect,
                                                   unordered_set<shared_ptr<SoftwareInfoSenderObserverInterface>> observers,
                                                   shared_ptr<AVSConnectionManagerInterface> connection,
                                                   shared_ptr<MessageSenderInterface> messageSender,
                                                   shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender) :
                                                   CapabilityAgent{NAMESPACE_SYSTEM, exceptionEncounteredSender},
                                                   RequiresShutdown{"SoftwareInfoSender"}, m_firmwareVersion{firmwareVersion},
                                                   m_sendSoftwareInfoUponConnect{sendSoftwareInfoUponConnect}, m_observers{observers},
                                                   m_connection{connection}, m_messageSender{messageSender},
                                                   m_exceptionEncounteredSender{exceptionEncounteredSender},
                                                   m_connectionStatus{Status::DISCONNECTED} {
                ACSDK_DEBUG5(LX("SoftwareInfoSender"));
            }
            void SoftwareInfoSender::removeDirective(shared_ptr<DirectiveInfo> info) {
                ACSDK_DEBUG5(LX("removeDirective"));
                if (info && info->result) CapabilityAgent::removeDirective(info->directive->getMessageId());
            }
        }
    }
}