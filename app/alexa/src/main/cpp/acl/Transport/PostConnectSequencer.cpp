#include <logger/Logger.h>
#include "PostConnectSequencer.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        static const std::string TAG("PostConnectSequencer");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<PostConnectSequencer> PostConnectSequencer::create(const PostConnectOperationsSet& postConnectOperations) {
            for (auto& postConnectOperation : postConnectOperations) {
                if (!postConnectOperation) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "invalid PostConnectOperation found"));
                    return nullptr;
                }
            }
            return std::shared_ptr<PostConnectSequencer>(new PostConnectSequencer(postConnectOperations));
        }
        PostConnectSequencer::PostConnectSequencer(const PostConnectOperationsSet& postConnectOperations) : m_isStopping{false},
                                                   m_postConnectOperations{postConnectOperations} {}
        PostConnectSequencer::~PostConnectSequencer() {
            ACSDK_DEBUG5(LX(__func__));
            stop();
        }
        bool PostConnectSequencer::doPostConnect(shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver) {
            ACSDK_DEBUG5(LX(__func__));
            if (!postConnectSender) {
                ACSDK_ERROR(LX("doPostConnectFailed").d("reason", "nullPostConnectSender"));
                return false;
            }
            if (!postConnectObserver) {
                ACSDK_ERROR(LX("doPostConnectFailed").d("reason", "nullPostConnectObserver"));
                return false;
            }
            {
                lock_guard<mutex> lock{m_mutex};
                if (m_mainLoopThread.joinable()) {
                    ACSDK_ERROR(LX("doPostConnectFailed").d("reason", "mainLoop already running"));
                    return false;
                }
                m_mainLoopThread = thread(&PostConnectSequencer::mainLoop, this, postConnectSender, postConnectObserver);
            }
            return true;
        }
        void PostConnectSequencer::mainLoop(shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver) {
            ACSDK_DEBUG5(LX(__func__));
            if (!postConnectSender) {
                ACSDK_ERROR(LX("mainLoopError").d("reason", "nullPostConnectSender"));
                return;
            }
            if (!postConnectObserver) {
                ACSDK_ERROR(LX("mainLoopError").d("reason", "nullPostConnectObserver"));
                return;
            }
            for (auto postConnectOperation : m_postConnectOperations) {
                {
                    lock_guard<std::mutex> lock{m_mutex};
                    if (m_isStopping) {
                        ACSDK_DEBUG5(LX(__func__).m("stop called, exiting mainloop"));
                        return;
                    }
                    m_currentPostConnectOperation = postConnectOperation;
                }
                if (!m_currentPostConnectOperation->performOperation(postConnectSender)) {
                    if (!isStopping() && postConnectObserver) postConnectObserver->onUnRecoverablePostConnectFailure();
                    resetCurrentOperation();
                    ACSDK_ERROR(LX(__func__).m("performOperation failed, exiting mainloop"));
                    return;
                }
            }
            resetCurrentOperation();
            if (postConnectObserver) postConnectObserver->onPostConnected();
            ACSDK_DEBUG5(LX("mainLoopReturning"));
        }
        void PostConnectSequencer::onDisconnect() {
            ACSDK_DEBUG5(LX(__func__));
            stop();
        }
        void PostConnectSequencer::resetCurrentOperation() {
            lock_guard<mutex> lock{m_mutex};
            m_currentPostConnectOperation.reset();
        }
        bool PostConnectSequencer::isStopping() {
            lock_guard<mutex> lock{m_mutex};
            return m_isStopping;
        }
        void PostConnectSequencer::stop() {
            ACSDK_DEBUG5(LX(__func__));
            {
                lock_guard<mutex> lock{m_mutex};
                if (m_isStopping) return;
                m_isStopping = true;
                if (m_currentPostConnectOperation) m_currentPostConnectOperation->abortOperation();
            }
            {
                lock_guard<mutex> lock{m_mainLoopThreadMutex};
                if (m_mainLoopThread.joinable()) m_mainLoopThread.join();
            }
        }
    }
}