#include <memory>
#include <logger/Logger.h>
#include "ChannelVolumeManager.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            using namespace speakerConstants;
            using namespace logger;
            static const string TAG("ChannelVolumeManager");
            #define LX(event) LogEntry(TAG, event)
            static const float UPPER_VOLUME_CURVE_FRACTION = 0.40;
            static const float LOWER_VOLUME_CURVE_FRACTION = 0.20;
            template <class T> static bool withinBounds(T value, T min, T max) {
                if (value < min || value > max) {
                    ACSDK_ERROR(LX("checkBoundsFailed").d("value", value).d("min", min).d("max", max));
                    return false;
                }
                return true;
            }
            shared_ptr<ChannelVolumeManager> ChannelVolumeManager::create(shared_ptr<SpeakerInterface> speaker, Type type, VolumeCurveFunction volumeCurve) {
                if (!speaker) {
                    ACSDK_ERROR(LX(__func__).d("reason", "Null SpeakerInterface").m("createFailed"));
                    return nullptr;
                }
                auto channelVolumeManager = shared_ptr<ChannelVolumeManager>(new ChannelVolumeManager(speaker, type, volumeCurve));
                SpeakerInterface::SpeakerSettings settings;
                if (!channelVolumeManager->getSpeakerSettings(&settings)) {
                    ACSDK_ERROR(LX(__func__).m("createFailed").d("reason", "Unable To Retrieve Speaker Settings"));
                    return nullptr;
                }
                channelVolumeManager->m_unduckedVolume = settings.volume;
                return channelVolumeManager;
            }
            ChannelVolumeManager::ChannelVolumeManager(shared_ptr<SpeakerInterface> speaker, Type type, VolumeCurveFunction volumeCurve) :
                                                       ChannelVolumeInterface{}, m_speaker{speaker}, m_isDucked{false}, m_unduckedVolume{AVS_SET_VOLUME_MIN},
                                                       m_volumeCurveFunction{volumeCurve ? volumeCurve : defaultVolumeAttenuateFunction},
                                                       m_type{type} {}
            Type ChannelVolumeManager::getSpeakerType() const {
                lock_guard<mutex> locker{m_mutex};
                return m_type;
            }
            bool ChannelVolumeManager::startDucking() {
                lock_guard<mutex> locker{m_mutex};
                ACSDK_DEBUG5(LX(__func__));
                if (m_isDucked) {
                    ACSDK_WARN(LX(__func__).m("Channel is Already Attenuated"));
                    return true;
                }
                auto desiredVolume = m_volumeCurveFunction(m_unduckedVolume);
                ACSDK_DEBUG9(LX(__func__).d("currentVolume", static_cast<int>(m_unduckedVolume)).d("desiredAttenuatedVolume", static_cast<int>(desiredVolume)));
                if (!m_speaker->setVolume(desiredVolume)) {
                    ACSDK_WARN(LX(__func__).m("Failed to Attenuate Channel Volume"));
                    return false;
                }
                m_isDucked = true;
                return true;
            }
            bool ChannelVolumeManager::stopDucking() {
                lock_guard<mutex> locker{m_mutex};
                ACSDK_DEBUG5(LX(__func__));
                if (!m_isDucked) return true;
                if (!m_speaker->setVolume(m_unduckedVolume)) return false;
                ACSDK_DEBUG5(LX(__func__).d("Restored Channel Volume", static_cast<int>(m_unduckedVolume)));
                m_isDucked = false;
                return true;
            }
            bool ChannelVolumeManager::setUnduckedVolume(int8_t volume) {
                ACSDK_DEBUG5(LX(__func__).d("volume", static_cast<int>(volume)));
                if (!withinBounds(volume, AVS_SET_VOLUME_MIN, AVS_SET_VOLUME_MAX)) {
                    ACSDK_ERROR(LX(__func__).m("Invalid Volume"));
                    return false;
                }
                lock_guard<mutex> locker{m_mutex};
                m_unduckedVolume = volume;
                if (m_isDucked) {
                    ACSDK_WARN(LX(__func__).m("Channel is Attenuated, Deferring Operation"));
                    return true;
                }
                ACSDK_DEBUG5(LX(__func__).d("Unducked Channel Volume", static_cast<int>(volume)));
                return m_speaker->setVolume(m_unduckedVolume);
            }
            bool ChannelVolumeManager::setMute(bool mute) {
                lock_guard<mutex> locker{m_mutex};
                ACSDK_DEBUG5(LX(__func__).d("mute", static_cast<int>(mute)));
                return m_speaker->setMute(mute);
            }
            bool ChannelVolumeManager::getSpeakerSettings(SpeakerInterface::SpeakerSettings* settings) const {
                ACSDK_DEBUG(LX(__func__));
                if (!settings) return false;
                lock_guard<mutex> locker{m_mutex};
                if (!m_speaker->getSpeakerSettings(settings)) {
                    ACSDK_ERROR(LX(__func__).m("Unable To Retrieve SpeakerSettings"));
                    return false;
                }
                if (m_isDucked) {
                    ACSDK_DEBUG5(LX(__func__).m("Channel is Already Attenuated"));
                    settings->volume = m_unduckedVolume;
                }
                return true;
            }
            int8_t ChannelVolumeManager::defaultVolumeAttenuateFunction(int8_t currentVolume) {
                const int8_t lowerBreakPoint = static_cast<int8_t>(AVS_SET_VOLUME_MAX * LOWER_VOLUME_CURVE_FRACTION);
                const int8_t upperBreakPoint = static_cast<int8_t>(AVS_SET_VOLUME_MAX * UPPER_VOLUME_CURVE_FRACTION);
                if (currentVolume >= upperBreakPoint) return lowerBreakPoint;
                else if (currentVolume >= lowerBreakPoint && currentVolume <= upperBreakPoint) return (currentVolume - lowerBreakPoint);
                else return AVS_SET_VOLUME_MIN;
            }
        }
    }
}