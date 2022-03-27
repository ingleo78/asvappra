#include "../logger/Logger.h"
#include "JSONUtils.h"
#include "en.h"
#include "stringbuffer.h"
#include "writer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace json {
                namespace jsonUtils {
                    using namespace rapidjson;
                    static const std::string TAG("JsonUtils");
                    #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                    static std::ostream& operator <<(std::ostream& stream, rapidjson::Type type) {
                        switch (type) {
                            case rapidjson::Type::kNullType: stream << "NULL"; break;
                            case rapidjson::Type::kFalseType: stream << "FALSE"; break;
                            case rapidjson::Type::kTrueType: stream << "TRUE"; break;
                            case rapidjson::Type::kObjectType: stream << "OBJECT"; break;
                            case rapidjson::Type::kArrayType: stream << "ARRAY"; break;
                            case rapidjson::Type::kStringType: stream << "STRING"; break;
                            case rapidjson::Type::kNumberType: stream << "NUMBER"; break;
                        }
                        return stream;
                    }
                    bool parseJSON(const char* jsonContent, Document* document) {
                        if (document == NULL) {
                            ACSDK_ERROR(LX("parseJSONFailed").d("reason", "nullDocument"));
                            return false;
                        }
                        document->Parse(jsonContent);
                        if (document->HasParseError()) {
                            ACSDK_ERROR(LX("parseJSONFailed").d("offset", document->GetErrorOffset())
                                       .d("error", GetParseError_En(document->GetParseError())));
                            return false;
                        }
                        return true;
                    }
                    static void serializeJSONObjectToString(const Value& documentNode, char* value) {
                        if (!documentNode.IsObject()) {
                            ACSDK_ERROR(LX("serializeJSONObjectToStringFailed").d("reason", "invalidType")
                                            .d("expectedType", Type::kObjectType)
                                            .d("type", documentNode.GetType()));
                            return;
                        }
                        StringBuffer stringBuffer;
                        Writer<StringBuffer> writer;
                        if (!documentNode.Accept(writer)) {
                            ACSDK_ERROR(LX("serializeJSONObjectToStringFailed").d("reason", "acceptFailed").d("handler", "Writer"));
                            return;
                        }
                        stringBuffer = writer.getOs();
                        value = (char*)stringBuffer.GetString();
                    }
                    bool findNode(const Value& jsonNode, std::string &key, Value::ConstMemberIterator* iteratorPtr) {
                        if (!iteratorPtr) {
                            ACSDK_ERROR(LX("findNodeFailed").d("reason", "nullIteratorPtr"));
                            return false;
                        }
                        auto iterator = jsonNode.FindMember((char*)key.data());
                        if (iterator == jsonNode.MemberEnd()) {
                            ACSDK_DEBUG5(LX("findNode").d("reason", "missingDirectChild").d("child", key));
                            return false;
                        }
                        *iteratorPtr = iterator;
                        return true;
                    }
                    bool convertToValue(const Value& documentNode, char* value) {
                        if (!value) {
                            ACSDK_ERROR(LX("convertToStringValueFailed").d("reason", "nullValue"));
                            return false;
                        }
                        if (!documentNode.IsString() && !documentNode.IsObject()) {
                            std::ostringstream expectedTypes;
                            expectedTypes << rapidjson::Type::kObjectType << "/" << rapidjson::Type::kStringType;
                            ACSDK_ERROR(LX("convertToStringValueFailed")
                                            .d("reason", "invalidType")
                                            .d("expectedTypes", expectedTypes.str())
                                            .d("type", documentNode.GetType()));
                            return false;
                        }
                        if (documentNode.IsString()) value = (char*)documentNode.GetString();
                        else if (documentNode.IsObject()) serializeJSONObjectToString(documentNode, value);
                        return true;
                    }
                    bool convertToValue(const Value& documentNode, uint64_t* value) {
                        if (!value) {
                            ACSDK_ERROR(LX("convertToUnsignedInt64ValueFailed").d("reason", "nullValue"));
                            return false;
                        }
                        if (!documentNode.IsUint64()) {
                            ACSDK_ERROR(LX("convertToUnsignedInt64ValueFailed")
                                            .d("reason", "invalidValue")
                                            .d("expectedValue", "Uint64")
                                            .d("type", documentNode.GetType()));
                            return false;
                        }
                        *value = documentNode.GetUint64();
                        return true;
                    }
                    bool convertToValue(const Value& documentNode, int64_t* value) {
                        if (!value) {
                            ACSDK_ERROR(LX("convertToInt64ValueFailed").d("reason", "nullValue"));
                            return false;
                        }
                        if (!documentNode.IsInt64()) {
                            ACSDK_ERROR(LX("convertToInt64ValueFailed")
                                            .d("reason", "invalidValue")
                                            .d("expectedValue", "Int64")
                                            .d("type", documentNode.GetType()));

                            return false;
                        }
                        *value = documentNode.GetInt64();
                        return true;
                    }
                    bool convertToValue(const Value& documentNode, bool* value) {
                        if (!value) {
                            ACSDK_ERROR(LX("convertToBoolValueFailed").d("reason", "nullValue"));
                            return false;
                        }
                        if (!documentNode.IsBool()) {
                            ACSDK_ERROR(LX("convertToBoolValueFailed")
                                            .d("reason", "invalidValue")
                                            .d("expectedValue", "Bool")
                                            .d("type", documentNode.GetType()));
                            return false;
                        }
                        *value = documentNode.GetBool();
                        return true;
                    }
                    bool convertToValue(const Value& documentNode, double* value) {
                        if (!value) {
                            ACSDK_ERROR(LX("convertToDoubleValueFailed").d("reason", "nullValue"));
                            return false;
                        }
                        if (!documentNode.IsDouble()) {
                            ACSDK_ERROR(LX("convertToDoubleValueFailed")
                                            .d("reason", "invalidValue")
                                            .d("expectedValue", "Double")
                                            .d("type", documentNode.GetType()));
                            return false;
                        }
                        *value = documentNode.GetDouble();
                        return true;
                    }
                    bool jsonArrayExists(const Value& parsedDocument, const std::string& key) {
                        auto iter = parsedDocument.FindMember((char*)key.data());
                        if (parsedDocument.MemberEnd() == iter) {
                            ACSDK_ERROR(LX("lookupArrayExistsFailed").d("reason", "keyNotFound").d("key", key));
                            return false;
                        }
                        if (!iter->value.IsArray()) {
                            ACSDK_ERROR(LX("lookupArrayExistsFailed").d("reason", "notArrayType"));
                            return false;
                        }
                        return true;
                    }
                    template <> std::vector<std::string> retrieveStringArray<std::vector<std::string>>(const std::string& jsonString, const std::string& key) {
                        Document document;
                        document.Parse(jsonString.data());
                        if (document.HasParseError()) {
                            ACSDK_ERROR(LX("retrieveElementsFailed")
                                            .d("offset", document.GetErrorOffset())
                                            .d("error", GetParseError_En(document.GetParseError())));
                            return std::vector<std::string>();
                        }
                        auto iter = document.FindMember(key.data());
                        if (document.MemberEnd() == iter) {
                            ACSDK_ERROR(LX("retrieveElementsFailed").d("reason", "keyNotFound").d("key", key));
                            return std::vector<std::string>();
                        }
                        return retrieveStringArray<std::vector<std::string>>((std::string&)iter->value, key);
                    }
                    template <> std::vector<std::string> retrieveStringArray<std::vector<std::string>>(const std::string& jsonString) {
                        rapidjson::Document document;
                        document.Parse(jsonString.data());
                        if (document.HasParseError()) {
                            ACSDK_ERROR(LX("retrieveElementsFailed")
                                            .d("offset", document.GetErrorOffset())
                                            .d("error", GetParseError_En(document.GetParseError())));
                            return std::vector<std::string>();
                        }
                        return retrieveStringArray<std::vector<std::string>>(document.GetString());
                    }
                    template <>
                    std::vector<std::string> retrieveStringArray<std::vector<std::string>>(const Value& value) {
                        std::vector<std::string> elements;
                        if (!value.IsArray()) {
                            ACSDK_ERROR(LX("retrieveElementsFailed").d("reason", "nonArrayString"));
                            return elements;
                        }
                        for (auto& item : value.GetArray()) {
                            if (item.IsString()) elements.push_back(item.GetString());
                            else ACSDK_WARN(LX("retrieveStringArray").d("result", "ignoredEntry"));
                        }
                        return elements;
                    }
                    template <> std::string convertToJsonString<std::vector<std::string>>(const std::vector<std::string>& elements) {
                        rapidjson::Document document{rapidjson::kArrayType};
                        for (auto& item : elements) {
                            document.PushBack(rapidjson::StringRef(item.c_str()), document.GetAllocator());
                        }
                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer;
                        if (!document.Accept(writer)) {
                            ACSDK_ERROR(LX("convertToJsonStringFailed")
                                            .d("reason", "")
                                            .d("offset", document.GetErrorOffset())
                                            .d("error", GetParseError_En(document.GetParseError())));
                            return std::string();
                        }
                        buffer = writer.getOs();
                        return buffer.GetString();
                    }
                    std::map<std::string, std::string> retrieveStringMap(const Value& value, const std::string& key) {
                        std::map<std::string, std::string> elements;
                        auto objectIt = value.FindMember((char*)key.data());
                        if (value.MemberEnd() == objectIt || !objectIt->value.IsObject()) {
                            ACSDK_DEBUG0(LX("retrieveElementsFailed").d("reason", "couldNotFindObject").d("key", key));
                            return elements;
                        }
                        for (auto it = objectIt->value.MemberBegin(); it != objectIt->value.MemberEnd(); ++it) {
                            if (it->name.IsString() && it->value.IsString()) {
                                elements[it->name.GetString()] = it->value.GetString();
                            } else ACSDK_WARN(LX(__func__).d("result", "ignoredEntry"));
                        }
                        return elements;
                    }
                    void retrieveStringMapFromArray(const Value& value, const std::string& key, std::map<std::string, std::string>& elements) {
                        Value::ConstMemberIterator objectIt = value.FindMember((char*)key.data());
                        if (objectIt == value.MemberEnd()) {
                            ACSDK_DEBUG0(LX("retrieveStringMapFromArrayFailed").d("reason", "couldNotFindObject").d("key", key));
                            return;
                        }
                        const Value& mapOfAttributes = objectIt->value;
                        if (!mapOfAttributes.IsArray()) {
                            ACSDK_DEBUG0(LX("retrieveStringMapFromArrayFailed").d("reason", "NotAnArray").d("key", key));
                            return;
                        }
                        for (Value::ConstValueIterator itr = mapOfAttributes.Begin(); itr != mapOfAttributes.End(); ++itr) {
                            const rapidjson::Value& attribute = *itr;
                            if (!attribute.IsObject()) {
                                ACSDK_DEBUG0(LX("retrieveStringMapFromArrayFailed").d("reason", "NotAnObject").d("key", key));
                                return;
                            }
                            Value::ConstMemberIterator keyEntry = attribute.MemberBegin();
                            if (keyEntry != attribute.MemberEnd()) {
                                Value::ConstMemberIterator valueEntry = keyEntry + 1;
                                if (valueEntry != attribute.MemberEnd()) {
                                    if (keyEntry->value.IsString() && valueEntry->value.IsString()) {
                                        elements[keyEntry->value.GetString()] = valueEntry->value.GetString();
                                    } else {
                                        ACSDK_DEBUG0(LX("retrieveStringMapFromArrayFailed").d("reason", "NotAString").d("key", key));
                                        elements.clear();
                                        return;
                                    }
                                }
                            }
                        }
                    }
                    bool retrieveArrayOfStringMapFromArray(const Value& value, const std::string& key, std::vector<std::map<std::string, std::string>>& elements) {
                        rapidjson::Value::ConstMemberIterator objectIt = value.FindMember((char*)key.data());
                        if (objectIt == value.MemberEnd()) {
                            ACSDK_DEBUG0(LX(__func__).d("reason", "couldNotFindObject").d("key", key));
                            return false;
                        }
                        const rapidjson::Value& mapOfAttributes = objectIt->value;
                        if (!mapOfAttributes.IsArray()) {
                            ACSDK_DEBUG0(LX(__func__).d("reason", "NotAnArray").d("key", key));
                            return false;
                        }
                        for (Value::ConstValueIterator itr = mapOfAttributes.Begin(); itr != mapOfAttributes.End(); ++itr) {
                            std::map<std::string, std::string> element;
                            const rapidjson::Value& attribute = *itr;
                            if (!attribute.IsObject()) {
                                ACSDK_DEBUG0(LX(__func__).d("reason", "NotAnObject").d("key", key));
                                elements.clear();
                                return false;
                            }
                            for (auto keyEntry = attribute.MemberBegin(); keyEntry != attribute.MemberEnd(); keyEntry++) {
                                if (keyEntry->name.IsString() && keyEntry->value.IsString()) {
                                    element[keyEntry->name.GetString()] = keyEntry->value.GetString();
                                } else {
                                    ACSDK_DEBUG0(LX(__func__).d("reason", "ObjectValuesNotString").d("key", key));
                                    elements.clear();
                                    return false;
                                }
                            }
                            elements.push_back(element);
                        }
                        return true;
                    }
                }
            }
        }
    }
}
