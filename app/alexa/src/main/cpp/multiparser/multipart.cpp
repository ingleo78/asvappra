#include "MultipartParser.h"
#include "MultipartReader.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <android/log.h>
#define INPUT_FILE "input3.txt"
#define BOUNDARY "-----------------------------168072824752491622650073"
#define TIMES 10
#define SLURP
#define QUIET

using namespace std;
#ifdef TEST_PARSER
	static void onPartBegin(const char *buffer, size_t start, size_t end, void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onPartBegin", "");
	}
	static void onHeaderField(const char *buffer, size_t start, size_t end, void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onHeaderField: (%s)", string(buffer + start, end - start).c_str());
	}
	static void onHeaderValue(const char *buffer, size_t start, size_t end, void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onHeaderValue: (%s)", string(buffer + start, end - start).c_str());
	}
	static void onPartData(const char *buffer, size_t start, size_t end, void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onPartData: (%s)\n", string(buffer + start, end - start).c_str());
	}
	static void onPartEnd(const char *buffer, size_t start, size_t end, void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onPartEnd", "");
	}
	static void onEnd(const char *buffer, size_t start, size_t end, void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onEnd", "");
	}
#else
	void onPartBegin(const MultipartHeaders &headers, void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onPartBegin:", "");
		MultipartHeaders::const_iterator it;
		MultipartHeaders::const_iterator end = headers.end();
		for (it = headers.begin(); it != headers.end(); it++) {
		    __android_log_print(ANDROID_LOG_INFO, "  %s = %s", it->first.c_str(), it->second.c_str());
		}
		__android_log_print(ANDROID_LOG_INFO, "  aaa:", "%s", headers["aaa"].c_str());
	}
	void onPartData(const char *buffer, size_t size, void *userData) {
		//__android_log_print(ANDROID_LOG_INFO, "onPartData: (%s)", string(buffer, size).c_str());
	}
	void onPartEnd(void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onPartEnd", "");
	}
	void onEnd(void *userData) {
		__android_log_print(ANDROID_LOG_INFO, "onEnd", "");
	}
#endif
int main() {
	#ifdef TEST_PARSER
		MultipartParser parser;
		#ifndef QUIET
			parser.onPartBegin = onPartBegin;
			parser.onHeaderField = onHeaderField;
			parser.onHeaderValue = onHeaderValue;
			parser.onPartData = onPartData;
			parser.onPartEnd = onPartEnd;
			parser.onEnd = onEnd;
		#endif
	#else
		MultipartReader parser;
		#ifndef QUIET
			parser.onPartBegin = onPartBegin;
			parser.onPartData = onPartData;
			parser.onPartEnd = onPartEnd;
			parser.onEnd = onEnd;
		#endif
	#endif
	
	struct timeval stime, etime;
	struct stat sbuf;
	stat(INPUT_FILE, &sbuf);
	#ifdef SLURP
		size_t bufsize = sbuf.st_size;
		char *buf = (char*)malloc(bufsize);
		FILE *f = fopen(INPUT_FILE, "rb");
		fread(buf, 1, bufsize, f);
		fclose(f);
		gettimeofday(&stime, NULL);
		for (int i = 0; i < TIMES; i++) {
			#ifndef QUIET
				__android_log_print("------------", "");
			#endif
			parser.setBoundary(BOUNDARY);
			size_t fed = 0;
			do {
				size_t ret = parser.feed(buf + fed, bufsize - fed);
				fed += ret;
			} while (fed < bufsize && !parser.stopped());
			#ifndef QUIET
				printf("%s\n", parser.getErrorMessage());
			#endif
		}
		gettimeofday(&etime, NULL);
	#else
		size_t bufsize = 1024 * 32;
		char *buf = (char *) malloc(bufsize);
		gettimeofday(&stime, NULL);
		for (int i = 0; i < TIMES; i++) {
			#ifndef QUIET
		        __android_log_print(ANDROID_LOG_INFO, "------------", "");
			#endif
			parser.setBoundary(BOUNDARY);
			FILE *f = fopen(INPUT_FILE, "rb");
			while (!parser.stopped() && !feof(f)) {
				size_t len = fread(buf, 1, bufsize, f);
				size_t fed = 0;
				do {
					size_t ret = parser.feed(buf + fed, len - fed);
					fed += ret;
				} while (fed < len && !parser.stopped());
			}
			#ifndef QUIET
				__android_log_print(ANDROID_LOG_INFO, "Error", "%s", parser.getErrorMessage());
			#endif
			fclose(f);
		}
		gettimeofday(&etime, NULL);
	#endif
	unsigned long long a = (unsigned long long) stime.tv_sec * 1000000 + stime.tv_usec;
	unsigned long long b = (unsigned long long) etime.tv_sec * 1000000 + etime.tv_usec;
	__android_log_print(ANDROID_LOG_INFO, "(C++)", "    Total: %.2fs   Per run: %.2fs   Throughput: %.2f MB/sec", (b - a) / 1000000.0,
		(b - a) / TIMES / 1000000.0, ((unsigned long long) sbuf.st_size * TIMES) / ((b - a) / 1000000.0) / 1024.0 / 1024.0);
	return 0;
}
