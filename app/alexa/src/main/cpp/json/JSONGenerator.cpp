#include "../logger/Logger.h"
#include "document.h"
#include "rapidjson.h"
#include "JSONGenerator.h"
#include "JSONUtils.h"

static const std::string TAG("JSONGenerator");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace json {
                bool JsonGenerator::startObject(const std::string& key) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.StartObject();
                }
                bool JsonGenerator::finishObject() { return checkWriter() && m_writer.EndObject(); }
                bool JsonGenerator::addMember(const std::string& key, const std::string& value) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.String(value.data(), (unsigned int)value.length());
                }
                bool JsonGenerator::addMember(const std::string& key, uint64_t value) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.Uint64(value);
                }
                bool JsonGenerator::addMember(const std::string& key, unsigned int value) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.Uint(value);
                }
                bool JsonGenerator::addMember(const std::string& key, int64_t value) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.Int64(value);
                }
                bool JsonGenerator::addMember(const std::string& key, int value) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.Int(value);
                }
                bool JsonGenerator::addMember(const std::string& key, bool value) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.Bool(value);
                }
                bool JsonGenerator::addMember(const std::string& key, const char* value) {
                    return value && checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.String(value);
                }
                bool JsonGenerator::addMember(const std::string& key, double value) {
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.Double(value);
                }
                bool JsonGenerator::addRawJsonMember(const std::string& key, const std::string& json, bool validate) {
                    if (validate) {
                        rapidjson::Document document;
                        if (!jsonUtils::parseJSON(json, &document)) {
                            ACSDK_ERROR(LX("addRawJsonMemberFailed").d("reason", "invalidJson").sensitive("rawJson", json));
                            return false;
                        }
                    }
                    return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.RawValue(json.c_str(), json.length(), rapidjson::kStringType);
                }
                bool JsonGenerator::startArray(const std::string& key) { return checkWriter() && m_writer.Key(key.c_str(), key.length()) && m_writer.StartArray(); }
                bool JsonGenerator::finishArray() { return checkWriter() && m_writer.EndArray(); }
                bool JsonGenerator::startArrayElement() { return checkWriter() && m_writer.StartObject(); }
                bool JsonGenerator::finishArrayElement() { return finishObject(); }
                bool JsonGenerator::finalize() {
                    while (!m_writer.IsComplete()) {
                        if (!m_writer.EndObject()) {
                            ACSDK_ERROR(LX("finishFailed").d("reason", "failToEndObject"));
                            return false;
                        }
                    }
                    return true;
                }
                std::string JsonGenerator::toString(bool finalizeJson) {
                    if (finalizeJson) finalize();
                    return m_buffer.GetString();
                }
                bool JsonGenerator::checkWriter() {
                    if (m_writer.IsComplete()) {
                        ACSDK_ERROR(LX("addMemberFailed").d("reason", "finalizedGenerator"));
                        return false;
                    }
                    return true;
                }
                bool JsonGenerator::isFinalized() { return !checkWriter(); }
                template <typename CollectionT, typename ValueT>
                bool JsonGenerator::addStringArray(const std::string& key, const CollectionT& collection) {
                    if (!checkWriter() || !m_writer.Key(key.c_str(), key.length())) return false;
                    m_writer.StartArray();
                    for (const auto& value : collection) {
                        static_assert(std::is_same<ValueT, std::string>::value, "We only support string collection.");
                        m_writer.String(value);
                    }
                    m_writer.EndArray();
                    return true;
                }
                template <typename CollectionT, typename ValueT>
                bool JsonGenerator::addMembersArray(const std::string& key, const CollectionT& collection) {
                    if (!checkWriter() || !m_writer.Key(key.c_str(), key.length())) return false;
                    m_writer.StartArray();
                    for (const auto& value : collection) {
                        static_assert(std::is_same<ValueT, std::string>::value, "We only support string collection.");
                        m_writer.RawValue(value.c_str(), value.length(), rapidjson::kStringType);
                    }
                    m_writer.EndArray();
                    return true;
                }
                template <typename CollectionArrayT, typename CollectionValueT, typename ValueT>
                bool JsonGenerator::addCollectionOfStringArray(const std::string& key, const CollectionArrayT& collection) {
                    if (!checkWriter() || !m_writer.Key(key.c_str(), key.length())) return false;
                    m_writer.StartArray();
                    for (const auto& stringArray : collection) {
                        m_writer.StartArray();
                        for (const auto& value : stringArray) {
                            static_assert(std::is_same<ValueT, std::string>::value, "We only support string collection.");
                            m_writer.String(value);
                        }
                        m_writer.EndArray();
                    }
                    m_writer.EndArray();
                    return true;
                }
            }
        }
    }
}
