#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_GVARIANTTUPLEREADER_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_GVARIANTTUPLEREADER_H_

#include <functional>
#include <gio/gio.h>
#include <gobject/gtype.h>
#include <gio/config.h>
#include <glib/glib-object.h>
#include "ManagedGVariant.h"
#include "ManagedGError.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class GVariantTupleReader {
            public:
                ~GVariantTupleReader();
                explicit GVariantTupleReader(GVariant* originalVariant);
                explicit GVariantTupleReader(ManagedGVariant& originalVariant);
                GVariantTupleReader(const GVariantTupleReader& other);
                char* getCString(gsize index) const;
                char* getObjectPath(gsize index) const;
                gint32 getInt32(gsize index) const;
                gboolean getBoolean(gsize index) const;
                ManagedGVariant getVariant(gsize index) const;
                gsize size() const;
                bool forEach(std::function<bool(GVariant* value)> iteratorFunction) const;
            private:
                GVariant* m_tuple;
            };
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_GVARIANTTUPLEREADER_H_
