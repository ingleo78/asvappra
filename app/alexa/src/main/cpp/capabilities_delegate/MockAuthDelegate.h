#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_TEST_MOCKAUTHDELEGATE_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_TEST_MOCKAUTHDELEGATE_H_

#include <memory>
#include <string>
#include <sdkinterfaces/AuthDelegateInterface.h>

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            class MockAuthDelegate : public AuthDelegateInterface {
            public:
                MOCK_METHOD1(addAuthObserver, void(shared_ptr<AuthObserverInterface>));
                MOCK_METHOD1(removeAuthObserver, void(shared_ptr<AuthObserverInterface>));
                MOCK_METHOD0(getAuthToken, string());
                MOCK_METHOD1(onAuthFailure, void (const string&));
            };
        }
    }
}
#endif