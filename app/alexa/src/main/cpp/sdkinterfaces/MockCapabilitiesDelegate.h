#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCAPABILITIESDELEGATE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCAPABILITIESDELEGATE_H_

#include <gmock/gmock.h>
#include "CapabilitiesDelegateInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                class MockCapabilitiesDelegate : public CapabilitiesDelegateInterface {
                public:
                    MOCK_METHOD2(addOrUpdateEndpoint, bool(const AVSDiscoveryEndpointAttributes& endpointAttributes,
                                 const vector<CapabilityConfiguration>& capabilities));
                    MOCK_METHOD2(deleteEndpoint, bool(const AVSDiscoveryEndpointAttributes& endpointAttributes,
                                 const vector<CapabilityConfiguration>& capabilities));
                    MOCK_METHOD1(addCapabilitiesObserver, void(shared_ptr<CapabilitiesObserverInterface> observer));
                    MOCK_METHOD1(removeCapabilitiesObserver, void(shared_ptr<CapabilitiesObserverInterface> observer));
                    MOCK_METHOD0(invalidateCapabilities, void());
                    MOCK_METHOD1(setMessageSender, void(const shared_ptr<MessageSenderInterface>& messageSender));
                    MOCK_METHOD1(onAlexaEventProcessedReceived, void(const string& eventCorrelationToken));
                    MOCK_METHOD1(onAVSGatewayChanged, void(const string& avsGateway));
                    MOCK_METHOD2(onConnectionStatusChanged, void(const Status, const ChangedReason));
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKCAPABILITIESDELEGATE_H_
