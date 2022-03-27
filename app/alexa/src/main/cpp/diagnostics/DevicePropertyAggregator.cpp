#include <logger/Logger.h>
#include "DevicePropertyAggregator.h"

namespace alexaClientSDK {
    namespace diagnostics {
        static const string TAG{"DevicePropertyAggregator"};
        static const string INVALID_VALUE{"INVALID"};
        static const string IDLE{"IDLE"};
        static const string NONE{"NONE"};
        static const seconds TIMEOUT{2};
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<DevicePropertyAggregator> DevicePropertyAggregator::create() {
            return shared_ptr<DevicePropertyAggregator>(new DevicePropertyAggregator());
        }
        DevicePropertyAggregator::DevicePropertyAggregator() {
            initializePropertyMap();
        }
        void DevicePropertyAggregator::setContextManager(shared_ptr<ContextManagerInterface> contextManager) {
            ACSDK_DEBUG5(LX(__func__));
            m_contextManager = contextManager;
        }
        void DevicePropertyAggregator::initializeVolume(shared_ptr<SpeakerManagerInterface> speakerManager) {
            ACSDK_DEBUG5(LX(__func__));
            if (!speakerManager) {
                ACSDK_ERROR(LX("initializeVolumeFailed").d("reason", "nullptr"));
                return;
            }
            SpeakerSettings settings;
            auto result = speakerManager->getSpeakerSettings(Type::AVS_SPEAKER_VOLUME, &settings);
            if (future_status::ready == result.wait_for(TIMEOUT) && result.get()) {
                updateSpeakerSettingsInPropertyMap(Type::AVS_SPEAKER_VOLUME, settings);
            }
            result = speakerManager->getSpeakerSettings(Type::AVS_ALERTS_VOLUME, &settings);
            if (future_status::ready == result.wait_for(TIMEOUT) && result.get()) {
                updateSpeakerSettingsInPropertyMap(Type::AVS_ALERTS_VOLUME, settings);
            }
        }
        void DevicePropertyAggregator::initializePropertyMap() {
            m_propertyMap[DevicePropertyAggregatorInterface::TTS_PLAYER_STATE] = IDLE;
            m_propertyMap[DevicePropertyAggregatorInterface::AUDIO_PLAYER_STATE] = IDLE;
            m_propertyMap[DevicePropertyAggregatorInterface::CONTENT_ID] = NONE;
            m_propertyMap[DevicePropertyAggregatorInterface::ALERT_TYPE_AND_STATE] = IDLE;
            auto contextOptional = getDeviceContextJson();
            if (contextOptional.hasValue()) {
                m_propertyMap[DevicePropertyAggregatorInterface::DEVICE_CONTEXT] = contextOptional.value();
            }
        }
        unordered_map<string, string> DevicePropertyAggregator::getAllDeviceProperties() {
            auto future = m_executor.submit([this]() { return m_propertyMap; });
            return future.get();
        }
        Optional<string> DevicePropertyAggregator::getDeviceProperty(const string& propertyKey) {
            auto maybePropertyValueFuture = m_executor.submit([this, propertyKey]() {
                Optional<string> maybePropertyValue;
                if (DevicePropertyAggregatorInterface::DEVICE_CONTEXT == propertyKey) {
                    maybePropertyValue = getDeviceContextJson();
                } else {
                    auto it = m_propertyMap.find(propertyKey);
                    if (it != m_propertyMap.end()) maybePropertyValue = it->second;
                }
                return maybePropertyValue;
            });
            auto maybePropertyValue = maybePropertyValueFuture.get();
            if (maybePropertyValue.hasValue()) {
                ACSDK_DEBUG5(LX(__func__).d("propertyKey", propertyKey).d("propertyValue", maybePropertyValue.value()));
            } else { ACSDK_DEBUG5(LX(__func__).d("propertyKey", propertyKey).m("unknown property")); }
            return maybePropertyValue;
        }
        void DevicePropertyAggregator::onContextFailure(const ContextRequestError error) {
            ACSDK_ERROR(LX(__func__).d("reason", error));
            m_contextWakeTrigger.notify_all();
        }
        void DevicePropertyAggregator::onContextAvailable(const string& jsonContext) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_deviceContextMutex);
            m_deviceContext.set(jsonContext);
            m_contextWakeTrigger.notify_all();
        }
        Optional<string> DevicePropertyAggregator::getDeviceContextJson() {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_contextManager) {
                Optional<string> empty;
                return empty;
            }
            unique_lock<mutex> lock(m_deviceContextMutex);
            m_deviceContext.reset();
            m_contextManager->getContext(shared_from_this());
            m_contextWakeTrigger.wait_for(lock, TIMEOUT, [this]() { return m_deviceContext.hasValue(); });
            return m_deviceContext;
        }
        void DevicePropertyAggregator::onAlertStateChange(const string& alertToken, const string& alertType, State state,
            const string& reason) {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.submit([this, alertType, state]() {
                ostringstream _state;
                _state << (int)state;
                string ss = alertType + ":" + _state.str();
                m_propertyMap[DevicePropertyAggregatorInterface::ALERT_TYPE_AND_STATE] = ss;
            });
        }
        void DevicePropertyAggregator::onPlayerActivityChanged(PlayerActivity state, const Context& context) {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.submit([this, state, context]() {
                string playerActivityState = playerActivityToString(state);
                m_propertyMap[DevicePropertyAggregatorInterface::AUDIO_PLAYER_STATE] = playerActivityState;
                m_propertyMap[DevicePropertyAggregatorInterface::CONTENT_ID] = context.audioItemId;
            });
        }
        void DevicePropertyAggregator::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.submit([this, status]() {
                stringstream ss;
                ss << status;
                m_propertyMap[DevicePropertyAggregatorInterface::CONNECTION_STATE] = ss.str();
            });
        }
        void DevicePropertyAggregator::onSetIndicator(IndicatorState state) {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.submit([this, state]() {
                stringstream ss;
                ss << state;
                m_propertyMap[DevicePropertyAggregatorInterface::NOTIFICATION_INDICATOR] = ss.str();
            });
        }
        void DevicePropertyAggregator::onNotificationReceived() {}
        void DevicePropertyAggregator::onDialogUXStateChanged(DialogUXState newState) {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.submit([this, newState]() {
                m_propertyMap[DevicePropertyAggregatorInterface::TTS_PLAYER_STATE] =
                    DialogUXStateObserverInterface::stateToString(newState);
            });
        }
        void DevicePropertyAggregator::onSpeakerSettingsChanged(const Source& source, const Type& type, const SpeakerSettings& settings) {
            ACSDK_DEBUG5(LX(__func__));
            m_executor.submit([this, type, settings]() { updateSpeakerSettingsInPropertyMap(type, settings); });
        }
        void DevicePropertyAggregator::updateSpeakerSettingsInPropertyMap(const Type& type, const SpeakerSettings& settings) {
            ACSDK_DEBUG5(LX(__func__));
            string volumeAsString = to_string(settings.volume);
            stringstream ss;
            ss << boolalpha << settings.mute;
            string isMuteAsString = ss.str();
            switch(type) {
                case Type::AVS_SPEAKER_VOLUME:
                    m_propertyMap[DevicePropertyAggregatorInterface::AVS_SPEAKER_VOLUME] = volumeAsString;
                    m_propertyMap[DevicePropertyAggregatorInterface::AVS_SPEAKER_MUTE] = isMuteAsString;
                    break;
                case Type::AVS_ALERTS_VOLUME:
                    m_propertyMap[DevicePropertyAggregatorInterface::AVS_ALERTS_VOLUME] = volumeAsString;
                    m_propertyMap[DevicePropertyAggregatorInterface::AVS_ALERTS_MUTE] = isMuteAsString;
                    break;
            }
        }
    }
}