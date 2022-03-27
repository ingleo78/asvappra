#include <map>
#include <json/document.h>
#include <json/JSONUtils.h>
#include <logger/Logger.h>
#include "BlockingPolicy.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace rapidjson;
            using namespace avs;
            using namespace utils;
            using namespace json;
            using namespace logger;
            static const long MEDIUM_FLAG_AUDIO = 1;
            static const long MEDIUM_FLAG_VISUAL = 2;
            static const string TAG("BlockingPolicy");
            #define LX(event) LogEntry(TAG, event)
            const BlockingPolicy::Mediums BlockingPolicy::MEDIUM_AUDIO{MEDIUM_FLAG_AUDIO};
            const BlockingPolicy::Mediums BlockingPolicy::MEDIUM_VISUAL{MEDIUM_FLAG_VISUAL};
            const BlockingPolicy::Mediums BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL{MEDIUM_FLAG_AUDIO | MEDIUM_FLAG_VISUAL};
            const BlockingPolicy::Mediums BlockingPolicy::MEDIUMS_NONE{};
            BlockingPolicy::BlockingPolicy(const Mediums& mediums, bool isBlocking) : m_mediums{mediums}, m_isBlocking{isBlocking} { }
            bool BlockingPolicy::isValid() const {
                auto isValid = !((MEDIUMS_NONE == m_mediums) && m_isBlocking);
                return isValid;
            }
            bool BlockingPolicy::isBlocking() const { return m_isBlocking; }
            BlockingPolicy::Mediums BlockingPolicy::getMediums() const { return m_mediums; }
            bool operator==(const BlockingPolicy& lhs, const BlockingPolicy& rhs) {
                return ((lhs.getMediums() == rhs.getMediums()) && (lhs.isBlocking() == rhs.isBlocking()));
            }
            bool operator!=(const BlockingPolicy& lhs, const BlockingPolicy& rhs) { return !(lhs == rhs); }
        }
    }
}