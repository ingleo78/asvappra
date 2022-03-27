#ifndef ACSDKALERTS_RENDERER_RENDEREROBSERVERINTERFACE_H_
#define ACSDKALERTS_RENDERER_RENDEREROBSERVERINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace renderer {
            using namespace std;
            class RendererObserverInterface {
            public:
                enum class State {
                    UNSET,
                    STARTED,
                    STOPPED,
                    COMPLETED,
                    ERROR
                };
                virtual ~RendererObserverInterface() = default;
                virtual void onRendererStateChange(State state, const std::string& reason = "") = 0;
                static string stateToString(State state);
            };
            inline string RendererObserverInterface::stateToString(State state) {
                switch(state) {
                    case State::UNSET: return "UNSET";
                    case State::STARTED: return "STARTED";
                    case State::STOPPED: return "STOPPED";
                    case State::COMPLETED: return "COMPLETED";
                    case State::ERROR: return "ERROR";
                }
                return "unknown State";
            }
            inline ostream& operator<<(ostream& stream, const RendererObserverInterface::State& state) {
                return stream << RendererObserverInterface::stateToString(state);
            }
        }
    }
}
#endif