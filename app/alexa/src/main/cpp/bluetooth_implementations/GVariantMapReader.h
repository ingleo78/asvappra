#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_GVARIANTMAPREADER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_GVARIANTMAPREADER_H_

#include <functional>
#include <gio/gio.h>
#include "ManagedGVariant.h"
#include "ManagedGError.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class GVariantMapReader {
            public:
                ~GVariantMapReader();
                explicit GVariantMapReader(GVariant* originalVariant, bool useObjectPathAsKey = false);
                explicit GVariantMapReader(ManagedGVariant& originalVariant, bool useObjectPathAsKey = false);
                GVariantMapReader(const GVariantMapReader& other);
                bool getCString(const char* name, char** value) const;
                bool getInt32(const char* name, gint32* value) const;
                bool getBoolean(const char* name, gboolean* value) const;
                ManagedGVariant getVariant(const char* name) const;
                bool forEach(std::function<bool(char* key, GVariant* value)> iteratorFunction) const;
                GVariant* get() const;
            private:
                GVariant* m_map;
                bool m_useObjectPathKeys;
            };
        }
    }
}
#endif