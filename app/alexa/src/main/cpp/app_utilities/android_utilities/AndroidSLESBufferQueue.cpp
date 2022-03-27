#include <string>
#include "AndroidSLESBufferQueue.h"
#include "AndroidSLESObject.h"

static const std::string TAG{"AndroidSLESBufferQueue"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            using namespace std;
            using namespace avsCommon::avs;
            AndroidSLESBufferQueue::AndroidSLESBufferQueue(shared_ptr<AndroidSLESObject> object, SLAndroidSimpleBufferQueueItf bufferQueue,
                                                           shared_ptr<AudioInputStream::Writer> writer) : m_slObject{object}, m_queueInterface{bufferQueue},
                                                           m_writer{writer}, m_index{0} {}
            void AndroidSLESBufferQueue::onBufferCompleted() {
                std::lock_guard<std::mutex> lock{m_mutex};
                m_writer->write(m_buffers[m_index].data(), m_buffers[m_index].size());
                enqueueBufferLocked();
            }
            static void recorderCallback(SLAndroidSimpleBufferQueueItf slQueue, void* bufferQueue) {
                static_cast<AndroidSLESBufferQueue*>(bufferQueue)->onBufferCompleted();
            };
            unique_ptr<AndroidSLESBufferQueue> AndroidSLESBufferQueue::create(shared_ptr<AndroidSLESObject> queueObject, shared_ptr<AudioInputStream::Writer> writer) {
                SLAndroidSimpleBufferQueueItf queueInterface;
                if (!queueObject->getInterface(SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &queueInterface)) {
                    ACSDK_ERROR(LX("initializeAndroidMicFailed").d("reason", "Failed to get buffer queue."));
                    return nullptr;
                }
                auto bufferQueue = unique_ptr<AndroidSLESBufferQueue>(new AndroidSLESBufferQueue(queueObject, queueInterface, move(writer)));
                if ((*queueInterface)->RegisterCallback(queueInterface, recorderCallback, bufferQueue.get()) != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("initializeAndroidMicFailed").d("reason", "Failed to register callback."));
                    return nullptr;
                }
                return bufferQueue;
            }
            AndroidSLESBufferQueue::~AndroidSLESBufferQueue() {
                if (m_queueInterface) {
                    if (!clearBuffers() || (*m_queueInterface)->RegisterCallback(m_queueInterface, nullptr, nullptr) != SL_RESULT_SUCCESS) {
                        ACSDK_WARN(LX("cleanBufferQueueFailed"));
                    }
                }
            }
            bool AndroidSLESBufferQueue::enqueueBuffers() {
                lock_guard<std::mutex> lock{m_mutex};
                SLAndroidSimpleBufferQueueState state;
                auto result = (*m_queueInterface)->GetState(m_queueInterface, &state);
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("enqueueBuffersFailed").d("reason", "getStateFailed").d("result", result));
                    return false;
                }
                for (size_t i = state.count; i < NUMBER_OF_BUFFERS; ++i) {
                    if (!enqueueBufferLocked()) {
                        if (i == 0) {
                            ACSDK_ERROR(LX("enqueueBuffersFailed").d("reason", "noBufferEnqueued"));
                            return false;
                        }
                        ACSDK_WARN(LX("enqueueBuffersIncomplete").d("reason", "failedToEnqueueAllBuffers").d("enqueued", i));
                        break;
                    }
                }
                return true;
            }
            bool AndroidSLESBufferQueue::clearBuffers() {
                lock_guard<std::mutex> lock{m_mutex};
                auto result = (*m_queueInterface)->Clear(m_queueInterface);
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("clearBuffersFailed").d("result", result));
                    return false;
                }
                return true;
            }
            bool AndroidSLESBufferQueue::enqueueBufferLocked() {
                auto result = (*m_queueInterface)->Enqueue(m_queueInterface, m_buffers[m_index].data(), m_buffers[m_index].size() * sizeof(m_buffers[m_index][0]));
                if (result != SL_RESULT_SUCCESS) {
                    ACSDK_ERROR(LX("enqueueBufferFailed").d("result", result));
                    return false;
                }
                m_index = (m_index + 1) % NUMBER_OF_BUFFERS;
                return true;
            }
        }
    }
}