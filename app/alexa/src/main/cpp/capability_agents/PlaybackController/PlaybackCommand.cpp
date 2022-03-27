#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <logger/Logger.h>
#include <json/en.h>
#include "PlaybackCommand.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            using namespace rapidjson;
            using namespace utils;
            using namespace logger;
            static const string TAG("PlaybackCommand");
            #define LX(event) LogEntry(TAG, event)
            static const string BUTTON_COMMAND_EVENT_NAME = "ButtonCommandIssued";
            static const string TOGGLE_COMMAND_EVENT_NAME = "ToggleCommandIssued";
            static const string PLAYBACK_TOGGLE_ACTION_SELECT = "SELECT";
            static const string PLAYBACK_TOGGLE_ACTION_DESELECT = "DESELECT";
            static const string PLAYBACK_TOGGLE_NAME_SHUFFLE = "SHUFFLE";
            static const string PLAYBACK_TOGGLE_NAME_LOOP = "LOOP";
            static const string PLAYBACK_TOGGLE_NAME_REPEAT = "REPEAT";
            static const string PLAYBACK_TOGGLE_NAME_THUMBSUP = "THUMBSUP";
            static const string PLAYBACK_TOGGLE_NAME_THUMBSDOWN = "THUMBSDOWN";
            static const string PLAYBACK_NAME_UNKNOWN = "UNKNOWN";
            static const string PLAYBACK_CONTROLLER_EVENT_NAME_KEY = "name";
            static const string PLAYBACK_CONTROLLER_EVENT_ACTION_KEY = "action";
            static const string JSON_EMPTY_PAYLOAD = "{}";
            static const string JSON_BEGIN = "{\"";
            static const string JSON_COLON = "\": \"";
            static const string JSON_COMMA = "\", \"";
            static const string JSON_END = "\"}";
            static const ButtonCommand_v1_0 g_playButton_v1_0 = ButtonCommand_v1_0("PlayCommandIssued");
            static const ButtonCommand_v1_0 g_pauseButton_v1_0 = ButtonCommand_v1_0("PauseCommandIssued");
            static const ButtonCommand_v1_0 g_nextButton_v1_0 = ButtonCommand_v1_0("NextCommandIssued");
            static const ButtonCommand_v1_0 g_previousButton_v1_0 = ButtonCommand_v1_0("PreviousCommandIssued");
            static const ButtonCommand_v1_1 g_skipForwardButton_v1_0 = ButtonCommand_v1_1("SKIPFORWARD");
            static const ButtonCommand_v1_1 g_skipBackwardButton_v1_0 = ButtonCommand_v1_1("SKIPBACKWARD");
            static const ButtonCommand_v1_1 g_unknownButton_v1_0 = ButtonCommand_v1_1(PLAYBACK_NAME_UNKNOWN);
            static const ToggleCommand g_shuffleSelectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_SHUFFLE, true);
            static const ToggleCommand g_shuffleDeselectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_SHUFFLE, false);
            static const ToggleCommand g_loopSelectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_LOOP, true);
            static const ToggleCommand g_loopDeselectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_LOOP, false);
            static const ToggleCommand g_repeatSelectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_REPEAT, true);
            static const ToggleCommand g_repeatDeselectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_REPEAT, false);
            static const ToggleCommand g_thumbsUpSelectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_THUMBSUP, true);
            static const ToggleCommand g_thumbsUpDeselectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_THUMBSUP, false);
            static const ToggleCommand g_thumbsDownSelectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_THUMBSDOWN, true);
            static const ToggleCommand g_thumbsDownDeselectToggle = ToggleCommand(PLAYBACK_TOGGLE_NAME_THUMBSDOWN, false);
            static const ToggleCommand g_unknownToggle = ToggleCommand(PLAYBACK_NAME_UNKNOWN, false);
            PlaybackCommand::PlaybackCommand(const string& name) : m_name(name) {}
            const PlaybackCommand& PlaybackCommand::buttonToCommand(PlaybackButton button) {
                switch(button) {
                    case PlaybackButton::PLAY: return g_playButton_v1_0;
                    case PlaybackButton::PAUSE: return g_pauseButton_v1_0;
                    case PlaybackButton::NEXT: return g_nextButton_v1_0;
                    case PlaybackButton::PREVIOUS: return g_previousButton_v1_0;
                    case PlaybackButton::SKIP_FORWARD: return g_skipForwardButton_v1_0;
                    case PlaybackButton::SKIP_BACKWARD: return g_skipBackwardButton_v1_0;
                }
                return g_unknownButton_v1_0;
            }
            const PlaybackCommand& PlaybackCommand::toggleToCommand(PlaybackToggle toggle, bool state) {
                switch(toggle) {
                    case PlaybackToggle::LOOP: return (state ? g_loopSelectToggle : g_loopDeselectToggle);
                    case PlaybackToggle::REPEAT: return (state ? g_repeatSelectToggle : g_repeatDeselectToggle);
                    case PlaybackToggle ::SHUFFLE: return (state ? g_shuffleSelectToggle : g_shuffleDeselectToggle);
                    case PlaybackToggle::THUMBS_DOWN: return (state ? g_thumbsDownSelectToggle : g_thumbsDownDeselectToggle);
                    case PlaybackToggle::THUMBS_UP: return (state ? g_thumbsUpSelectToggle : g_thumbsUpDeselectToggle);
                }
                return g_unknownToggle;
            }
            string PlaybackCommand::toString() const {
                return m_name;
            }
            string ToggleCommand::toString() const {
                return m_name + "_" + getActionString();
            }
            ostream& operator<<(ostream& stream, const PlaybackCommand& command) {
                return stream << command.toString();
            }
            ButtonCommand_v1_0::ButtonCommand_v1_0(const string& name) : PlaybackCommand(name) {}
            string ButtonCommand_v1_0::getEventName() const {
                return m_name;
            }
            string ButtonCommand_v1_0::getEventPayload() const {
                return JSON_EMPTY_PAYLOAD;
            }
            ButtonCommand_v1_1::ButtonCommand_v1_1(const string& name) : PlaybackCommand(name) {}
            string ButtonCommand_v1_1::getEventName() const {
                return BUTTON_COMMAND_EVENT_NAME;
            }
            string ButtonCommand_v1_1::getEventPayload() const {
                return JSON_BEGIN + PLAYBACK_CONTROLLER_EVENT_NAME_KEY + JSON_COLON + m_name + JSON_END;
            }
            ToggleCommand::ToggleCommand(const string& name, bool action) : PlaybackCommand(name), m_action(action) {}
            string ToggleCommand::getEventName() const {
                return TOGGLE_COMMAND_EVENT_NAME;
            }
            string ToggleCommand::getEventPayload() const {
                return JSON_BEGIN + PLAYBACK_CONTROLLER_EVENT_NAME_KEY + JSON_COLON + m_name + JSON_COMMA +
                       PLAYBACK_CONTROLLER_EVENT_ACTION_KEY + JSON_COLON + getActionString() + JSON_END;
            }
            string ToggleCommand::getActionString() const {
                return (m_action ? PLAYBACK_TOGGLE_ACTION_SELECT : PLAYBACK_TOGGLE_ACTION_DESELECT);
            }
        }
    }
}