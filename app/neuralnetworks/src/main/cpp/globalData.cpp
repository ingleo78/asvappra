#include "globalData.h"

ANeuralNetworksModel *modelList[100];
ANeuralNetworksMemory *memoryList[100];
ANeuralNetworksCompilation *compilationList[100];
ANeuralNetworksExecution *executionList[100];
ANeuralNetworksEvent *eventList[10000];
AHardwareBuffer *bufferList[100];
AHardwareBuffer_Planes *planesList[12];
ANeuralNetworksOperandType operandTypeList[100];
#if __ANDROID_API__ >= 29
ANeuralNetworksDevice *devicesList[10000];
ANeuralNetworksBurst *burstList[100];
#endif
#if __ANDROID__API >= 30
ANeuralNetworksMemoryDesc *memoryDescriptorList[100];
#endif
jint currentPositionModel = 0;
jint currentPositionCompilation = 0;
jint currentPositionDevice = 0;
jint currentPositionExecution = 0;
jint currentPositionMemory = 0;
jint currentPositionMemoryDescriptor = 0;
jint currentPositionOperandType = 0;
jint currentPositionEvent = 0;
jint currentPositionBurst = 0;
jint currentPositionBuffer = 0;
jint currentPositionPlanes = 0;