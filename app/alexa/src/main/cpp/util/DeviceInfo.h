#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_DEVICEINFO_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_DEVICEINFO_H_

#include <string>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <configuration/ConfigurationNode.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            using namespace std;
            using namespace sdkInterfaces;
            using namespace endpoints;
            class DeviceInfo {
            public:
                static shared_ptr<DeviceInfo> createFromConfiguration(const shared_ptr<configuration::ConfigurationNode>& configurationRoot);
                static unique_ptr<DeviceInfo> create(const configuration::ConfigurationNode& configurationRoot);
                static unique_ptr<DeviceInfo> create(const string& clientId, const string& productId, const string& deviceSerialNumber, const string& manufacturerName,
                                                     const string& description, const string& friendlyName = "", const string& deviceType = "",
                                                     const EndpointIdentifier& endpointId = "", const string& registrationKey = "", const string& productIdKey = "");
                string getClientId() const;
                string getProductId() const;
                string getDeviceSerialNumber() const;
                string getManufacturerName() const;
                string getDeviceDescription() const;
                string getFriendlyName() const;
                string getDeviceType() const;
                EndpointIdentifier getDefaultEndpointId() const;
                string getRegistrationKey() const;
                string getProductIdKey() const;
                bool operator==(const DeviceInfo& rhs);
                bool operator!=(const DeviceInfo& rhs);
            private:
                DeviceInfo(const string& clientId, const string& productId, const string& deviceSerialNumber, const string& manufacturerName,
                           const string& description, const string& friendlyName, const string& deviceType, const EndpointIdentifier& endpointId,
                           const string& registrationKey, const string& productIdKey);
                std::string m_clientId;
                std::string m_productId;
                std::string m_deviceSerialNumber;
                std::string m_manufacturer;
                std::string m_deviceDescription;
                std::string m_friendlyName;
                std::string m_deviceType;
                sdkInterfaces::endpoints::EndpointIdentifier m_defaultEndpointId;
                std::string m_registrationKey;
                std::string m_productIdKey;
            };
            inline std::string DeviceInfo::getManufacturerName() const {
                return m_manufacturer;
            }
            inline std::string DeviceInfo::getDeviceDescription() const {
                return m_deviceDescription;
            }
            inline sdkInterfaces::endpoints::EndpointIdentifier DeviceInfo::getDefaultEndpointId() const {
                return m_defaultEndpointId;
            }
        }
    }
}
#endif