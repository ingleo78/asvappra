#include "CaptionData.h"

namespace alexaClientSDK {
    namespace captions {
        bool CaptionData::isValid() const {
            return (CaptionFormat::UNKNOWN != format) && !content.empty();
        }
    }
}