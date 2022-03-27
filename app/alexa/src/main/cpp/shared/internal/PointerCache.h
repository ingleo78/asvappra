#ifndef ACSDKMANUFACTORY_INTERNAL_POINTERCACHE_H_
#define ACSDKMANUFACTORY_INTERNAL_POINTERCACHE_H_

#include "AbstractPointerCache.h"

namespace alexaClientSDK {
    namespace acsdkManufactory {
        namespace internal {
            class RuntimeManufactory;
            template <typename Type> class PointerCache : public AbstractPointerCache {
            public:
                virtual Type get(RuntimeManufactory& runtimeManufactory) = 0;
            };
        }
    }
}
#endif