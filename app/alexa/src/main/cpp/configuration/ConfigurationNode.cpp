#include "ConfigurationNode.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace configuration {
                using namespace utils;
                using namespace logger;
                using namespace rapidjson;
                using namespace json;
                using namespace jsonUtils;
                static const string TAG("ConfigurationNode");
                #define LX(event) LogEntry(TAG, event)
                mutex ConfigurationNode::m_mutex;
                Document ConfigurationNode::m_document;
                ConfigurationNode ConfigurationNode::m_root = ConfigurationNode();
                #ifdef ACSDK_DEBUG_LOG_ENABLED
                    static string valueToString(const Value& value) {
                        StringBuffer stringBuffer;
                        rapidjson::Writer<StringBuffer> writer(stringBuffer);
                        if (value.Accept(writer)) return stringBuffer.GetString();
                        ACSDK_ERROR(LX("valueToStringFailed").d("reason", "AcceptReturnedFalse"));
                        return "";
                    }
                #endif
                static void mergeDocument(const string& path, Value& out, Value& in, Document::AllocatorType& allocator) {
                    for (auto inIt = in.MemberBegin(); inIt != in.MemberEnd(); inIt++) {
                        auto outIt = out.FindMember(inIt->name);
                        if (outIt != out.MemberEnd() && inIt->value != outIt->value) {
                            auto memberPath = path + "." + outIt->name.GetString();
                            if (outIt->value.IsObject()) {
                                ACSDK_DEBUG(LX("mergeDocument").d("reason", "objectsMerged").d("path", memberPath));
                                mergeDocument(memberPath, outIt->value, inIt->value, allocator);
                            } else {
                                ACSDK_DEBUG(LX("mergeDocument").d("reason", "valueReplaced").d("path", memberPath)
                                    .sensitive("old", valueToString(outIt->value)).sensitive("new", valueToString(inIt->value)));
                                outIt->value = inIt->value;
                            }
                        } else {
                            auto *members = out.GetMembersPointer();
                            SizeType *size = out.Size();
                            SizeType *capacity = out.Capacity();
                            if (*size >= *capacity) out.MemberReserve(*capacity == 0 ? RAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY : (*capacity + (*capacity + 1) / 2));
                            members[*size].name.SetRawAssign(inIt->name);
                            members[*size].value.SetRawAssign(inIt->value);
                            *size = *size + 1;
                            out.AddMember(inIt->name, inIt->value);
                        }
                    }
                }
                bool ConfigurationNode::initialize(const vector<shared_ptr<istream>>& jsonStreams) {
                    lock_guard<mutex> lock(m_mutex);
                    if (m_root) {
                        ACSDK_ERROR(LX("initializeFailed").d("reason", "alreadyInitialized"));
                        return false;
                    }
                    m_document.SetObject();
                    for (auto jsonStream : jsonStreams) {
                        if (!jsonStream) {
                            m_document.SetObject();
                            return false;
                        }
                        IStreamWrapper wrapper(*jsonStream);
                        Document overlay(&m_document.GetAllocator());
                        overlay.ParseStream<kParseCommentsFlag>(wrapper);
                        if (overlay.HasParseError()) {
                            ACSDK_ERROR(LX("initializeFailed").d("reason", "parseFailure").d("offset", overlay.GetErrorOffset())
                                .d("message", GetParseError_En(overlay.GetParseError())));
                            m_document.SetObject();
                            return false;
                        }
                        mergeDocument("root", (Value&)m_document, (Value&)overlay, m_document.GetAllocator());
                    }
                    m_root = ConfigurationNode((const Value&)m_document.Move());
                    ACSDK_DEBUG0(LX("initializeSuccess").sensitive("configuration", valueToString(m_document)));
                    return true;
                }
                void ConfigurationNode::uninitialize() {
                    lock_guard<mutex> lock(m_mutex);
                    m_document.SetObject();
                    m_root = ConfigurationNode();
                }
                shared_ptr<ConfigurationNode> ConfigurationNode::createRoot() { return make_shared<ConfigurationNode>(getRoot()); }
                ConfigurationNode ConfigurationNode::getRoot() { return m_root; }
                ConfigurationNode::ConfigurationNode() : m_object{nullptr} {}
                bool ConfigurationNode::getBool(const string& key, bool* out, bool defaultValue) const {
                    return getValue(key, out, defaultValue, &Value::IsBool, &Value::GetBool);
                }
                bool ConfigurationNode::getInt(const string& key, int* out, int defaultValue) const {
                    return getValue(key, out, defaultValue, &Value::IsInt, &Value::GetInt);
                }
                bool ConfigurationNode::getString(const string& key, string* out, string defaultValue) const {
                    const char* temp;
                    auto result = getString(key, &temp, defaultValue.c_str());
                    if (out) *out = temp;
                    return result;
                }
                bool ConfigurationNode::getString(const string& key, const char** out, const char* defaultValue) const {
                    return getValue(key, out, defaultValue, &Value::IsString, &Value::GetString);
                }
                ConfigurationNode ConfigurationNode::operator[](const string& key) const {
                    if (!*this) return ConfigurationNode();
                    auto it = m_object->FindMember(key.c_str());
                    if (m_object->MemberEnd() == it || !it->value.IsObject()) return ConfigurationNode();
                    return ConfigurationNode((Value&)it->value);
                }
                ConfigurationNode::operator bool() const { return m_object; }
                ConfigurationNode::ConfigurationNode(const Value& object) : m_object{&object} { }
                string ConfigurationNode::serialize() const {
                    StringBuffer buffer;
                    rapidjson::Writer<StringBuffer> writer;
                    if (!m_object->Accept(writer)) {
                        ACSDK_ERROR(LX("serializeFailed").d("reason", "writerRefusedObject"));
                        return "";
                    }
                    buffer = writer.getOs();
                    const char* bufferData = buffer.GetString();
                    if (!bufferData) {
                        ACSDK_ERROR(LX("serializeFailed").d("reason", "nullptrBufferString"));
                        return "";
                    }
                    return string(bufferData);
                }
                bool ConfigurationNode::getStringValues(const string& key, set<string>* out) const {
                    if (!m_object) {
                        ACSDK_ERROR(LX("getStringValuesFailed").d("reason", "invalidRoot"));
                        return false;
                    }
                    auto it = m_object->FindMember(key.c_str());
                    if (m_object->MemberEnd() == it || !it->value.IsArray()) {
                        ACSDK_ERROR(LX("getStringValuesFailed").d("reason", "invalidKey/value").d("key", key));
                        return false;
                    }
                    if (out) *out = retrieveStringArray<set<string>>(it->value);
                    return true;
                }
                ConfigurationNode ConfigurationNode::getArray(const string& key) const {
                    if (!*this) {
                        ACSDK_ERROR(LX("getArrayFailed").d("reason", "emptyConfigurationNode"));
                        return ConfigurationNode();
                    }
                    auto it = m_object->FindMember(key.c_str());
                    if (m_object->MemberEnd() == it) {
                        return ConfigurationNode();
                    }
                    if (!it->value.IsArray()) {
                        ACSDK_ERROR(LX("getArrayFailed").d("reason", "notAnArray"));
                        return ConfigurationNode();
                    }
                    return ConfigurationNode((Value&)it->value);
                }
                size_t ConfigurationNode::getArraySize() const {
                    if (!*this) {
                        ACSDK_ERROR(LX("getArraySizeFailed").d("reason", "emptyConfigurationNode"));
                        return 0;
                    }
                    if (!m_object->IsArray()) {
                        ACSDK_ERROR(LX("getArraySizeFailed").d("reason", "notAnArray"));
                        return 0;
                    }
                    return m_object->Size();
                }
                ConfigurationNode ConfigurationNode::operator[](const size_t index) const {
                    auto size = getArraySize();
                    if (index >= size) {
                        ACSDK_ERROR(LX("operator[]Failed").d("reason", "indexOutOfRange").d("size", size).d("index", index));
                        return ConfigurationNode();
                    }
                    const Value& objectRef = *m_object;
                    return ConfigurationNode((Value&)objectRef[index]);
                }
            }
        }
    }
}
