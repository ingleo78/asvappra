#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTMESSAGESENDER_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_TESTMESSAGESENDER_H_

#include <string>
#include <chrono>
#include <deque>
#include <mutex>
#include <acl/AVSConnectionManager.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/MessageObserverInterface.h>
#include <util/RequiresShutdown.h>

namespace alexaClientSDK {
    namespace integration {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace acl;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            class TestMessageSender : public MessageSenderInterface, public RequiresShutdown {
            public:
                ~TestMessageSender() = default;
                void sendMessage(shared_ptr<MessageRequest> request) override;
                TestMessageSender(shared_ptr<MessageRouterInterface> messageRouter, bool isEnabled,
                                  shared_ptr<ConnectionStatusObserverInterface> connectionStatusObserver,
                                  shared_ptr<MessageObserverInterface> messageObserver);
                class SendParams {
                public:
                    enum class Type { SEND, TIMEOUT };
                    Type type;
                    shared_ptr<MessageRequest> request;
                };
                SendParams waitForNext(const seconds duration);
                void enable();
                void disable();
                bool isEnabled();
                void reconnect();
                void setAVSGateway(const string& avsGateway);
                void addConnectionStatusObserver(shared_ptr<ConnectionStatusObserverInterface> observer);
                void removeConnectionStatusObserver(shared_ptr<ConnectionStatusObserverInterface> observer);
                void addMessageObserver(shared_ptr<MessageObserverInterface> observer);
                void removeMessageObserver(shared_ptr<MessageObserverInterface> observer);
                void doShutdown() override;
                shared_ptr<AVSConnectionManager> getConnectionManager() const;
            private:
                mutex m_mutex;
                condition_variable m_wakeTrigger;
                deque<SendParams> m_queue;
                shared_ptr<AVSConnectionManager> m_connectionManager;
            };
        }
    }
}
#endif