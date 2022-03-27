#ifndef ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_ACLTESTCONTEXT_H_
#define ALEXA_CLIENT_SDK_INTEGRATION_INCLUDE_INTEGRATION_ACLTESTCONTEXT_H_

#include <memory>
#include <string>
#include <acl/Transport/MessageRouter.h>
#include <avs/attachment/AttachmentManager.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <autorization/CBLAuthDelegate/CBLAuthRequesterInterface.h>
#include <context_manager/ContextManager.h>
#include <registration_manager/CustomerDataManager.h>
#include "AuthDelegateTestContext.h"
#include "ConnectionStatusObserver.h"
#include "SDKTestContext.h"

namespace alexaClientSDK {
    namespace integration {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace registrationManager;
            using namespace acl;
            using namespace avs;
            using namespace attachment;
            class ACLTestContext {
            public:
                static unique_ptr<ACLTestContext> create(const string& filePath, const string& overlay = "");
                ~ACLTestContext();
                shared_ptr<AuthDelegateInterface> getAuthDelegate() const;
                shared_ptr<CustomerDataManager> getCustomerDataManager() const;
                shared_ptr<AttachmentManager> getAttachmentManager() const;
                shared_ptr<MessageRouter> getMessageRouter() const;
                shared_ptr<ConnectionStatusObserver> getConnectionStatusObserver() const;
                shared_ptr<ContextManagerInterface> getContextManager() const;
                void waitForConnected();
                void waitForDisconnected();
            private:
                ACLTestContext(const string& filePath, const string& overlay = "");
                unique_ptr<AuthDelegateTestContext> m_authDelegateTestContext;
                shared_ptr<AttachmentManager> m_attachmentManager;
                shared_ptr<MessageRouter> m_messageRouter;
                shared_ptr<ConnectionStatusObserver> m_connectionStatusObserver;
                shared_ptr<ContextManagerInterface> m_contextManager;
            };
        }
    }
}
#endif