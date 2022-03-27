#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGENTRY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGENTRY_H_

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include "LogEntryStream.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class LogEntry {
                    public:
                        LogEntry(const char *source, const char* event);
                        LogEntry(const std::string& source, const std::string& event);
                        LogEntry& d(const std::string& key, const char* value);
                        LogEntry& d(const char* key, char* value);
                        LogEntry& d(const char* key, const char* value);
                        LogEntry& d(const std::string& key, const std::string& value);
                        LogEntry& d(const char* key, const std::string& value);
                        LogEntry& d(const std::string& key, bool value);
                        LogEntry& d(const char* key, bool value);
                        template <typename ValueType> inline LogEntry& d(const char* key, const ValueType& value);
                        template <typename ValueType> inline LogEntry& sensitive(const char* key, const ValueType& value);
                        inline LogEntry& obfuscatePrivateData(const char* key, const std::string& value);
                        LogEntry& m(const char* message);
                        LogEntry& m(const std::string& message);
                        template <typename PtrType> LogEntry& p(const char* key, const std::shared_ptr<PtrType>& ptr);
                        LogEntry& p(const char* key, void* ptr);
                        ~LogEntry() {}
                        const char* c_str() const;
                    private:
                        void prefixKeyValuePair();
                        void prefixMessage();
                        static std::vector<std::string> getPrivateLabelBlacklist() {
                            static std::vector<std::string> privateLabelBlacklist = {"ssid"};
                            return privateLabelBlacklist;
                        }
                        void appendEscapedString(const char* in);
                        static const char KEY_VALUE_SEPARATOR = '=';
                        bool m_hasMetadata;
                        LogEntryStream m_stream;
                };
                template <typename ValueType> LogEntry& LogEntry::d(const char* key, const ValueType& value) {
                    prefixKeyValuePair();
                    m_stream << key << KEY_VALUE_SEPARATOR << value;
                    return *this;
                }
                template <typename PtrType> LogEntry& LogEntry::p(const char* key, const std::shared_ptr<PtrType>& ptr) {
                    return d(key, ptr.get());
                }
                #ifdef ACSDK_EMIT_SENSITIVE_LOGS
                    template <typename ValueType> LogEntry& LogEntry::sensitive(const char* key, const ValueType& value) {
                        return d(key, value);
                    }
                #else
                    template <typename ValueType> LogEntry& LogEntry::sensitive(const char*, const ValueType&) {
                        return *this;
                    }
                #endif
                LogEntry& LogEntry::obfuscatePrivateData(const char* key, const std::string& value) {
                    auto firstPosition = value.length();
                    for (auto privateLabel : getPrivateLabelBlacklist()) {
                        auto it = std::search(value.begin(), value.end(), privateLabel.begin(), privateLabel.end(), [](char valueChar, char blackListChar) {
                            return std::tolower(valueChar) == std::tolower(blackListChar);
                        });
                        if (it != value.end()) {
                            auto thisPosition = std::distance(value.begin(), it) + privateLabel.length();
                            if (thisPosition < firstPosition) {
                                firstPosition = thisPosition;
                            }
                        }
                    }
                    if (firstPosition <= value.length()) {
                        auto labelPart = value.substr(0, firstPosition);
                        auto obfuscatedPart = std::to_string(std::hash<std::string>{}(value.substr(firstPosition)));
                        return d(key, labelPart + obfuscatedPart);
                    }
                    return d(key, value);
                }
            }
        }
    }
}
#endif