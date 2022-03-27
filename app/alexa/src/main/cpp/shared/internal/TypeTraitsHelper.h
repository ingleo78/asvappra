#ifndef ACSDKMANUFACTORY_INTERNAL_TYPETRAITSHELPER_H_
#define ACSDKMANUFACTORY_INTERNAL_TYPETRAITSHELPER_H_

#include <type_traits>

namespace alexaClientSDK {
    namespace acsdkManufactory {
        namespace internal {
            template <class Type> struct RemoveCvref {
                typedef typename std::remove_cv<typename std::remove_reference<Type>::type>::type type;
            };
            template <class Type> using RemoveCvref_t = typename RemoveCvref<Type>::type;
        }
    }
}
#endif