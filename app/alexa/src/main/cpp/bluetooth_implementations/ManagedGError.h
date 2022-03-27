#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MANAGEDGERROR_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MANAGEDGERROR_H_

#include <string>
#include <gio/gio.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class ManagedGError {
            public:
                explicit ManagedGError(GError* error);
                ManagedGError();
                bool hasError();
                GError** toOutputParameter();
                ~ManagedGError();
                const char* getMessage();
            private:
                GError* m_error;
            };
            inline ManagedGError::ManagedGError(GError* error) : m_error{error} {}
            inline ManagedGError::ManagedGError() {
                m_error = nullptr;
            }
            inline bool ManagedGError::hasError() {
                return m_error != nullptr;
            }
            inline GError** ManagedGError::toOutputParameter() {
                return &m_error;
            }
            inline ManagedGError::~ManagedGError() {
                if (m_error != nullptr) g_error_free(m_error);
            }
            inline const char* ManagedGError::getMessage() {
                return m_error == nullptr ? nullptr : m_error->message;
            }
        }
    }
}
#endif