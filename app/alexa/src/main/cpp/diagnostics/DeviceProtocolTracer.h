#ifndef ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_DEVICEPROTOCOLTRACER_H_
#define ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_DEVICEPROTOCOLTRACER_H_

#include <memory>
#include <mutex>
#include <vector>
#include <sdkinterfaces/Diagnostics/ProtocolTracerInterface.h>

namespace alexaClientSDK {
    namespace diagnostics {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace configuration;
        using namespace logger;
        using namespace json;
        using namespace jsonUtils;
        using namespace rapidjson;
        using namespace sdkInterfaces::diagnostics;
        class DeviceProtocolTracer : public ProtocolTracerInterface {
        public:
            static shared_ptr<DeviceProtocolTracer> create();
            unsigned int getMaxMessages() override;
            bool setMaxMessages(unsigned int limit) override;
            void setProtocolTraceFlag(bool enabled) override;
            string getProtocolTrace() override;
            void clearTracedMessages() override;
            void traceEvent(const string& messageContent) override;
            void receive(const string& contextId, const string& message) override;
        private:
            DeviceProtocolTracer(unsigned int maxMessages);
            void clearTracedMessagesLocked();
            void traceMessageLocked(const string& messageContent);
            mutex m_mutex;
            bool m_isProtocolTraceEnabled;
            unsigned int m_maxMessages;
            vector<string> m_tracedMessages;
        };
    }
}
#endif