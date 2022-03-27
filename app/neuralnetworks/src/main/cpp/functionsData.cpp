#include "functionsData.h"

void *Result[2];
char *message;
int result;

void **createNeuralNetworkModel() {
    result = ANeuralNetworksModel_create(&modelList[currentPositionModel]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "Can't be create NeuralNetworkModel object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **finishNeuralNetworkModel(jint currentModel) {
    result = ANeuralNetworksModel_finish(modelList[currentModel]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the finishNeuralNetworkModel method because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void deleteNeuralNetworkModel(jint currentModel) {
    ANeuralNetworksModel_free(modelList[currentModel]);
}
void **addOperandModel(jint currentModel, jint currentOperandType) {
    result = ANeuralNetworksModel_addOperand(modelList[currentModel], &operandTypeList[currentOperandType]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the addOperand method of the Model object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **addOperationModel(jint currentModel, jint operationType, jsize sizeInputs, jint *inputs, jsize sizeOutputs, jint *outputs) {
    result = ANeuralNetworksModel_addOperation(modelList[currentModel], (uint32_t)operationType, (uint32_t)sizeInputs, (const uint32_t*)inputs,
                                               (uint32_t)sizeOutputs, (const uint32_t*)outputs);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        char *message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the addOperation method of the Model object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **identifyInputsAndOutputs(jint currentModel, jint sizeInputs, jint sizeOutputs, jint *dataInputs, jint *dataOutputs) {
    ANeuralNetworksModel *model = modelList[currentModel];
    result = ANeuralNetworksModel_identifyInputsAndOutputs(model, (uint32_t)sizeInputs, (uint32_t*)dataInputs, (uint32_t)sizeOutputs,
                                                           (uint32_t*)dataOutputs);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        char *message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the identifyInputsAndOutputs method of the Model object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **relaxComputationFloat32toFloat16(jint currentModel, jboolean allow) {
    #if __ANDROID_API__ >= 28
        result = ANeuralNetworksModel_relaxComputationFloat32toFloat16(modelList[currentModel], allow);
        if (result != ANEURALNETWORKS_NO_ERROR) {
            message = (char*)malloc(sizeof(char) * 150);
            strcpy(message, "You cannot use the relaxComputationFloat32toFloat16 method of the Model object you are using because ");
            findErrorType(result, message);
        } else message = NULL;
    #else
        message = (char*)malloc(sizeof(char) * 189);
        strcpy(message, "You cannot use the relaxComputationFloat32toFloat16 method of the Model object you are using because the android version ");
        strcat(message, "that your android device has installed is lower than the pie version");
        result = -1;
    #endif
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setOperandValue(jint currentModel, jint indexOperand, const void *data, jint sizeData) {
    ANeuralNetworksModel *model = modelList[currentModel];
    result = ANeuralNetworksModel_setOperandValue(model, (int32_t)indexOperand, data, (int32_t)sizeData);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char *) malloc(sizeof(char) * 150);
        strcpy(message,
               "You cannot use the setOperandValue method of the Model object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setOperandValueFromMemory(jint currentModel, jint currentMemory, jint index, jint offset, jint size) {
    ANeuralNetworksModel *model = modelList[currentModel];
    ANeuralNetworksMemory *memory = memoryList[currentMemory];
    result = ANeuralNetworksModel_setOperandValueFromMemory(model, (int32_t)index, memory, (size_t)offset, (size_t)size);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **createNeuralNetworkCompilation(jint currentModel) {
    ANeuralNetworksModel *model = modelList[currentModel];
    result = ANeuralNetworksCompilation_create(model, &compilationList[currentPositionCompilation]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot create a Compilation object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **finishCompilation(jint currentCompilation) {
    result = ANeuralNetworksCompilation_finish(compilationList[currentCompilation]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the finish method of the Compilation object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void deleteCompilation(jint currentCompilation) {
    ANeuralNetworksCompilation_free(compilationList[currentCompilation]);
}
void **setPreferenceCompilation(jint currentCompilation, jint preference) {
    ANeuralNetworksCompilation *compilation = compilationList[currentCompilation];
    result = ANeuralNetworksCompilation_setPreference(compilation, (int32_t)preference);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setPreference method of the Compilation object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **createExecution(jint currentCompilation) {
    ANeuralNetworksCompilation *compilation = compilationList[currentCompilation];
    result = ANeuralNetworksExecution_create(compilation, &executionList[currentPositionExecution]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the newExecution method because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setInput(jint currentExecution, jint currentOperandType, void *data, jint indexInput, jint sizeData) {
    ANeuralNetworksExecution *execution = executionList[currentExecution];
    ANeuralNetworksOperandType *operandType = &operandTypeList[currentOperandType];
    result = ANeuralNetworksExecution_setInput(execution, (int32_t)indexInput, operandType, data, (size_t)sizeData);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setInput method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setInputFromMemory(jint currentExecution, jint currentOperandType, jint currentMemory, jint index, jint offset, jint size) {
    ANeuralNetworksExecution *execution = executionList[currentExecution];
    ANeuralNetworksOperandType *operandType = &operandTypeList[currentOperandType];
    ANeuralNetworksMemory *memory = memoryList[currentMemory];
    result = ANeuralNetworksExecution_setInputFromMemory(execution, (int32_t)index, operandType, memory, (size_t)offset, (size_t)size);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setInputFromMemory method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setOutput(jint currentExecution, jint currentOperandType, jint indexOutput, void *data, jsize sizeData) {
    ANeuralNetworksExecution *execution = executionList[currentExecution];
    ANeuralNetworksOperandType *operandType = &operandTypeList[currentOperandType];
    result = ANeuralNetworksExecution_setOutput(execution, (int32_t)indexOutput, operandType, data, (size_t)sizeData);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the setOutput method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setOutputFromMemory(jint currentExecution, jint currentOperandType, jint currentMemory, jint indexOutput, jint offset, jint size) {
    ANeuralNetworksExecution *execution = executionList[currentExecution];
    ANeuralNetworksOperandType *operandType = &operandTypeList[currentOperandType];
    ANeuralNetworksMemory *memory = memoryList[currentMemory];
    result = ANeuralNetworksExecution_setOutputFromMemory(execution, (int32_t)indexOutput, operandType, memory, (size_t)offset, (size_t)size);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the setOutputFromMemory method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **startCompute(jint currentExecution, jint currentEvent) {
    result = ANeuralNetworksExecution_startCompute(executionList[currentExecution], &eventList[currentEvent]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the startCompute method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **newMemoryFromFileDescriptor(jint size, jint mode, jint fileDescriptor, jint offset) {
    result = ANeuralNetworksMemory_createFromFd((size_t)size, mode, fileDescriptor, (size_t)offset, &memoryList[currentPositionMemory]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the newMemoryFromFileDescriptor method to create a Memory object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void deleteMemory(jint currentMemory) {
    ANeuralNetworksMemory_free(memoryList[currentMemory]);
}
void **waitExecutionComplete(jint currentEvent) {
    result = ANeuralNetworksEvent_wait(eventList[currentEvent]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the waitExecutionComplete method of the Event object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void deleteEvent(jint currentEvent) {
    ANeuralNetworksEvent_free(eventList[currentEvent]);
}
#if __ANDROID_API__ >= 29
void **setOperandSymmPerChannelQuantParams(jint currentModel, jint indexOperand, ANeuralNetworksSymmPerChannelQuantParams *params) {
    ANeuralNetworksModel *model = modelList[currentModel];
    result = ANeuralNetworksModel_setOperandSymmPerChannelQuantParams(model, (int32_t)indexOperand, params);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method of the Model object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **createForDevices(jint currentModel, jint *positionDevices, jsize numberDevices) {
    ANeuralNetworksModel *model = modelList[currentModel];
    ANeuralNetworksDevice devices[numberDevices];
    for (jint position = 0; position < numberDevices; position++) {
        devices[position] = devicesList[positionDevices[position]];
    }
    result = ANeuralNetworksCompilation_createForDevices(model, devices, (uint32_t)numberDevices, &devicesList[currentPositionCompilation]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You can't use createForDevices method because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setCaching(jint currentCompilation, const char *cacheDir, jbyte *token) {
    ANeuralNetworksCompilation *compilation = compilationList[currentCompilation];
    result = ANeuralNetworksCompilation_setCaching(compilation, cacheDir, (const uint8_t*)token);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **burstCompute(jint currentExecution, jint currentBurst) {
    ANeuralNetworksExecution *execution = executionList[currentExecution];
    ANeuralNetworksBurst *burst = burstList[currentBurst];
    result = ANeuralNetworksExecution_burstCompute(execution, burst);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the burstCompute method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **compute(jint currentExecution) {
    result = ANeuralNetworksExecution_compute(executionList[currentExecution]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the compute method of the execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getDuration(jint currentExecution, jint durationCode, jlong *duration) {
    ANeuralNetworksExecution *execution = executionList[currentExecution];
    result = ANeuralNetworksExecution_getDuration(execution, (int32_t)durationCode, (uint64_t*)duration);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getDuration method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getOutputOperandDimensions(jint currentExecution, jint index, jint *dimensions) {
    result = ANeuralNetworksExecution_getOutputOperandDimensions(executionList[currentExecution], (int32_t)index, (uint32_t*)dimensions);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getOutputOperandDimensions method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getOutputOperandRank(jint currentExecution, jint index, jint *rank) {
    result = ANeuralNetworksExecution_getOutputOperandRank(executionList[currentExecution], (int32_t)index, (uint32_t*)rank);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getOutputOperandRank method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setMeasureTiming(jint currentExecution, jboolean isMeasure) {
    result = ANeuralNetworksExecution_setMeasureTiming(executionList[currentExecution], isMeasure);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setMeasureTiming method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **createBurst(jint currentCompilation) {
    result = ANeuralNetworksBurst_create(compilationList[currentCompilation], &burstList[currentPositionBurst]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the newBurst method to create a Burst object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void deleteBurst(jint currentBurst) {
    ANeuralNetworksBurst_free(burstList[currentBurst]);
}
void **newMemoryFromBuffer(jint currentBuffer) {
    AHardwareBuffer *buffer = bufferList[currentBuffer];
    result = ANeuralNetworksMemory_createFromAHardwareBuffer(buffer, &memoryList[currentPositionMemory]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the newMemoryFromBuffer method to create a Memory object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **devicesAvailable(jint *devices) {
    result = ANeuralNetworks_getDeviceCount((uint32_t*)devices);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the devicesAvaible method because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getDevice(jint indexDevice) {
    result = ANeuralNetworks_getDevice((uint32_t)indexDevice, &devicesList[currentPositionDevice]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getDevices method because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getDeviceType(jint currentDevice, jint *deviceType) {
    ANeuralNetworksDevice *device = devicesList[currentDevice];
    result = ANeuralNetworksDevice_getType(device, deviceType);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getType method of the Device object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getDeviceName(jint currentDevice, char **name) {
    ANeuralNetworksDevice *device = devicesList[currentDevice];
    result = ANeuralNetworksDevice_getName(device, (const char**)name);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getName method of the Device object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getDeviceVersion(jint currentDevice, char **name) {
    ANeuralNetworksDevice *device = devicesList[currentDevice];
    result = ANeuralNetworksDevice_getVersion(device, (const char**)name);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getVersion method of the Device object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getNeuralNetworksAPIVersion(jint currentDevice, jlong *apiVersion) {
    ANeuralNetworksDevice *device = devicesList[currentDevice];
    result = ANeuralNetworksDevice_getFeatureLevel(device, (int64_t*)apiVersion);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getNeuralNetworksAPIVersion method of the Device object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
#endif
#if __ANDROID_API__ >= 30
void **setOperandValueFromModel(jint currentModel, jint currentModelSrc, jint indexOperand) {
    ANeuralNetworksModel *model = modelList[currentModel];
    ANeuralNetworksModel *modelSrc = modelList[currentModelSrc];
    result = ANeuralNetworksModel_setOperandValueFromModel(model, (int32_t)indexOperand, modelSrc);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setOperandValueFromModel method because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setPriority(jint currentCompilation, jint priority) {
    result = ANeuralNetworksCompilation_setPriority(compilationList[currentCompilation], priority);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the serPriority method of the Compilation object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setTimeOut(jint currentCompilation, jlong duration) {
    result = ANeuralNetworksCompilation_setTimeout(compilationList[currentCompilation], (uint64_t)duration);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setTimeout method of the Compilation object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setLoopTimeout(jint currentExecution, jlong duration) {
    result = ANeuralNetworksExecution_setLoopTimeout(executionList[currentExecution], (uint64_t)duration);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setLoopTimeout method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setTimeout(jint currentExecution, jlong duration) {
    result = ANeuralNetworksExecution_setTimeout(executionList[currentExecution], (uint64_t)duration);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setTimeout method of the Execution object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **startComputeWithDependencies(jint currentExecution, jint *eventsList, jsize sizeList, jlong duration) {
    ANeuralNetworksExecution *execution = executionList[currentExecution];
    ANeuralNetworksEvent *_eventsList[sizeList];
    for (jint position = 0; position < sizeList; position++) {
        _eventsList[position] = eventList[eventsList[position]];
    }
    result = ANeuralNetworksExecution_startComputeWithDependencies(execution, _eventsList, (uint32_t)sizeList, (uint64_t)duration, &eventList[currentPositionEvent]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **newMemory(jint currentMemoryDescriptor) {
    ANeuralNetworksMemoryDesc *memoryDescriptor = memoryDescriptorList[currentMemoryDescriptor];
    result = ANeuralNetworksMemory_createFromDesc(memoryDescriptor, &memoryList[currentPositionMemory]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the newMemory method to create a Memory object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **copyMemory(jint currentMemory, jint currentMemorySrc) {
    ANeuralNetworksMemory *memory = memoryList[currentMemory];
    ANeuralNetworksMemory *memorySrc = memoryList[currentMemorySrc];
    result = ANeuralNetworksMemory_copy(memorySrc, memory);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the copy method of the Memory object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **createMemoryDescriptor() {
    result = ANeuralNetworksMemoryDesc_create(&memoryDescriptorList[currentPositionMemoryDescriptor]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the newMemoryDescriptor method to create a MemoryDescriptor object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **finishMemoryDescriptor(jint currentMemoryDescriptor) {
    result = ANeuralNetworksMemoryDesc_finish(memoryDescriptorList[currentMemoryDescriptor]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the finish method of the MemoryDescriptor object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void deleteMemoryDescriptor(jint currentMemoryDescriptor) {
    ANeuralNetworksMemoryDesc_free(memoryDescriptorList[currentMemoryDescriptor]);
}
void **addInputRoleMemoryDescriptor(jint currentMemoryDescriptor, jint currentCompilation, jint indexInput, jfloat frequency) {
    ANeuralNetworksMemoryDesc *memoryDescriptor = memoryDescriptorList[currentMemoryDescriptor];
    ANeuralNetworksCompilation *compilation = compilationList[currentCompilation];
    result = ANeuralNetworksMemoryDesc_addInputRole(memoryDescriptor, compilation, (uint32_t)indexInput, frequency);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the addInputRole method of the MemoryDescriptor object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **addOutputRoleMemoryDescriptor(jint currentMemoryDescriptor, jint currentCompilation, jint indexOutput, jfloat frequency) {
    ANeuralNetworksMemoryDesc *memoryDescriptor = memoryDescriptorList[currentMemoryDescriptor];
    ANeuralNetworksCompilation *compilation = compilationList[currentCompilation];
    result = ANeuralNetworksMemoryDesc_addOutputRole(memoryDescriptor, compilation, (uint32_t)indexOutput, frequency);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the addOutputRoleMemoryDescriptor method of the MemoryDescriptor object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **copyMemoryDescriptor(jint currentMemoryDescriptor, jint currentMemoryDescriptorSrc) {
    ANeuralNetworksMemoryDesc *memoryDescriptor = memoryDescriptorList[currentMemoryDescriptor];
    ANeuralNetworksMemoryDesc *memoryDescriptorSrc = memoryDescriptorList[currentMemoryDescriptorSrc];
    result = ANeuralNetworksMemoryDesc_copy(memoryDescriptorSrc, memoryDescriptor);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the copy method of the MemoryDescriptor object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **setDimensionsMemoryDescriptor(jint currentMemoryDescriptor, jint *dataDimensions, jsize sizeDimensions) {
    ANeuralNetworksMemoryDesc *memoryDescriptor = memoryDescriptorList[currentMemoryDescriptor];
    result = ANeuralNetworksMemoryDesc_setDimensions(memoryDescriptor, (uint32_t)sizeDimensions, (const uint32_t*)dataDimensions);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the setDimensionsMemoryDescriptor method of the MemoryDescriptor object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **createEventFromSyncFenceFd(jint sync_fence_fd) {
    result = ANeuralNetworksEvent_createFromSyncFenceFd(sync_fence_fd, &eventList[currentPositionEvent]);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the newEvent method to create a new Event object because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **getSyncFenceFd(jint currentEvent, jint *sync_fence_fd) {
    ANeuralNetworksEvent *event = eventList[currentEvent];
    result = ANeuralNetworksEvent_getSyncFenceFd(event, sync_fence_fd);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the getSyncFenceFD method of the Event object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
void **isLiveStateDevice(jint currentDevice) {
    ANeuralNetworksDevice *device = devicesList[currentDevice];
    result = ANeuralNetworksDevice_wait(device);
    if (result != ANEURALNETWORKS_NO_ERROR) {
        message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the isLiveState method of the Device object you are using because ");
        findErrorType(result, message);
    } else message = NULL;
    Result[0] = message;
    Result[1] = &result;
    return Result;
}
#endif
void findErrorType(jint typeError, char *message) {
    switch(typeError) {
        case ANEURALNETWORKS_OUT_OF_MEMORY:
            strcat(message, "your device has run out of memory");
            break;
        case ANEURALNETWORKS_INCOMPLETE:
            strcat(message, "operation could not be completed");
            break;
        case ANEURALNETWORKS_UNEXPECTED_NULL:
            strcat(message, "a null value occurred during operation");
            break;
        case ANEURALNETWORKS_BAD_DATA:
            strcat(message, "There are different possibilities of the error, which are the following:\n1.- invalid function arguments\n2.- invalid ");
            strcat(message, "model definition\n3.- execution of an invalid definition\n4.- invalid data at execution time");
            break;
        case ANEURALNETWORKS_OP_FAILED:
            strcat(message, "there was a failure in the model execution");
            break;
        case ANEURALNETWORKS_BAD_STATE:
            strcat(message, "was caused by a wrong state of some data");
            break;
        case ANEURALNETWORKS_UNMAPPABLE:
            strcat(message, "some memory file could not be mapped, this can be caused because the descriptor of a file could not be mapped or a Buffer ");
            strcat(message, "object is not supported by any device in the device list that it is using, or the memory could not be read");
            break;
        case ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE:
            strcat(message, "the buffer size you provided is insufficient to obtain the model outputs");
            break;
        case ANEURALNETWORKS_UNAVAILABLE_DEVICE:
            strcat(message, "some of the devices in the list of devices that you provided to be used are not yet ready");
            break;
    #if __ANDROID_API__ >= 30
        case ANEURALNETWORKS_MISSED_DEADLINE_TRANSIENT: case ANEURALNETWORKS_MISSED_DEADLINE_PERSISTENT:
            strcat(message, "the problem is due to the limitations of the driver resources");
            break;
        case ANEURALNETWORKS_RESOURCE_EXHAUSTED_TRANSIENT: case ANEURALNETWORKS_RESOURCE_EXHAUSTED_PERSISTENT:
            strcat(message, "any of the devices from the list of devices you provided is not yet available");
            break;
        case ANEURALNETWORKS_DEAD_OBJECT:
            strcat(message, "some type of information you are handling is dead, null, etc.");
            break;
    #endif
        default:
            strcat(message, "There was a problem that could be caused by an erroneous data that you are handling that you have adjusted before, for ");
            strcat(message, "example, when you use the identifyInputsAndOutputs method of a Model object and use an index in the setInput or setOutput ");
            strcat(message, "method of an Execution object and that this index is different from that you used in the input and output arrays that you ");
            strcat(message, "provided in the identifyInputsAndOutputs method, be careful with the data you handle");
    }
}