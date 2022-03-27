#include <expat/internal.h>
#include "codepage.h"

#if defined(_WIN32)
#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif
int codepageMap(int cp, int *map) {
#if defined(_WIN32)
  int i;
  CPINFO info;
  if (!GetCPInfo(cp, &info) || info.MaxCharSize > 2) return 0;
  for (i = 0; i < 256; i++) map[i] = -1;
  if (info.MaxCharSize > 1) {
      for (i = 0; i < MAX_LEADBYTES; i += 2) {
          int j, lim;
          if (info.LeadByte[i] == 0 && info.LeadByte[i + 1] == 0) break;
          lim = info.LeadByte[i + 1];
          for (j = info.LeadByte[i]; j <= lim; j++) map[j] = -2;
      }
  }
  for (i = 0; i < 256; i++) {
      if (map[i] == -1) {
          char c = (char)i;
          unsigned short n;
          if (MultiByteToWideChar(cp, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, &c, 1, &n, 1) == 1) map[i] = n;
      }
  }
  return 1;
#else
  UNUSED_P(cp);
  UNUSED_P(map);
  return 0;
#endif
}
int codepageConvert(int cp, const char *p) {
#if defined(_WIN32)
  unsigned short c;
  if (MultiByteToWideChar(cp, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, p, 2, &c, 1) == 1) return c;
  return -1;
#else
  UNUSED_P(cp);
  UNUSED_P(p);
  return -1;
#endif
}