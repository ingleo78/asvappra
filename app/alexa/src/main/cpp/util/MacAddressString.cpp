#include <memory>
#include <string>
#include "MacAddressString.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            std::unique_ptr<MacAddressString> MacAddressString::create(const std::string& macAddress) {
                enum class State { FIRST_OCTET_BYTE, SECOND_OCTET_BYTE, DIVIDER };
                State parseState = State::FIRST_OCTET_BYTE;
                int numOctets = 0;
                for (const auto& c : macAddress) {
                    switch (parseState) {
                        case State::FIRST_OCTET_BYTE:
                            if (!std::isxdigit(c)) return nullptr;
                            parseState = State::SECOND_OCTET_BYTE;
                            break;
                        case State::SECOND_OCTET_BYTE:
                            if (!std::isxdigit(c)) return nullptr;
                            parseState = State::DIVIDER;
                            ++numOctets;
                            break;
                        case State::DIVIDER:
                            if ((':' != c) && ('-' != c)) return nullptr;
                            parseState = State::FIRST_OCTET_BYTE;
                            break;
                    }
                }
                if ((6 != numOctets) || (parseState != State::DIVIDER)) return nullptr;
                return std::unique_ptr<MacAddressString>(new MacAddressString(macAddress));
            }
            std::string MacAddressString::getString() const {
                return m_macAddress;
            }
            std::string MacAddressString::getTruncatedString() const {
                const char X = 'X';
                std::string truncatedMac = m_macAddress;
                truncatedMac.at(0) = X;
                truncatedMac.at(1) = X;
                truncatedMac.at(3) = X;
                truncatedMac.at(4) = X;
                truncatedMac.at(6) = X;
                truncatedMac.at(7) = X;
                truncatedMac.at(9) = X;
                truncatedMac.at(10) = X;
                return truncatedMac;
            }
            MacAddressString::MacAddressString(const std::string& macAddress) : m_macAddress(macAddress) {}
        }
    }
}
