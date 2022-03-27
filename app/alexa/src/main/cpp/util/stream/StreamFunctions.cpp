#include <sstream>
#include <string>
#include "StreamFunctions.h"
#include "Streambuf.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace stream {
                std::unique_ptr<std::istream> streamFromData(const unsigned char* data, size_t length) {
                    class ResourceStream : public std::istream {
                    public:
                        ResourceStream(std::unique_ptr<Streambuf> buf) : std::istream(buf.get()), m_buf(std::move(buf)) {}
                    private:
                        std::unique_ptr<Streambuf> m_buf;
                    };
                    return std::unique_ptr<ResourceStream>(new ResourceStream(std::unique_ptr<Streambuf>(new Streambuf(data, length))));
                }
            }
        }
    }
}
