#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_SOFTWAREINFOSENDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_SOFTWAREINFOSENDER_H_

#include <cstdint>
#include <future>
#include <memory>
#include <string>
#include <acl/AVSConnectionManager.h>
#include <avs/CapabilityAgent.h>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/SoftwareInfoSenderObserverInterface.h>
#include <util/RequiresShutdown.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace softwareInfo;
            using Status = ConnectionStatusObserverInterface::Status;
            using ChangedReason = ConnectionStatusObserverInterface::ChangedReason;
            class SoftwareInfoSendRequest;
            class SoftwareInfoSender: public CapabilityAgent, public RequiresShutdown, public enable_shared_from_this<SoftwareInfoSender>,
                                      public ConnectionStatusObserverInterface, public SoftwareInfoSenderObserverInterface {
            public:
                static shared_ptr<SoftwareInfoSender> create(FirmwareVersion firmwareVersion, bool sendSoftwareInfoUponConnect,
                                                             unordered_set<shared_ptr<SoftwareInfoSenderObserverInterface>> observers,
                                                             shared_ptr<AVSConnectionManagerInterface> connection,
                                                             shared_ptr<MessageSenderInterface> messageSender,
                                                             shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                bool setFirmwareVersion(FirmwareVersion firmwareVersion);
                DirectiveHandlerConfiguration getConfiguration() const override;
                void handleDirectiveImmediately(shared_ptr<AVSDirective> directive) override;
                void preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
                void doShutdown() override;
                void onConnectionStatusChanged(Status status, ChangedReason reason) override;
                void onFirmwareVersionAccepted(FirmwareVersion firmwareVersion) override;
            private:
                SoftwareInfoSender(FirmwareVersion firmwareVersion, bool sendSoftwareInfoUponConnect,
                                   unordered_set<shared_ptr<SoftwareInfoSenderObserverInterface>> observers,
                                   shared_ptr<AVSConnectionManagerInterface> connection,
                                   shared_ptr<MessageSenderInterface> messageSender,
                                   shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender);
                void removeDirective(shared_ptr<DirectiveInfo> info);
                FirmwareVersion m_firmwareVersion;
                bool m_sendSoftwareInfoUponConnect;
                unordered_set<shared_ptr<SoftwareInfoSenderObserverInterface>> m_observers;
                shared_ptr<AVSConnectionManagerInterface> m_connection;
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<ExceptionEncounteredSenderInterface> m_exceptionEncounteredSender;
                mutex m_mutex;
                Status m_connectionStatus;
                shared_ptr<SoftwareInfoSendRequest> m_clientInitiatedSendRequest;
                shared_ptr<SoftwareInfoSendRequest> m_handleDirectiveSendRequest;
            };
        }
    }
}
#endif