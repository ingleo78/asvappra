#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_CHANNELVOLUMEMANAGER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_SPEAKERMANAGER_INCLUDE_SPEAKERMANAGER_CHANNELVOLUMEMANAGER_H_

#include <sdkinterfaces/ChannelVolumeInterface.h>
#include <mutex>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using Type = ChannelVolumeInterface::Type;
            class ChannelVolumeManager : public ChannelVolumeInterface {
            public:
                static shared_ptr<ChannelVolumeManager> create(shared_ptr<SpeakerInterface> speaker, ChannelVolumeInterface::Type type = Type::AVS_SPEAKER_VOLUME,
                                                               function<int8_t(int8_t)> volumeCurve = nullptr);
                Type getSpeakerType() const override;
                bool startDucking() override;
                bool stopDucking() override;
                bool setUnduckedVolume(int8_t volume) override;
                bool setMute(bool mute) override;
                bool getSpeakerSettings(SpeakerInterface::SpeakerSettings* settings) const override;
            private:
                using VolumeCurveFunction = function<int8_t(int8_t unduckedVolume)>;
                ChannelVolumeManager(shared_ptr<SpeakerInterface> speaker, Type type, VolumeCurveFunction volumeCurve);
                static int8_t defaultVolumeAttenuateFunction(int8_t unduckedVolume);
                mutable mutex m_mutex;
                shared_ptr<SpeakerInterface> m_speaker;
                bool m_isDucked;
                int8_t m_unduckedVolume;
                VolumeCurveFunction m_volumeCurveFunction;
                const Type m_type;
            };
        }
    }
}
#endif