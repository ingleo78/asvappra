#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MACADDRESSSTRING_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MACADDRESSSTRING_H_

#include <memory>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            class MacAddressString {
            public:
                MacAddressString(const MacAddressString&) = default;
                static std::unique_ptr<MacAddressString> create(const std::string& macAddress);
                std::string getString() const;
                std::string getTruncatedString() const;
            private:
                explicit MacAddressString(const std::string& macAddress);
                const std::string m_macAddress;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MACADDRESSSTRING_H_
