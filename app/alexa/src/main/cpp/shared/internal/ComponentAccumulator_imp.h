#ifndef ACSDKMANUFACTORY_INTERNAL_COMPONENTACCUMULATOR_IMP_H_
#define ACSDKMANUFACTORY_INTERNAL_COMPONENTACCUMULATOR_IMP_H_

#include <shared/Component.h>
#include <shared/ComponentAccumulator.h>

namespace alexaClientSDK {
    namespace acsdkManufactory {
        template <> ComponentAccumulator<>::ComponentAccumulator() {}
        template <typename... Parameters>template <typename... RhsParameters>
        ComponentAccumulator<Parameters...>::ComponentAccumulator(const ComponentAccumulator<RhsParameters...>& rhs) : m_cookBook{rhs.getCookBook()} {}
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addUniqueFactory(Type (*factory)(Dependencies...)) {
            m_cookBook.addUniqueFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addPrimaryFactory(std::function<Type(Dependencies...)> factory) {
            m_cookBook.addPrimaryFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addPrimaryFactory(Type (*factory)(Dependencies...)) {
            m_cookBook.addPrimaryFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addRequiredFactory(std::function<Type(Dependencies...)> factory) {
            m_cookBook.addRequiredFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addRequiredFactory(Type (*factory)(Dependencies...)) {
            m_cookBook.addRequiredFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addRetainedFactory(std::function<Type(Dependencies...)> factory) {
            m_cookBook.addRetainedFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addRetainedFactory(Type (*factory)(Dependencies...)) {
            m_cookBook.addRetainedFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addUnloadableFactory(std::function<Type(Dependencies...)> factory) {
            m_cookBook.addUnloadableFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type, typename... Dependencies>
        ComponentAccumulator<Import<internal::RemoveCvref_t<Dependencies>>..., Type, Parameters...> ComponentAccumulator<Parameters...>::addUnloadableFactory(Type (*factory)(Dependencies...)) {
            m_cookBook.addUnloadableFactory(factory);
            return *this;
        }
        template <typename... Parameters>template <typename Type>
        ComponentAccumulator<Type, Parameters...> ComponentAccumulator<Parameters...>::addInstance(Type instance) {
            m_cookBook.addInstance(instance);
            return *this;
        }
        template <typename... Parameters>template <typename... SubComponentParameters>
        ComponentAccumulator<SubComponentParameters..., Parameters...> ComponentAccumulator<Parameters...>::addComponent(const Component<SubComponentParameters...>& component) {
            m_cookBook.addCookBook(component.getCookBook());
            return *this;
        }
        template <typename... Parameters> internal::CookBook ComponentAccumulator<Parameters...>::getCookBook() const {
            return m_cookBook;
        }
    }
}
#endif