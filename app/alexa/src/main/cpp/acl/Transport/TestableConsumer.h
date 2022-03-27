#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_TESTABLECONSUMER_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_TESTABLECONSUMER_H_

#include <iostream>
#include <sdkinterfaces/MessageObserverInterface.h>
#include "MessageConsumerInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            class TestableConsumer : public acl::MessageConsumerInterface {
            public:
                void setMessageObserver(std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> observer) {
                    m_messageObserver = observer;
                }
                void consumeMessage(const std::string& contextId, const std::string& message) override {
                    if (m_messageObserver) m_messageObserver->receive(contextId, message);
                }
            private:
                std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface> m_messageObserver;
            };
        }
    }
}
#endif