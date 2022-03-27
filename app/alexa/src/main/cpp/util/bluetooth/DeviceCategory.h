#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_DEVICECATEGORY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_DEVICECATEGORY_H_

#include <ostream>
#include <string>

enum class DeviceCategory {
    REMOTE_CONTROL,
    GADGET,
    AUDIO_VIDEO,
    PHONE,
    OTHER,
    UNKNOWN
};
inline std::string_view deviceCategoryToString(const DeviceCategory& category) {
    switch (category) {
        case DeviceCategory::REMOTE_CONTROL: return "REMOTE_CONTROL";
        case DeviceCategory::GADGET: return "GADGET";
        case DeviceCategory::AUDIO_VIDEO: return "AUDIO_VIDEO";
        case DeviceCategory::PHONE: return "PHONE";
        case DeviceCategory::OTHER: return "OTHER";
        case DeviceCategory::UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN";
}
inline DeviceCategory stringToDeviceCategory(const std::string& category) {
    if (category == "REMOTE_CONTROL") return DeviceCategory::REMOTE_CONTROL;
    if (category == "GADGET") return DeviceCategory::GADGET;
    if (category == "AUDIO_VIDEO") return DeviceCategory::AUDIO_VIDEO;
    if (category == "PHONE") return DeviceCategory::PHONE;
    if (category == "OTHER") return DeviceCategory::OTHER;
    if (category == "UNKNOWN") return DeviceCategory::UNKNOWN;
    return DeviceCategory::UNKNOWN;
}
inline std::ostream& operator<<(std::ostream& stream, const DeviceCategory category) {
    return stream << deviceCategoryToString(category);
}
#endif