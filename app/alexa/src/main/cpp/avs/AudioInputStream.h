#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AUDIOINPUTSTREAM_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AUDIOINPUTSTREAM_H_

#include <util/sds/SharedDataStream.h>
#ifdef CUSTOM_SDS_TRAITS_HEADER
#include CUSTOM_SDS_TRAITS_HEADER
#else
#include <util/sds/InProcessSDS.h>
#endif

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace utils::sds;
        #ifdef CUSTOM_SDS_TRAITS_CLASS
            using AudioInputStream = SharedDataStream<CUSTOM_SDS_TRAITS_CLASS>;
        #else
            using AudioInputStream = SharedDataStream<InProcessSDS>;
        #endif
        }
    }
}
#endif