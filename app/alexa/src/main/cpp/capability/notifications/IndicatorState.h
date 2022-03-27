#ifndef ACSDKNOTIFICATIONS_INDICATORSTATE_H_
#define ACSDKNOTIFICATIONS_INDICATORSTATE_H_

#include <ostream>

namespace alexaClientSDK {
    namespace acsdkNotifications {
        enum class IndicatorState { OFF = 0, ON = 1, UNDEFINED };
        inline int indicatorStateToInt(IndicatorState state) {
            return static_cast<int>(state);
        }
        inline const IndicatorState intToIndicatorState(int stateNum) {
            if (stateNum < 0 || stateNum >= static_cast<int>(IndicatorState::UNDEFINED)) return IndicatorState::UNDEFINED;
            return static_cast<IndicatorState>(stateNum);
        }
        inline std::ostream& operator<<(std::ostream& stream, IndicatorState state) {
            switch(state) {
                case IndicatorState::ON:
                    stream << "ON";
                    return stream;
                case IndicatorState::OFF:
                    stream << "OFF";
                    return stream;
                case IndicatorState::UNDEFINED:
                    stream << "UNDEFINED";
                    return stream;
            }
            return stream << "UNKNOWN STATE";
        }
    }
}
#endif