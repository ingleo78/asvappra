#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKDIRECTIVESEQUENCER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKDIRECTIVESEQUENCER_H_

#include <gmock/gmock.h>
#include "DirectiveSequencerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                class MockDirectiveSequencer : public DirectiveSequencerInterface {
                public:
                    MockDirectiveSequencer();
                    MOCK_METHOD1(addDirectiveHandler, bool(shared_ptr<DirectiveHandlerInterface> handler));
                    MOCK_METHOD1(removeDirectiveHandler, bool(shared_ptr<DirectiveHandlerInterface> handler));
                    inline void setDialogRequestId(const string& dialogRequestId) {
                        m_dialogRequestId = dialogRequestId;
                    };
                    inline string getDialogRequestId() {
                        return m_dialogRequestId;
                    };
                    MOCK_METHOD1(onDirective, bool(shared_ptr<AVSDirective> directive));
                    MOCK_METHOD0(doShutdown, void());
                    MOCK_METHOD0(disable, void());
                    MOCK_METHOD0(enable, void());
                    string m_dialogRequestId;
                };
                inline MockDirectiveSequencer::MockDirectiveSequencer() : DirectiveSequencerInterface{"MockDirectiveSequencer"} {}
            }
        }
    }
}
#endif