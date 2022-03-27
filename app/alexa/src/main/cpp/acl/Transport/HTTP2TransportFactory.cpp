#include <threading/Executor.h>
#include <configuration/ConfigurationNode.h>
#include "HTTP2TransportFactory.h"
#include "HTTP2Transport.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace sdkInterfaces;
        using namespace avs;
        using namespace attachment;
        shared_ptr<TransportInterface> HTTP2TransportFactory::createTransport(shared_ptr<AuthDelegateInterface> authDelegate,
                                                                              shared_ptr<AttachmentManager> attachmentManager, const string& avsGateway,
                                                                              shared_ptr<MessageConsumerInterface> messageConsumerInterface,
                                                                              shared_ptr<TransportObserverInterface> transportObserverInterface,
                                                                              shared_ptr<SynchronizedMessageRequestQueue> sharedMessageRequestQueue) {
            auto connection = m_connectionFactory->createHTTP2Connection();
            if (!connection) return nullptr;
            return HTTP2Transport::create(authDelegate, avsGateway, connection, messageConsumerInterface, attachmentManager, transportObserverInterface,
                         m_postConnectFactory, sharedMessageRequestQueue,HTTP2Transport::Configuration(),m_metricRecorder,
                               m_eventTracer);
        }
        HTTP2TransportFactory::HTTP2TransportFactory(shared_ptr<HTTP2ConnectionFactoryInterface> connectionFactory,
                                                     shared_ptr<PostConnectFactoryInterface> postConnectFactory,
                                                     shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<EventTracerInterface> eventTracer) :
                                                     m_connectionFactory{move(connectionFactory)}, m_postConnectFactory{move(postConnectFactory)},
                                                     m_metricRecorder{move(metricRecorder)}, m_eventTracer{eventTracer} {}
    }
}