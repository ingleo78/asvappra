#ifndef ALEXA_CLIENT_SDK_ADSL_TEST_ADSL_MOCKDIRECTIVESEQUENCER_H_
#define ALEXA_CLIENT_SDK_ADSL_TEST_ADSL_MOCKDIRECTIVESEQUENCER_H_

#include <avs/AVSDirective.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>

namespace alexaClientSDK {
    namespace adsl {
        namespace test {
            class MockDirectiveSequencer : public avsCommon::sdkInterfaces::DirectiveSequencerInterface {
            public:
                MockDirectiveSequencer();
                MOCK_METHOD0(doShutdown, void());
                MOCK_METHOD1(addDirectiveHandler, bool(std::shared_ptr<avsCommon::sdkInterfaces::DirectiveHandlerInterface> handler));
                MOCK_METHOD1(removeDirectiveHandler, bool(std::shared_ptr<avsCommon::sdkInterfaces::DirectiveHandlerInterface> handler));
                inline void setDialogRequestId(const std::string& dialogRequestId) {
                    m_dialogRequestId = dialogRequestId;
                };
                inline std::string getDialogRequestId() {
                    return m_dialogRequestId;
                };
                MOCK_METHOD1(onDirective, bool(std::shared_ptr<avsCommon::avs::AVSDirective> directive));
                MOCK_METHOD0(disable, void());
                MOCK_METHOD0(enable, void());
                std::string m_dialogRequestId;
            };
            inline MockDirectiveSequencer::MockDirectiveSequencer() :
                    avsCommon::sdkInterfaces::DirectiveSequencerInterface{"MockDirectiveSequencer"} {
            }
        }
    }
}
#endif