#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_STORAGE_CAPABILITIESDELEGATESTORAGEINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_STORAGE_CAPABILITIESDELEGATESTORAGEINTERFACE_H_

#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/string"
#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/unordered_map"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace storage {
            class CapabilitiesDelegateStorageInterface {
            public:
                virtual ~CapabilitiesDelegateStorageInterface() = default;
                virtual bool createDatabase() = 0;
                virtual bool open() = 0;
                virtual void close() = 0;
                virtual bool store(const std::string& endpointId, const std::string& endpointConfig) = 0;
                virtual bool store(const std::unordered_map<std::string, std::string>& endpointIdToConfigMap) = 0;
                virtual bool load(std::unordered_map<std::string, std::string>* endpointConfigMap) = 0;
                virtual bool load(const std::string& endpointId, std::string* endpointConfig) = 0;
                virtual bool erase(const std::string& endpointId) = 0;
                virtual bool erase(const std::unordered_map<std::string, std::string>& endpointIdToConfigMap) = 0;
                virtual bool clearDatabase() = 0;
            };
        }
    }
}
#endif