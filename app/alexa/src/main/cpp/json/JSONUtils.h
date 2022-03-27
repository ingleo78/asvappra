#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_JSON_JSONUTILS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_JSON_JSONUTILS_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "../logger/LoggerUtils.h"
#include "document.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace json {
                namespace jsonUtils {
                    using namespace rapidjson;
                    inline static std::string getTag() { return "JsonUtils"; }
                    bool findNode(const Value& jsonNode, std::string& key, Value::ConstMemberIterator* iteratorPtr);
                    bool parseJSON(const char* jsonContent, rapidjson::Document* document);
                    bool convertToValue(const Value& documentNode, char* value);
                    bool convertToValue(const Value& documentNode, int64_t* value);
                    bool convertToValue(const Value& documentNode, uint64_t* value);
                    bool convertToValue(const Value& documentNode, bool* value);
                    bool convertToValue(const Value& documentNode, double* value);
                    template<typename T> bool convertToValue(const ValueMemoryPool& documentNode, T value) {
                        convertToValue(documentNode, value);
                    }
                    template <typename T> bool retrieveValue(const Value& jsonNode, const std::string& key, T* value) {
                        if (!value) {
                            logger::acsdkError(logger::LogEntry(getTag(), "retrieveValueFailed").d("reason", "nullValue"));
                            return false;
                        }
                        rapidjson::Value::ConstMemberIterator iterator;
                        if (!findNode(jsonNode, (char*)key.c_str(), &iterator)) return false;
                        return convertToValue(iterator->value, value);
                    }
                    template<typename T> bool retrieveValue(const Document& jsonNode, const std::string& key, T* value) {
                        return retrieveValue(jsonNode, key, value);
                    }
                    bool retrieveValue1(const Value& jsonNode, const std::string& key, std::string *value) {
                        return retrieveValue(jsonNode, key, value);
                    }
                    bool retrieveValue2(const Value& jsonNode, const std::string& key, bool *value) {
                        return retrieveValue(jsonNode, key, value);
                    }
                    bool retrieveValue3(const Value& jsonNode, const std::string& key, int64_t *value) {
                        return retrieveValue(jsonNode, key, value);
                    }
                    bool retrieveValue4(const Value& jsonNode, const std::string& key, uint64_t *value) {
                        return retrieveValue(jsonNode, key, value);
                    }
                    template <typename T> bool retrieveValue(const char* jsonString, const std::string& key, T* value) {
                        if (!value) {
                            logger::acsdkError(logger::LogEntry(getTag(), "retrieveValueFailed").d("reason", "nullValue"));
                            return false;
                        }
                        Document document;
                        if (!parseJSON(jsonString, &document)) {
                            logger::acsdkError(logger::LogEntry(getTag(), "retrieveValueFailed").d("reason", "parsingError"));
                            return false;
                        }
                        return retrieveValue(document, key, value);
                    }
                    bool jsonArrayExists(const rapidjson::Value& parsedDocument, const std::string& key);
                    template <class CollectionT> CollectionT retrieveStringArray(const std::string& jsonString, const std::string& key);
                    template <class CollectionT> CollectionT retrieveStringArray(const std::string& jsonString);
                    template <class CollectionT> CollectionT retrieveStringArray(const Value& value);
                    template <class CollectionT> std::string convertToJsonString(const CollectionT& elements);
                    std::map<std::string, std::string> retrieveStringMap(const Value& value, const std::string& key);
                    void retrieveStringMapFromArray(const Value& value, const std::string& key, std::map<std::string, std::string>& elements);
                    bool retrieveArrayOfStringMapFromArray(const Value& value, const std::string& key, std::vector<std::map<std::string, std::string>>& elements);
                    template <> std::vector<std::string> retrieveStringArray<std::vector<std::string>>(const std::string& jsonString,const std::string& key);
                    template <> std::vector<std::string> retrieveStringArray<std::vector<std::string>>(const std::string& jsonString);
                    template <> std::vector<std::string> retrieveStringArray<std::vector<std::string>>(const rapidjson::Value& value);
                    template <> std::string convertToJsonString<std::vector<std::string>>(const std::vector<std::string>& elements);
                    template <class CollectionT> CollectionT retrieveStringArray(const std::string& jsonString, const std::string& key) {
                        auto values = retrieveStringArray<std::vector<std::string>>(jsonString, key);
                        return CollectionT{values.begin(), values.end()};
                    }
                    template <class CollectionT> CollectionT retrieveStringArray(const std::string& jsonString) {
                        auto values = retrieveStringArray<std::vector<std::string>>(jsonString);
                        return CollectionT{values.begin(), values.end()};
                    }
                    template <class CollectionT> CollectionT retrieveStringArray(const rapidjson::Value& value) {
                        auto values = retrieveStringArray<std::vector<std::string>>(value);
                        return CollectionT{values.begin(), values.end()};
                    }
                    template <class CollectionT> std::string convertToJsonString(const CollectionT& elements) {
                        return convertToJsonString(std::vector<std::string>{elements.begin(), elements.end()});
                    }
                }
            }
        }
    }
}
#endif