#include "Reminder.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace settings;
        using namespace avsCommon::utils;
        Reminder::Reminder(function<pair<unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                           shared_ptr<settings::DeviceSettingsManager> settingsManager) : Alert(defaultAudioFactory, shortAudioFactory, settingsManager) {}
        string Reminder::getTypeName() const {
            return Reminder::getTypeNameStatic();
        }
    }
}