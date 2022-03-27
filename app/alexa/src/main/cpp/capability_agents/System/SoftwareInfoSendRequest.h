#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_SOFTWAREINFOSENDREQUEST_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SYSTEM_INCLUDE_SYSTEM_SOFTWAREINFOSENDREQUEST_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/RequiresShutdown.h>
#include <timing/Timer.h>
#include "SoftwareInfoSender.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace timing;
            class SoftwareInfoSendRequest : public MessageRequestObserverInterface, public RequiresShutdown,
                                            public enable_shared_from_this<SoftwareInfoSendRequest> {
            public:
                static shared_ptr<SoftwareInfoSendRequest> create(FirmwareVersion firmwareVersion, shared_ptr<MessageSenderInterface> messageSender,
                                                                  shared_ptr<SoftwareInfoSenderObserverInterface> observer);
                void send();
                void onSendCompleted(MessageRequestObserverInterface::Status status) override;
                void onExceptionReceived(const string& message) override;
                void doShutdown() override;
            private:
                SoftwareInfoSendRequest(FirmwareVersion firmwareVersion, shared_ptr<MessageSenderInterface> messageSender,
                                        shared_ptr<SoftwareInfoSenderObserverInterface> observer);
                static bool buildJsonForSoftwareInfo(string* jsonContent, FirmwareVersion firmwareVersion);
                const FirmwareVersion m_firmwareVersion;
                shared_ptr<MessageSenderInterface> m_messageSender;
                shared_ptr<SoftwareInfoSenderObserverInterface> m_observer;
                mutex m_mutex;
                int m_retryCounter;
                Timer m_retryTimers[2];
            };
        }
    }
}
#endif