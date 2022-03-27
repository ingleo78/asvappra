#ifndef ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGEVENTSENDER_H_
#define ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGEVENTSENDER_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <json/JSONGenerator.h>
#include <avs/EventBuilder.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include <util/RetryTimer.h>
#include <util/WaitEvent.h>
#include <settings/SettingEventMetadata.h>
#include <settings/SettingEventSenderInterface.h>

namespace alexaClientSDK {
namespace settings {

/**
 * An implementation of the @c SettingEventSenderInterface.
 */
class SettingEventSender : public SettingEventSenderInterface {
public:
    /**
     * Creates an instance of the @SettingEventSender.
     *
     * @param metadata Contains the information needed to construct AVS events.
     * @param messageSender The delivery service for the AVS events.
     * @return The shared pointer to the created @c SettingEventSender instance.
     */
    static std::unique_ptr<SettingEventSender> create(
        const SettingEventMetadata& metadata,
        std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        const std::vector<int>& retryTable = getDefaultRetryTable());

    /**
     * Destructor.
     * Cancels any pending requests.
     */
    ~SettingEventSender();

    /// @name SettingEventSenderInterface Functions
    /// @{
    std::shared_future<bool> sendChangedEvent(const std::string& value) override;
    std::shared_future<bool> sendReportEvent(const std::string& value) override;
    std::shared_future<bool> sendStateReportEvent(const std::string& payload) override;
    void cancel() override;
    /// @}

private:
    /*
     * Constructor.
     *
     * @param messageSender The delivery service for the AVS events.
     * @param retryTable A list of back-off times in milliseconds used in resending events.
     */
    SettingEventSender(
        const SettingEventMetadata& metadata,
        std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        const std::vector<int>& retryTable);

    /*
     * Helper function to send the changed and report events.
     *
     * @param eventJson The event json to be sent.
     * @return A future expressing if the event has been sent to AVS.
     */
    std::shared_future<bool> sendEvent(const std::string& eventJson);

    /**
     * Creates the event content.
     *
     * @param eventName The name of the event.
     * @param value The value of the setting. It should be a valid JSON string value.
     * @return The event json string.
     */
    std::string buildEventJson(const std::string& eventName, const std::string& value) const;

    /**
     * Retrieves the default back-off times for resending events.
     *
     * @return The default back-off times for resending events.
     */
    static const std::vector<int>& getDefaultRetryTable();

    /// Contains information needed to construct AVS events.
    const SettingEventMetadata m_metadata;

    /// The delivery service for the AVS events.
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// A mutex to ensure only one event is sent at a time.
    std::mutex m_sendMutex;

    /// Object used to wait for event transmission cancellation.
    avsCommon::utils::WaitEvent m_waitCancelEvent;

    /// Retry Timer object.
    avsCommon::utils::RetryTimer m_retryTimer;

    /// The number of retries that will be done on an event in case of send failure.
    const std::size_t m_maxRetries;
};

}  // namespace settings
}  // namespace alexaClientSDK
#endif  // ALEXA_CLIENT_SDK_SETTINGS_INCLUDE_SETTINGS_SETTINGEVENTSENDER_H_
