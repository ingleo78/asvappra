#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKCOMMAND_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_PLAYBACKCONTROLLER_INCLUDE_PLAYBACKCONTROLLER_PLAYBACKCOMMAND_H_

#include <string>
#include <ostream>
#include <avs/PlaybackButtons.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            class PlaybackCommand {
            public:
                PlaybackCommand(const string& name);
                virtual ~PlaybackCommand() = default;
                virtual string getEventName() const;
                virtual string getEventPayload() const;
                friend ostream& operator<<(ostream&, const PlaybackCommand& command);
                static const PlaybackCommand& buttonToCommand(PlaybackButton button);
                static const PlaybackCommand& toggleToCommand(PlaybackToggle toggle, bool action);
            protected:
                const string m_name;
            private:
                virtual string toString() const;
            };
            class ButtonCommand_v1_0 : public PlaybackCommand {
            public:
                ButtonCommand_v1_0(const string& name);
                string getEventName() const override;
                string getEventPayload() const override;
            };
            class ButtonCommand_v1_1 : public PlaybackCommand {
            public:
                ButtonCommand_v1_1(const string& name);
                string getEventName() const override;
                string getEventPayload() const override;
            };
            class ToggleCommand : public PlaybackCommand {
            public:
                ToggleCommand(const string& name, bool action);
                string getEventName() const override;
                string getEventPayload() const override;
            private:
                string getActionString() const;
                string toString() const override;
                bool m_action;
            };
        }
    }
}
#endif