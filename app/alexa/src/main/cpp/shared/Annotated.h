#ifndef ACSDKMANUFACTORY_ANNOTATED_H_
#define ACSDKMANUFACTORY_ANNOTATED_H_

#include <memory>

namespace alexaClientSDK {
    namespace acsdkManufactory {
        template <typename Annotation, typename Type> struct Annotated {
        public:
            using element_type = Type;
            Annotated() = default;
            Annotated(Type* value);
            Annotated(const std::shared_ptr<Type>& value);
            Annotated(std::shared_ptr<Type>&& value);
            void reset();
            Type* get() const;
            Type& operator*() const;
            Type* operator->() const;
            operator std::shared_ptr<Type>() const;
            explicit operator bool() const;
        private:
            std::shared_ptr<Type> m_value;
        };
        template <typename Annotation, typename Type> inline Annotated<Annotation, Type>::Annotated(Type* value) : m_value{value} {}
        template <typename Annotation, typename Type> inline Annotated<Annotation, Type>::Annotated(const std::shared_ptr<Type>& value) : m_value{value} {}
        template <typename Annotation, typename Type> inline Annotated<Annotation, Type>::Annotated(std::shared_ptr<Type>&& value) : m_value{value} {}
        template <typename Annotation, typename Type> inline void Annotated<Annotation, Type>::reset() {
            m_value.reset();
        }
        template <typename Annotation, typename Type> inline Type* Annotated<Annotation, Type>::get() const {
            return m_value.get();
        }
        template <typename Annotation, typename Type> inline Type& Annotated<Annotation, Type>::operator*() const {
            return *m_value;
        }
        template <typename Annotation, typename Type> inline Type* Annotated<Annotation, Type>::operator->() const {
            return m_value.get();
        }
        template <typename Annotation, typename Type> inline Annotated<Annotation, Type>::operator std::shared_ptr<Type>() const {
            return m_value;
        }
        template <typename Annotation, typename Type> inline Annotated<Annotation, Type>::operator bool() const {
            return !!m_value;
        }
        template <typename Annotation, typename Type> inline bool operator==(const Annotated<Annotation, Type>& lhs, const Annotated<Annotation, Type>& rhs) {
            return lhs.get() == rhs.get();
        };
        template <typename Annotation, typename Type> inline bool operator!=(const Annotated<Annotation, Type>& lhs, const Annotated<Annotation, Type>& rhs) {
            return !(lhs == rhs);
        };
        template <typename Annotation, typename Type> inline bool operator==(const Annotated<Annotation, Type>& lhs, const std::shared_ptr<Type>& rhs) {
            return lhs.get() == rhs.get();
        };
        template <typename Annotation, typename Type> inline bool operator!=(const Annotated<Annotation, Type>& lhs, const std::shared_ptr<Type>& rhs) {
            return !(lhs == rhs);
        };
    }
}
#endif