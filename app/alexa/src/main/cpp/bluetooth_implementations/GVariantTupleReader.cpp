#include <logger/Logger.h>
#include "GVariantTupleReader.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            static const std::string TAG{"GVariantTupleReader"};
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            bool GVariantTupleReader::forEach(std::function<bool(GVariant* value)> iteratorFunction) const {
                if (nullptr == m_tuple) {
                    ACSDK_ERROR(LX("forEachFailed").d("reason", "m_tuple is null"));
                    return false;
                }
                GVariantIter iter;
                g_variant_iter_init(&iter, m_tuple);
                GVariant* value = nullptr;
                bool stopped = false;
                while (g_variant_iter_next(&iter, "*", &value)) {
                    bool shouldContinue = iteratorFunction(value);
                    g_variant_unref(value);
                    if (!shouldContinue) {
                        stopped = true;
                        break;
                    }
                }
                return stopped;
            }
            GVariantTupleReader::GVariantTupleReader(GVariant* originalVariant) : m_tuple{originalVariant} {
                if (originalVariant) g_variant_ref(originalVariant);
            }
            GVariantTupleReader::GVariantTupleReader(ManagedGVariant& originalVariant) : m_tuple{originalVariant.get()} {
                if (originalVariant.hasValue()) g_variant_ref(originalVariant.get());
            }
            GVariantTupleReader::~GVariantTupleReader() {
                if (m_tuple != nullptr) g_variant_unref(m_tuple);
            }
            char* GVariantTupleReader::getCString(gsize index) const {
                if (nullptr == m_tuple) {
                    ACSDK_ERROR(LX("getCStringFailed").d("reason", "m_tuple is null"));
                    return nullptr;
                }
                if (index >= g_variant_n_children(m_tuple)) {
                    ACSDK_ERROR(LX("getCStringFailed").d("reason", "index out of range"));
                    return nullptr;
                }
                char* value = nullptr;
                g_variant_get_child(m_tuple, index, "&s", &value);
                return value;
            }
            char* GVariantTupleReader::getObjectPath(gsize index) const {
                if (nullptr == m_tuple) {
                    ACSDK_ERROR(LX("getObjectPathFailed").d("reason", "m_tuple is null"));
                    return nullptr;
                }
                if (index >= g_variant_n_children(m_tuple)) {
                    ACSDK_ERROR(LX("getObjectPathFailed").d("reason", "index out of range"));
                    return nullptr;
                }
                char* value = nullptr;
                g_variant_get_child(m_tuple, index, "&o", &value);
                return value;
            }
            gint32 GVariantTupleReader::getInt32(gsize index) const {
                if (nullptr == m_tuple) {
                    ACSDK_ERROR(LX("getInt32Failed").d("reason", "m_tuple is null"));
                    return 0;
                }
                if (index >= g_variant_n_children(m_tuple)) {
                    ACSDK_ERROR(LX("getInt32Failed").d("reason", "index out of range"));
                    return 0;
                }
                gint32 value = 0;
                g_variant_get_child(m_tuple, index, "i", &value);
                return value;
            }
            gboolean GVariantTupleReader::getBoolean(gsize index) const {
                if (nullptr == m_tuple) {
                    ACSDK_ERROR(LX("getBooleanFailed").d("reason", "m_tuple is null"));
                    return false;
                }
                if (index >= g_variant_n_children(m_tuple)) {
                    ACSDK_ERROR(LX("getBooleanFailed").d("reason", "index out of range"));
                    return false;
                }
                gboolean value = false;
                g_variant_get_child(m_tuple, index, "b", &value);
                return value;
            }
            ManagedGVariant GVariantTupleReader::getVariant(gsize index) const {
                if (nullptr == m_tuple) {
                    ACSDK_ERROR(LX("getVariantFailed").d("reason", "m_tuple is null"));
                    return ManagedGVariant();
                }
                if (index >= g_variant_n_children(m_tuple)) {
                    ACSDK_ERROR(LX("getVariantFailed").d("reason", "index out of range"));
                    return ManagedGVariant();
                }
                GVariant* value = nullptr;
                value = g_variant_get_child_value(m_tuple, index);
                return ManagedGVariant(value);
            }
            gsize GVariantTupleReader::size() const {
                if (nullptr == m_tuple) {
                    ACSDK_ERROR(LX("sizeFailed").d("reason", "m_tuple is null"));
                    return 0;
                }
                return g_variant_n_children(m_tuple);
            }
            GVariantTupleReader::GVariantTupleReader(const GVariantTupleReader& other) {
                m_tuple = other.m_tuple;
                if (nullptr != m_tuple) g_variant_ref(m_tuple);
            }
        }
    }
}