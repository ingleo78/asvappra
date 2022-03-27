#include <avs/EventBuilder.h>
#include <json/JSONGenerator.h>
#include "StateReportGenerator.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace constants;
            using namespace rapidjson;
            using namespace json;
            static const string TAG("StateReportGenerator");
            #define LX(event) LogEntry(TAG, event)
            StateReportGenerator::StateReportGenerator(const vector<function<string()>>& reportFunctions) :
                    m_reportFunctions{reportFunctions} {
            }
            vector<string> StateReportGenerator::generateReport() {
                vector<string> states;
                for (auto& reportFn : m_reportFunctions) {
                    auto state = reportFn();
                    if (!state.empty()) states.push_back(state);
                }
                return states;
            }
            string StateReportGenerator::generateSettingStateReport(const SettingEventMetadata& metadata, const string& value) {
                if (value.empty()) {
                    ACSDK_DEBUG5(LX(__func__).d("emptySetting", metadata.settingName));
                    return string();
                }
                JsonGenerator jsonGenerator;
                jsonGenerator.startObject(HEADER_KEY_STRING);
                {
                    jsonGenerator.addMember(NAMESPACE_KEY_STRING, metadata.eventNamespace);
                    jsonGenerator.addMember(NAME_KEY_STRING, metadata.eventReportName);
                }
                jsonGenerator.finishObject();
                jsonGenerator.startObject(PAYLOAD_KEY_STRING);
                { jsonGenerator.addRawJsonMember(metadata.settingName, value); }
                jsonGenerator.finishObject();
                auto report = jsonGenerator.toString();
                ACSDK_DEBUG5(LX(__func__).sensitive("settingReport", report));
                return report;
            }
        }
    }
}