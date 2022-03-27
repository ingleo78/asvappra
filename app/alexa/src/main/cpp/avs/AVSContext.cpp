#include <json/JSONGenerator.h>
#include <logger/Logger.h>
#include "AVSContext.h"
#include "EventBuilder.h"
#include "CapabilityTag.h"
#include "CapabilityState.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            static const std::string PROPERTIES_KEY_STRING = "properties";
            static const std::string INSTANCE_KEY_STRING = "instance";
            static const std::string VALUE_KEY_STRING = "value";
            static const std::string TIME_OF_SAMPLE_KEY_STRING = "timeOfSample";
            static const std::string UNCERTAINTY_KEY_STRING = "uncertaintyInMilliseconds";
            static const std::string TAG("AVSContext");
            #define LX(event) utils::logger::LogEntry(TAG, event)
            utils::Optional<CapabilityState> AVSContext::getState(const CapabilityTag& identifier) const {
                auto it = m_states.find(identifier);
                return (it != m_states.end()) ? utils::Optional<CapabilityState>(it->second) : utils::Optional<CapabilityState>();
            }
            std::map<CapabilityTag, CapabilityState> AVSContext::getStates() const { return m_states; }
            void AVSContext::addState(const CapabilityTag& identifier, const CapabilityState& state) {
                m_states.insert(std::make_pair(identifier, state));
            }
            void AVSContext::removeState(const CapabilityTag& identifier) { m_states.erase(identifier); }
            std::string AVSContext::toJson() const {
                utils::json::JsonGenerator jsonGenerator;
                jsonGenerator.startArray(PROPERTIES_KEY_STRING);
                for (const auto& element : m_states) {
                    auto& identifier = element.first;
                    auto& state = element.second;
                    if (!state.valuePayload.empty()) {
                        jsonGenerator.startArrayElement();
                        jsonGenerator.addMember(constants::NAMESPACE_KEY_STRING, identifier.nameSpace);
                        jsonGenerator.addMember(constants::NAME_KEY_STRING, identifier.name);
                        if (identifier.instance.hasValue()) jsonGenerator.addMember(INSTANCE_KEY_STRING, identifier.instance.value());
                        jsonGenerator.addRawJsonMember(VALUE_KEY_STRING, state.valuePayload);
                        jsonGenerator.addMember(TIME_OF_SAMPLE_KEY_STRING, state.timeOfSample.getTime_ISO_8601());
                        jsonGenerator.addMember(UNCERTAINTY_KEY_STRING, state.uncertaintyInMilliseconds);
                        jsonGenerator.finishArrayElement();
                    } else { ACSDK_DEBUG0(LX(__func__).d("stateIgnored", identifier.nameSpace + "::" + identifier.name)); }
                }
                jsonGenerator.finishArray();
                ACSDK_DEBUG5(LX(__func__).sensitive("context", jsonGenerator.toString()));
                return jsonGenerator.toString();
            }
        }
    }
}