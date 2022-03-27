#ifndef PRUEBA_DATAOPENCL_H
#define PRUEBA_DATAOPENCL_H

#include <jni.h>
#include "CL/cl.h"
#include "FunctionsOpenCL.h"

void *getPlatformInfo(JNIEnv *env, jobject platform, char *platformInfo);
void setVersionOpenCL(JNIEnv *env, jstring versionOpenCL);
jstring getTypeRound(JNIEnv *env, cl_device_fp_config typeRound);
jstring getCommandQueueProperties(JNIEnv *env, cl_command_queue_properties commandQueueProperties);
void *getDataDeviceSelected(JNIEnv *env, jobject device, const char *message);
void showMessageError(JNIEnv *env, const char *Message);
jobject newEvent(JNIEnv *env, jobject object, jobject commandQueue);
void **getProfilingEvent(JNIEnv *env, jobject event, cl_profiling_info profileInfo, cl_ulong *profileEvent);
cl_mem createBuffer(cl_int currentContext, void *data, cl_int sizeData);
void writeBuffer(cl_int commandQueue, cl_mem *buffer, void *data, cl_int sizeData);
void readBuffer(cl_int commandQueue, cl_mem buffer, void *data, cl_int sizeData);
void executeCommandQueue(JNIEnv *env, jobject commandQueue);
void executeKernel(JNIEnv *env, jobject commandQueue, jint _currentBlockMemory, jint _sizeBlockMemory, jint _flagsBlockMemory,
                   jint _flagsMapMemory, jint typeData);
extern cl_float VersionOpenCL;
extern cl_uint numberDevicesInCurrentPlatform;
extern cl_uint numberPlatforms;
extern cl_int sizeArrayKernelArguments;
extern cl_int numberSVMPointer;
extern jbyte *dataByte;
extern jchar *dataChar;
extern jboolean *dataBoolean;
extern jshort *dataShort;
extern jint *dataInt;
extern jfloat *dataFloat;
extern jlong *dataLong;
extern int *sizeArraysKernelArguments;
extern cl_mem dataBufferKernelArguments[1000];
extern void *dataKernelArguments[1000];
extern int *dataExtraTypeIntKernelArguments[1000];
extern cl_bool *dataExtraTypeBoolKernelArguments[1000];
extern jobject *dataExtraTypeObjectKernelArguments[1000];
extern char **scriptKernel;
extern int *typeDataKernelArguments;
extern int **sizeArraysDataKernelArguments;
#endif