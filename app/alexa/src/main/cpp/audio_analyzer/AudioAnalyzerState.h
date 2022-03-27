#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_AUDIOANALYZER_AUDIOANALYZERSTATE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_AUDIOANALYZER_AUDIOANALYZERSTATE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace audioAnalyzer {
                struct AudioAnalyzerState {
                    AudioAnalyzerState(const std::string& interfaceName, const std::string& enableState) : name(interfaceName),
                                                                                                           enableState(enableState) {}
                    std::string name;
                    std::string enableState;
                    bool operator==(const AudioAnalyzerState& state) const {
                        bool state1 = enableState == state.enableState;
                        bool state2 = name == state.name;
                        return state1 && state2;
                    }
                };
            }
        }
    }
}
#endif