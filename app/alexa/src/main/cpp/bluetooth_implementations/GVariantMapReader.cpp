#include <logger/Logger.h>
#include "GVariantMapReader.h"

namespace alexaClientSDK {
    namespace bluetoothImplementations {
        namespace blueZ {
            using namespace std;
            using namespace avsCommon;
            using namespace utils;
            using namespace logger;
            static const string TAG{"GVariantMapReader"};
            #define LX(event) LogEntry(TAG, event)
            bool GVariantMapReader::forEach(function<bool(char* key, GVariant* value)> iteratorFunction) const {
                if (nullptr == m_map) {
                    ACSDK_ERROR(LX("forEachFailed").d("reason", "m_map is null"));
                    return false;
                }
                GVariantIter iter;
                g_variant_iter_init(&iter, m_map);
                GVariant* value = nullptr;
                char* key = nullptr;
                bool stopped = false;
                while (g_variant_iter_next(&iter, m_useObjectPathKeys ? "{&o*}" : "{&s*}", &key, &value)) {
                    bool shouldContinue = iteratorFunction(key, value);
                    g_variant_unref(value);
                    if (!shouldContinue) {
                        stopped = true;
                        break;
                    }
                }
                return stopped;
            }
            GVariantMapReader::GVariantMapReader(GVariant* originalVariant, bool useObjectPathAsKey) : m_map{originalVariant},
                                                                                                       m_useObjectPathKeys{useObjectPathAsKey} {
                if (originalVariant) g_variant_ref(originalVariant);
            }
            GVariantMapReader::GVariantMapReader(ManagedGVariant& originalVariant, bool useObjectPathAsKey) : m_map{originalVariant.get()},
                                                                                                              m_useObjectPathKeys{useObjectPathAsKey} {
                if (originalVariant.hasValue()) g_variant_ref(originalVariant.get());
            }

            GVariantMapReader::~GVariantMapReader() {
                if (m_map != nullptr) g_variant_unref(m_map);
            }
            bool GVariantMapReader::getCString(const char* name, char** value) const {
                if (nullptr == m_map) {
                    ACSDK_ERROR(LX("getCStringFailed").d("reason", "m_map is null"));
                    return false;
                }
                if (nullptr == name) {
                    ACSDK_ERROR(LX("getCStringFailed").d("reason", "name is null"));
                    return false;
                }
                if (nullptr == value) {
                    ACSDK_ERROR(LX("getCStringFailed").d("reason", "value is null"));
                    return false;
                }
                return g_variant_lookup(m_map, name, "&s", value) != 0;
            }
            bool GVariantMapReader::getInt32(const char* name, gint32* value) const {
                if (nullptr == m_map) {
                    ACSDK_ERROR(LX("getInt32Failed").d("reason", "m_map is null"));
                    return false;
                }
                if (nullptr == name) {
                    ACSDK_ERROR(LX("getInt32Failed").d("reason", "name is null"));
                    return false;
                }
                if (nullptr == value) {
                    ACSDK_ERROR(LX("getInt32Failed").d("reason", "value is null"));
                    return false;
                }
                return g_variant_lookup(m_map, name, "i", value) != 0;
            }
            bool GVariantMapReader::getBoolean(const char* name, gboolean* value) const {
                if (nullptr == m_map) {
                    ACSDK_ERROR(LX("getBooleanFailed").d("reason", "m_map is null"));
                    return false;
                }
                if (nullptr == name) {
                    ACSDK_ERROR(LX("getBooleanFailed").d("reason", "name is null"));
                    return false;
                }
                if (nullptr == value) {
                    ACSDK_ERROR(LX("getBooleanFailed").d("reason", "value is null"));
                    return false;
                }
                return g_variant_lookup(m_map, name, "b", value) != 0;
            }
            ManagedGVariant GVariantMapReader::getVariant(const char* name) const {
                if (nullptr == m_map) {
                    ACSDK_ERROR(LX("getVariantFailed").d("reason", "m_map is null"));
                    return ManagedGVariant();
                }
                if (nullptr == name) {
                    ACSDK_ERROR(LX("getVariantFailed").d("reason", "name is null"));
                    return ManagedGVariant();
                }
                GVariant* value = nullptr;
                g_variant_lookup(m_map, name, "*", &value);
                return ManagedGVariant(value);
            }
            GVariant* GVariantMapReader::get() const {
                return m_map;
            }
            GVariantMapReader::GVariantMapReader(const GVariantMapReader& other) {
                m_map = other.m_map;
                if (nullptr != m_map) g_variant_ref(m_map);
                m_useObjectPathKeys = other.m_useObjectPathKeys;
            }
        }
    }
}