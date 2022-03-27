#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_TEST_MOCKDISCOVERYEVENTSENDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_TEST_MOCKDISCOVERYEVENTSENDER_H_

#include "DiscoveryEventSenderInterface.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            class MockDiscoveryEventSender : public DiscoveryEventSenderInterface {
            public:
                MOCK_METHOD0(stop, void());
                MOCK_METHOD1(sendDiscoveryEvents, bool(const shared_ptr<MessageSenderInterface>&));
                MOCK_METHOD1(addDiscoveryStatusObserver, void(const shared_ptr<DiscoveryStatusObserverInterface>&));
                MOCK_METHOD1(removeDiscoveryStatusObserver, void(const shared_ptr<DiscoveryStatusObserverInterface>&));
                MOCK_METHOD1(onAlexaEventProcessedReceived, void(const string&));
                //MOCK_METHOD2(onAuthStateChange, void(AuthObserverInterface::State, AuthObserverInterface::Error));
            };
        }
    }
}
#endif