#include <chrono>
#include <string>
#include <logger/Logger.h>
#include "PulseAudioBluetoothInitializer.h"

static const std::string TAG{"PulseAudioBluetoothInitializer"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace chrono;
            static string BLUETOOTH_DISCOVER = "module-bluetooth-discover";
            static string BLUETOOTH_POLICY = "module-bluetooth-policy";
            static const int PA_CONTEXT_CB_SUCCESS{1};
            static const int PA_MODULE_CB_EOL_EOL{1};
            static const int PA_MODULE_CB_EOL_ERR{-1};
            static const seconds TIMEOUT{2};
            string stateToString(pa_context_state_t state) {
                switch(state) {
                    case PA_CONTEXT_UNCONNECTED: return "PA_CONTEXT_UNCONNECTED";
                    case PA_CONTEXT_CONNECTING: return "PA_CONTEXT_CONNECTING";
                    case PA_CONTEXT_AUTHORIZING: return "PA_CONTEXT_AUTHORIZING";
                    case PA_CONTEXT_SETTING_NAME: return "PA_CONTEXT_SETTING_NAME";
                    case PA_CONTEXT_READY: return "PA_CONTEXT_READY";
                    case PA_CONTEXT_FAILED: return "PA_CONTEXT_FAILED";
                    case PA_CONTEXT_TERMINATED: return "PA_CONTEXT_TERMINATED";
                }
                return "UNKNOWN";
            }
            shared_ptr<PulseAudioBluetoothInitializer> PulseAudioBluetoothInitializer::create(shared_ptr<BluetoothEventBus> eventBus) {
                ACSDK_DEBUG5(LX(__func__));
                if (!eventBus) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullEventBus"));
                    return nullptr;
                }
                auto pulseAudio = shared_ptr<PulseAudioBluetoothInitializer>(new PulseAudioBluetoothInitializer(eventBus));
                pulseAudio->init();
                return pulseAudio;
            }
            PulseAudioBluetoothInitializer::PulseAudioBluetoothInitializer(shared_ptr<BluetoothEventBus> eventBus) : m_eventBus{eventBus}, m_paLoop{nullptr},
                                                                                                                     m_paLoopStarted{false}, m_context{nullptr},
                                                                                                                     m_policyState{ModuleState::UNKNOWN},
                                                                                                                     m_discoverState{ModuleState::UNKNOWN},
                                                                                                                     m_connected{false} {}
            void PulseAudioBluetoothInitializer::init() {
                ACSDK_DEBUG5(LX(__func__));
                m_eventBus->addListener({BluetoothEventType::BLUETOOTH_DEVICE_MANAGER_INITIALIZED}, shared_from_this());
            }
            void PulseAudioBluetoothInitializer::onLoadDiscoverResult(pa_context* context, uint32_t index, void* userdata) {
                ACSDK_DEBUG5(LX(__func__));
                if (!userdata) {
                    ACSDK_ERROR(LX("onLoadDiscoverResultFailed").d("reason", "nullUserData"));
                    return;
                }
                PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userdata);
                initializer->handleLoadModuleResult(context, index, BLUETOOTH_DISCOVER);
            }
            void PulseAudioBluetoothInitializer::onLoadPolicyResult(pa_context* context, uint32_t index, void* userdata) {
                ACSDK_DEBUG5(LX(__func__));
                if (!userdata) {
                    ACSDK_ERROR(LX("onLoadPolicyResultFailed").d("reason", "nullUserData"));
                    return;
                }
                PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userdata);
                initializer->handleLoadModuleResult(context, index, BLUETOOTH_POLICY);
            }
            void PulseAudioBluetoothInitializer::handleLoadModuleResult(
                pa_context* context,
                uint32_t index,
                const string& moduleName) {
                ACSDK_DEBUG5(LX(__func__).d("module", moduleName).d("index", index));
                unique_lock<mutex> lock(m_mutex);
                if (!context) {
                    ACSDK_ERROR(LX("handleLoadModuleResultFailed").d("reason", "nullContext"));
                    return;
                } else if (PA_INVALID_INDEX == index) {
                    ACSDK_ERROR(LX("handleLoadModuleResultFailed").d("reason", "loadFailed"));
                    return;
                }

                if (updateStateLocked(ModuleState::LOADED_BY_SDK, moduleName)) {
                    m_mainThreadCv.notify_one();
                }
            }
            void PulseAudioBluetoothInitializer::onUnloadPolicyResult(pa_context* context, int success, void* userdata) {
                ACSDK_DEBUG5(LX(__func__));
                if (!userdata) {
                    ACSDK_ERROR(LX("onUnloadPolicyResultFailed").d("reason", "nullUserData"));
                    return;
                }
                PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userdata);
                initializer->handleUnloadModuleResult(context, success, BLUETOOTH_POLICY);
            }
            void PulseAudioBluetoothInitializer::onUnloadDiscoverResult(pa_context* context, int success, void* userdata) {
                ACSDK_DEBUG5(LX(__func__));
                if (!userdata) {
                    ACSDK_ERROR(LX("onUnloadDiscoverResultFailed").d("reason", "nullUserData"));
                    return;
                }
                PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userdata);
                initializer->handleUnloadModuleResult(context, success, BLUETOOTH_DISCOVER);
            }
            void PulseAudioBluetoothInitializer::handleUnloadModuleResult(pa_context* context, int success, const string& moduleName) {
                ACSDK_DEBUG5(LX(__func__).d("module", moduleName).d("success", success));
                if (!context) {
                    ACSDK_ERROR(LX("handleUnloadModuleResultFailed").d("reason", "nullContext"));
                    return;
                } else if (PA_CONTEXT_CB_SUCCESS != success) {
                    ACSDK_ERROR(LX("handleUnloadModuleResult").d("reason", "unloadFailed"));
                    return;
                }
                if (updateStateLocked(ModuleState::UNLOADED, moduleName)) m_mainThreadCv.notify_one();
            }
            void PulseAudioBluetoothInitializer::onModuleFound(pa_context* context, const pa_module_info* info, int eol, void* userdata) {
                ACSDK_DEBUG9(LX(__func__));
                if (!context) {
                    ACSDK_ERROR(LX("moduleFoundFailed").d("reason", "nullContext"));
                    return;
                } else if (!userdata) {
                    ACSDK_ERROR(LX("moduleFoundFailed").d("reason", "nullUserData"));
                    return;
                } else if (PA_MODULE_CB_EOL_ERR == eol) {
                    ACSDK_ERROR(LX("moduleFoundFailed").d("reason", "pulseAudioError").d("eol", PA_MODULE_CB_EOL_ERR));
                    return;
                }
                PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userdata);
                unique_lock<mutex> lock(initializer->m_mutex);
                if (PA_MODULE_CB_EOL_EOL == eol) {
                    ACSDK_DEBUG(LX(__func__).m("endOfList"));
                    if (ModuleState::INITIALLY_LOADED != initializer->m_policyState) {
                        initializer->updateStateLocked(ModuleState::UNLOADED, BLUETOOTH_POLICY);
                    }
                    if (ModuleState::INITIALLY_LOADED != initializer->m_discoverState) {
                        initializer->updateStateLocked(ModuleState::UNLOADED, BLUETOOTH_DISCOVER);
                    }
                    initializer->m_mainThreadCv.notify_one();
                    return;
                } else if (!info || !info->name) {
                    ACSDK_ERROR(LX("moduleFoundFailed").d("reason", "invalidInfo"));
                    return;
                }
                ACSDK_DEBUG9(LX(__func__).d("name", info->name));
                if (BLUETOOTH_POLICY == info->name) {
                    initializer->updateStateLocked(ModuleState::INITIALLY_LOADED, BLUETOOTH_POLICY);
                    pa_context_unload_module(context, info->index, &PulseAudioBluetoothInitializer::onUnloadPolicyResult, userdata);
                } else if (BLUETOOTH_DISCOVER == info->name) {
                    initializer->updateStateLocked(ModuleState::INITIALLY_LOADED, BLUETOOTH_DISCOVER);
                    pa_context_unload_module(context, info->index, &PulseAudioBluetoothInitializer::onUnloadDiscoverResult, userdata);
                }
            }
            bool PulseAudioBluetoothInitializer::updateStateLocked(const ModuleState& state, const std::string& module) {
                if (BLUETOOTH_POLICY == module) {
                    ACSDK_DEBUG5(LX(__func__).d("currentState", m_policyState));
                    m_policyState = state;
                } else if (BLUETOOTH_DISCOVER == module) {
                    ACSDK_DEBUG5(LX(__func__).d("currentState", m_discoverState));
                    m_discoverState = state;
                } else {
                    ACSDK_ERROR(LX("updateStateLockedFailed").d("reason", "invalidModule"));
                    return false;
                }
                ACSDK_DEBUG5(LX(__func__).d("module", module).d("desiredState", state));
                return true;
            }
            void PulseAudioBluetoothInitializer::onStateChanged(pa_context* context, void* userdata) {
                ACSDK_DEBUG5(LX(__func__));
                if (!context) {
                    ACSDK_ERROR(LX("onStateChangedFailed").d("reason", "nullContext"));
                    return;
                } else if (!userdata) {
                    ACSDK_ERROR(LX("onStateChangedFailed").d("reason", "nullUserData"));
                    return;
                }
                pa_context_state_t state;
                state = pa_context_get_state(context);
                ACSDK_DEBUG5(LX(__func__).d("state", stateToString(state)));
                PulseAudioBluetoothInitializer* initializer = static_cast<PulseAudioBluetoothInitializer*>(userdata);
                initializer->setStateAndNotify(state);
            }
            void PulseAudioBluetoothInitializer::setStateAndNotify(pa_context_state_t state) {
                ACSDK_DEBUG5(LX(__func__));
                unique_lock<mutex> lock(m_mutex);
                switch(state) {
                    case PA_CONTEXT_READY: m_connected = true;
                    case PA_CONTEXT_FAILED: case PA_CONTEXT_TERMINATED: m_mainThreadCv.notify_one(); break;
                    default: break;
                }
            }
            void PulseAudioBluetoothInitializer::cleanup() {
                ACSDK_DEBUG5(LX(__func__));
                if (m_context) {
                    pa_context_disconnect(m_context);
                    pa_context_unref(m_context);
                    m_context = nullptr;
                }
                if (m_paLoop) {
                    pa_threaded_mainloop_stop(m_paLoop);
                    pa_threaded_mainloop_free(m_paLoop);
                    m_paLoop = nullptr;
                }
                ACSDK_DEBUG(LX("cleanup").m("cleanupCompleted"));
            }
            PulseAudioBluetoothInitializer::~PulseAudioBluetoothInitializer() {
                ACSDK_DEBUG5(LX(__func__));
                m_executor.shutdown();
                cleanup();
            }
            void PulseAudioBluetoothInitializer::run() {
                ACSDK_DEBUG5(LX(__func__));
                m_paLoop = pa_threaded_mainloop_new();
                pa_mainloop_api* mainLoopApi = pa_threaded_mainloop_get_api(m_paLoop);
                m_context = pa_context_new(mainLoopApi, "Application to unload and reload Pulse Audio BT modules");
                pa_context_set_state_callback(m_context, &PulseAudioBluetoothInitializer::onStateChanged, this);
                pa_context_connect(m_context, NULL, PA_CONTEXT_NOFLAGS, NULL);
                if (pa_threaded_mainloop_start(m_paLoop) < 0) {
                    ACSDK_ERROR(LX("runFailed").d("reason", "runningMainLoopFailed"));
                    cleanup();
                    return;
                }
                {
                    unique_lock<mutex> lock(m_mutex);
                    if (cv_status::timeout == m_mainThreadCv.wait_for(lock, TIMEOUT) || !m_connected) {
                        cleanup();
                        return;
                    }
                }
                pa_context_get_module_info_list(m_context, &PulseAudioBluetoothInitializer::onModuleFound, this);
                {
                    unique_lock<std::mutex> lock(m_mutex);
                    if (m_mainThreadCv.wait_for(lock, TIMEOUT, [this] {
                            return ModuleState::UNLOADED == m_policyState && ModuleState::UNLOADED == m_discoverState;
                        })) {
                        ACSDK_DEBUG(LX(__func__).d("success", "bluetoothModulesUnloaded"));
                    } else {
                        ACSDK_ERROR(LX("runFailed").d("reason", "unloadModulesFailed"));
                        cleanup();
                        return;
                    }
                }
                pa_context_load_module(m_context, BLUETOOTH_POLICY.c_str(), nullptr, &PulseAudioBluetoothInitializer::onLoadPolicyResult, this);
                pa_context_load_module(m_context, BLUETOOTH_DISCOVER.c_str(), nullptr, &PulseAudioBluetoothInitializer::onLoadDiscoverResult, this);
                {
                    unique_lock<mutex> lock(m_mutex);
                    if (m_mainThreadCv.wait_for(lock, TIMEOUT, [this] {
                            return ModuleState::LOADED_BY_SDK == m_policyState && ModuleState::LOADED_BY_SDK == m_discoverState;
                        })) {
                        ACSDK_DEBUG(LX(__func__).d("reason", "loadModulesSuccesful"));
                    } else {
                        ACSDK_ERROR(LX("runFailed").d("reason", "loadModulesFailed"));
                        cleanup();
                        return;
                    }
                }
                ACSDK_DEBUG(LX(__func__).m("Reloading PulseAudio Bluetooth Modules Successful"));
                cleanup();
            }
            void PulseAudioBluetoothInitializer::onEventFired(const BluetoothEvent& event) {
                ACSDK_DEBUG5(LX(__func__));
                if (BluetoothEventType::BLUETOOTH_DEVICE_MANAGER_INITIALIZED != event.getType()) {
                    ACSDK_ERROR(LX("onEventFiredFailed").d("reason", "unexpectedEventReceived"));
                    return;
                }
                m_executor.submit([this] {
                    if (!m_paLoopStarted) {
                        m_paLoopStarted = true;
                        run();
                    } else { ACSDK_WARN(LX(__func__).d("reason", "loopAlreadyStarted")); }
                });
            }
        }
    }
}