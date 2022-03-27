#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_CONFIGURATION_CONFIGURATIONNODE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_CONFIGURATION_CONFIGURATIONNODE_H_

#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <json/document.h>
#include <json/istreamwrapper.h>
#include <json/writer.h>
#include <json/en.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace configuration {
                using namespace std;
                using namespace chrono;
                using namespace rapidjson;
                class ConfigurationNode {
                    public:
                        static bool initialize(const vector<shared_ptr<istream>>& jsonStreams);
                        static void uninitialize();
                        static shared_ptr<ConfigurationNode> createRoot();
                        static ConfigurationNode getRoot();
                        static void mergeDocument(const string& path, Value& out, Value& in, Document::AllocatorType& allocator);
                        ConfigurationNode();
                        ConfigurationNode(const Value& object);
                        bool getBool(const string& key, bool* out = nullptr, bool defaultValue = false) const;
                        bool getInt(const string& key, int* out = nullptr, int defaultValue = 0) const;
                        bool getString(const string& key, string* out = nullptr, string defaultValue = "") const;
                        bool getStringValues(const string& key, set<string>* out = nullptr) const;
                        template <typename InputType, typename OutputType, typename DefaultType>
                        bool getDuration(const string& key, OutputType* out = static_cast<seconds>(0), DefaultType defaultValue = seconds(0)) const;
                        ConfigurationNode operator[](const string& key) const;
                        operator bool() const;
                        template <typename Type> bool getValue(const string& key, Type* out, Type defaultValue, bool (Value::*isType)() const, Type (Value::*getType)() const) const;
                        string serialize() const;
                        ConfigurationNode getArray(const string& key) const;
                        size_t getArraySize() const;
                        ConfigurationNode operator[](const size_t index) const;
                    private:
                        bool getString(const string& key, const char** out, const char* defaultValue) const;
                        const Value* m_object;
                        static mutex m_mutex;
                        static Document m_document;
                        static ConfigurationNode m_root;
                };
                template <typename InputType, typename OutputType, typename DefaultType> bool ConfigurationNode::getDuration(const string& key, OutputType* out, DefaultType defaultValue) const {
                    int temp;
                    auto result = getInt(key, &temp);
                    if (out) *out = OutputType(result ? InputType(temp) : defaultValue);
                    return result;
                }
                template <typename Type> bool ConfigurationNode::getValue(const string& key, Type* out, Type defaultValue, bool (Value::*isType)() const,
                                                        Type (Value::*getType)() const) const {
                    if (key.empty() || !m_object) {
                        if (out) *out = defaultValue;
                        return false;
                    }
                    auto it = m_object->FindMember(key.c_str());
                    if (m_object->MemberEnd() == it || !(it->value.*isType)()) {
                        if (out) *out = defaultValue;
                        return false;
                    }
                    if (out) *out = (it->value.*getType)();
                    return true;
                }
            }
        }
    }
}
#endif