#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLATFORMDEFINITIONS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLATFORMDEFINITIONS_H_
    #if defined(_MSC_VER)
        #include <BaseTsd.h>
        typedef SSIZE_T ssize_t;
    #endif
    #if defined(_MSC_VER)
        #if defined(IN_AVSCOMMON)
            #define avscommon_EXPORT __declspec(dllexport)
        #else
            #define avscommon_EXPORT __declspec(dllimport)
        #endif
    #else
        #define avscommon_EXPORT
    #endif
#endif