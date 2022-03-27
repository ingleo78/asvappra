#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSTRINGCONVERSION_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGSTRINGCONVERSION_H_

#include <list>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <json/JSONUtils.h>

namespace alexaClientSDK {
    namespace settings {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace json;
        using namespace jsonUtils;
        constexpr char QUOTE = '"';
        template <typename ValueT> static constexpr bool isIntegralByteType() {
            return is_same<ValueT, int8_t>::value || is_same<ValueT, uint8_t>::value;
        }
        template <typename ValueT> using IntegralByteType = typename enable_if<isIntegralByteType<ValueT>(), ValueT>::type;
        template <typename ValueT, typename = IntegralByteType<ValueT>> pair<bool, string> toSettingString(const IntegralByteType<ValueT>& value) {
            stringstream stream;
            int valueInt = value;
            stream << valueInt;
            return {!stream.fail(), stream.str()};
        }
        template <typename ValueT, typename = IntegralByteType<ValueT>> pair<bool, ValueT> fromSettingString(const string& str, const IntegralByteType<ValueT>& defaultValue) {
            int16_t value = defaultValue;
            stringstream stream;
            stream << str;
            stream >> value;
            return {!stream.fail(), stream.fail() ? defaultValue : value};
        }
        template <typename ValueT> static constexpr bool isEnumOrString() {
            return is_enum<ValueT>::value || is_same<ValueT, string>::value;
        }
        template <typename ValueT> using EnumOrString = typename enable_if<isEnumOrString<ValueT>(), ValueT>::type;
        template <typename ValueT, typename = EnumOrString<ValueT>> pair<bool, string> toSettingString(const ValueT& value) {
            stringstream stream;
            stream << QUOTE << value << QUOTE;
            return {!stream.fail(), stream.str()};
        }
        template <typename ValueT, typename = EnumOrString<ValueT>> pair<bool, ValueT> fromSettingString(const string& str, const ValueT& defaultValue) {
            if (str.length() < 2 || str[0] != QUOTE || str[str.length() - 1] != QUOTE) return {false, defaultValue};
            ValueT value = defaultValue;
            stringstream stream;
            stream << str.substr(1, str.length() - 2);
            stream >> value;
            return {!stream.fail(), value};
        }
        template <typename ValueT> static constexpr bool isStringCollection() {
            return is_same<ValueT, set<string>>::value || is_same<ValueT, vector<string>>::value || is_same<ValueT, list<string>>::value;
        }
        template <typename ValueT> using StringCollection = typename enable_if<isStringCollection<ValueT>(), ValueT>::type;
        template <typename ValueT, StringCollection<ValueT>* = nullptr> pair<bool, string> toSettingString(const StringCollection<ValueT>& value) {
            vector<string> values{value.begin(), value.end()};
            auto retValue = convertToJsonString(values);
            return make_pair(!retValue.empty(), retValue);
        }
        static const string EMPTY_JSON_LIST = "[]";
        template <typename ValueT, StringCollection<ValueT>* = nullptr> pair<bool, ValueT> fromSettingString(const string& str, const StringCollection<ValueT>& defaultValue) {
            auto values = retrieveStringArray<ValueT>(str);
            if (values.empty() && str != EMPTY_JSON_LIST) {
                return make_pair(false, defaultValue);
            }
            ValueT retValue{values.begin(), values.end()};
            return make_pair(true, retValue);
        }
        template <typename ValueT> using OtherTypes = typename enable_if<!isEnumOrString<ValueT>() && !isIntegralByteType<ValueT>() && !isStringCollection<ValueT>(), ValueT>::type;
        template <typename ValueT> pair<bool, string> toSettingString(const OtherTypes<ValueT>& value) {
            stringstream stream;
            stream << boolalpha << value;
            return {!stream.fail(), stream.str()};
        }
        template <typename ValueT> pair<bool, ValueT> fromSettingString(const string& str, const OtherTypes<ValueT>& defaultValue) {
            ValueT value = defaultValue;
            stringstream stream;
            stream << str;
            stream >> boolalpha >> value;
            return {!stream.fail(), stream.fail() ? defaultValue : value};
        }
    }
}
#endif