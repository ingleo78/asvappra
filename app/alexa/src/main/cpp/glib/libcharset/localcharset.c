#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "localcharset.h"

#if defined _WIN32 || defined __WIN32__
# define WIN32_NATIVE
#endif
#if defined __EMX__
# define OS2
#endif
#if !defined WIN32_NATIVE
# if HAVE_LANGINFO_CODESET
#  include <langinfo.h>
# else
#  if 0
#   include <locale.h>
#  endif
# endif
# ifdef __CYGWIN__
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif
#elif defined WIN32_NATIVE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif
#if defined OS2
# define INCL_DOS
# include <os2.h>
#endif
#if ENABLE_RELOCATABLE
# include "relocatable.h"
#else
# define relocate(pathname) (pathname)
#endif
#ifdef LIBDIR
# include "configmake.h"
#endif
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __EMX__ || defined __DJGPP__
# define ISSLASH(C) ((C) == '/' || (C) == '\\')
#endif
#ifndef DIRECTORY_SEPARATOR
# define DIRECTORY_SEPARATOR '/'
#endif
#ifndef ISSLASH
# define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
#endif
#if HAVE_DECL_GETC_UNLOCKED
# undef getc
# define getc getc_unlocked
#endif
#if __STDC__ != 1
# define volatile
#endif
static const char * volatile charset_aliases;
const char *_g_locale_get_charset_aliases(void) {
  const char *cp;
  cp = charset_aliases;
  if (cp == NULL) {
  #if !(defined VMS || !defined WIN32_NATIVE || defined __CYGWIN__)
      FILE *fp;
      const char *dir;
      const char *base = "charset.alias";
      char *file_name;
      dir = getenv("CHARSETALIASDIR");
      if (dir == NULL || dir[0] == '\0') dir = relocate(LIBDIR);
      {
          size_t dir_len = strlen(dir);
          size_t base_len = strlen(base);
          int add_slash = (dir_len > 0 && !ISSLASH(dir[dir_len - 1]));
          file_name = (char*)malloc(dir_len + add_slash + base_len + 1);
          if (file_name != NULL) {
              memcpy(file_name, dir, dir_len);
              if (add_slash) file_name[dir_len] = DIRECTORY_SEPARATOR;
              memcpy(file_name + dir_len + add_slash, base, base_len + 1);
          }
      }
      if (file_name == NULL || (fp = fopen(file_name, "r")) == NULL) cp = "";
      else {
          char *res_ptr = NULL;
          size_t res_size = 0;
          for (;;) {
              int c;
              char buf1[50+1];
              char buf2[50+1];
              size_t l1, l2;
              char *old_res_ptr;
              c = getc(fp);
              if (c == EOF) break;
              if (c == '\n' || c == ' ' || c == '\t') continue;
              if (c == '#') {
                  do {
                      c = getc(fp);
                  } while(!(c == EOF || c == '\n'));
                  if (c == EOF) break;
                  continue;
              }
              ungetc(c, fp);
              if (fscanf(fp, "%50s %50s", buf1, buf2) < 2) break;
              l1 = strlen(buf1);
              l2 = strlen(buf2);
              old_res_ptr = res_ptr;
              if (res_size == 0) {
                  res_size = l1 + 1 + l2 + 1;
                  res_ptr = (char*)malloc(res_size + 1);
              } else {
                  res_size += l1 + 1 + l2 + 1;
                  res_ptr = (char*)realloc(res_ptr, res_size + 1);
              }
              if (res_ptr == NULL) {
                  res_size = 0;
                  if (old_res_ptr != NULL) free (old_res_ptr);
                  break;
              }
              strcpy(res_ptr + res_size - (l2 + 1) - (l1 + 1), buf1);
              strcpy(res_ptr + res_size - (l2 + 1), buf2);
          }
          fclose(fp);
          if (res_size == 0) cp = "";
          else {
              *(res_ptr + res_size) = '\0';
              cp = res_ptr;
          }
      }
      if (file_name != NULL) free(file_name);
  #else
  #if defined VMS
      cp = "ISO8859-1\0ISO-8859-1\0ISO8859-2\0ISO-8859-2\0ISO8859-5\0ISO-8859-5\0ISO8859-7\0ISO-8859-7\0ISO8859-8\0ISO-8859-8\0ISO8859-9\0ISO-8859-9\0eucJP\0"
           "EUC-JP\0""SJIS\0SHIFT_JIS\0DECKANJI\0DEC-KANJI\0SDECKANJI\0EUC-JP\0eucTW\0EUC-TW\0DECHANYU\0DEC-HANYU\0DECHANZI\0GB2312\0DECKOREAN\0EUC-KR\0";
  #endif
  #if defined WIN32_NATIVE || defined __CYGWIN__
      cp = "CP936\0GBK\0CP1361\0JOHAB\0CP20127\0ASCII\0CP20866\0KOI8-R\0CP20936\0GB2312\0CP21866\0KOI8-RU\0CP28591\0ISO-8859-1\0CP28592\0ISO-8859-2\0"
	       "CP28593\0ISO-8859-3\0CP28594\0ISO-8859-4\0CP28595\0ISO-8859-5\0CP28596\0ISO-8859-6\0CP28597\0ISO-8859-7\0CP28598\0ISO-8859-8\0CP28599\0ISO-8859-9\0"
           "CP28605\0ISO-8859-15\0CP38598\0ISO-8859-8\0CP51932\0EUC-JP\0CP51936\0GB2312\0CP51949\0EUC-KR\0CP51950\0EUC-TW\0CP54936\0GB18030\0CP65001\0UTF-8\0";
  #endif
  #endif
      charset_aliases = cp;
  }
  return cp;
}
const char *_g_locale_charset_raw(void) {
  const char *codeset;
#if !(defined WIN32_NATIVE || defined OS2)
#if HAVE_LANGINFO_CODESET
  codeset = nl_langinfo(CODESET);
#ifdef __CYGWIN__
  if (codeset != NULL && strcmp (codeset, "US-ASCII") == 0) {
      const char *locale;
      static char buf[2 + 10 + 1];
      locale = getenv("LC_ALL");
      if (locale == NULL || locale[0] == '\0') {
          locale = getenv("LC_CTYPE");
          if (locale == NULL || locale[0] == '\0') locale = getenv("LANG");
	  }
      if (locale != NULL && locale[0] != '\0') {
          const char *dot = strchr(locale, '.');
          if (dot != NULL) {
              const char *modifier;
              dot++;
              modifier = strchr(dot, '@');
              if (modifier == NULL) return dot;
              if (modifier - dot < sizeof(buf)) {
                  memcpy(buf, dot, modifier - dot);
                  buf[modifier - dot] = '\0';
                  return buf;
              }
          }
	  }
      sprintf(buf, "CP%u", GetACP());
      codeset = buf;
  }
#endif
#else
  const char *locale = NULL;
#if 0
  locale = setlocale(LC_CTYPE, NULL);
#endif
  if (locale == NULL || locale[0] == '\0') {
      locale = getenv("LC_ALL");
      if (locale == NULL || locale[0] == '\0') {
          locale = getenv("LC_CTYPE");
          if (locale == NULL || locale[0] == '\0') locale = getenv("LANG");
	  }
  }
  codeset = locale;
#endif
#elif defined WIN32_NATIVE
  static char buf[2 + 10 + 1];
  sprintf(buf, "CP%u", GetACP ());
  codeset = buf;
#elif defined OS2
  const char *locale;
  static char buf[2 + 10 + 1];
  ULONG cp[3];
  ULONG cplen;
  locale = getenv("LC_ALL");
  if (locale == NULL || locale[0] == '\0') {
      locale = getenv ("LC_CTYPE");
      if (locale == NULL || locale[0] == '\0') locale = getenv("LANG");
  }
  if (locale != NULL && locale[0] != '\0') {
      const char *dot = strchr(locale, '.');
      if (dot != NULL) {
          const char *modifier;
          dot++;
          modifier = strchr (dot, '@');
          if (modifier == NULL) return dot;
          if (modifier - dot < sizeof(buf)) {
              memcpy (buf, dot, modifier - dot);
              buf[modifier - dot] = '\0';
              return buf;
          }
	  }
      codeset = locale;
  } else {
      if (DosQueryCp(sizeof(cp), cp, &cplen)) codeset = "";
      else {
          sprintf(buf, "CP%u", cp[0]);
          codeset = buf;
	  }
  }
#endif
  return codeset;
}
const char *_g_locale_charset_unalias(const char *codeset) {
  const char *aliases;
  if (codeset == NULL) codeset = "";
  for (aliases = _g_locale_get_charset_aliases(); *aliases != '\0'; aliases += strlen(aliases) + 1, aliases += strlen(aliases) + 1)
      if (strcmp(codeset, aliases) == 0 || (aliases[0] == '*' && aliases[1] == '\0')) {
          codeset = aliases + strlen(aliases) + 1;
          break;
      }
  if (codeset[0] == '\0') codeset = "ASCII";
  return codeset;
}
