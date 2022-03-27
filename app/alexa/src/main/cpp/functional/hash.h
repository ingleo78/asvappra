#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_FUNCTIONAL_HASH_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_FUNCTIONAL_HASH_H_

#include <cstdlib>
#include <climits>
#include <functional>
#include <limits.h>

typedef unsigned int size_t;

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace functional {
                template <typename Type> void hashCombine(size_t& seed, Type const& value) {
                    constexpr int bitsMinus1 = (CHAR_BIT * sizeof(size_t)) - 1;
                    std::hash<Type> hasher;
                    seed = hasher(value) ^ ((seed << 1) | ((seed >> bitsMinus1) & 1));
                }
                struct EnumClassHash {
                    template <typename T>size_t operator()(T t) const {
                        return static_cast<size_t>(t);
                    }
                };
            }
        }
    }
}
#endif