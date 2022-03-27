#ifndef ACSDKMANUFACTORY_INTERNAL_COMPONENT_IMP_H_
#define ACSDKMANUFACTORY_INTERNAL_COMPONENT_IMP_H_

#include <shared/Component.h>
#include "Utils.h"

namespace alexaClientSDK {
namespace acsdkManufactory {

template <typename... Parameters>
template <typename... ComponentAccumulatorParameters>
Component<Parameters...>::Component(ComponentAccumulator<ComponentAccumulatorParameters...>&& componentAccumulator) :
        m_cookBook{componentAccumulator.getCookBook()} {
    using AccumulatorTypes = typename internal::GetImportsAndExports<ComponentAccumulatorParameters...>::type;

    using ComponentTypes = typename internal::GetImportsAndExports<Parameters...>::type;

    using MissingExports =
        typename internal::RemoveTypes<typename ComponentTypes::exports, typename AccumulatorTypes::exports>::type;

    ACSDK_STATIC_ASSERT_IS_SAME(
        std::tuple<>, MissingExports, "One or more export declared by this Component was not provided.");

    using MissingImports =
        typename internal::RemoveTypes<typename AccumulatorTypes::imports, typename ComponentTypes::imports>::type;

    ACSDK_STATIC_ASSERT_IS_SAME(
        std::tuple<>, MissingImports, "One or more Import<Type> was not declared by this Component.");
}

template <typename... Parameters>
internal::CookBook Component<Parameters...>::getCookBook() const {
    return m_cookBook;
}

}  // namespace acsdkManufactory
}  // namespace alexaClientSDK

#endif  // ACSDKMANUFACTORY_INTERNAL_COMPONENT_IMP_H_