#include "curl_setup.h"
#include "curl_setup_once.h"

#ifndef CURL_DOES_CONVERSIONS
    #define FALSE 0
    #undef _U
    #define _U (1<<0)
    #undef _L
    #define _L (1<<1)
    #undef _N
    #define _N (1<<2)
    #undef _S
    #define _S (1<<3)
    #undef _P
    #define _P (1<<4)
    #undef _C
    #define _C (1<<5)
    #undef _X
    #define _X (1<<6)
    #undef _B
    #define _B (1<<7)
    static const unsigned char ascii[128] = {
      _C,   _C,     _C,     _C,     _C,     _C,     _C,     _C,
      _C,   _C|_S,  _C|_S,  _C|_S,  _C|_S,  _C|_S,  _C,     _C,
      _C,   _C,     _C,     _C,     _C,     _C,     _C,     _C,
      _C,   _C,     _C,     _C,     _C,     _C,     _C,     _C,
      _S|_B, _P,    _P,     _P,     _P,     _P,     _P,     _P,
      _P,   _P,     _P,     _P,     _P,     _P,     _P,     _P,
      _N,   _N,     _N,     _N,     _N,     _N,     _N,     _N,
      _N,   _N,     _P,     _P,     _P,     _P,     _P,     _P,
      _P,   _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U,
      _U,   _U,     _U,     _U,     _U,     _U,     _U,     _U,
      _U,   _U,     _U,     _U,     _U,     _U,     _U,     _U,
      _U,   _U,     _U,     _P,     _P,     _P,     _P,     _P,
      _P,   _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L,
      _L,   _L,     _L,     _L,     _L,     _L,     _L,     _L,
      _L,   _L,     _L,     _L,     _L,     _L,     _L,     _L,
      _L,   _L,     _L,     _P,     _P,     _P,     _P,     _C
    };
    int Curl_isspace(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & _S);
    }
    int Curl_isdigit(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & _N);
    }
    int Curl_isalnum(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & (_N|_U|_L));
    }
    int Curl_isxdigit(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & (_N|_X));
    }
    int Curl_isgraph(int c) {
      if ((c < 0) || (c >= 0x80) || (c == ' ')) return FALSE;
      return (ascii[c] & (_N|_X|_U|_L|_P|_S));
    }
    int Curl_isprint(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & (_N|_X|_U|_L|_P|_S));
    }
    int Curl_isalpha(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & (_U|_L));
    }
    int Curl_isupper(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & (_U));
    }
    int Curl_islower(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & (_L));
    }
    int Curl_iscntrl(int c) {
      if ((c < 0) || (c >= 0x80)) return FALSE;
      return (ascii[c] & (_C));
    }
#endif