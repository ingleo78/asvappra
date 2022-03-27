#include <algorithm>
#include <json/JSONGenerator.h>
#include <logger/Logger.h>
#include "CapabilityResources.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace resources;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace json;
            using namespace logger;
            static const string TAG("CapabilityResources");
            #define LX(event) LogEntry(TAG, event)
            static constexpr size_t MAX_FRIENDLY_NAME_LENGTH = 16000;
            CapabilityResources::CapabilityResources() : m_isValid{true}, m_items{vector<FriendlyName>()} {}
            bool CapabilityResources::FriendlyName::operator==(const CapabilityResources::FriendlyName& rhs) const {
                return value == rhs.value && locale.valueOr("") == rhs.locale.valueOr("");
            }
            bool CapabilityResources::addFriendlyNameWithAssetId(const AlexaAssetId& assetId) {
                if (assetId.empty()) {
                    ACSDK_ERROR(LX("addFriendlyNameWithAssetIdFailed").d("reason", "invalidAssetId"));
                    m_isValid = false;
                    return false;
                }
                FriendlyName friendlyName{assetId, Optional<sdkInterfaces::LocaleAssetsManagerInterface::Locale>()};
                if (find(m_items.begin(), m_items.end(), friendlyName) != m_items.end()) {
                    ACSDK_ERROR(LX("addFriendlyNameWithAssetIdFailed").d("reason", "duplicateAssetId").d("assetId", assetId));
                    m_isValid = false;
                    return false;
                }
                m_items.push_back(friendlyName);
                return true;
            }
            bool CapabilityResources::addFriendlyNameWithText(const string& text, const LocaleAssetsManagerInterface::Locale& locale) {
                if (text.length() == 0 || text.length() > MAX_FRIENDLY_NAME_LENGTH) {
                    ACSDK_ERROR(LX("addFriendlyNameWithTextFailed").d("reason", "invalidText"));
                    m_isValid = false;
                    return false;
                }
                if (locale.empty()) {
                    ACSDK_ERROR(LX("addFriendlyNameWithTextFailed").d("reason", "invalidLocale"));
                    m_isValid = false;
                    return false;
                }
                if (find(m_items.begin(),m_items.end(),CapabilityResources::FriendlyName({text,
                    Optional<LocaleAssetsManagerInterface::Locale>(locale)})) != m_items.end()) {
                    ACSDK_ERROR(LX("addFriendlyNameWithTextFailed").d("reason", "duplicateText").sensitive("text", text).sensitive("locale", locale));
                    m_isValid = false;
                    return false;
                }
                m_items.push_back({text, Optional<LocaleAssetsManagerInterface::Locale>(locale)});
                return true;
            }
            bool CapabilityResources::isValid() const {
                return m_isValid && m_items.size() > 0;
            }
            string CapabilityResources::toJson() const {
                if (!isValid()) {
                    ACSDK_ERROR(LX("toJsonFailed").d("reason", "invalidCapabilityResources"));
                    return "{}";
                }
                JsonGenerator jsonGenerator;
                jsonGenerator.startArray("friendlyNames");
                for (const auto& item : m_items) {
                    jsonGenerator.startArrayElement();
                    if (item.locale.hasValue()) {
                        jsonGenerator.addMember("@type", "text");
                        jsonGenerator.startObject("value");
                        jsonGenerator.addMember("text", item.value.data());
                        jsonGenerator.addMember("locale", item.locale.value());
                    } else {
                        jsonGenerator.addMember("@type", "asset");
                        jsonGenerator.startObject("value");
                        jsonGenerator.addMember("assetId", item.value.data());
                    }
                    jsonGenerator.finishObject();
                    jsonGenerator.finishArrayElement();
                }
                jsonGenerator.finishArray();
                return jsonGenerator.toString();
            }
        }
    }
}