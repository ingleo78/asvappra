#ifndef AMAZINGGPS_GLOBALDATA_H
#define AMAZINGGPS_GLOBALDATA_H

#include <jni.h>
#include <android/NeuralNetworks.h>

extern ANeuralNetworksOperandType operandTypeList[100];
extern ANeuralNetworksModel *modelList[100];
extern ANeuralNetworksMemory *memoryList[100];
extern ANeuralNetworksCompilation *compilationList[100];
extern ANeuralNetworksExecution *executionList[100];
extern ANeuralNetworksEvent *eventList[10000];
extern AHardwareBuffer *bufferList[100];
extern AHardwareBuffer_Planes *planesList[12];
#if __ANDROID_API__ >= 29
extern ANeuralNetworksDevice *devicesList[10000];
extern ANeuralNetworksBurst *burstList[100];
#endif
#if __ANDROID_API >= 30
extern ANeuralNetworksMemoryDesc *memoryDescriptorList[100];
#endif
extern jint currentPositionModel;
extern jint currentPositionCompilation;
extern jint currentPositionExecution;
extern jint currentPositionDevice;
extern jint currentPositionMemory;
extern jint currentPositionMemoryDescriptor;
extern jint currentPositionOperandType;
extern jint currentPositionEvent;
extern jint currentPositionBurst;
extern jint currentPositionBuffer;
extern jint currentPositionPlanes;
#endif