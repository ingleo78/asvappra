#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKAUTHDELEGATE_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKAUTHDELEGATE_H_

#include <sdkinterfaces/AuthObserverInterface.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            class MockAuthDelegate : public AuthDelegateInterface {
            public:
                MOCK_METHOD1(addAuthObserver, void(shared_ptr<AuthObserverInterface>));
                MOCK_METHOD1(removeAuthObserver, void(shared_ptr<AuthObserverInterface>));
                string getAuthToken();
                MOCK_METHOD1(onAuthFailure, void(const string& token));
                void setAuthToken(string authToken);
            private:
                string m_authToken;
            };
            inline string MockAuthDelegate::getAuthToken() {
                return m_authToken;
            }
            inline void MockAuthDelegate::setAuthToken(string authToken) {
                m_authToken = authToken;
            }
        }
    }
}
#endif