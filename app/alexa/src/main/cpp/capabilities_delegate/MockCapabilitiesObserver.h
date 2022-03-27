#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_TEST_MOCKCAPABILITIESOBSERVER_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_TEST_MOCKCAPABILITIESOBSERVER_H_

#include <sdkinterfaces/CapabilitiesObserverInterface.h>

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            class MockCapabilitiesObserver : public CapabilitiesObserverInterface {
            public:
                MOCK_METHOD4(onCapabilitiesStateChange, void(CapabilitiesObserverInterface::State, CapabilitiesObserverInterface::Error, const vector<string>&,
                             const vector<string>&));
            };
        }
    }
}
#endif