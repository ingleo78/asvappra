#ifndef AMAZINGGPS_FUNCTIONSDATA_H
#define AMAZINGGPS_FUNCTIONSDATA_H

#include <android/log.h>
#include <android/NeuralNetworks.h>
#include <jni.h>
#include <cstring>
#include <malloc.h>
#include "globalData.h"

void **createNeuralNetworkModel();
void **finishNeuralNetworkModel(jint currentModel);
void deleteNeuralNetworkModel(jint currentModel);
void **addOperandModel(jint currentModel, jint currentOperandType);
void **addOperationModel(jint currentMOdel, jint operationType, jsize sizeInputs, jint *inputs, jsize sizeOutputs, jint *outputs);
void **identifyInputsAndOutputs(jint currentModel, jint sizeInputs, jint sizeOutputs, jint *dataInputs, jint *dataOutputs);
void **relaxComputationFloat32toFloat16(jint currentModel, jboolean allow);
void **setOperandValue(jint currentModel, jint indexOperand, const void *data, jint sizeData);
void **setOperandValueFromMemory(jint currentModel, jint _currentMemory, jint index, jint offset, jint size);
void **createNeuralNetworkCompilation(jint currentModel);
void **finishCompilation(jint currentCompilation);
void deleteCompilation(jint currentCompilation);
void **setPreferenceCompilation(jint currentCompilation, jint preference);
void **createExecution(jint currentCompilation);
void **setInput(jint currentExecution, jint currentOperandType, void *data, jint indexInput, jint sizeData);
void **setInputFromMemory(jint currentExecution, jint currentOperandType, jint currentMemory, jint index, jint offset, jint size);
void **setOutput(jint currentExecution, jint currentOperandType, jint indexOutput, void *data, jsize sizeData);
void **setOutputFromMemory(jint currentExecution, jint currentOperandType, jint currentMemory, jint indexOutput, jint offset, jint size);
void **startCompute(jint currentExecution, jint currentEvent);
void **newMemoryFromFileDescriptor(jint size, jint mode, jint fileDescriptor, jint offset);
void deleteMemory(jint currentMemory);
void **waitExecutionComplete(jint currentEvent);
void deleteEvent(jint currentEvent);
#if __ANDROID_API__ >= 29
void **setOperandSymmPerChannelQuantParams(jint currentModel, jint indexOperand, ANeuralNetworksSymmPerChannelQuantParams *symmPerchannelQuantParams);
void **createForDevices(jint currentModel, jint *positionDevices, jsize numberDevices);
void **setCaching(jint currentCompilation, const char *cacheDir, jbyte *token);
void **burstCompute(jint currentExecution, jint currentBurst);
void **compute(jint currentExecution);
void **getDuration(jint currentExecution, jint durationCode, jlong *duration);
void **getOutputOperandDimensions(jint currentExecution, jint index, jint *dimensions);
void **getOutputOperandRank(jint currentExecution, jint index, jint *rank);
void **setMeasureTiming(jint currentExecution, jboolean isMeasure);
void **createBurst(jint currentCompilation);
void deleteBurst(jint currentBurst);
void **newMemoryFromBuffer(jint currentBuffer);
void **devicesAvailable(jint *devices);
void **getDevice(jint indexDevice);
void **getDeviceType(jint currentDevice, jint *deviceType);
void **getDeviceName(jint currentDevice, char **name);
void **getDeviceVersion(jint currentDevice, char **name);
void **getNeuralNetworksAPIVersion(jint currentDevice, jlong *apiVersion);
#endif
#if __ANDROID_API__ >= 30
void **setOperandValueFromModel(jint currentModel, jint currentModelSrc, jint indexOperand);
void **setPriority(jint currentCompilation, jint priority);
void **setTimeOut(jint currentCompilation, jlong duration);
void **setLoopTimeout(jint currentExecution, jlong duration);
void **setTimeout(jint currentExecution, jlong duration);
void **startComputeWithDependencies(jint currentExecution, jint *eventsList, jsize sizeList, jlong duration);
void **newMemory(jint currentMemoryDescriptor);
void **copyMemory(jint currentMemory, jint currentMemorySrc);
void **createMemoryDescriptor();
void **finishMemoryDescriptor(jint currentMemoryDescriptor);
void deleteMemoryDescriptor(jint currentMemoryDescriptor);
void **addInputRoleMemoryDescriptor(jint currentMemoryDescriptor, jint currentCompilation, jint indexInput, jfloat frequency);
void **addOutputRoleMemoryDescriptor(jint currentMemoryDescriptor, jint currentCompilation, jint indexOutput, jfloat frequency);
void **copyMemoryDescriptor(jint currentMemoryDescriptor, jint currentMemoryDescriptorSrc);
void **setDimensionsMemoryDescriptor(jint currentMemoryDescriptor, jint *dataDimensions, jsize sizeDimensions);
void **createEventFromSyncFenceFd(jint sync_fence_fd);
void **getSyncFenceFd(jint currentEvent, jint *sync_fence_fd);
void **isLiveStateDevice(jint currentDevice);
#endif
void findErrorType(jint typeError, char *message);

#endif