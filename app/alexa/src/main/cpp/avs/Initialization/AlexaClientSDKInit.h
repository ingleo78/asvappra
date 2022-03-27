#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_INITIALIZATION_ALEXACLIENTSDKINIT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_INITIALIZATION_ALEXACLIENTSDKINIT_H_

#include <atomic>
#include <iostream>
#include <memory>
#include <vector>
#include <logger/Logger.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace initialization {
                using namespace std;
                using namespace utils::logger;
                class AlexaClientSDKInit {
                public:
                    static function<shared_ptr<AlexaClientSDKInit>(shared_ptr<Logger>)> getCreateAlexaClientSDKInit(const vector<shared_ptr<istream>>& jsonStreams);
                    ~AlexaClientSDKInit();
                    static bool isInitialized();
                    static bool initialize(const vector<shared_ptr<istream>>& jsonStreams);
                    static void uninitialize();
                private:
                    static std::atomic_int g_isInitialized;
                };
            }
        }
    }
}
#endif