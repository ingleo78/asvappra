#ifndef ACSDKMANUFACTORY_INTERNAL_TYPEINDEX_H_
#define ACSDKMANUFACTORY_INTERNAL_TYPEINDEX_H_

#include <functional>
#include <sstream>
#include <typeindex>
#include <type_traits>
#include <string>
#include "TypeTraitsHelper.h"

namespace alexaClientSDK {
    namespace acsdkManufactory {
        namespace internal {
            struct TypeIndex {
            #if ACSDK_USE_RTTI
                using Value = std::type_index;
            #else
                using Value = void*;
                template <typename Type> struct TypeSpecific {
                    static char m_instance;
                };
            #endif
                std::string getName() const;
                bool operator==(TypeIndex rhs) const;
                bool operator!=(TypeIndex rhs) const;
                bool operator<(TypeIndex rhs) const;
            private:
                friend struct std::hash<TypeIndex>;
                template <typename Type> friend TypeIndex getTypeIndex();
                TypeIndex(Value value);
                Value m_value;
            };
            #if ACSDK_USE_RTTI
            template <typename Type>
            inline TypeIndex getTypeIndex() {
                return TypeIndex{typeid(Type)};
            }
            inline std::string TypeIndex::getName() const {
                return m_value.name();
            }
            #else
            template <typename Type> char TypeIndex::TypeSpecific<Type>::m_instance;
            template <typename Type> inline TypeIndex getTypeIndex() {
                return TypeIndex{&TypeIndex::TypeSpecific<RemoveCvref_t<Type> >::m_instance};
            }
            inline std::string TypeIndex::getName() const {
                std::stringstream ss;
                ss << '[' << m_value << ']';
                return ss.str();
            }
            #endif
            inline TypeIndex::TypeIndex(Value value) : m_value{value} {}
            inline bool TypeIndex::operator==(TypeIndex x) const {
                return m_value == x.m_value;
            }
            inline bool TypeIndex::operator!=(TypeIndex x) const {
                return m_value != x.m_value;
            }
            inline bool TypeIndex::operator<(TypeIndex x) const {
                return m_value < x.m_value;
            }
        }
    }
}
namespace std {
    template <> struct hash<alexaClientSDK::acsdkManufactory::internal::TypeIndex> {
        std::size_t operator()(alexaClientSDK::acsdkManufactory::internal::TypeIndex typeIndex) const;
    };
    inline std::size_t hash<alexaClientSDK::acsdkManufactory::internal::TypeIndex>::operator()(
        alexaClientSDK::acsdkManufactory::internal::TypeIndex typeIndex) const {
        return hash<alexaClientSDK::acsdkManufactory::internal::TypeIndex::Value>()(typeIndex.m_value);
    }
}
#endif