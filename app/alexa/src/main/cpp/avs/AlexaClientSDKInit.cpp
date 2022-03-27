#include "curl/curl.h"
#include "avs/AlexaClientSDKInit.h"
#include "configuration/ConfigurationNode.h"
#include "logger/Logger.h"
#include "util/SDKVersion.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace initialization {
                static const std::string TAG("AlexaClientSdkInit");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                std::atomic_int AlexaClientSDKInit::g_isInitialized{0};
                std::function<std::shared_ptr<AlexaClientSDKInit>(std::shared_ptr<utils::logger::Logger>)> AlexaClientSDKInit::
                    getCreateAlexaClientSDKInit(const std::vector<std::shared_ptr<std::istream>>& jsonStreams) {
                    return [jsonStreams](std::shared_ptr<utils::logger::Logger> logger) -> std::shared_ptr<AlexaClientSDKInit> {
                        if (!logger) {
                            ACSDK_ERROR(LX("getCreateAlexaClientSDKInitFailed").d("reason", "nullLogger"));
                            return nullptr;
                        }
                        if (!initialize(jsonStreams)) {
                            ACSDK_ERROR(LX("getCreateAlexaClientSDKInitFailed").d("reason", "initializeFailed"));
                            return nullptr;
                        }
                        return std::shared_ptr<AlexaClientSDKInit>(new AlexaClientSDKInit);
                    };
                }
                AlexaClientSDKInit::~AlexaClientSDKInit() { uninitialize(); }
                bool AlexaClientSDKInit::isInitialized() { return g_isInitialized > 0; }
                bool AlexaClientSDKInit::initialize(const std::vector<std::shared_ptr<std::istream>>& jsonStreams) {
                    ACSDK_INFO(LX(__func__).d("sdkversion", avsCommon::utils::sdkVersion::getCurrentVersion()));
                    if (!(curl_version_info(CURLVERSION_NOW)->features & CURL_VERSION_HTTP2)) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "curlDoesNotSupportHTTP2"));
                        return false;
                    }
                    if (!utils::configuration::ConfigurationNode::initialize(jsonStreams)) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "ConfigurationNode::initializeFailed"));
                        return false;
                    }
                    if (CURLE_OK != curl_global_init(CURL_GLOBAL_ALL)) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "curl_global_initFailed"));
                        utils::configuration::ConfigurationNode::uninitialize();
                        return false;
                    }
                    g_isInitialized++;
                    return true;
                }
                void AlexaClientSDKInit::uninitialize() {
                    if (0 == g_isInitialized) {
                        ACSDK_ERROR(LX("initializeError").d("reason", "notInitialized"));
                        return;
                    }
                    g_isInitialized--;
                    curl_global_cleanup();
                    utils::configuration::ConfigurationNode::uninitialize();
                }
            }
        }
    }
}
