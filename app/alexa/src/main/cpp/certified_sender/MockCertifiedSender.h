#ifndef ALEXA_CLIENT_SDK_CERTIFIEDSENDER_TEST_MOCKCERTIFIEDSENDER_H_
#define ALEXA_CLIENT_SDK_CERTIFIEDSENDER_TEST_MOCKCERTIFIEDSENDER_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <sdkinterfaces/MockAVSConnectionManager.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <certified_sender/CertifiedSender.h>
#include <registration_manager/CustomerDataHandler.h>
#include "MockMessageStorage.h"

namespace alexaClientSDK {
    namespace certifiedSender {
        namespace test {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace registrationManager;
            using namespace sdkInterfaces::test;
            class MockCertifiedSender {
            public:
                MockCertifiedSender();
                shared_ptr<CertifiedSender> get();
                shared_ptr<MockMessageSender> getMockMessageSender();
            private:
                shared_ptr<CertifiedSender> m_certifiedSender;
                shared_ptr<MockMessageSender> m_mockMessageSender;
                shared_ptr<MockMessageStorage> m_mockMessageStorage;
                shared_ptr<MockAVSConnectionManager> m_mockAVSConnectionManager;
                shared_ptr<CustomerDataManager> m_customerDataManager;
            };
        }
    }
}
#endif