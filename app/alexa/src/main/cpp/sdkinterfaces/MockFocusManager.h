#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKFOCUSMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKFOCUSMANAGER_H_

#include <gmock/gmock.h>
#include "FocusManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                using namespace std;
                using namespace avs;
                class MockFocusManager : public FocusManagerInterface {
                public:
                    MOCK_METHOD3(acquireCh/annel, bool(const string& channelName, shared_ptr<ChannelObserverInterface> channelObserver, const string& interfaceName));
                    //MOCK_METHOD2(acquireChannel, bool(const string& channelName, shared_ptr<FocusManagerInterface::Activity> channelActivity));
                    //MOCK_METHOD2(releaseChannel, future<bool>(const string& channelName, shared_ptr<ChannelObserverInterface> channelObserver));
                    MOCK_METHOD0(stopForegroundActivity, void());
                    MOCK_METHOD1(addObserver, void(const shared_ptr<FocusManagerObserverInterface>& observer));
                    MOCK_METHOD1(removeObserver, void(const shared_ptr<FocusManagerObserverInterface>& observer));
                    MOCK_METHOD0(stopAllActivities, void());
                    MOCK_METHOD3(modifyContentType, void(const string&, const string&, ContentType));
                };
            }
        }
    }
}
#endif