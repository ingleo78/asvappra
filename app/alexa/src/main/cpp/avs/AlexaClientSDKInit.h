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
                class AlexaClientSDKInit {
                    public:
                        static std::function<std::shared_ptr<AlexaClientSDKInit>(std::shared_ptr<utils::logger::Logger>)>
                        getCreateAlexaClientSDKInit(const std::vector<std::shared_ptr<std::istream>>& jsonStreams);
                        ~AlexaClientSDKInit();
                        static bool isInitialized();
                        static bool initialize(const std::vector<std::shared_ptr<std::istream>>& jsonStreams);
                        static void uninitialize();
                    private:
                        static std::atomic_int g_isInitialized;
                };
            }
        }
    }
}
#endif