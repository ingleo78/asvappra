#ifndef ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MANAGEDGVARIANT_H_
#define ALEXA_CLIENT_SDK_BLUETOOTHIMPLEMENTATIONS_BLUEZ_INCLUDE_BLUEZ_MANAGEDGVARIANT_H_

#include <memory>
#include <string>
#include <gio/gio.h>
#include <glib/glib.h>
#include <glib/gvariant.h>

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            class ManagedGVariant {
            public:
                ~ManagedGVariant();
                ManagedGVariant();
                explicit ManagedGVariant(GVariant* variant);
                GVariant** asOutputParameter();
                GVariant* get();
                std::string dumpToString(bool withAnnotations);
                ManagedGVariant unbox();
                bool hasValue();
                void swap(ManagedGVariant& other);
            private:
                GVariant* m_variant;
            };
            inline ManagedGVariant::~ManagedGVariant() {
                if (m_variant) g_variant_unref(m_variant);
            }
            inline ManagedGVariant::ManagedGVariant() : m_variant{nullptr} {}
            inline ManagedGVariant::ManagedGVariant(GVariant* variant) : m_variant{variant} {
                if (variant && g_variant_is_floating(variant)) g_variant_ref_sink(variant);
            }
            inline GVariant** ManagedGVariant::asOutputParameter() {
                return &m_variant;
            }
            inline GVariant* ManagedGVariant::get() {
                return m_variant;
            }
            inline std::string ManagedGVariant::dumpToString(bool withAnnotations) {
                if (!m_variant) return "<NULL>";
                char* cstring = g_variant_print(m_variant, (gboolean)(&withAnnotations));
                std::string result = cstring;
                g_free(cstring);
                return result;
            }
            inline ManagedGVariant ManagedGVariant::unbox() {
                if (!m_variant) return ManagedGVariant();
                return ManagedGVariant(g_variant_get_variant(m_variant));
            }
            inline bool ManagedGVariant::hasValue() {
                return m_variant != nullptr;
            }
            inline void ManagedGVariant::swap(ManagedGVariant& other) {
                GVariant* temp = other.m_variant;
                other.m_variant = m_variant;
                m_variant = temp;
            }
        }
    }
}
#endif