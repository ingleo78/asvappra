#include <avs/ComponentConfiguration.h>
#include <logger/Logger.h>
#include <util/SDKVersion.h>
#include "SDKComponent.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace SDKComponent {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace sdkVersion;
            static const string SDK_COMPONENT_NAME = "com.amazon.sdk";
            static const string TAG("SDKComponent");
            #define LX(event) LogEntry(TAG, event)
            static shared_ptr<ComponentConfiguration> getSDKConfig() {
                return ComponentConfiguration::createComponentConfiguration(SDK_COMPONENT_NAME, getCurrentVersion());
            }
            bool SDKComponent::registerComponent(shared_ptr<ComponentReporterInterface> componentReporter) {
                if (!componentReporter) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullcomponentReporter"));
                    return false;
                }
                if (!componentReporter->addConfiguration(getSDKConfig())) {
                    ACSDK_ERROR(LX("addConfigurationFailed"));
                    return false;
                };
                return true;
            }
        }
    }
}