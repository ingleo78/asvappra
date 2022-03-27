#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_PULSEAUDIOBLUETOOTHINITIALIZER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_PULSEAUDIOBLUETOOTHINITIALIZER_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <util/bluetooth/BluetoothEventBus.h>
#include <threading/Executor.h>
#include <pulse_audio/pulse/pulseaudio.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace threading;
            using namespace utils::bluetooth;
            class PulseAudioBluetoothInitializer : public BluetoothEventListenerInterface, public enable_shared_from_this<PulseAudioBluetoothInitializer> {
            public:
                enum class ModuleState {
                    UNKNOWN,
                    INITIALLY_LOADED,
                    UNLOADED,
                    LOADED_BY_SDK,
                };
                static shared_ptr<PulseAudioBluetoothInitializer> create(shared_ptr<BluetoothEventBus> eventBus);
                ~PulseAudioBluetoothInitializer();
            protected:
                void onEventFired(const BluetoothEvent& event) override;
            private:
                static void onStateChanged(pa_context* context, void* userdata);
                static void onModuleFound(pa_context* context, const pa_module_info* info, int eol, void* userData);
                static void onLoadPolicyResult(pa_context* context, uint32_t index, void* userdata);
                static void onLoadDiscoverResult(pa_context* context, uint32_t index, void* userdata);
                static void onUnloadPolicyResult(pa_context* context, int success, void* userdata);
                static void onUnloadDiscoverResult(pa_context* context, int success, void* userdata);
                void handleLoadModuleResult(pa_context* context, uint32_t index, const std::string& moduleName);
                void handleUnloadModuleResult(pa_context* context, int success, const std::string& moduleName);
                bool updateStateLocked(const ModuleState& state, const std::string& module);
                void setStateAndNotify(pa_context_state_t state);
                void init();
                void run();
                void cleanup();
                PulseAudioBluetoothInitializer(shared_ptr<BluetoothEventBus> eventBus);
                condition_variable m_mainThreadCv;
                mutex m_mutex;
                shared_ptr<BluetoothEventBus> m_eventBus;
                pa_threaded_mainloop* m_paLoop;
                bool m_paLoopStarted;
                pa_context* m_context;
                ModuleState m_policyState;
                ModuleState m_discoverState;
                bool m_connected;
                Executor m_executor;
            };
            inline string moduleStateToString(const PulseAudioBluetoothInitializer::ModuleState& state) {
                switch(state) {
                    case PulseAudioBluetoothInitializer::ModuleState::UNKNOWN: return "UNKNOWN";
                    case PulseAudioBluetoothInitializer::ModuleState::INITIALLY_LOADED: return "INITIALLY_LOADED";
                    case PulseAudioBluetoothInitializer::ModuleState::UNLOADED: return "UNLOADED";
                    case PulseAudioBluetoothInitializer::ModuleState::LOADED_BY_SDK: return "LOADED_BY_SDK";
                }
                return "INVALID";
            }
            inline ostream& operator<<(ostream& stream, const PulseAudioBluetoothInitializer::ModuleState& state) {
                return stream << moduleStateToString(state);
            }
        }
    }
}
#endif