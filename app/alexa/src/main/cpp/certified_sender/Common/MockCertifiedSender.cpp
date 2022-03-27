#include <certified_sender/MockCertifiedSender.h>

namespace alexaClientSDK {
    namespace certifiedSender {
        namespace test {
            using namespace avs;
            using namespace utils;
            using namespace testing;
            using namespace certifiedSender::test;
            MockCertifiedSender::MockCertifiedSender() {
                m_mockMessageSender = make_shared<NiceMock<MockMessageSender>>();
                m_mockAVSConnectionManager = make_shared<NiceMock<MockAVSConnectionManager>>();
                m_customerDataManager = make_shared<CustomerDataManager>();
                //m_mockMessageStorage = make_shared<NiceMock<MockMessageStorage>>();
                ON_CALL(*(m_mockMessageStorage), createDatabase()).WillByDefault(Return(true));
                ON_CALL(*(m_mockMessageStorage), open()).WillByDefault(Return(true));
                ON_CALL(*(m_mockMessageStorage), load(_)).WillByDefault(Return(true));
                ON_CALL(*(m_mockMessageStorage), erase(_)).WillByDefault(Return(true));
                //ON_CALL(*(m_mockMessageStorage), store(_, _)).WillByDefault(Return(true));
                ON_CALL(*(m_mockMessageStorage), store(_, _, _)).WillByDefault(Return(true));
                m_certifiedSender = CertifiedSender::create(m_mockMessageSender, m_mockAVSConnectionManager,
                                                            m_mockMessageStorage, m_customerDataManager);
            }
            shared_ptr<CertifiedSender> MockCertifiedSender::get() {
                return m_certifiedSender;
            }
            shared_ptr<MockMessageSender> MockCertifiedSender::getMockMessageSender() {
                return m_mockMessageSender;
            }
        }
    }
}