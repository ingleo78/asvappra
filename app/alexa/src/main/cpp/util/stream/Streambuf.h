#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STREAM_STREAMBUF_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STREAM_STREAMBUF_H_

#include <sstream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace stream {
                using namespace std;
                class Streambuf : public streambuf {
                public:
                    Streambuf(const unsigned char* data, size_t length);
                    streampos seekoff(streamoff off, ios_base::seekdir way, ios_base::openmode which = ios_base::in) override;
                    streampos seekpos(streampos sp, ios_base::openmode which = ios_base::in) override;
                private:
                    int_type underflow() override;
                    int_type pbackfail(int_type ch) override;
                    streamsize showmanyc() override;
                    char* const m_begin;
                    char* const m_end;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_STREAM_STREAMBUF_H_
