#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_JSON_JSONGENERATOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_JSON_JSONGENERATOR_H_

#include <cstdint>
#include <string>
#include <type_traits>
#include "stringbuffer.h"
#include "writer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace json {
                class JsonGenerator {
                    public:
                        JsonGenerator() : m_buffer(), m_writer() { m_writer.StartObject(); };
                        ~JsonGenerator() = default;
                        bool isFinalized();
                        bool startObject(const std::string& key);
                        bool finishObject();
                        bool startArray(const std::string& key);
                        bool startArrayElement();
                        bool finishArrayElement();
                        bool finishArray();
                        bool addMember(const std::string& key, const char* value);
                        bool addMember(const std::string& key, const std::string& value);
                        bool addMember(const std::string& key, int64_t value);
                        bool addMember(const std::string& key, uint64_t value);
                        bool addMember(const std::string& key, int value);
                        bool addMember(const std::string& key, unsigned int value);
                        bool addMember(const std::string& key, bool value);
                        bool addMember(const std::string& key, double value);
                        template <typename CollectionT, typename ValueT = typename CollectionT::value_type>
                        bool addStringArray(const std::string& key, const CollectionT& collection);
                        template <typename CollectionT, typename ValueT = typename CollectionT::value_type>
                        bool addMembersArray(const std::string& key, const CollectionT& collection);
                        bool addRawJsonMember(const std::string& key, const std::string& json, bool validate = true);
                        template <
                            typename CollectionArrayT,
                            typename CollectionValueT = typename CollectionArrayT::value_type,
                            typename ValueT = typename CollectionValueT::value_type>
                        bool addCollectionOfStringArray(const std::string& key, const CollectionArrayT& collection);
                        std::string toString(bool finalize = true);
                    private:
                        bool checkWriter();
                        bool finalize();
                        rapidjson::StringBuffer m_buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> m_writer;
                };
            }
        }
    }
}
#endif