#include "Streambuf.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace stream {
                Streambuf::Streambuf(const unsigned char* data, size_t length) :
                                     m_begin(reinterpret_cast<char*>(const_cast<unsigned char*>(data))), m_end(m_begin + length) {
                    setg(m_begin, m_begin, m_end);
                }
                std::streampos Streambuf::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) {
                    switch (way) {
                        case std::ios_base::beg: setg(m_begin, m_begin + off, m_end); break;
                        case std::ios_base::cur: gbump(off); break;
                        case std::ios_base::end: setg(m_begin, m_end + off, m_end); break;
                        default: return std::streampos(std::streamoff(-1));
                    }
                    if (!gptr() || gptr() >= egptr() || gptr() < eback()) {
                        return std::streampos(std::streamoff(-1));
                    }
                    return gptr() - eback();
                }
                std::streampos Streambuf::seekpos(std::streampos sp, std::ios_base::openmode which) {
                    return seekoff(sp - pos_type(off_type(0)), std::ios_base::beg, which);
                }
                Streambuf::int_type Streambuf::underflow() {
                    if (gptr() == m_end) return Streambuf::traits_type::eof();
                    return Streambuf::traits_type::to_int_type(*gptr());
                }
                Streambuf::int_type Streambuf::pbackfail(int_type ch) {
                    if (gptr() <= eback() || gptr() > egptr() || (ch != Streambuf::traits_type::eof() && ch != egptr()[-1])) {
                        return Streambuf::traits_type::eof();
                    }
                    gbump(-1);
                    return ch;
                }
                std::streamsize Streambuf::showmanyc() {
                    return egptr() - gptr();
                }
            }
        }
    }
}
