#ifndef ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_REGISTRATIONOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_REGISTRATIONOBSERVERINTERFACE_H_

namespace alexaClientSDK {
namespace registrationManager {

/**
 * This interface is used to observe changes to the device registration, such as user logout.
 */
class RegistrationObserverInterface {
public:
    /**
     * Virtual destructor to assure proper cleanup of derived types.
     */
    virtual ~RegistrationObserverInterface() = default;

    /**
     * Notification that the current customer has logged out.
     *
     * @warning This method is called while RegistrationManager is in a locked state. The callback must not block on
     * calls to RegistrationManager methods either.
     */
    virtual void onLogout() = 0;
};

}  // namespace registrationManager
}  // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_REGISTRATIONOBSERVERINTERFACE_H_
