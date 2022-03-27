#include <json/JSONGenerator.h>
#include <logger/Logger.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "DeviceProtocolTracer.h"

namespace alexaClientSDK {
    namespace diagnostics {
        static const string TAG("DeviceProtocolTracer");
        static const unsigned int DEFAULT_MAX_MESSAGES = 1;
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<DeviceProtocolTracer> DeviceProtocolTracer::create() {
            return shared_ptr<DeviceProtocolTracer>(new DeviceProtocolTracer(DEFAULT_MAX_MESSAGES));
        }
        DeviceProtocolTracer::DeviceProtocolTracer(unsigned int maxMessages) :
                m_isProtocolTraceEnabled{false},
                m_maxMessages(maxMessages) {
        }
        unsigned int DeviceProtocolTracer::getMaxMessages() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_mutex);
            return m_maxMessages;
        }
        bool DeviceProtocolTracer::setMaxMessages(unsigned int limit) {
            lock_guard<mutex> lock(m_mutex);
            ACSDK_DEBUG5(LX(__func__).d("current", m_maxMessages).d("new", limit));
            if (limit < m_tracedMessages.size()) {
                ACSDK_ERROR(LX(__func__).d("reason", "storedMessagesExceedLimit").d("storedMessages", m_tracedMessages.size()).d("limt", limit));
                return false;
            }
            m_maxMessages = limit;
            return true;
        }
        void DeviceProtocolTracer::setProtocolTraceFlag(bool enabled) {
            ACSDK_DEBUG5(LX(__func__).d("enabled", enabled));
            std::lock_guard<std::mutex> lock(m_mutex);
            m_isProtocolTraceEnabled = enabled;
        }
        void DeviceProtocolTracer::clearTracedMessages() {
            lock_guard<mutex> lock{m_mutex};
            clearTracedMessagesLocked();
        }
        void DeviceProtocolTracer::clearTracedMessagesLocked() {
            m_tracedMessages.clear();
        }
        void DeviceProtocolTracer::receive(const string& contextId, const string& message) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock{m_mutex};
            traceMessageLocked(message);
        }
        void DeviceProtocolTracer::traceEvent(const string& message) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock{m_mutex};
            traceMessageLocked(message);
        }
        void DeviceProtocolTracer::traceMessageLocked(const string& messageContent) {
            if (m_isProtocolTraceEnabled) {
                if (m_tracedMessages.size() < m_maxMessages) m_tracedMessages.push_back(messageContent);
                else { ACSDK_WARN(LX(__func__).d("maxMessages", m_maxMessages).m("reached max trace message limit.")); }
            } else { ACSDK_DEBUG5(LX(__func__).m("protocol trace disabled")); }
        }
        string DeviceProtocolTracer::getProtocolTrace() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock{m_mutex};
            StringBuffer buffer;
            rapidjson::Writer<StringBuffer> writer(buffer);
            writer.StartArray();
            for (const auto& message : m_tracedMessages) {
                writer.RawValue(message.c_str(), message.length(),kStringType);
            }
            writer.EndArray();
            return buffer.GetString();
        }
    }
}