#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <logger/Logger.h>
#include "DeviceInfo.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            static const std::string TAG("DeviceInfo");
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            using namespace avsCommon::sdkInterfaces::endpoints;
            using namespace configuration;
            using namespace std;
            const string CONFIG_KEY_DEVICE_INFO = "deviceInfo";
            const string CONFIG_KEY_CLIENT_ID = "clientId";
            const static string CONFIG_KEY_REGISTRATION_KEY = "registrationKey";
            const string CONFIG_KEY_PRODUCT_ID_KEY = "productIdKey";
            const static string CONFIG_KEY_REGISTRATION = "registration";
            const string CONFIG_KEY_PRODUCT_ID = "productId";
            const string CONFIG_KEY_DSN = "deviceSerialNumber";
            const string CONFIG_KEY_DEVICE_TYPE = "deviceType";
            const string CONFIG_KEY_FRIENDLY_NAME = "friendlyName";
            const string CONFIG_KEY_MANUFACTURER_NAME = "manufacturerName";
            const string CONFIG_KEY_DESCRIPTION = "description";
            const string ENDPOINT_ID_CONCAT = "::";
            inline EndpointIdentifier generateEndpointId(const string& clientId, const string& productId, const string& deviceSerialNumber) {
                return clientId + ENDPOINT_ID_CONCAT + productId + ENDPOINT_ID_CONCAT + deviceSerialNumber;
            }
            shared_ptr<DeviceInfo> DeviceInfo::createFromConfiguration(const shared_ptr<ConfigurationNode>& configurationRoot) {
                if (!configurationRoot) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullConfigurationRoot"));
                    return nullptr;
                }
                return create(*configurationRoot);
            }
            unique_ptr<DeviceInfo> DeviceInfo::create(const ConfigurationNode& configurationRoot) {
                string clientId;
                string productId;
                string deviceSerialNumber;
                string manufacturerName;
                string description;
                string friendlyName;
                string deviceType;
                string registrationKey;
                string productIdKey;
                const string errorEvent = "createFailed";
                const string errorReasonKey = "reason";
                const string errorValueKey = "key";
                auto deviceInfoConfiguration = configurationRoot[CONFIG_KEY_DEVICE_INFO];
                if (!deviceInfoConfiguration) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingDeviceInfoConfiguration").d(errorValueKey, CONFIG_KEY_DEVICE_INFO));
                    return nullptr;
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_CLIENT_ID, &clientId)) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingClientId").d(errorValueKey, CONFIG_KEY_CLIENT_ID));
                    return nullptr;
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_PRODUCT_ID, &productId)) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingProductId").d(errorValueKey, CONFIG_KEY_PRODUCT_ID));
                    return nullptr;
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_DSN, &deviceSerialNumber)) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingDeviceSerialNumber").d(errorValueKey, CONFIG_KEY_DSN));
                    return nullptr;
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_DEVICE_TYPE, &deviceType)) {
                    ACSDK_INFO(LX(__func__).d("result", "skipDeviceType").d(errorValueKey, CONFIG_KEY_DEVICE_TYPE));
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_FRIENDLY_NAME, &friendlyName)) {
                    ACSDK_INFO(LX(__func__).d("result", "skipFriendlyName").d(errorValueKey, CONFIG_KEY_FRIENDLY_NAME));
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_MANUFACTURER_NAME, &manufacturerName)) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingManufacturerName").d(errorValueKey, CONFIG_KEY_MANUFACTURER_NAME));
                    return nullptr;
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_DESCRIPTION, &description)) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingDescription").d(errorValueKey, CONFIG_KEY_DESCRIPTION));
                    return nullptr;
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_REGISTRATION_KEY, &registrationKey)) {
                    ACSDK_INFO(LX(__func__).d("result", "skipRegistrationKey").d(errorValueKey, CONFIG_KEY_REGISTRATION_KEY));
                    registrationKey = CONFIG_KEY_REGISTRATION;
                }
                if (!deviceInfoConfiguration.getString(CONFIG_KEY_PRODUCT_ID_KEY, &productIdKey)) {
                    ACSDK_INFO(LX(__func__).d("result", "skipProductIdKey").d(errorValueKey, CONFIG_KEY_PRODUCT_ID_KEY));
                    productIdKey = CONFIG_KEY_PRODUCT_ID;
                }
                return create(clientId, productId, deviceSerialNumber, manufacturerName, description, friendlyName, deviceType, "", registrationKey,
                              productIdKey);
            }
            unique_ptr<DeviceInfo> DeviceInfo::create(const string& clientId, const string& productId, const string& deviceSerialNumber,
                                                      const string& manufacturerName, const string& description, const string& friendlyName,
                                                      const string& deviceType, const EndpointIdentifier& endpointId, const string& registrationKey,
                                                      const string& productIdKey) {
                const string errorEvent = "createFailed";
                const string errorReasonKey = "reason";
                const string errorValueKey = "key";
                if (clientId.empty()) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingClientId").d(errorValueKey, CONFIG_KEY_CLIENT_ID));
                    return nullptr;
                }
                if (productId.empty()) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingProductId").d(errorValueKey, CONFIG_KEY_PRODUCT_ID));
                    return nullptr;
                }
                if (deviceSerialNumber.empty()) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingDeviceSerialNumber").d(errorValueKey, CONFIG_KEY_DSN));
                    return nullptr;
                }
                if (manufacturerName.empty()) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingManufacturerName").d(errorValueKey, CONFIG_KEY_MANUFACTURER_NAME));
                    return nullptr;
                }
                if (description.empty()) {
                    ACSDK_ERROR(LX(errorEvent).d(errorReasonKey, "missingDescription").d(errorValueKey, CONFIG_KEY_DESCRIPTION));
                    return nullptr;
                }
                auto defaultEndpointId = endpointId.empty() ? generateEndpointId(clientId, productId, deviceSerialNumber) : endpointId;
                auto defaultRegistrationKey = registrationKey.empty() ? CONFIG_KEY_REGISTRATION : registrationKey;
                auto defaultProductIdKey = productIdKey.empty() ? CONFIG_KEY_PRODUCT_ID : productIdKey;
                unique_ptr<DeviceInfo> instance(new DeviceInfo(clientId, productId, deviceSerialNumber, manufacturerName, description,
                                                friendlyName, deviceType, defaultEndpointId, defaultRegistrationKey, defaultProductIdKey));
                return instance;
            }
            string DeviceInfo::getClientId() const {
                return m_clientId;
            }
            string DeviceInfo::getProductId() const {
                return m_productId;
            }
            string DeviceInfo::getDeviceSerialNumber() const {
                return m_deviceSerialNumber;
            }
            string DeviceInfo::getFriendlyName() const {
                return m_friendlyName;
            }
            string DeviceInfo::getDeviceType() const {
                return m_deviceType;
            }
            string DeviceInfo::getRegistrationKey() const {
                return m_registrationKey;
            }
            string DeviceInfo::getProductIdKey() const {
                return m_productIdKey;
            }
            bool DeviceInfo::operator==(const DeviceInfo& rhs) {
                return tie(m_clientId,m_productId,m_deviceSerialNumber,m_manufacturer,m_deviceDescription,m_friendlyName,m_deviceType,
                           m_defaultEndpointId) == std::tie(rhs.m_clientId, rhs.m_productId, rhs.m_deviceSerialNumber, rhs.m_manufacturer,
                           rhs.m_deviceDescription, rhs.m_friendlyName, rhs.m_deviceType, rhs.m_defaultEndpointId);
            }
            bool DeviceInfo::operator!=(const DeviceInfo& rhs) {
                return !(*this == rhs);
            }
            DeviceInfo::DeviceInfo(const string& clientId, const string& productId, const string& deviceSerialNumber, const string& manufacturerName,
                                   const string& description, const string& friendlyName, const string& deviceType, const EndpointIdentifier& endpointId,
                                   const string& registrationKey, const string& productIdKey) : m_clientId{clientId}, m_productId{productId},
                                   m_deviceSerialNumber{deviceSerialNumber}, m_manufacturer{manufacturerName}, m_deviceDescription{description},
                                   m_friendlyName{friendlyName}, m_deviceType{deviceType}, m_defaultEndpointId{endpointId}, m_registrationKey{registrationKey},
                                   m_productIdKey{productIdKey} {}
        }
    }
}
