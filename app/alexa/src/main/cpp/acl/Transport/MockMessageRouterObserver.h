#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMESSAGEROUTEROBSERVER_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMESSAGEROUTEROBSERVER_H_

#include <gmock/gmock.h>
#include <memory>
#include "MessageRouterObserverInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace transport {
            namespace test {
                using namespace std;
                using namespace avsCommon::sdkInterfaces;
                class MockMessageRouterObserver : public MessageRouterObserverInterface {
                public:
                    void reset() {
                        notifiedOfReceive = false;
                        notifiedOfStatusChanged = false;
                    }
                    bool wasNotifiedOfStatusChange() {
                        return notifiedOfStatusChanged;
                    }
                    bool wasNotifiedOfReceive() {
                        return notifiedOfReceive;
                    }
                    ConnectionStatusObserverInterface::Status getLatestConnectionStatus() {
                        return m_status;
                    }
                    ConnectionStatusObserverInterface::ChangedReason getLatestConnectionChangedReason() {
                        return m_reason;
                    }
                    string getLatestMessage() {
                        return m_message;
                    }
                    string getAttachmentContextId() {
                        return m_attachmentContextId;
                    }
                private:
                    virtual void onConnectionStatusChanged(ConnectionStatusObserverInterface::Status status,
                                                           const ConnectionStatusObserverInterface::ChangedReason reason) override {
                        notifiedOfStatusChanged = true;
                        m_status = status;
                        m_reason = reason;
                    }
                    virtual void receive(const string& contextId, const string& message) override {
                        notifiedOfReceive = true;
                        m_attachmentContextId = contextId;
                        m_message = message;
                    }
                    ConnectionStatusObserverInterface::Status m_status;
                    ConnectionStatusObserverInterface::ChangedReason m_reason;
                    string m_attachmentContextId;
                    string m_message;
                    bool notifiedOfStatusChanged;
                    bool notifiedOfReceive;
                };
            }
        }
    }
}
#endif