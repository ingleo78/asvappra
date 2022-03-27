#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_HTTP2TRANSPORT_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_HTTP2TRANSPORT_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <avs/attachment/AttachmentManager.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/EventTracerInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/HTTP2/HTTP2ConnectionInterface.h>
#include <util/HTTP2/HTTP2ConnectionObserverInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include "MessageConsumerInterface.h"
#include "PingHandler.h"
#include "PostConnectFactoryInterface.h"
#include "PostConnectObserverInterface.h"
#include "TransportInterface.h"
#include "TransportObserverInterface.h"
#include "SynchronizedMessageRequestQueue.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace sdkInterfaces;
        using namespace metrics;
        using namespace http2;
        using namespace attachment;
        using namespace metrics;
        class HTTP2Transport : public enable_shared_from_this<HTTP2Transport> , public TransportInterface , public PostConnectObserverInterface,
                               public MessageSenderInterface, public AuthObserverInterface , public HTTP2ConnectionObserverInterface,
                               public ExchangeHandlerContextInterface {
        public:
            struct Configuration {
                Configuration();
                seconds inactivityTimeout;
            };
            static shared_ptr<HTTP2Transport> create(shared_ptr<AuthDelegateInterface> authDelegate, const string& avsGateway,
                                                     shared_ptr<HTTP2ConnectionInterface> http2Connection, shared_ptr<MessageConsumerInterface> messageConsumer,
                                                     shared_ptr<AttachmentManager> attachmentManager, shared_ptr<TransportObserverInterface> transportObserver,
                                                     shared_ptr<PostConnectFactoryInterface> postConnectFactory,
                                                     shared_ptr<SynchronizedMessageRequestQueue> sharedRequestQueue, Configuration configuration = Configuration(),
                                                     shared_ptr<MetricRecorderInterface> metricRecorder = nullptr,
                                                     shared_ptr<EventTracerInterface> eventTracer = nullptr);
            void addObserver(shared_ptr<TransportObserverInterface> transportObserver);
            void removeObserver(shared_ptr<TransportObserverInterface> observer);
            shared_ptr<HTTP2ConnectionInterface> getHTTP2Connection();
            bool connect() override;
            void disconnect() override;
            bool isConnected() override;
            void onRequestEnqueued() override;
            void onWakeConnectionRetry() override;
            void onWakeVerifyConnectivity() override;
            void sendMessage(shared_ptr<MessageRequest> request) override;
            void onPostConnected() override;
            void onUnRecoverablePostConnectFailure() override;
            void onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error error) override;
            void doShutdown() override;
            void onDownchannelConnected() override;
            void onDownchannelFinished() override;
            void onMessageRequestSent(const shared_ptr<MessageRequest>& request) override;
            void onMessageRequestTimeout() override;
            void onMessageRequestAcknowledged(const shared_ptr<MessageRequest>& request) override;
            void onMessageRequestFinished() override;
            void onPingRequestAcknowledged(bool success) override;
            void onPingTimeout() override;
            void onActivity() override;
            void onForbidden(const string& authToken = "") override;
            shared_ptr<HTTP2RequestInterface> createAndSendRequest(const HTTP2RequestConfig& cfg) override;
            string getAVSGateway() override;
            void onGoawayReceived() override;
        private:
            enum class State {
                INIT,
                AUTHORIZING,
                CONNECTING,
                WAITING_TO_RETRY_CONNECTING,
                POST_CONNECTING,
                CONNECTED,
                SERVER_SIDE_DISCONNECT,
                DISCONNECTING,
                SHUTDOWN
            };
            friend ostream& operator<<(ostream& stream, HTTP2Transport::State state);
            HTTP2Transport(shared_ptr<AuthDelegateInterface> authDelegate, const string& avsGateway, shared_ptr<HTTP2ConnectionInterface> http2Connection,
                           shared_ptr<MessageConsumerInterface> messageConsumer, shared_ptr<AttachmentManager> attachmentManager,
                           shared_ptr<TransportObserverInterface> transportObserver, shared_ptr<PostConnectFactoryInterface> postConnectFactory,
                           shared_ptr<SynchronizedMessageRequestQueue> sharedRequestQueue, Configuration configuration,
                           shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<EventTracerInterface> eventTracer);
            void mainLoop();
            State handleInit();
            State handleAuthorizing();
            State handleConnecting();
            State handleWaitingToRetryConnecting();
            State handlePostConnecting();
            State handleConnected();
            State handleServerSideDisconnect();
            State handleDisconnecting();
            State handleShutdown();
            State monitorSharedQueueWhileWaiting(State whileState, time_point<steady_clock> maxWakeTime = time_point<steady_clock>::max());
            State sendMessagesAndPings(State whileState, MessageRequestQueueInterface& requestQueue);
            bool setState(State newState, ConnectionStatusObserverInterface::ChangedReason changedReason);
            bool setStateLocked(State newState, ConnectionStatusObserverInterface::ChangedReason reason);
            void notifyObserversOnConnected();
            void notifyObserversOnDisconnect(ConnectionStatusObserverInterface::ChangedReason reason);
            void notifyObserversOnServerSideDisconnect();
            State getState();
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            mutex m_mutex;
            condition_variable m_wakeEvent;
            State m_state;
            shared_ptr<AuthDelegateInterface> m_authDelegate;
            string m_avsGateway;
            shared_ptr<HTTP2ConnectionInterface> m_http2Connection;
            shared_ptr<MessageConsumerInterface> m_messageConsumer;
            shared_ptr<AttachmentManager> m_attachmentManager;
            shared_ptr<PostConnectFactoryInterface> m_postConnectFactory;
            shared_ptr<SynchronizedMessageRequestQueue> m_sharedRequestQueue;
            shared_ptr<EventTracerInterface> m_eventTracer;
            mutex m_observerMutex;
            unordered_set<shared_ptr<TransportObserverInterface>> m_observers;
            thread m_thread;
            shared_ptr<PostConnectInterface> m_postConnect;
            MessageRequestQueue m_requestQueue;
            int m_connectRetryCount;
            int m_countOfUnfinishedMessageHandlers;
            shared_ptr<PingHandler> m_pingHandler;
            time_point<steady_clock> m_timeOfLastActivity;
            atomic<bool> m_postConnected;
            const Configuration m_configuration;
            ConnectionStatusObserverInterface::ChangedReason m_disconnectReason;
        };
    }
}
#endif