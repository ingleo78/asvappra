#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MEDIACONTEXT_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MEDIACONTEXT_H_

#include <sbc/sbc.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class MediaContext {
            public:
                static constexpr int INVALID_FD = -1;
                ~MediaContext();
                MediaContext();
                void setStreamFD(int streamFD);
                int getStreamFD();
                void setReadMTU(int readMTU);
                int getReadMTU();
                void setWriteMTU(int writeMTU);
                int getWriteMTU();
                sbc_t* getSBCContextPtr();
                bool isSBCInitialized();
                void setSBCInitialized(bool initialized);
            private:
                int m_mediaStreamFD;
                int m_readMTU;
                int m_writeMTU;
                sbc_t m_sbcContext;
                bool m_isSBCInitialized;
            };
        }
    }
}
#endif