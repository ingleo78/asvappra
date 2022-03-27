#ifndef RAPIDJSON_STRINGBUFFER_H_
#define RAPIDJSON_STRINGBUFFER_H_

#include "stream.h"
#include "stack.h"
#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
#include <utility>
#endif
#include "stack.h"

#if defined(__clang__)
    RAPIDJSON_DIAG_PUSH
    RAPIDJSON_DIAG_OFF(c++98-compat)
#endif
RAPIDJSON_NAMESPACE_BEGIN
    template <typename Encoding, typename Allocator> class GenericStringBuffer {
        public:
            typedef typename Encoding::Ch Ch;
            GenericStringBuffer(Allocator* allocator = NULL, size_t capacity = kDefaultCapacity) : stack_(allocator, capacity) {}
            #if RAPIDJSON_HAS_CXX11_RVALUE_REFS
                GenericStringBuffer(GenericStringBuffer& rhs) : stack_(std::move(rhs.stack_)) {}
                GenericStringBuffer& operator=(GenericStringBuffer&& rhs) {
                    if (&rhs != this) stack_ = std::move(rhs.stack_);
                    return *this;
                }
            #endif
            ~GenericStringBuffer() { Clear(); }
            void Put(Ch c) { *stack_.template Push<Ch>() = c; }
            void PutUnsafe(Ch c) { *stack_.template PushUnsafe<Ch>() = c; }
            void Flush() { Clear(); }
            void Clear() { stack_.Clear(); }
            void ShrinkToFit() {
                *stack_.template Push<Ch>() = '\0';
                stack_.ShrinkToFit();
                stack_.template Pop<Ch>(1);
            }
            void Reserve(size_t count) { stack_.template Reserve<Ch>(count); }
            Ch* Push(size_t count) { return stack_.template Push<Ch>(count); }
            Ch* PushUnsafe(size_t count) { return stack_.template PushUnsafe<Ch>(count); }
            void Pop(size_t count) { stack_.template Pop<Ch>(count); }
            const Ch* GetString() const {
                *stack_.template Push<Ch>() = '\0';
                stack_.template Pop<Ch>(1);
                return stack_.template Bottom<Ch>();
            }
            size_t GetSize() const { return stack_.GetSize(); }
            size_t GetLength() const { return stack_.GetSize() / sizeof(Ch); }
            GenericStringBuffer& operator=(const GenericStringBuffer& rhs) { Clear(); *this = rhs; return *this; };
            static const size_t kDefaultCapacity = 256;
            mutable internal::Stack<Allocator> stack_;
        private:
            GenericStringBuffer(const GenericStringBuffer& rhs) : stack_(rhs.stack_) { };
    };
    typedef GenericStringBuffer<UTF8<>, CrtAllocator> StringBuffer;
    template<typename Encoding, typename Allocator> inline void PutReserve(GenericStringBuffer<Encoding, Allocator>& stream, size_t count) {
        stream.Reserve(count);
    }
    template<typename Encoding, typename Allocator> inline void PutUnsafe(GenericStringBuffer<Encoding, Allocator>& stream, typename Encoding::Ch c) {
        stream.PutUnsafe(c);
    }
    template<> inline void PutN(GenericStringBuffer<UTF8<>, std::string>& stream, char c, size_t n) {
        std::memset(stream.stack_.Push<char>(n), c, n * sizeof(c));
    }
RAPIDJSON_NAMESPACE_END
#if defined(__clang__)
    RAPIDJSON_DIAG_POP
#endif
#endif