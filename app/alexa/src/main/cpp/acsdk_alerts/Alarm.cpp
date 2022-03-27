#include "Alarm.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace settings;
        using namespace utils;
        Alarm::Alarm(function<pair<unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                     shared_ptr<DeviceSettingsManager> settingsManager) : Alert(defaultAudioFactory, shortAudioFactory, settingsManager) {}
        std::string Alarm::getTypeName() const {
            return Alarm::getTypeNameStatic();
        }
    }
}