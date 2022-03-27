#include "reader.h"

using namespace rapidjson;

template <typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::GenericReader(StackAllocator* stackAllocator, size_t stackCapacity) :
    stack_(stackAllocator, stackCapacity), parseResult_(), state_(IterativeParsingStartState) { }
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template <unsigned parseFlags, typename InputStream, typename Handler> ParseResult GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::Parse(InputStream& is, Handler& handler) {
    if (parseFlags & kParseIterativeFlag) return IterativeParse<parseFlags>(is, handler);
    parseResult_.Clear();
    ClearStackOnExit scope(*this);
    SkipWhitespaceAndComments<parseFlags>(is);
    RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);
    if (RAPIDJSON_UNLIKELY(is.Peek() == '\0')) {
        RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorDocumentEmpty, is.Tell());
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);
    } else {
        ParseValue<parseFlags>(is, handler);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);
        if (!(parseFlags & kParseStopWhenDoneFlag)) {
            SkipWhitespaceAndComments<parseFlags>(is);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);
            if (RAPIDJSON_UNLIKELY(is.Peek() != '\0')) {
                RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorDocumentRootNotSingular, is.Tell());
                RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);
            }
        }
    }
    return parseResult_;
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<typename InputStream, typename Handler> ParseResult GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::Parse(InputStream& is, Handler& handler) {
    return Parse<kParseDefaultFlags>(is, handler);
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::IterativeParseInit() {
    parseResult_.Clear();
    state_ = IterativeParsingStartState;
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream, typename Handler> bool GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::IterativeParseNext(InputStream& is, Handler& handler) {
    while (RAPIDJSON_LIKELY(is.Peek() != '\0')) {
        SkipWhitespaceAndComments<parseFlags>(is);
        Token t = Tokenize(is.Peek());
        IterativeParsingState n = Predict(state_, t);
        IterativeParsingState d = Transit<parseFlags>(state_, t, n, is, handler);
        if (RAPIDJSON_UNLIKELY(IsIterativeParsingCompleteState(d))) {
            if (d == IterativeParsingErrorState) {
                HandleError(state_, is);
                return false;
            }
            RAPIDJSON_ASSERT(d == IterativeParsingFinishState);
            state_ = d;
            if (!(parseFlags & kParseStopWhenDoneFlag)) {
                SkipWhitespaceAndComments<parseFlags>(is);
                if (is.Peek() != '\0') {
                    HandleError(state_, is);
                    return false;
                }
            }
            return true;
        }
        state_ = d;
        if (!IsIterativeParsingDelimiterState(n)) return true;
    }
    stack_.Clear();
    if (state_ != IterativeParsingFinishState) {
        HandleError(state_, is);
        return false;
    }
    return true;
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
RAPIDJSON_FORCEINLINE bool GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::IterativeParseComplete() const {
    return IsIterativeParsingCompleteState(state_);
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
bool GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::HasParseError() const { return parseResult_.IsError(); }
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
ParseErrorCode GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::GetParseErrorCode() const { return parseResult_.Code(); }
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
size_t GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::GetErrorOffset() const { return parseResult_.Offset(); }
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::SetParseError(ParseErrorCode code, size_t offset) { parseResult_.Set(code, offset); }
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream> void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::SkipWhitespaceAndComments(InputStream& is) {
    SkipWhitespace(is);
    if (parseFlags & kParseCommentsFlag) {
        while (RAPIDJSON_UNLIKELY(Consume(is, '/'))) {
            if (Consume(is, '*')) {
                while (true) {
                    if (RAPIDJSON_UNLIKELY(is.Peek() == '\0')) {
                        RAPIDJSON_PARSE_ERROR(kParseErrorUnspecificSyntaxError, is.Tell());
                    } else if (Consume(is, '*')) {
                        if (Consume(is, '/')) break;
                    } else is.Take();
                }
            } else if (RAPIDJSON_LIKELY(Consume(is, '/'))) while (is.Peek() != '\0' && is.Take() != '\n') {}
            else RAPIDJSON_PARSE_ERROR(kParseErrorUnspecificSyntaxError, is.Tell());
            SkipWhitespace(is);
        }
    }
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream, typename Handler> void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseObject(InputStream& is, Handler& handler) {
    RAPIDJSON_ASSERT(is.Peek() == '{');
    is.Take();
    if (RAPIDJSON_UNLIKELY(!handler.StartObject())) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
    SkipWhitespaceAndComments<parseFlags>(is);
    RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
    if (Consume(is, '}')) {
        if (RAPIDJSON_UNLIKELY(!handler.EndObject(0))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
        return;
    }
    for (SizeType memberCount = 0;;) {
        if (RAPIDJSON_UNLIKELY(is.Peek() != '"')) RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissName, is.Tell());
        ParseString<parseFlags>(is, handler, true);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        SkipWhitespaceAndComments<parseFlags>(is);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        if (RAPIDJSON_UNLIKELY(!Consume(is, ':'))) RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissColon, is.Tell());
        SkipWhitespaceAndComments<parseFlags>(is);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        ParseValue<parseFlags>(is, handler);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        SkipWhitespaceAndComments<parseFlags>(is);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        ++memberCount;
        switch (is.Peek()) {
            case ',':
                is.Take();
                SkipWhitespaceAndComments<parseFlags>(is);
                RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
                break;
            case '}':
                is.Take();
                if (RAPIDJSON_UNLIKELY(!handler.EndObject(memberCount))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
                return;
            default: RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissCommaOrCurlyBracket, is.Tell()); break;
        }
        if (parseFlags & kParseTrailingCommasFlag) {
            if (is.Peek() == '}') {
                if (RAPIDJSON_UNLIKELY(!handler.EndObject(memberCount))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
                is.Take();
                return;
            }
        }
    }
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream, typename Handler> void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseArray(InputStream& is, Handler& handler) {
    RAPIDJSON_ASSERT(is.Peek() == '[');
    is.Take();
    if (RAPIDJSON_UNLIKELY(!handler.StartArray())) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
    SkipWhitespaceAndComments<parseFlags>(is);
    RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
    if (Consume(is, ']')) {
        if (RAPIDJSON_UNLIKELY(!handler.EndArray(0))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
        return;
    }
    for (SizeType elementCount = 0;;) {
        ParseValue<parseFlags>(is, handler);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        ++elementCount;
        SkipWhitespaceAndComments<parseFlags>(is);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        if (Consume(is, ',')) {
            SkipWhitespaceAndComments<parseFlags>(is);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        } else if (Consume(is, ']')) {
            if (RAPIDJSON_UNLIKELY(!handler.EndArray(elementCount))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
            return;
        } else RAPIDJSON_PARSE_ERROR(kParseErrorArrayMissCommaOrSquareBracket, is.Tell());
        if (parseFlags & kParseTrailingCommasFlag) {
            if (is.Peek() == ']') {
                if (RAPIDJSON_UNLIKELY(!handler.EndArray(elementCount))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
                is.Take();
                return;
            }
        }
    }
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream, typename Handler> void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseNull(InputStream& is, Handler& handler) {
    RAPIDJSON_ASSERT(is.Peek() == 'n');
    is.Take();
    if (RAPIDJSON_LIKELY(Consume(is, 'u') && Consume(is, 'l') && Consume(is, 'l'))) {
        if (RAPIDJSON_UNLIKELY(!handler.Null())) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
    } else RAPIDJSON_PARSE_ERROR(kParseErrorValueInvalid, is.Tell());
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream, typename Handler> void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseTrue(InputStream& is, Handler& handler) {
    RAPIDJSON_ASSERT(is.Peek() == 't');
    is.Take();
    if (RAPIDJSON_LIKELY(Consume(is, 'r') && Consume(is, 'u') && Consume(is, 'e'))) {
        if (RAPIDJSON_UNLIKELY(!handler.Bool(true))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
    } else RAPIDJSON_PARSE_ERROR(kParseErrorValueInvalid, is.Tell());
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream, typename Handler> void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseFalse(InputStream& is, Handler& handler) {
    RAPIDJSON_ASSERT(is.Peek() == 'f');
    is.Take();
    if (RAPIDJSON_LIKELY(Consume(is, 'a') && Consume(is, 'l') && Consume(is, 's') && Consume(is, 'e'))) {
        if (RAPIDJSON_UNLIKELY(!handler.Bool(false))) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
    } else RAPIDJSON_PARSE_ERROR(kParseErrorValueInvalid, is.Tell());
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<typename InputStream> RAPIDJSON_FORCEINLINE static bool GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::Consume(InputStream& is, typename InputStream::Ch expect) {
    if (RAPIDJSON_LIKELY(is.Peek() == expect)) {
        is.Take();
        return true;
    } else return false;
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<typename InputStream> unsigned GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseHex4(InputStream& is, size_t escapeOffset) {
    unsigned codepoint = 0;
    for (int i = 0; i < 4; i++) {
        Ch c = is.Peek();
        codepoint <<= 4;
        codepoint += static_cast<unsigned>(c);
        if (c >= '0' && c <= '9') codepoint -= '0';
        else if (c >= 'A' && c <= 'F') codepoint -= 'A' - 10;
        else if (c >= 'a' && c <= 'f') codepoint -= 'a' - 10;
        else {
            RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorStringUnicodeEscapeInvalidHex, escapeOffset);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN(0);
        }
        is.Take();
    }
    return codepoint;
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename InputStream, typename Handler> void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseString(InputStream& is, Handler& handler, bool isKey) {
    internal::StreamLocalCopy<InputStream> copy(is);
    InputStream& s(copy.s);
    RAPIDJSON_ASSERT(s.Peek() == '\"');
    s.Take();
    bool success = false;
    if (parseFlags & kParseInsituFlag) {
        typename InputStream::Ch *head = s.PutBegin();
        ParseStringToStream<parseFlags, SourceEncoding, SourceEncoding>(s, s);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        size_t length = s.PutEnd(head) - 1;
        RAPIDJSON_ASSERT(length <= 0xFFFFFFFF);
        const typename TargetEncoding::Ch* const str = reinterpret_cast<typename TargetEncoding::Ch*>(head);
        success = (isKey ? handler.Key(str, SizeType(length), false) : handler.String(str, SizeType(length), false));
    } else {
        StackStream<typename TargetEncoding::Ch> stackStream(stack_);
        ParseStringToStream<parseFlags, SourceEncoding, TargetEncoding>(s, stackStream);
        RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
        SizeType length = static_cast<SizeType>(stackStream.Length()) - 1;
        const typename TargetEncoding::Ch* const str = stackStream.Pop();
        success = (isKey ? handler.Key(str, length, true) : handler.String(str, length, true));
    }
    if (RAPIDJSON_UNLIKELY(!success)) RAPIDJSON_PARSE_ERROR(kParseErrorTermination, s.Tell());
}
template<typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
template<unsigned parseFlags, typename SEncoding, typename TEncoding, typename InputStream, typename OutputStream>
RAPIDJSON_FORCEINLINE void GenericReader<SourceEncoding, TargetEncoding, StackAllocator>::ParseStringToStream(InputStream& is, OutputStream& os) {
    #define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    static const char escape[256] = {
        Z16, Z16, 0, 0,'\"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '/',
        Z16, Z16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'\\', 0, 0, 0,
        0, 0,'\b', 0, 0, 0,'\f', 0, 0, 0, 0, 0, 0, 0,'\n', 0,
        0, 0,'\r', 0,'\t', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16
    };
    #undef Z16
    for ( ; ; ) {
        if (!(parseFlags & kParseValidateEncodingFlag)) ScanCopyUnescapedString(is, os);
        Ch c = is.Peek();
        if (RAPIDJSON_UNLIKELY(c == '\\')) {
            size_t escapeOffset = is.Tell();
            is.Take();
            Ch e = is.Peek();
            if ((sizeof(Ch) == 1 || unsigned(e) < 256) && RAPIDJSON_LIKELY(escape[static_cast<unsigned char>(e)])) {
                is.Take();
                os.Put(static_cast<typename TEncoding::Ch>(escape[static_cast<unsigned char>(e)]));
            } else if ((parseFlags & kParseEscapedApostropheFlag) && RAPIDJSON_LIKELY(e == '\'')) {
                is.Take();
                os.Put('\'');
            } else if (RAPIDJSON_LIKELY(e == 'u')) {
                is.Take();
                unsigned codepoint = ParseHex4(is, escapeOffset);
                RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
                if (RAPIDJSON_UNLIKELY(codepoint >= 0xD800 && codepoint <= 0xDBFF)) {
                    if (RAPIDJSON_UNLIKELY(!Consume(is, '\\') || !Consume(is, 'u')))
                        RAPIDJSON_PARSE_ERROR(kParseErrorStringUnicodeSurrogateInvalid, escapeOffset);
                    unsigned codepoint2 = ParseHex4(is, escapeOffset);
                    RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
                    if (RAPIDJSON_UNLIKELY(codepoint2 < 0xDC00 || codepoint2 > 0xDFFF))
                        RAPIDJSON_PARSE_ERROR(kParseErrorStringUnicodeSurrogateInvalid, escapeOffset);
                    codepoint = (((codepoint - 0xD800) << 10) | (codepoint2 - 0xDC00)) + 0x10000;
                }
                TEncoding::Encode(os, codepoint);
            } else RAPIDJSON_PARSE_ERROR(kParseErrorStringEscapeInvalid, escapeOffset);
        } else if (RAPIDJSON_UNLIKELY(c == '"')) {
            is.Take();
            os.Put('\0');
            return;
        }
        else if (RAPIDJSON_UNLIKELY(static_cast<unsigned>(c) < 0x20)) {
            if (c == '\0') {
                RAPIDJSON_PARSE_ERROR(kParseErrorStringMissQuotationMark, is.Tell());
            } else RAPIDJSON_PARSE_ERROR(kParseErrorStringInvalidEncoding, is.Tell());
        } else {
            size_t offset = is.Tell();
            if (RAPIDJSON_UNLIKELY((parseFlags & kParseValidateEncodingFlag ? !Transcoder<SEncoding, TEncoding>::Validate(is, os) :
                                    !Transcoder<SEncoding, TEncoding>::Transcode(is, os))))
                RAPIDJSON_PARSE_ERROR(kParseErrorStringInvalidEncoding, offset);
        }
    }
}