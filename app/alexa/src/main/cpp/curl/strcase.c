#include "curl_setup.h"
#include "curl.h"
#include "strcase.h"

static char raw_tolower(char in);
char Curl_raw_toupper(char in) {
#if !defined(CURL_DOES_CONVERSIONS)
    if(in >= 'a' && in <= 'z') return (char)('A' + in - 'a');
#else
    switch(in) {
        case 'a': return 'A';
        case 'b': return 'B';
        case 'c': return 'C';
        case 'd': return 'D';
        case 'e': return 'E';
        case 'f': return 'F';
        case 'g': return 'G';
        case 'h': return 'H';
        case 'i': return 'I';
        case 'j': return 'J';
        case 'k': return 'K';
        case 'l': return 'L';
        case 'm': return 'M';
        case 'n': return 'N';
        case 'o': return 'O';
        case 'p': return 'P';
        case 'q': return 'Q';
        case 'r': return 'R';
        case 's': return 'S';
        case 't': return 'T';
        case 'u': return 'U';
        case 'v': return 'V';
        case 'w': return 'W';
        case 'x': return 'X';
        case 'y': return 'Y';
        case 'z': return 'Z';
    }
#endif
    return in;
}
static char raw_tolower(char in) {
#if !defined(CURL_DOES_CONVERSIONS)
    if(in >= 'A' && in <= 'Z') return (char)('a' + in - 'A');
#else
    switch(in) {
        case 'A': return 'a';
        case 'B': return 'b';
        case 'C': return 'c';
        case 'D': return 'd';
        case 'E': return 'e';
        case 'F': return 'f';
        case 'G': return 'g';
        case 'H': return 'h';
        case 'I': return 'i';
        case 'J': return 'j';
        case 'K': return 'k';
        case 'L': return 'l';
        case 'M': return 'm';
        case 'N': return 'n';
        case 'O': return 'o';
        case 'P': return 'p';
        case 'Q': return 'q';
        case 'R': return 'r';
        case 'S': return 's';
        case 'T': return 't';
        case 'U': return 'u';
        case 'V': return 'v';
        case 'W': return 'w';
        case 'X': return 'x';
        case 'Y': return 'y';
        case 'Z': return 'z';
    }
#endif
    return in;
}
int Curl_strcasecompare(const char *first, const char *second) {
    while(*first && *second) {
        if(Curl_raw_toupper(*first) != Curl_raw_toupper(*second)) break;
        first++;
        second++;
    }
    return (Curl_raw_toupper(*first) == Curl_raw_toupper(*second));
}
int Curl_safe_strcasecompare(const char *first, const char *second) {
    if(first && second) return Curl_strcasecompare(first, second);
    return (NULL == first && NULL == second);
}
int Curl_strncasecompare(const char *first, const char *second, size_t max) {
    while(*first && *second && max) {
        if(Curl_raw_toupper(*first) != Curl_raw_toupper(*second)) break;
        max--;
        first++;
        second++;
    }
    if(0 == max) return 1;
    return Curl_raw_toupper(*first) == Curl_raw_toupper(*second);
}
void Curl_strntoupper(char *dest, const char *src, size_t n) {
    if(n < 1) return;
    do {
        *dest++ = Curl_raw_toupper(*src);
    } while(*src++ && --n);
}
void Curl_strntolower(char *dest, const char *src, size_t n) {
    if(n < 1) return;
    do {
        *dest++ = raw_tolower(*src);
    } while(*src++ && --n);
}
int curl_strequal(const char *first, const char *second) {
    return Curl_strcasecompare(first, second);
}
int curl_strnequal(const char *first, const char *second, size_t max) {
    return Curl_strncasecompare(first, second, max);
}