#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_HTTP2TRANSPORTFACTORY_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_HTTP2TRANSPORTFACTORY_H_

#include <memory>
#include <string>
#include <avs/attachment/AttachmentManager.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/EventTracerInterface.h>
#include <util/HTTP2/HTTP2ConnectionFactoryInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include "MessageConsumerInterface.h"
#include "PostConnectFactoryInterface.h"
#include "SynchronizedMessageRequestQueue.h"
#include "TransportFactoryInterface.h"
#include "TransportObserverInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace sdkInterfaces;
        using namespace avs;
        using namespace metrics;
        using namespace http2;
        using namespace attachment;
        class HTTP2TransportFactory : public TransportFactoryInterface {
        public:
            HTTP2TransportFactory(shared_ptr<HTTP2ConnectionFactoryInterface> connectionFactory, shared_ptr<PostConnectFactoryInterface> postConnectFactory,
                                  shared_ptr<MetricRecorderInterface> metricRecorder = nullptr, shared_ptr<EventTracerInterface> eventTracer = nullptr);
            shared_ptr<TransportInterface> createTransport(shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AttachmentManager> attachmentManager,
                                                           const string& avsGateway, shared_ptr<MessageConsumerInterface> messageConsumerInterface,
                                                           shared_ptr<TransportObserverInterface> transportObserverInterface,
                                                           shared_ptr<SynchronizedMessageRequestQueue> sharedMessageRequestQueue) override;
            HTTP2TransportFactory() = delete;
        private:
            shared_ptr<HTTP2ConnectionFactoryInterface> m_connectionFactory;
            shared_ptr<PostConnectFactoryInterface> m_postConnectFactory;
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            shared_ptr<EventTracerInterface> m_eventTracer;
        };
    }
}
#endif