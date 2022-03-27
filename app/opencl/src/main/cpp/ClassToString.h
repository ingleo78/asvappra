#ifndef PRUEBA_CLASSTOSTRING_H
#define PRUEBA_CLASSTOSTRING_H

#include <jni.h>
#include <android/log.h>
#include <math.h>
#include "FunctionsOpenCL.h"

jstring platformToString(JNIEnv *env, jobject platform);
jstring deviceToString(JNIEnv *env, jobject device);
jstring deviceTimerToString(JNIEnv *env, jobject deviceTimer);
jstring contextToString(JNIEnv *env, jobject context);
jstring commandQueueToString(JNIEnv *env, jobject commandQueue);
jstring memoryToString(JNIEnv *env, jobject memory);
jstring bufferToString(JNIEnv *env, jobject buffer);
jstring eventToString(JNIEnv *env, jobject event);
jstring imageFormatToString(JNIEnv *env, jobject imageFormat);
jstring imageDescriptorToString(JNIEnv *env, jobject imageDescriptor);
jstring imageToString(JNIEnv *env, jobject image);
jstring pipeToString(JNIEnv *env, jobject pipe);
jstring sampleToString(JNIEnv *env, jobject sample);
jstring programToString(JNIEnv *env, jobject program);
jstring kernelToString(JNIEnv *env, jobject kernel);
jstring concatString(JNIEnv *env, jstring toString, jstring nameInfo, jstring value, jstring dataExtra);
jstring concatString2(JNIEnv *env, jstring toString, jstring nameInfo, jstring data, jstring value, jstring dataExtra);
jstring getNameImageChannelOrder(JNIEnv *env, jint value);
jstring getNameImageChannelDataType(JNIEnv *env, jint value);

#endif