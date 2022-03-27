#ifndef NEURALNETWORKS_H
#define NEURALNETWORKS_H

#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include <fcntl.h>
#include <android/NeuralNetworks.h>
#include <android/rect.h>
#include <android/hardware_buffer.h>
#include <android/hardware_buffer_jni.h>
#include "globalData.h"
#include "functionsData.h"
#include "ShowMessageError.h"

extern "C" {
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Model_newModel(JNIEnv *env, jclass Model);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_neuralnetworks_Model_getDefaultLoopTimeout(JNIEnv *env, jclass Model);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_neuralnetworks_Model_getMaximumLoopTimeout(JNIEnv *env, jclass Model);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_finish(JNIEnv *env, jobject model);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_delete(JNIEnv *env, jobject model);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_addOperand(JNIEnv *env, jobject model, jobject operandType);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_addOperation(JNIEnv *env, jobject model, jint typeOperation, jintArray inputs, jintArray outputs);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_getSupportedOperationsForDevices(JNIEnv *env, jobject model, jobjectArray devices, jbooleanArray isSupportedOps);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_identifyInputsAndOutputs(JNIEnv *env, jobject model, jintArray inputs, jintArray outputs);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_relaxComputationFloat32toFloat16(JNIEnv *env, jobject model, jboolean allow);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_setOperandSymmPerChannelQuantParams(JNIEnv *env, jobject model, jobject symmPerChannelQuantParams,
                                                                 jint indexOperand);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_setOperandValue(JNIEnv *env, jobject model, jint indexOperand, jobject value);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_setOperandValueFromMemory(JNIEnv *env, jobject model, jobject memory, jint indexOperand, jint offset, jint size);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Model_setOperandValueFromModel(JNIEnv *env, jobject model, jobject modelSrc, jint indexOperand);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_newNeuralNetworkCompilation(JNIEnv *env, jclass Compilation, jobject model);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_newNeuralNetworkCompilationForDevices(JNIEnv *env, jclass Compilation, jobject model, jobjectArray devices);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_finish(JNIEnv *env, jobject compilation);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_delete(JNIEnv *env, jobject compilation);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_setCaching(JNIEnv *env, jobject compilation, jobject cacheDir, jbyteArray token);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_setPreference(JNIEnv *env, jobject compilation, jint preference);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_setPriority(JNIEnv *env, jobject compilation, jint priority);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Compilation_setTimeout(JNIEnv *env, jobject compilation, jlong duration);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_newNeuralNetworkExecution(JNIEnv *env, jclass Execution, jobject compilation);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_burstCompute(JNIEnv *env, jobject execution, jobject burst);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_compute(JNIEnv *env, jobject execution);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_delete(JNIEnv *env, jobject execution);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_getDuration(JNIEnv *env, jobject execution, jint durationType);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_getOutputOperandDimensions(JNIEnv *env, jobject execution, jint indexOutput);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_getOutputOperandRank(JNIEnv *env, jobject execution, jint indexOutput);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_setInput(JNIEnv *env, jobject execution, jobject operandType, jobject data, jint indexInput);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_setInputFromMemory(JNIEnv *env, jobject execution, jobject operandType, jobject memory,
                                                    jint indexInput, jint offset, jint size);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_setLoopTimeout(JNIEnv *env, jobject execution, jlong duration);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_setMeasureTiming(JNIEnv *env,jobject execution, jboolean isMeasure);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_setOutput(JNIEnv *env, jobject execution, jobject operandType, jobject data, jint indexOutput);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_setOutputFromMemory(JNIEnv *env, jobject execution, jobject operandType, jobject memory,
                                                     jint indexOutput, jint offset, jint size);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_setTimeout(JNIEnv *env, jobject execution, jlong duration);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_startCompute(JNIEnv *env, jobject execution, jobject event);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Execution_startComputeWithDependencies(JNIEnv *env, jobject execution, jobjectArray events, jlong duration);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Burst_newNeuralNetworkBurst(JNIEnv *env, jclass Burst, jobject compilation);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Burst_delete(JNIEnv *env, jobject burst);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Memory_newMemory(JNIEnv *env, jclass Memory, jobject memoryDescriptor);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Memory_newMemoryFromFileDescriptor(JNIEnv *env, jclass Memory, jint fileDescriptor, jint size, jint offset, jint mode);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Memory_newMemoryFromBuffer(JNIEnv *env, jclass Memory, jobject buffer);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Memory_copy(JNIEnv *env, jobject memory, jobject memorySrc);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Memory_delete(JNIEnv *env, jobject memory);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_newMemoryDescriptor(JNIEnv *env, jclass MemoryDescriptor);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_finish(JNIEnv *env, jobject memoryDescriptor);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_delete(JNIEnv *env, jobject memoryDescriptor);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_addInputRole(JNIEnv *env, jobject memoryDescriptor, jobject compilation, jint indexInput,
                                                     jfloat frequency);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_addOutputRole(JNIEnv *env, jobject memoryDescriptor, jobject compilation, jint indexOutput,
                                                      jfloat frequency);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_copy(JNIEnv *env, jobject memoryDescriptor,jobject memoryDescriptorSrc);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_setDimensions(JNIEnv *env, jobject memoryDescriptor, jintArray dimensions);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_Event_newNeuralNetworkEvent(JNIEnv *env, jclass event, jint sync_fence_fd);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_neuralnetworks_Event_getSyncFenceFD(JNIEnv *env, jobject event);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Event_waitExecutionComplete(JNIEnv *env, jobject event);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Event_delete(JNIEnv *env, jobject event);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_OperandType_newNeuralNetworkOperandType(JNIEnv *env, jclass  OperandType, jintArray valueDimensions, jfloat scale, jint operandType,
                                                                  jint zeroPoint);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_OperandType_delete(JNIEnv *env, jobject operandType);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_neuralnetworks_Device_getDevices(JNIEnv *env, jclass device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_neuralnetworks_Device_getType(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_neuralnetworks_Device_getName(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_neuralnetworks_Device_getVersion(JNIEnv *env, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_neuralnetworks_Device_getNeuralNetworksAPIVersion(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_neuralnetworks_Device_isLiveState(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_neuralnetworks_Device_devicesAvailable(JNIEnv *env, jclass Device);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_Device_delete(JNIEnv *env, jobject device);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_getBuffer(JNIEnv *env, jclass Buffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_allocate(JNIEnv *env, jclass Buffer, jobject bufferDescription);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_fromHardwareBuffer(JNIEnv *env, jclass Buffer, jobject hardwareBuffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_getDescription(JNIEnv *env, jobject buffer);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_isSupported(JNIEnv *env, jclass Buffer, jobject bufferDescription);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_delete(JNIEnv *env, jobject buffer);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_lock(JNIEnv *env, jobject buffer, jobject memoryVirtual, jobject bufferArea, jlong usageType, jint fence);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_lockAndGetInfo(JNIEnv *env, jobject buffer, jobject memoryVirtual, jobject bufferArea, jint usageType, jint fence,
                                             jint bytesPerPixel, jint bytesPerStride);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_getHardwareBuffer(JNIEnv *env, jobject buffer);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_lockPlanes(JNIEnv *env, jobject buffer, jobject bufferArea, jlong usageType, jint fence);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_sendHandleToUnixSocket(JNIEnv *env, jobject buffer, jint sockedFd);
JNIEXPORT void JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_unlock(JNIEnv *env, jobject buffer, jint fence);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Plane_getNumberBytesPerPixel(JNIEnv *env, jobject plane);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Plane_getNumberBytesPerRow(JNIEnv *env, jobject plane);
JNIEXPORT jbyteArray JNICALL Java_com_draico_asvappra_neuralnetworks_hardware_Plane_getPixels(JNIEnv *env, jobject plane);
};

#endif