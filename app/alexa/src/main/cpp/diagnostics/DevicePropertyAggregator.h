#ifndef ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_DEVICEPROPERTYAGGREGATOR_H_
#define ALEXA_CLIENT_SDK_DIAGNOSTICS_INCLUDE_DIAGNOSTICS_DEVICEPROPERTYAGGREGATOR_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <util/Optional.h>
#include <threading/Executor.h>
#include <sdkinterfaces/Diagnostics/DevicePropertyAggregatorInterface.h>

namespace alexaClientSDK {
    namespace diagnostics {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace threading;
        using namespace sdkInterfaces::diagnostics;
        using Type = ChannelVolumeInterface::Type;
        using SpeakerSettings = SpeakerInterface::SpeakerSettings;
        using Source = SpeakerManagerObserverInterface::Source;
        using DialogUXState = DialogUXStateObserverInterface::DialogUXState;
        using Context = AudioPlayerObserverInterface::Context;
        using Status = ConnectionStatusObserverInterface::Status;
        using ChangedReason = ConnectionStatusObserverInterface::ChangedReason;
        using State = AlertObserverInterface::State;
        class DevicePropertyAggregator : public DevicePropertyAggregatorInterface, public enable_shared_from_this<DevicePropertyAggregator> {
        public:
            static shared_ptr<DevicePropertyAggregator> create();
            Optional<string> getDeviceProperty(const string& propertyKey) override;
            unordered_map<string, string> getAllDeviceProperties() override;
            void setContextManager(shared_ptr<ContextManagerInterface> contextManager) override;
            void initializeVolume(shared_ptr<SpeakerManagerInterface> speakerManager) override;
            void onAlertStateChange(const string& alertToken, const string& alertType, State state, const string& reason = "");
            void onPlayerActivityChanged(PlayerActivity state, const Context& context) override;
            void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;
            void onContextAvailable(const string& jsonContext) override;
            void onContextFailure(const ContextRequestError error) override;
            void onSetIndicator(IndicatorState state) override;
            void onNotificationReceived() override;
            void onSpeakerSettingsChanged(const Source& source, const Type& type, const SpeakerSettings& settings) override;
            void onDialogUXStateChanged(DialogUXState newState) override;
        private:
            DevicePropertyAggregator();
            Optional<string> getDeviceContextJson();
            void initializePropertyMap();
            void updateSpeakerSettingsInPropertyMap(const Type& type, const SpeakerSettings& settings);
            Executor m_executor;
            unordered_map<string, string> m_propertyMap;
            mutex m_deviceContextMutex;
            Optional<string> m_deviceContext;
            condition_variable m_contextWakeTrigger;
            shared_ptr<ContextManagerInterface> m_contextManager;
        };
    }
}
#endif