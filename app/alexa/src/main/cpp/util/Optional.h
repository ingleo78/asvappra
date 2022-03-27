#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_OPTIONAL_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_OPTIONAL_H_

#include <logger/LoggerUtils.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            template <typename ValueT> class Optional {
            public:
                Optional();
                Optional(const ValueT& value);
                Optional(const Optional<ValueT>& other);
                void set(const ValueT& value);
                void reset();
                bool hasValue() const;
                ValueT valueOr(const ValueT& other) const;
                ValueT value() const;
                ~Optional();
                Optional<ValueT>& operator=(const Optional<ValueT>& rhs);
                bool operator==(const Optional<ValueT>& rhs) const;
                bool operator!=(const Optional<ValueT>& rhs) const;
                bool operator<(const Optional& rhs) const;
                bool operator>(const Optional& rhs) const;
                bool operator<=(const Optional& rhs) const;
                bool operator>=(const Optional& rhs) const;
            private:
                inline ValueT& getReference();
                inline const ValueT& getReference() const;
                bool m_hasValue;
                typename std::aligned_storage<sizeof(ValueT), alignof(ValueT)>::type m_value;
            };
            template <typename ValueT> Optional<ValueT>::Optional() : m_hasValue{false} {}
            template <typename ValueT> Optional<ValueT>::Optional(const ValueT& other) : m_hasValue{true} {
                new (&m_value) ValueT(other);
            }
            template <typename ValueT> Optional<ValueT>::Optional(const Optional<ValueT>& other) : m_hasValue{other.m_hasValue} {
                if (hasValue()) new (&m_value) ValueT(other.getReference());
            }
            template <typename ValueT> ValueT& Optional<ValueT>::getReference() {
                return *reinterpret_cast<ValueT*>(&m_value);
            }
            template <typename ValueT> const ValueT& Optional<ValueT>::getReference() const {
                return *reinterpret_cast<const ValueT*>(&m_value);
            }
            template <typename ValueT> void Optional<ValueT>::set(const ValueT& other) {
                if (hasValue()) getReference() = other;
                else {
                    m_hasValue = true;
                    new (&m_value) ValueT(other);
                }
            }
            template <typename ValueT> void Optional<ValueT>::reset() {
                if (hasValue()) {
                    m_hasValue = false;
                    getReference().~ValueT();
                }
            }
            template <typename ValueT> bool Optional<ValueT>::hasValue() const {
                return m_hasValue;
            }
            template <typename ValueT> ValueT Optional<ValueT>::value() const {
                if (hasValue()) return getReference();
                logger::acsdkError(logger::LogEntry("Optional", "valueFailed").d("reason", "optionalHasNoValue"));
                return ValueT();
            }
            template <typename ValueT> ValueT Optional<ValueT>::valueOr(const ValueT& other) const {
                if (hasValue()) return getReference();
                return other;
            }
            template <typename ValueT> Optional<ValueT>::~Optional() {
                if (hasValue()) getReference().~ValueT();
            }
            template <typename ValueT> Optional<ValueT>& Optional<ValueT>::operator=(const Optional<ValueT>& rhs) {
                if (hasValue()) {
                    if (rhs.hasValue()) getReference() = rhs.value();
                    else {
                        m_hasValue = false;
                        getReference().~ValueT();
                    }
                } else if (rhs.hasValue()) {
                    m_hasValue = true;
                    new (&m_value) ValueT(rhs.value());
                }
                return *this;
            }
            template <typename ValueT> bool Optional<ValueT>::operator==(const Optional<ValueT>& rhs) const {
                if (this->hasValue()) return rhs.hasValue() && (this->value() == rhs.value());
                return !rhs.hasValue();
            }
            template <typename ValueT> bool Optional<ValueT>::operator!=(const Optional<ValueT>& rhs) const {
                return !(*this == rhs);
            }
            template <typename ValueT> bool Optional<ValueT>::operator<(const Optional& rhs) const {
                if (m_hasValue && rhs.m_hasValue) return getReference() < rhs.getReference();
                return m_hasValue < rhs.m_hasValue;
            }
            template <typename ValueT> bool Optional<ValueT>::operator>(const Optional& rhs) const {
                return rhs < *this;
            }
            template <typename ValueT> bool Optional<ValueT>::operator<=(const Optional& rhs) const {
                return !(rhs < *this);
            }
            template <typename ValueT> bool Optional<ValueT>::operator>=(const Optional& rhs) const {
                return !(*this < rhs);
            }
        }
    }
}
#endif