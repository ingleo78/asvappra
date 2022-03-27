#ifndef OPENCL
#define OPENCL

#include <jni.h>
#include <string>

extern "C" {
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_OpenCL_LoadOpenCL(JNIEnv *env, jobject openCL);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Platform_getPlatforms(JNIEnv *env, jclass platform);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Platform_getName(JNIEnv *env, jobject platform);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Platform_getProfile(JNIEnv *env, jobject platform);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Platform_getVendor(JNIEnv *env, jobject platform);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Platform_getVersion(JNIEnv *env, jobject platform);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Platform_getExtensions(JNIEnv *env, jobject platform);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Platform_getHostTimerResolution(JNIEnv *env, jobject platform);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Platform_toString(JNIEnv *env, jobject platform);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Device_getDevices(JNIEnv *env, jclass device, jobject platform, jint DeviceType);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Device_createPartition(JNIEnv *env, jobject obj, jint partitionType, jint affinityType);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getType(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getVendorId(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxComputeUnits(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxWorkItemDimensions(JNIEnv *env, jobject device);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_opencl_Device_getMaxWorkItemSizes(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxWorkGroupSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredVectorWidthChar(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredVectorWidthShort(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredVectorWidthInt(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredVectorWidthLong(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredVectorWidthFloat(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredVectorWidthDouble(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredVectorWidthHalf(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getNativeVectorWidthChar(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getNativeVectorWidthShort(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getNativeVectorWidthInt(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getNativeVectorWidthLong(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getNativeVectorWidthFloat(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getNativeVectorWidthDouble(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getNativeVectorWidthHalf(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxClockFrequency(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getAddressBits(JNIEnv *env, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Device_getMaxMemAllocSize(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getImageSupport(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxReadImageArgs(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxWriteImageArgs(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxReadWriteImageArgs(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getILVersion(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImage2DMaxWidth(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImage2DMaxHeight(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImage3DMaxWidth(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImage3DMaxHeight(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImage3DMaxDepth(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImageMaxBufferSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImageMaxArraySize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxSamplers(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImagePitchAlignment(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getImageBaseAddressAlignment(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxPipeArgs(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPipeMaxActiveReservations(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPipeMaxPacketSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxParameterSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMemBaseAddressAlign(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getSingleFPConfig(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getDoubleFPConfig(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getGlobalMemCacheType(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getGlobalMemCachelineSize(JNIEnv *env, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Device_getGlobalMemCacheSize(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getLocalMemType(JNIEnv *env, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Device_getGlobalMemSize(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getErrorCorrectionSupport(JNIEnv *env, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Device_getMaxConstantBufferSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxConstantArgs(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxGlobalVariableSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getGlobalVariablePreferredTotalSize(JNIEnv *env, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Device_getLocalMemSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getProfilingTimerResolution(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getEndianLittle(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getAvailable(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getCompilerAvailable(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getLinkerAvailable(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getExecutionCapabilities(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getQueueOnHostProperties(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getQueueOnDeviceProperties(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getQueueOnDevicePreferredSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getQueueOnDeviceMaxSize(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxOnDeviceQueues(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxOnDeviceEvents(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getBuildInKernels(JNIEnv *env, jobject device);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Device_getPlatform(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getName(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getVendor(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getDriverVersion(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getProfile(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getVersion(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getOpenCL_1C_1Version(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getExtensions(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPrintfBufferSize(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredInteropUserSync(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPartitionMaxSubDevices(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getPartitionProperties(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getPartitionAffinityDomain(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getPartitionType(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getReferenceCount(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_getSVMCapabilities(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredPlatformAtomicAlignment(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredGlobalAtomicAlignment(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getPreferredLocalAtomicAlignment(JNIEnv *env, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Device_getMaxNumSubGroups(JNIEnv *env, jobject device);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Device_getSubGroupIndependentForwardProgress(JNIEnv *env, jobject device);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Device_releaseDevice(JNIEnv *env, jobject device);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Device_toString(JNIEnv *env, jobject device);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_DeviceTimer_getDeviceTimer(JNIEnv *env, jclass deviceTimer, jobject device);
JNIEXPORT jlongArray JNICALL Java_com_draico_asvappra_opencl_DeviceTimer_getDeviceAndHostTimestamp(JNIEnv *env, jobject deviceTimer);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_DeviceTimer_getHostTimestamp(JNIEnv *env, jobject deviceTimer);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_DeviceTimer_toString(JNIEnv *env, jobject deviceTimer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Context_createContext(JNIEnv *env, jclass context, jobjectArray devices,
                                                       jobject callbackContext);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Context_createContextFromType(JNIEnv *env, jclass context, jobject platform,
                                                               jobject callbackContext, jlong deviceType);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Context_getReferenceCount(JNIEnv *env, jobject context);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Context_getDevices(JNIEnv *env, jobject context);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Context_releaseContext(JNIEnv *env, jobject context);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Context_toString(JNIEnv *env, jobject context);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_CommandQueue_getCommandQueueWithProperties(JNIEnv *env, jclass commandQueue,
                                                                                  jobject context, jobject device,
                                                                                  jobject hashMap);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_CommandQueue_setDefaultDeviceCommandQueue(JNIEnv *env, jobject commandQueue, jobject context,
                                                                 jobject device);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_CommandQueue_releaseCommandQueue(JNIEnv *env, jobject commandQueue);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_CommandQueue_flush(JNIEnv *env, jobject commandQueue);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_CommandQueue_finish(JNIEnv *env, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_CommandQueue_getContext(JNIEnv *env, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_CommandQueue_getDevice(JNIEnv *env, jobject commandQueue);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_CommandQueue_getReferenceCount(JNIEnv *env, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_CommandQueue_getDeviceDefault(JNIEnv *env, jobject commandQueue);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_CommandQueue_toString(JNIEnv *env, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_createBuffer(JNIEnv *env, jclass buffer, jobject context, jobject memory,
                                                    jint flagsBuffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_createBufferWithPrimitiveArray(JNIEnv *env, jclass Buffer, jobject context,
                                                                      jobject data, jint flagsBuffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_createSubBuffer(JNIEnv *env, jobject buffer, jint flagsBuffer, jint sizeBlockMemory);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_readBuffer(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                  jboolean isBlockingRead, jint offsetRead, jint sizeBlock);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_getIntArray(JNIEnv *env, jobject buffer, jobject commandQueue, jint sizeArray);
JNIEXPORT jshortArray JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_getShortArray(JNIEnv *env, jobject buffer, jobject commandQueue, jint sizeArray);
JNIEXPORT jlongArray JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_getLongArray(JNIEnv *env, jobject buffer, jobject commandQueue, jint sizeArray);
JNIEXPORT jfloatArray JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_getFloatArray(JNIEnv *env, jobject buffer, jobject commandQueue, jint sizeArray);
JNIEXPORT jbyteArray JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_getByteArray(JNIEnv *env, jobject buffer, jobject commandQueue, jint sizeArray);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_writeBuffer(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                   jboolean isblockingWrite, jint offsetWrite, jint sizeBlock);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_writeIntArray(JNIEnv *env, jobject buffer, jobject commandQueue, jintArray data);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_writeShortArray(JNIEnv *env, jobject buffer, jobject commandQueue, jshortArray data);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_writeLongArray(JNIEnv *env, jobject buffer, jobject commandQueue, jlongArray data);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_writeFloatArray(JNIEnv *env, jobject buffer, jobject commandQueue, jfloatArray data);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_writeByteArray(JNIEnv *env, jobject buffer, jobject commandQueue, jbyteArray data);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_readBufferRect(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                      jobject dataBlock, jboolean isBlockingRead, jintArray offsetXYZDataBlock,
                                                      jintArray offsetXYBuffer, jintArray regionRead, jint lengthColumnsPerRowInDataBlock,
                                                      jint sizeAreaDataBlock, jint lengthColumnsPerRowInBuffer, jint areaInBytesIntBuffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_writeBufferRect(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                       jobject dataBlock, jboolean isBlockingWrite, jintArray offsetXYZDataBlock,
                                                       jintArray offsetXYZBuffer, jintArray regionWrite, jint lengthColumnsPerRowInDataBlock,
                                                       jint sizeAreaDataBlock, jint lengthColumnsPerDataInBuffer, jint sizeAreaBuffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_copyBuffer(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                  jobject srcBuffer, jint offsetSrcBuffer, jint offsetDstBuffer, jint sizeCopy);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_copyBufferRect(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                      jobject srcBuffer, jintArray offsetXYZSrcBuffer, jintArray offsetXYZDstBuffer,
                                                      jintArray regionCopy, jint lengthColumnsPerRowInSrcBuffer, jint sizeAreaSrcBuffer,
                                                      jint lengthColumnsPerRowInDstBuffer, jint sizeAreaDstBuffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_fillBuffer(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                  jobject patternFill, jint sizeFillPattern);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_mapBuffer(JNIEnv *env, jobject buffer, jobject commandQueue,jboolean isBlockingMap,
                                                 jlong flagsMapBuffer, jint offsetMap, jint sizeMap);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_unmapBuffer(JNIEnv *env, jobject buffer, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_migrateBuffer(JNIEnv *env, jobject buffer, jobject commandQueue,
                                                     jobjectArray buffers, jint flagsMigrateBuffers);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_setReleaseCallback(JNIEnv *env, jobject buffer, jobject callbackBuffer);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_releaseBuffer(JNIEnv *env, jobject buffer);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_memory_buffer_Buffer_toString(JNIEnv *env, jobject buffer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_listeners_Event_createEvent(JNIEnv *env, jclass event, jobject context, jobject commandQueue);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_listeners_Event_setEventStatus(JNIEnv *env, jobject event, jint executionStatus);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_listeners_Event_waitForEvents(JNIEnv *env, jobject event, jobjectArray listEvents);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_listeners_Event_setEventCallback(JNIEnv *env, jobject event, jint executionStatus, jobject callbackEvent);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_listeners_Event_markerWithWaitListEvent(JNIEnv *env, jobject event, jobject commandQueue,
                                                              jobjectArray waitListEvent);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_listeners_Event_barrierWithWaitListEvent(JNIEnv *env, jobject event, jobject commandQueue,
                                                               jobjectArray waitListEvent);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getCommandQueue(JNIEnv *env, jobject event);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getContext(JNIEnv *env, jobject event);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_listeners_Event_releaseEvent(JNIEnv *env, jobject event);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getCommandType(JNIEnv *env, jobject event);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getExecutionStatus(JNIEnv *env, jobject event);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getReferenceCount(JNIEnv *env, jobject event);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getProfilingCommandQueue(JNIEnv *env, jobject event);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getProfilingCommandSubmit(JNIEnv *env, jobject event);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getProfilingCommantStart(JNIEnv *env, jobject event);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getProfilingCommandEnd(JNIEnv *env, jobject event);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_listeners_Event_getProfilingCommandComplete(JNIEnv *env, jobject event);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_listeners_Event_toString(JNIEnv *env, jobject event);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_Memory_mallocMemory(JNIEnv *env, jclass memory, jobject context, jint flagsMemory,
                                                     jint sizeBlock);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_Memory_freeMemory(JNIEnv *env, jobject memory);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_Memory_clear(JNIEnv *env, jobject memory, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_Memory_enqueueFreeBlocksMemory(JNIEnv *env, jobject memory, jobject commandQueue,
                                                               jobjectArray memoryArray, jobject callbackMemory);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_Memory_enqueueCopyBlockMemory(JNIEnv *env, jobject memory, jobject commandQueue,
                                                              jobject srcMemory, jboolean isBlockingMemory);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_Memory_enqueueFillBlockMemory(JNIEnv *env, jobject memory, jobject commandQueue,
                                                              jobject patternFill, jint sizePatternFill, jint sizeBlockToFill);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_Memory_enqueueMapMemory(JNIEnv *env, jobject memory, jobject commandQueue,
                                                        jboolean isBlockingMap, jint sizeBlock, jint flagsMap);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_Memory_enqueueUnmapMemory(JNIEnv *env, jobject memory, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_memory_Memory_enqueueMigrateMemory(JNIEnv *env, jobject memory, jobject commandQueue,
                                                            jobjectArray memoryList, jint flagsMigrateMemory);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_Memory_addByteArray(JNIEnv *env, jobject memory, jobject commandQueue, jbyteArray data,
                                           jint offset);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_Memory_addFloatArray(JNIEnv *env, jobject memory, jobject commandQueue,
                                            jfloatArray data, jint offset);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_Memory_addIntegerArray(JNIEnv *env, jobject memory, jobject commandQueue, jintArray data,
                                              jint offset);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_Memory_addShortArray(JNIEnv *env, jobject memory, jobject commandQueue, jshortArray data,
                                            jint offset);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_memory_Memory_addLongArray(JNIEnv *env, jobject memory, jobject commandQueue, jlongArray data,
                                           jint offset);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_memory_Memory_toString(JNIEnv *env, jobject memory);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_createImage(JNIEnv *env, jclass Image, jobject context, jobject imageFormat,
                                                  jobject imageDescriptor, jbyteArray dataImage, jint flags);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_readImage(JNIEnv *env, jobject image, jobject commandQueue, jobject buffer,
                                                jboolean isBlockingRead, jintArray origin, jintArray region, jint numberBytesPerRow,
                                                jint numberBytesPerLayer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_writeImage(JNIEnv *env, jobject image, jobject commandQueue, jobject buffer,
                                                 jboolean isBlockingWrite, jintArray origin, jintArray region, jint numberBytesPerRow,
                                                 jint numberBytesPerLayer);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_copyImage(JNIEnv *env, jobject image, jobject commandQueue, jobject imageSrc,
                                                jintArray srcOrigin, jintArray dstOrigin, jintArray region);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_fillImage(JNIEnv *env, jobject image, jobject commandQueue, jobject fillColor,
                                                jintArray origin, jintArray region);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_copyImageToBuffer(JNIEnv *env, jobject image, jobject commandQueue,
                                                        jobject buffer, jintArray origin, jintArray region);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_copyBufferToImage(JNIEnv *env, jobject image, jobject commandQueue,
                                                        jobject buffer, jintArray origin, jintArray region);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_mapImage(JNIEnv *env, jobject image, jobject commandQueue, jintArray origin,
                                               jintArray region, jintArray numberBytesPerRow, jintArray numberBytesPerLayer,
                                               jboolean isBlockingMap, jint flagMap);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_unmapImage(JNIEnv *env, jobject image, jobject commandQueue);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_migrateImage(JNIEnv *env, jobject image, jobject commandQueue,
                                                   jobjectArray images, jint flagMigrate);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_image_Image_release(JNIEnv *env, jobject image);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_Image_getImageFormat(JNIEnv *env, jobject image);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_Image_getSizePixel(JNIEnv *env, jobject image);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_Image_getNumberBytesPerRow(JNIEnv *env, jobject image);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_Image_getNumberBytesPerLayer(JNIEnv *env, jobject image);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_Image_getWidth(JNIEnv *env, jobject image);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_Image_getHeight(JNIEnv *env, jobject image);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_Image_getDepth(JNIEnv *env, jobject image);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_Image_getNumberImages(JNIEnv *env, jobject image);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_image_Image_toString(JNIEnv *env, jobject image);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_ImageFormat_createImageFormat(JNIEnv *env, jclass ImageFormat, jobject context,
                                                                   jint imageChannelOrder, jint imageChannelDataType);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_image_ImageFormat_getSupportedImageFormats(JNIEnv *env, jclass ImageFormat,
                                                                                jobject context, jint typeAccessImage, jint imageType);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_image_ImageFormat_toString(JNIEnv *env, jobject imageFormat);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_ImageDescriptor_createImageDescriptor(JNIEnv *env, jclass ImageDescriptor,
                                                                             jobject imageFormat, jobject buffer,
                                                                             jint imageType, jint width, jint height, jint depth,
                                                                             jint numberImages);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_image_ImageDescriptor_toString(JNIEnv *env, jobject imageDescriptor);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_pipe_Pipe_createPipe(JNIEnv *env, jclass Pipe, jobject context, jint typeAccess, jint packetSize,
                                               jint numberPackets);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_pipe_Pipe_getPacketSize(JNIEnv *env, jobject pipe);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_pipe_Pipe_getNumberPackets(JNIEnv *env, jobject pipe);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_pipe_Pipe_release(JNIEnv *env, jobject pipe);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_pipe_Pipe_toString(JNIEnv *env, jobject pipe);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_sample_Sample_createSampleWithProperties(JNIEnv *env, jclass Sample, jobject context,
                                                                   jboolean isNormalizedCoords, jint addressingMode, jint filterMode);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_image_sample_Sample_getContext(JNIEnv *env, jobject sample);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_image_sample_Sample_isNormalizedCoords(JNIEnv *env, jobject sample);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_sample_Sample_getAddressingMode(JNIEnv *env, jobject sample);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_image_sample_Sample_getFilterMode(JNIEnv *env, jobject sample);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_image_sample_Sample_release(JNIEnv *env, jobject sample);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_image_sample_Sample_toString(JNIEnv *env, jobject sample);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_createProgramWithSource(JNIEnv *env, jclass Program, jobject context,
                                                                  jobjectArray filesProgramSources);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_createProgramWithSourceScriptKernel(JNIEnv *env, jclass Program, jobject context,
                                                                              jbyteArray data);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_createProgramWithSourceAssets(JNIEnv *env, jclass Program, jobject context);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_createProgramWithIL(JNIEnv *env, jclass Program, jobject context,
                                                              jbyteArray dataProgram);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_createProgramWithBinary(JNIEnv *env, jclass Program, jobject context,
                                                                  jobjectArray devices, jobjectArray filesBinaries,
                                                                  jintArray stateFilesBinaries);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_createProgramWithBuiltInKernels(JNIEnv *env, jclass Program, jobject context,
                                                                          jobjectArray devices, jobjectArray kernelNames);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Program_releaseProgram(JNIEnv *env, jobject program);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Program_setProgramReleaseCallback(JNIEnv *env, jobject program, jobject callbackProgram);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Program_setProgramSpecializationConstant(JNIEnv *env, jobject program, jint specializationID,
                                                                jobject dataSpecialization);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Program_buildProgram(JNIEnv *env, jobject program, jobjectArray devices, jstring buildOptions,
                                            jobject callbackBuildProgram);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Program_compileProgram(JNIEnv *env, jobject program, jobjectArray devices, jstring compileOptions,
                                              jobjectArray programs, jobjectArray headersFiles,
                                              jobject callbackCompileProgram);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_linkProgram(JNIEnv *env, jobject program, jobject context,
                                                      jobjectArray devices, jstring linkOptions, jobjectArray programs,
                                                      jobject callbackLinkProgram);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Program_unloadPlatformCompiler(JNIEnv *env, jobject program, jobject platform);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Program_getContext(JNIEnv *env, jobject program);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Program_getDevices(JNIEnv *env, jobject program);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Program_getFilesProgramSource(JNIEnv *env, jobject program);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Program_getProgramIL(JNIEnv *env, jobject program);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_opencl_Program_getSizeFileProgramBinaries(JNIEnv *env, jobject program);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Program_getFilesProgramBinaries(JNIEnv *env, jobject program);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Program_getNumberKernels(JNIEnv *env, jobject program);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Program_getProgramKernelName(JNIEnv *env, jobject program);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Program_isScopeGlobalConstructorsPresent(JNIEnv *env, jobject program);
JNIEXPORT jboolean JNICALL Java_com_draico_asvappra_opencl_Program_isScopeGlobalDestructorsPresent(JNIEnv *env, jobject program);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Program_getBuildStatus(JNIEnv *env, jobject program);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Program_getBuildOptions(JNIEnv *env, jobject program);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Program_getBuildLog(JNIEnv *env, jobject program);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Program_getBinaryType(JNIEnv *env, jobject program);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_opencl_Program_getSize(JNIEnv *env, jobject program);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Program_toString(JNIEnv *env, jobject program);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Kernel_createKernel(JNIEnv *env, jclass Kernel, jobject program, jstring kernelName);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Kernel_createKernelsInProgram(JNIEnv *env, jclass Kernel, jobject program);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Kernel_NDRangeKernel(JNIEnv *env, jobject kernel, jobject commandQueue,
                                                     jint workNumberDimensions, jintArray globalWorkOffset, jintArray globalWorkSize,
                                                     jintArray localWorkSize);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Kernel_nativeKernel(JNIEnv *env, jclass kernel, jobject commandQueue,
                                                    jobject functionNativeKernel,
                                                    jobjectArray argsFunctionNativeKernel);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Kernel_releaseKernel(JNIEnv *env, jobject kernel);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Kernel_setKernelArguments(JNIEnv *env, jobject kernel, jobject commandQueue,
                                                 jobjectArray arguments);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Kernel_setKernelArgumentsSVMPointer(JNIEnv *env, jobject kernel, jobject commandQueue,
                                                           jobjectArray arguments);
JNIEXPORT void JNICALL Java_com_draico_asvappra_opencl_Kernel_setKernelExecInfoSVM(JNIEnv *env, jobject kernel, jobject commandQueue, jint execInfo,
                                                   jobjectArray arguments);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Kernel_cloneKernel(JNIEnv *env, jobject kernel);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_getFunctionName(JNIEnv *env, jobject kernel);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Kernel_getNumberArguments(JNIEnv *env, jobject kernel);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Kernel_getContext(JNIEnv *env, jobject kernel);
JNIEXPORT jobject JNICALL Java_com_draico_asvappra_opencl_Kernel_getProgram(JNIEnv *env, jobject kernel);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_getAtributes(JNIEnv *env, jobject kernel);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_opencl_Kernel_getGlobalWorkSize(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Kernel_getWorkGroupSize(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_opencl_Kernel_getCompileWorkGroupSize(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Kernel_getLocalMemorySize(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Kernel_getPreferredWorkSizeMultiple(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jlong JNICALL Java_com_draico_asvappra_opencl_Kernel_getPrivateMemorySize(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Kernel_getMaxSubGroupSizeForNDRange(JNIEnv *env, jobject kernel, jobject device, jintArray sizeSubGroups);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Kernel_getSubGroupCountForNDRange(JNIEnv *env, jobject kernel, jobject device, jintArray localWorkSize);
JNIEXPORT jintArray JNICALL Java_com_draico_asvappra_opencl_Kernel_getLocalSizeForSubGroupCount(JNIEnv *env, jobject kernel, jobject device, jint numberSubGroups);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Kernel_getMaxNumberSubGroups(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jint JNICALL Java_com_draico_asvappra_opencl_Kernel_getCompileNumSubGroups(JNIEnv *env, jobject kernel, jobject device);
JNIEXPORT jobjectArray JNICALL Java_com_draico_asvappra_opencl_Kernel_getArgumentsKernel(JNIEnv *env, jobject kernel, jobject commandQueue);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_getArgumentAddressQualifier(JNIEnv *env, jobject kernel);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_getArgumentAccessQualifier(JNIEnv *env, jobject kernel);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_getArgumentTypeName(JNIEnv *env, jobject kernel);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_getArgumentTypeQualifier(JNIEnv *env, jobject kernel);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_getName(JNIEnv *env, jobject kernel);
JNIEXPORT jstring JNICALL Java_com_draico_asvappra_opencl_Kernel_toString(JNIEnv *env, jobject kernel);
};

#endif