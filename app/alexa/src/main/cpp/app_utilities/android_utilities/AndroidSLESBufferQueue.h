#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESBUFFERQUEUE_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDSLESBUFFERQUEUE_H_

#include <array>
#include <vector>
#include <SLES/OpenSLES_Android.h>
#include <avs/AudioInputStream.h>
#include "AndroidSLESObject.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            using namespace std;
            using namespace avsCommon::avs;
            class AndroidSLESBufferQueue {
            public:
                static unique_ptr<AndroidSLESBufferQueue> create(shared_ptr<AndroidSLESObject> queueObject, shared_ptr<AudioInputStream::Writer> writer);
                void onBufferCompleted();
                bool enqueueBuffers();
                bool clearBuffers();
                ~AndroidSLESBufferQueue();
                static constexpr uint32_t NUMBER_OF_BUFFERS{2u};
            private:
                AndroidSLESBufferQueue(shared_ptr<AndroidSLESObject> slObject, SLAndroidSimpleBufferQueueItf bufferQueue, shared_ptr<AudioInputStream::Writer> writer);
                bool enqueueBufferLocked();
                static constexpr size_t BUFFER_SIZE{8192u};
                mutex m_mutex;
                shared_ptr<AndroidSLESObject> m_slObject;
                SLAndroidSimpleBufferQueueItf m_queueInterface;
                array<array<int16_t, BUFFER_SIZE>, NUMBER_OF_BUFFERS> m_buffers;
                shared_ptr<AudioInputStream::Writer> m_writer;
                size_t m_index;
            };
        }
    }
}
#endif