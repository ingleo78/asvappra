#ifndef PRUEBA_FUNCTIONSOPENCL_H
#define PRUEBA_FUNCTIONSOPENCL_H

#include <jni.h>
#include "CL/cl.h"

typedef cl_int (*_clGetPlatformIDs)(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms);
typedef cl_int (*_clGetPlatformInfo)(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size,
                                     void *param_value, size_t *param_value_size_ret);
typedef cl_int (*_clGetDeviceIDs)(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries,
                                  cl_device_id *devices, cl_uint *num_devices);
typedef cl_int (*_clGetDeviceInfo)(cl_device_id device, cl_device_info param_name, size_t param_value_size,
                                   void *param_value, size_t *param_value_size_ret);
typedef cl_int (*_clGetDeviceAndHostTimer)(cl_device_id device, cl_ulong *device_timestamp, cl_ulong *host_timestamp);
typedef cl_int (*_clGetHostTimer)(cl_device_id device, cl_ulong *host_timestamp);
typedef cl_int (*_clCreateSubDevices)(cl_device_id deviceIn, cl_device_partition_property *partitionProperty, cl_uint numDevicesToCreated,
                                      cl_device_id *devicesCreated, cl_uint *numDevicesFound);
typedef cl_int (*_clRetainDevice)(cl_device_id device);
typedef cl_int (*_clReleaseDevice)(cl_device_id device);
typedef cl_context (*_clCreateContext)(const cl_context_properties *properties, cl_uint num_devices, const cl_device_id *device,
                                       void(CL_CALLBACK *pfn_notify)(const char *errInfo, const void *private_info, size_t cb,
                                       void *user_data), void *user_data, cl_int *errorCode_ret);
typedef cl_context (*_clCreateContextFromType)(const cl_context_properties *properties, cl_device_type device_type,
                                               void(CL_CALLBACK *pfn_notify)(const char *errInfo, const void *private_info, size_t cb,
                                               void *user_data), void *user_data, cl_int *errCode_ret);
typedef cl_int (*_clRetainContext)(cl_context context);
typedef cl_int (*_clReleaseContext)(cl_context context);
typedef cl_int (*_clGetContextInfo)(cl_context context, cl_context_info param_name, size_t param_value_size, void *param_value,
                                    size_t *param_value_size_ret);
typedef void(CL_CALLBACK *_notifyMessageContext)(const char *errorInfo, const void *privateInfo, size_t cb, void *userData);
typedef cl_command_queue (*_clCreateCommandQueueWithProperties)(cl_context context, cl_device_id device,
                                                                const cl_queue_properties *properties, cl_int *errorCode_ret);
typedef cl_int (*_clSetDefaultDeviceCommandQueue)(cl_context context, cl_device_id device, cl_command_queue commandQueue);
typedef cl_int (*_clRetainCommandQueue)(cl_command_queue commandQueue);
typedef cl_int (*_clReleaseCommandQueue)(cl_command_queue commandQueue);
typedef cl_int (*_clFlush)(cl_command_queue commandQueue);
typedef cl_int (*_clFinish)(cl_command_queue commandQueue);
typedef cl_int (*_clGetCommandQueueInfo)(cl_command_queue commandQueue, cl_command_queue_info infoQueue, size_t _sizeData, void *data,
                                         size_t *sizeData);
typedef cl_mem (*_clCreateBuffer)(cl_context context, cl_mem_flags flagsBuffer, size_t sizeBuffer, void *ptrDataBuffer, cl_int *errorCode);
typedef cl_mem (*_clCreateSubBuffer)(cl_mem buffer, cl_mem_flags flagsBuffer, cl_buffer_create_type typeBuffer, const void *infoSubBuffer,
                                     cl_int *errorCode);
typedef cl_int (*_clEnqueueReadBuffer)(cl_command_queue commandQueue, cl_mem buffer, cl_bool isBlockingRead, size_t offsetRead,
                                       size_t sizeBlock, void *ptrDataBuffer, cl_uint lengthWaitListEvent, const cl_event *waitListEvent,
                                       cl_event *listEvent);
typedef cl_int (*_clEnqueueWriteBuffer)(cl_command_queue commandQueue, cl_mem buffer, cl_bool isBlockingRead, size_t offsetRead,
                                        size_t sizeBlock, const void *ptrDataBuffer, cl_uint lengthWaitListEvent,
                                        const cl_event *waitListEvent, cl_event *listEvent);
typedef cl_int (*_clEnqueueReadBufferRect)(cl_command_queue commandQueue, cl_mem buffer, cl_bool isBlockingRead,
                                           const size_t *offsetXYZBufferOrigin, const size_t *offsetXYZBufferHostOrigin,
                                           const size_t *region, size_t numberRowsBuffer, size_t numberColumnsBuffer,
                                           size_t numberRowsBufferHost, size_t numberColumnsBufferHost, void *ptrDataBuffer,
                                           cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *listEvent);
typedef cl_int (*_clEnqueueWriteBufferRect)(cl_command_queue commandQueue, cl_mem buffer, cl_bool isBlockingRead,
                                            const size_t *offsetXYZBufferOrigin, const size_t *offsetXYZBufferHostOrigin,
                                            const size_t *region, size_t numberRowsBuffer, size_t numberColumnsBuffer,
                                            size_t numberRowsBufferHost, size_t numberColumnsBufferHost, const void *ptrDataBuffer,
                                            cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *listEvent);
typedef cl_int (*_clEnqueueCopyBuffer)(cl_command_queue commandQueue, cl_mem srcBuffer, cl_mem dstBuffer, size_t offsetSrcBuffer,
                                       size_t offsetDstBuffer, size_t sizeCopy, cl_uint lengthWaitListEvent, const cl_event *waitListEvent,
                                       cl_event *listEvent);
typedef cl_int (*_clEnqueueCopyBufferRect)(cl_command_queue commandQueue, cl_mem srcBuffer, cl_mem dstBuffer,
                                           const size_t *offsetSrcBuffer, const size_t *offsetDstBuffer, const size_t *region,
                                           size_t numberRowsSrcBuffer, size_t numberColumnsSrcBuffer, size_t numberRowsDstBuffer,
                                           size_t numberColumnsDstBuffer, cl_uint lengthWaitListEvent, const cl_event *waitListEvent,
                                           cl_event *listEvent);
typedef cl_int (*_clEnqueueFillBuffer)(cl_command_queue commandQueue, cl_mem buffer, const void *patternFill, size_t sizeFillPattern,
                                       size_t offsetFill, size_t sizeAreaFillInBuffer, cl_uint lengthWaitListEvent,
                                       const cl_event *waitListEvent, cl_event *listEvent);
typedef void (*_clEnqueueMapBuffer)(cl_command_queue commandQueue, cl_mem buffer, cl_bool isBlockingMap, cl_map_flags flagsMapBuffer,
                                    size_t offsetMap, size_t sizeMap, cl_uint lengthWaitListEvent, cl_event *listEvent, cl_event *event,
                                    cl_int *errorCode);
typedef cl_int (*_clReleaseBuffer)(cl_mem buffer);
typedef cl_event (*_clCreateUserEvent)(cl_context context, cl_int *errorCode);
typedef cl_int (*_clSetUserEventStatus)(cl_event event, cl_int executionStatus);
typedef cl_int (*_clWaitForEvents)(cl_uint numEvents, const cl_event *listEvent);
typedef cl_int (*_clGetEventInfo)(cl_event event, cl_event_info paramName, size_t paramValueSize, void *paramValue,
                                  size_t *paramValueSizeRet);
typedef cl_int (*_clSetEventCallback)(cl_event event, cl_int callbackType, void (CL_CALLBACK *pfn_event_notify)(cl_event eventCallback,
                                      cl_int statusCommandExecution, void *userData), void *userData);
typedef cl_int (*_clRetainEvent)(cl_event event);
typedef cl_int (*_clReleaseEvent)(cl_event event);
typedef cl_int (*_clEnqueueMarkerWithWaitList)(cl_command_queue commandQueue, cl_uint lengthWaitListEvent, const cl_event *waitListEvent,
                                               cl_event *listEvent);
typedef cl_int (*_clEnqueueBarrierWithWaitList)(cl_command_queue commandQueue, cl_uint lengthWaitListEvent, const cl_event *waitListEvent,
                                                cl_event *listEvent);
typedef cl_int (*_clGetEventProfilingInfo)(cl_event event, cl_profiling_info paramName, size_t paramValueSize, void *paramValue,
                                           size_t *paramValueRet);
typedef void *(*_clSVMAlloc)(cl_context context, cl_svm_mem_flags flagsMemory, size_t sizeBlock, cl_uint numberBytesPerData);
typedef void (*_clSVMFree)(cl_context context, void *memory);
typedef cl_int (*_clEnqueueSVMFree)(cl_command_queue commandQueue, cl_uint numMemoryToBeFree, void **memory,
                                    void(CL_CALLBACK *CallbackMemory)(cl_command_queue command_queue, cl_uint numMemoryToBeFree,
                                    void **memory, void *userData), void *userData, cl_uint lengthWaitListEvent,
                                    const cl_event *waitListEvent, cl_event *listEvent);
typedef cl_int (*_clEnqueueSVMMemcpy)(cl_command_queue commandQueue, cl_bool isBlocking, void *dstMemory, const void *srcMemory,
                                      size_t sizeBlock, cl_uint lengthWaitListEvent, const cl_event *awaitListEvent, cl_event *listEvent);
typedef cl_int (*_clEnqueueSVMMemFill)(cl_command_queue commandQueue, void *memory, const void *patternFill, size_t sizePatternFill,
                                       size_t offsetSizeBlock, cl_uint lengthWaitListEvent, const cl_event *waitListEvent,
                                       cl_event *listEvent);
typedef cl_int (*_clEnqueueSVMMap)(cl_command_queue commandQueue, cl_bool isBlocking, cl_map_flags flagsMap,
                                   void *memory, size_t sizeBlock, cl_uint lengthWaitListEvent, const cl_event *waitListEvent,
                                   cl_event *listEvent);
typedef cl_int (*_clEnqueueSVMUnmap)(cl_command_queue commandQueue, void *memory, cl_uint lengthWaitListEvent,
                                     const cl_event *waitListEvent, cl_event *listEvent);
typedef cl_int (*_clEnqueueSVMMigrateMem)(cl_command_queue commandQueue, cl_uint lenghtListMemory, const void **memoryList,
                                          const size_t *sizeBlock, cl_mem_migration_flags flagsMigrateMemory, cl_uint lengthwaitListEvent,
                                          const cl_event *waitListEvent, cl_event *listEvent);
typedef cl_mem (*_clCreateImage)(cl_context context, cl_mem_flags typeAccessImage, const cl_image_format *imageFormat,
                                 const cl_image_desc *imageDescriptor, void *dataImage, cl_int *errorCode);
typedef cl_int (*_clGetSupportedImageFormats)(cl_context context, cl_mem_flags typeAccessImage, cl_mem_object_type imageType,
                                              cl_uint numberEntries, cl_image_format *imageFormats, cl_uint *numberImageFormats);
typedef cl_int (*_clEnqueueReadImage)(cl_command_queue commandQueue, cl_mem image, cl_bool blockingRead, const size_t *origin,
                                      const size_t *region, size_t numberBytesPerRow, size_t numberBytesPerLayer, void *buffer,
                                      cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clEnqueueWriteImage)(cl_command_queue commandQueue, cl_mem image, cl_bool blockingWrite, const size_t *origin,
                                       const size_t *region, size_t numberBytesPerRow, size_t numberBytesPerLayer,
                                       void *buffer, cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clEnqueueCopyImage)(cl_command_queue commandQueue, cl_mem imageSrc, cl_mem imageDst, const size_t *srcOrigin,
                                      const size_t *dstOrigin, const size_t *region, cl_uint lengthWaitListEvent,
                                      const cl_event *waitListEvent, const cl_event *event);
typedef cl_int (*_clEnqueueFillImage)(cl_command_queue commandQueue, cl_mem image, const void *fillColor, const size_t *origin,
                                      const size_t *region, cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clEnqueueCopyImageToBuffer)(cl_command_queue commandQueue, cl_mem image, cl_mem buffer, const size_t *origin,
                                              const size_t *region, size_t offsetCopy, cl_uint lengthWaitListEvent,
                                              const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clEnqueueCopyBufferToImage)(cl_command_queue commandQueue, cl_mem buffer, cl_mem image, size_t offsetCopy,
                                              const size_t *origin, const size_t *region, cl_uint lengthWaitListEvent,
                                              const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clEnqueueMapImage)(cl_command_queue commandQueue, cl_mem image, cl_bool blockingMap, cl_map_flags mapFlags,
                                     const size_t *origin, const size_t *region, size_t *numberBytesPerRow, size_t *numberBytesPerLayer,
                                     cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *event, cl_int *errorCode);
typedef cl_int (*_clGetImageInfo)(cl_mem image, cl_image_info informationRequest, size_t sizeInformationRequest, void *dataInformation, size_t sizeInformation);
typedef cl_mem (*_clCreatePipe)(cl_context context, cl_mem_flags typeAccess, cl_uint packetSize, cl_uint numberPackets, const cl_pipe_properties *properties,
                                cl_int *errorCode);
typedef cl_int (*_clGetPipeInfo)(cl_mem pipe, cl_pipe_info informationRequest, size_t sizeInformationRequest, void *dataInformation, size_t sizeInformation);
typedef cl_int (*_clRetainMemObject)(cl_mem ImageOrPipe);
typedef cl_int (*_clReleaseMemObject)(cl_mem ImageOrPipe);
typedef cl_int (*_clEnqueueUnmapMemObject)(cl_command_queue commandQueue, cl_mem ImageOrPipe, void *dataMapped,
                                           cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clEnqueueMigrateMemObjects)(cl_command_queue commandQueue, cl_uint numberObjects, const cl_mem *listObjects,
                                              cl_mem_migration_flags flagsMigrate, cl_uint lengthWaitListEvent,
                                              const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clSetMemObjectDestructorCallback)(cl_mem object, void (CL_CALLBACK *callback)(cl_mem object, void *userData),
                                                    void *_userData);
typedef cl_sampler (*_clCreateSamplerWithProperties)(cl_context context, const cl_sampler_properties *properties, cl_int *errorCode);
typedef cl_int (*_clRetainSampler)(cl_sampler sampler);
typedef cl_int (*_clReleaseSampler)(cl_sampler sampler);
typedef cl_int (*_clGetSamplerInfo)(cl_sampler sampler, cl_sampler infoRequest, size_t sizeInfoRequest, void *dataInfo, size_t *sizeInfo);
typedef cl_program (*_clCreateProgramWithSource)(cl_context context, cl_uint count, const char **filesSources,
                                                 const size_t *lengthFilesSources, cl_int *errorCode);
typedef cl_program (*_clCreateProgramWithIL)(cl_context context, const void *dataProgram, size_t sizeDataProgram, cl_int *errorCode);
typedef cl_program (*_clCreateProgramWithBinary)(cl_context context, cl_uint numDevices, const cl_device_id *deviceList,
                                                 const size_t *sizeFileBinaries, const unsigned char **listFileBinaries,
                                                 cl_int *binaryLoadStatus, cl_int *errorCode);
typedef cl_program (*_clCreateProgramWithBuiltInKernels)(cl_context context, cl_uint numDevices, const cl_device_id *deviceList,
                                                         const char *kernelNames, cl_int *errorCode);
typedef cl_int (*_clRetainProgram)(cl_program program);
typedef cl_int (*_clReleaseProgram)(cl_program program);
typedef cl_int (*_clSetProgramReleaseCallback)(cl_program program, void (CL_CALLBACK *notify)(cl_program program, void *userData),
                                               void *_userData);
typedef cl_int (*_clSetProgramSpecializationConstant)(cl_program program, cl_uint spectID, size_t specSize, const void *specValue);
typedef cl_int (*_clBuildProgram)(cl_program program, cl_uint numDevices, const cl_device_id *deviceList, const char *buildOptions,
                                  void (CL_CALLBACK *notify)(cl_program program, void *userData), void *userData);
typedef cl_int (*_clCompileProgram)(cl_program program, cl_uint numDevices, const cl_device_id *deviceList, const char *compileOptions,
                                    cl_uint numHeadersFiles, const cl_program *listPrograms, const char **headersFiles,
                                    void (CL_CALLBACK *notify)(cl_program program, void *dataUser), void *dataUser);
typedef cl_program (*_clLinkProgram)(cl_context context, cl_uint numDevices, const cl_device_id *deviceList, const char *linkOptions,
                                     cl_uint numPrograms, const cl_program *programs, void (CL_CALLBACK *notify)(cl_program program,
                                     void *userData), void *userData, cl_int *errorCode);
typedef cl_int (*_clUnloadPlatformCompiler)(cl_platform_id platform);
typedef cl_int (*_clGetProgramInfo)(cl_program program, cl_program_info nameInfo, size_t sizeNameInfo, void *dataNameInfo,
                                    size_t *_sizeNameInfo);
typedef cl_int (*_clGetProgramBuildInfo)(cl_program program, cl_device_id device, cl_program_build_info nameInfo, size_t sizeNameInfo,
                                         void *dataNameInfo, size_t *_sizeNameInfo);
typedef cl_kernel (*_clCreateKernel)(cl_program program, const char *kernelName, cl_int *errorCode);
typedef cl_int (*_clCreateKernelsInProgram)(cl_program program, cl_uint numKernels, cl_kernel *kernelList, cl_uint *_numKernels);
typedef cl_int (*_clRetainKernel)(cl_kernel kernel);
typedef cl_int (*_clReleaseKernel)(cl_kernel kernel);
typedef cl_int (*_clSetKernelArg)(cl_kernel kernel, cl_uint argumentIndex, size_t sizeArgument, const void *argumentValue);
typedef cl_int (*_clSetKernelArgSVMPointer)(cl_kernel kernel, cl_uint argumentIndex, const void *argumentValue);
typedef cl_int (*_clSetKernelExecInfo)(cl_kernel kernel, cl_kernel_exec_info nameInfo, size_t sizeValue, const void *value);
typedef cl_kernel (*_clCloneKernel)(cl_kernel kernel, cl_int *errorCode);
typedef cl_int (*_clGetKernelInfo)(cl_kernel kernel, cl_kernel_info nameInfo, size_t sizeNameInfo, void *valueInfo, size_t *_sizeNameInfo);
typedef cl_int (*_clGetKernelWorkGroupInfo)(cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info nameInfo, size_t sizeNameInfo, void *valueInfo,
                                            size_t *_sizeNameInfo);
typedef cl_int (*_clGetKernelSubGroupInfo)(cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info nameInfo, size_t sizeInputValue,
                                           const void *inputValue, size_t sizeNameInfo, void *valueInfo, size_t *_sizeNameInfo);
typedef cl_int (*_clGetKernelArgInfo)(cl_kernel kernel, cl_uint argumentIndex, cl_kernel_arg_info nameInfo, size_t sizeNameInfo,
                                      void *valueInfo, size_t *_sizeNameInfo);
typedef cl_int (*_clEnqueueNDRangeKernel)(cl_command_queue commandQueue, cl_kernel kernel, cl_uint workDimensions,
                                          const size_t *globalWorkOffset, const size_t *globalWorkSize, const size_t *localWorkSize,
                                          cl_uint lengthWaitListEvent, const cl_event *waitListEvent, cl_event *event);
typedef cl_int (*_clEnqueueNativeKernel)(cl_command_queue commandQueue, void (CL_CALLBACK *notify)(void *customArguments),
                                         void *customArguments, size_t sizeInBytesAllCustomArguments, cl_uint sizeListMemObjects,
                                         const cl_mem *listMemObjects, const void **listMemObjectAddress, cl_uint lenthWaitListEvent,
                                         const cl_event *waitListEvent, cl_event *event);
void **GetPlatformsIDs(cl_uint *numPlatforms);
void **GetDeviceID(cl_int currentPlatform, cl_long DeviceType, cl_uint *numDevices);
void **GetPlatformInfo(cl_int currentPlatform, cl_platform_info platformInfo);
void **GetDeviceInfo(cl_int currentDevice, cl_device_info DeviceInfo, cl_bool isDevicePartition, cl_int currentSubDevice);
void **GetDeviceAndHostTimer(cl_device_id device, cl_ulong *deviceTimeStamp, cl_ulong *hostTimestamp);
void **GetHostTimer(cl_device_id device, cl_ulong *hostTimeStamp);
void **GetCreateSubDevices(cl_device_id deviceIn, cl_device_partition_property partitionProperty,
                           cl_device_partition_property affinityDomainType, cl_uint maxSubDevicesPartitions, cl_uint maxComputeUnits,
                           cl_uint *numDevices);
void **GetReleaseDevice(cl_int currentDevice, cl_bool isSubDevice, cl_int currentSubDevice);
void **GetCreateContext(JNIEnv *env, cl_int *currentDevice, cl_int *currentSubDevice, cl_int currentPlatform, jobject callbackContext,
                        cl_bool *isDevicePartition, cl_int sizeListDevices);
void **GetCreateContextFromType(JNIEnv *env, cl_int currentPlatform, jobject callbackContext, cl_ulong deviceType);
void **GetReleaseContext(cl_int currentContext);
void **GetCreateCommandQueueWithProperties(cl_int currentContext, cl_int currentDevice, cl_int currentSubDevice, cl_bool isDevicePartition,
                                           cl_queue_properties *properties);
void **setDefaultDeviceCommandQueue(cl_int currentContext, cl_int currentDevice, cl_int currentSubDevice, cl_int currentCommandQueue,
                                    cl_bool isDevicePartition);
void **GetReleaseCommandQueue(cl_int currentCommandQueue);
void **GetFlushCommandQueue(cl_int currentCommandQueue);
void **GetFinishCommandQueue(cl_int currentCommandQueue);
void **GetCreateBuffer(cl_int currentContext, size_t sizeBuffer, cl_int currentMemory, cl_mem_flags flagsBuffer);
void **GetCreateBufferWithArrayPrimitive(cl_int currentContext, size_t sizeBuffer, void *data, cl_mem_flags flagsBuffer);
void **GetCreateSubBuffer(cl_int currentBufferSrc, cl_mem_flags flagsBuffer, cl_buffer_create_type flagSubBuffer, cl_int sizeBlockMemory);
void **GetEnqueueReadBuffer(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingRead,
                            size_t offsetRead, size_t sizeBlock);
void **GetEnqueueWriteBuffer(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingWrite,
                             size_t offsetWrite, size_t sizeBlock);
void **GetEnqueueReadBufferRect(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingRead,
                                const size_t *offsetXYZDataBlock, const size_t *offsetXYZBufferBuffer, const size_t *regionRead,
                                size_t lengthColumnsPerRowInDataBlock, size_t sizeAreaDataBlock, size_t lengthColumnsPerRowInBuffer,
                                size_t sizeAreaBuffer);
void **GetEnqueueWriteBufferRect(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingWrite,
                                 const size_t *offsetXYZDataBlock, const size_t *offsetXYZBuffer, const size_t *regionWrite,
                                 size_t lengthColumnsPerRowInDataBlock, size_t sizeAreaDataBlock, size_t lengthColumnsPerRowInBuffer,
                                 size_t sizeAreaBuffer);
void **GetEnqueueCopyBuffer(cl_int currentCommandQueue, cl_int positionSrcBuffer, cl_int positionDstBuffer, size_t offsetSrcBuffer,
                            size_t offsetDstBuffer, size_t sizeBlock);
void **GetEnqueueCopyBufferRect(cl_int currentCommandQueue, cl_int positionSrcBuffer, cl_int positionDstBuffer,
                                const size_t *offsetSrcBufferXYZ, const size_t *offsetDstBufferXYZ, const size_t *regionCopy,
                                size_t lengthColumnsPerRowInSrcBuffer, size_t sizeAreaSrcBuffer, size_t lengthColumnsPerRowInDstBuffer,
                                size_t sizeAreaDstBuffer);
void **GetEnqueueFillBuffer(cl_int currentCommandQueue, cl_int currentBuffer, const void *patternFill, size_t sizeFillPattern,
                            size_t offsetFill, size_t sizeAreaFillInBuffer);
void **GetEnqueueMapBuffer(cl_int currentCommandQueue, cl_int currentBuffer, cl_bool isBlockingMap, cl_map_flags flagsMapBuffer,
                           size_t offsetMap, size_t sizeMap);
void **GetEnqueueUnmapBuffer(cl_int currentCommandQueue,cl_int currentBuffer);
void **GetEnqueueMigrationBuffer(cl_int currentCommandQueue, cl_int *listBuffersMigration, cl_int sizeList, cl_int flagsMigrationBuffers);
void **SetBufferDestructorCallback(JNIEnv *env, cl_int currentBuffer, jobject callbackBuffer, jobject buffer);
void **GetReleaseBuffer(cl_int currentBuffer, cl_bool isSubBuffer, cl_int currentBufferRegion);
void **GetSVMAlloc(cl_int currentContext, cl_svm_mem_flags flagsMemory, size_t sizeBlock, cl_uint numberBytesPerData);
void **GetSVMFree(cl_int currentContext, cl_int currentSVMMemory);
void **GetEnqueueSVMFree(JNIEnv *env, jobject commandQueue, cl_uint numberBlocksMemory, cl_int *listMemory, cl_int *sizeBlocksMemory,
                         cl_int *flagsBlocksMemory, jobject callbackMemory);
void **GetEnqueueSVMMemcpy(cl_int currentCommandQueue, cl_bool isBlockingCopy, cl_int positionDstMemory, cl_int positionSrcMemory);
void **GetEnqueueSVMMemFill(cl_int currentCommandQueue, cl_int currentMemory, const void *pattern, size_t sizePattern,
                            size_t sizeBlockToFill);
void **GetEnqueueSVMMap(cl_int currentCommandQueue, cl_int currentMemory, cl_bool isBlockingMap, cl_int sizeBlockMap,
                        cl_map_flags flagsMap);
void **GetEnqueueSVMUnmap(cl_int currentCommandQueue, cl_int currentMemory);
void **GetEnqueueSVMMigrateMem(cl_int currentCommandQueue, cl_uint numberBlocksMemory, cl_int *currentListMemory,
                               cl_mem_migration_flags flagsMigrateMemory);
void **GetCreateEvent(cl_int currentContext);
void **SetEventStatus(cl_int currentEvent, cl_int eventStatus);
void **GetWaitForEvents(cl_int numEvents, cl_int *positionsListEvents);
void **GetEventInfo(cl_int currentEvent, cl_event_info infoEvent, cl_int *dataInfoEvent);
void **SetEventCallback(JNIEnv *env, jobject event, cl_int executionStatus, jobject callbackEvent);
void **GetRetainEvent(cl_int currentEvent);
void **GetReleaseEvent(cl_int currentEvent);
void **GetEnqueueMarkerWithWaitList(cl_int currentCommandQueue, cl_int *positionListEvent, cl_int sizeListEvents);
void **GetEnqueueBarrierWithWaitList(cl_int currentCommandQueue, cl_int *positionListEvent, cl_int sizeListEvents);
void **GetEventProfilingInfo(cl_int currentEvent, cl_profiling_info profilingInfo, cl_ulong *infoRequest);
void **GetSupportedImageFormats(cl_int currentContext, cl_int typeAccessImage, cl_int typeImage);
void **GetCreateImage(cl_int currentContext, cl_mem_flags typeAccessImage, cl_int currentImageFormat, cl_int currentImageDescriptor,
                      jint currentDataImage, jbyte *dataImage);
void **GetEnqueueReadImage(cl_int currentCommandQueue, cl_int currentImage, cl_bool isBlockingRead, const size_t *origin,
                           const size_t *region, size_t numberBytesPerRow, size_t numberBytesPerLayer, cl_int currentBuffer);
void **GetEnqueueWriteImage(cl_int currentCommandQueue, cl_int currentImage, cl_bool isBlockingRead, const size_t *origin,
                            const size_t *region, size_t numberBytesPerRow, size_t numberBytesPerLayer, cl_int currentBuffer);
void **GetEnqueueCopyImage(cl_int currentCommandQueue, cl_int currentImageSrc, cl_int currentImageDst, const size_t *srcOrigin,
                           const size_t *dstOrigin, const size_t *region);
void **GetEnqueueFillImage(cl_int currentCommandQueue, cl_int currentImage, const void *fillColor, const size_t *origin,
                           const size_t *region);
void **GetEnqueueCopyImageToBuffer(cl_int currentCommandQueue, cl_int currentImage, cl_int currentBuffer, const size_t *origin,
                                   const size_t *region, size_t offsetCopy);
void **GetEnqueueCopyBufferToImage(cl_int currentCommandQueue, cl_int currentImage, cl_int currentBuffer, const size_t *origin,
                                   const size_t *region, size_t offsetCopy);
void **GetEnqueueMapImage(cl_int currentCommandQueue, cl_int currentImage, const size_t *origin, const size_t *region,
                          size_t *numberBytesPerRow, size_t *numberBytesPerLayer, cl_bool isBlockingMap, cl_map_flags flagMap);
void **GetEnqueueUnmapImage(cl_int currentCommandQueue, cl_int currentImage, cl_int currentBuffer);
void **GetEnqueueMigrateImage(cl_int currentCommandQueue, cl_int sizeList, cl_int *listImagesToMigrate, cl_mem_migration_flags flagMigrate);
void **GetReleaseImage(cl_int currentImage, cl_int currentImageFormat, cl_int currentImageDescriptor);
void **GetCreatePipe(cl_int currentContext, cl_mem_flags typeAccess, cl_uint packetSize, cl_uint numberPacket);
void **GetReleasePipe(cl_int currentPipe);
void **GetCreateSample(cl_int currentContext, cl_bool isNormalizedCoords, cl_int addressingMode, cl_int filterMode);
void **GetReleaseSample(cl_int currentSample);
void **GetCreateProgramWithSource(cl_int currentContext, cl_uint numberFiles, const char **filesSources, const size_t *sizeNameFilesSources);
void **GetCreateProgramWithIL(cl_int currentContext, jsize sizeArray, jbyte *dataProgramSPIRV);
void **GetCreateProgramWithBinaries(cl_int currentContext, cl_int *listDevices, cl_int *listSubDevices, jint sizeListDevices,
                                    size_t *sizeFilesBinaries, const unsigned char **filesBinaries, cl_int *isLoadFileSuccessfulOverDevice);
void **GetCreateProgramWithBuiltInKernels(cl_int currentContext, cl_uint numDevices, cl_int *listDevices, cl_int *listSubDevices,
                                          const char *listKernelNames);
void **GetReleaseProgram(cl_int currentProgram);
void **SetProgramReleaseCallback(JNIEnv *env, jobject program, jobject callbackProgram);
void **SetProgramSpecializationConstant(cl_int currentProgram, cl_uint specializationID, size_t sizeData, const void *data);
void **GetBuildProgram(JNIEnv *env, cl_int currentProgram, cl_uint numDevices, jint *_listDevices, jint *listSubDevices,
                       const char *buildOptions, jobject program, jobject callbackBuildProgram);
void **GetCompileProgram(JNIEnv *env, cl_int currentProgram, cl_uint numDevices, jint *_listDevices, jint *listSubDevices,
                         jint sizeListPrograms, jint *_listPrograms, const char *compileOptions, cl_uint numHeaderFiles,
                         const char **headerFiles, jobject program, jobject callbackCompileProgram);
void **GetLinkProgram(JNIEnv *env, cl_int currentContext, cl_uint numDevices, jint *_listDevices, jint *listSubDevices,
                      const char *linkOptions, cl_uint numPrograms, jint *_listPrograms, jobject program, jobject callbackLinkPrograms);
void **UnloadPlatformCompiler(cl_int currentPlatform);
void **GetProgramInfo(cl_int currentProgram, cl_program_info nameInfo, size_t *sizeData);
void **GetProgramBuildInfo(cl_int currentProgram, cl_int currentDevice, cl_int currentSubDevice, cl_program_build_info nameInfo,
                           size_t *sizeData);
void **GetCreateKernel(cl_int currentProgram, const char *kernelName);
void **GetCreateKernelsInProgram(cl_int currentProgram, cl_uint *sizeArray);
void **GetReleaseKernel(cl_int currentKernel);
void **SetKernelArg(cl_int currentKernel, cl_uint argumentIndex, size_t argumentSize, const void *value);
void **SetKernelArgumentsSVMPointer(cl_int currentKernel, cl_uint argumentIndex, const void *value);
void **SetKernelExecInfo(cl_int currentKernel, cl_uint execInfo, size_t sizeData, const void *data);
void **GetCloneKernel(cl_int currentKernel);
void **GetKernelInfo(cl_int currentKernel, cl_kernel_info infoName);
void **GetKernelWorkGroupInfo(cl_int currentKernel, cl_int currentDevice, cl_int currentSubDevice, cl_kernel_work_group_info nameInfo);
void **GetKernelSubGroupInfo(cl_int currentKernel, cl_int currentDevice, cl_int currentSubDevice, cl_kernel_sub_group_info nameInfo,
                             size_t sizeDataInputInBytes, const void *dataInput, size_t *sizeData);
void **GetKernelArgInfo(cl_int currentKernel, cl_uint argumentIndex, cl_kernel_arg_info nameInfo);
void **GetEnqueueNDRangeKernel(cl_int currentKernel, cl_int currenCommandQueue, cl_uint workNumberDimensions,
                               const size_t *globalWorkOffset, const size_t *globalWorkSize, const size_t *localWorkSize);
void **GetEnqueueNativeKernel(JNIEnv *env, cl_int currentCommandQueue, void *listArguments, size_t sizeListArguments,
                              cl_int sizeListMemObjects, const cl_mem *listMemObjects, const void **listMemObjectsAddress,
                              jobject functionCustom, jobjectArray argumentsFunctionCustom);
cl_platform_id getPlatformsIDs(cl_int position);
void addImageFormatToListImageFormat(cl_int imageChannelOrder, cl_int imageChannelDataType);
void addImageDescriptorToListImageDescriptor(cl_mem_object_type imageType, size_t imageWidth, size_t imageHeight, size_t imageDepth,
                                             size_t numberImages, size_t numberBytesPerRow, size_t numberBytesPerLayer,
                                             cl_int currentBuffer, cl_bool addBuffer);
void **getListImageFormatSupportByTypImage();
void getDataImageDescriptorSelected(cl_int currentImageDescriptor);
void processAddPlatforms(cl_platform_id *platforms);
void processAddDevices(cl_device_id *devices, cl_int numberDevices);
void processAddDevicePartitions(cl_device_id *listDevicePartitions, cl_int numberPartitions);
void processAddContext(cl_context context);
void processReleaseContext(cl_int currentContext);
void processAddCommandQueue(cl_command_queue commandQueue);
void processReleaseCommandQueue(cl_int currentCommandQueue);
void processCreateBuffer(cl_mem buffer);
void processCreateSubBuffer(cl_mem buffer, cl_buffer_region bufferRegion);
void processReleaseBuffer(cl_int currentBuffer, cl_bool isSubBuffer, cl_int currentBufferRegion);
void processSVMFree(cl_int *listMemory, cl_int numberBlocksMemory);
void processAddEventToListEvent(cl_event event);
void processReleaseEvent(cl_int currentEvent);
void processAddImageToListImages(cl_mem image);
void processAddListImageFormats(cl_image_format *list, cl_int sizeList);
void processReleaseImage(cl_int currentImage, cl_int currentImageFormat, cl_int currentImageDescriptor);
void processAddPipe(cl_mem pipe);
void processReleasePipe(cl_int currentPipe);
void processAddSampler(cl_sampler sampler);
void processReleaseSampler(cl_int currentSampler);
void processAddProgram(cl_program program);
void processReleaseProgram(cl_int currentProgram);
void processAddKernel(cl_kernel kernel);
void processReleaseKernel(cl_int currentKernel);
void CL_CALLBACK notifyCallbackContext(const char *errorInfo, const void *privateInfo, size_t cb, void *userData);
void CL_CALLBACK notifyCallbackEvent(cl_event eventCallback, cl_int statusCommandExecution, void *userData);
void CL_CALLBACK notifyCallbackMemory(cl_command_queue commandQueue, cl_uint numBlocksMemory, void **listDataMemory, void *userData);
void CL_CALLBACK notifyCallbackBuffer(cl_mem buffer, void *userData);
void CL_CALLBACK notifyCallbackProgram(cl_program program, void *userData);
void CL_CALLBACK functionNativeKernel(void *data);
extern _clGetPlatformIDs CLGetPlatformIDs;
extern _clGetPlatformInfo CLGetPlatformInfo;
extern _clGetDeviceIDs CLGetDeviceIDs;
extern _clGetDeviceInfo CLGetDeviceInfo;
extern _clGetDeviceAndHostTimer CLGetDeviceAndHostTimer;
extern _clGetHostTimer CLGetHostTimer;
extern _clCreateSubDevices CLCreateSubDevices;
extern _clRetainDevice CLRetainDevice;
extern _clReleaseDevice CLReleaseDevice;
extern _clCreateContext CLCreateContext;
extern _clCreateContextFromType CLCreateContextFromType;
extern _clRetainContext CLRetainContext;
extern _clReleaseContext CLReleaseContext;
extern _clGetContextInfo CLGetContextInfo;
extern _notifyMessageContext notifyMessageContext;
extern cl_int positionCurrentDevice;
extern jint positionCurrentContext;
extern jint positionCurrentCommandQueue;
extern jint positionCurrentBuffer;
extern jint positionCurrentSubBuffer;
extern jint positionCurrentBufferRegion;
extern jint positionCurrentMemory;
extern jint positionCurrentEvent;
extern jint positionCurrentImage;
extern jint positionCurrentImageFormat;
extern jint positionCurrentImageDescriptor;
extern jint positionCurrentPipe;
extern jint positionCurrentSampler;
extern jint positionCurrentProgram;
extern jint positionCurrentKernel;
extern cl_platform_id *listPlatforms[10];
extern cl_device_id *listDevices[20000];
extern cl_context *listContexts[10];
extern cl_command_queue *listCommandQueue[100];
extern jbyte *listDataMemory[100];
extern cl_event listEvents[100];
extern cl_mem *listBuffers[1000];
extern cl_buffer_region *listSubBuffersRegion[1000];
extern cl_mem *listImage[100];
extern cl_sampler *listSamplers[100];
extern cl_mem *listPipes[100];
extern jbyte *listDataMemory[100];
extern JavaVM *jvm;
extern void *dataInfo[1];
extern void *dataImageDescriptor[9];
extern char *dataFile;
extern unsigned char **dataBinaryFiles;
extern size_t *sizeFiles;
extern size_t numKernels;
extern cl_bool dataProgramPresent;
extern char *dataBuildProgram;
extern char *dataKernel;
extern size_t *dataSizeTKernel;
extern cl_uint dataIntKernel;
extern cl_ulong dataLongKernel;
extern cl_bitfield dataBitfieldKernel;
extern cl_build_status buildStatus;
extern cl_program_binary_type programBinaryType;
extern size_t sizeProgramVariables;
extern _clCreateCommandQueueWithProperties CLCreateCommandQueueWithProperties;
extern _clSetDefaultDeviceCommandQueue CLSetDefaultDeviceCommandQueue;
extern _clRetainCommandQueue CLRetainCommandQueue;
extern _clReleaseCommandQueue CLReleaseCommandQueue;
extern _clFlush CLFlush;
extern _clFinish CLFinish;
extern _clGetCommandQueueInfo CLGetCommandQueueInfo;
extern _clCreateBuffer CLCreateBuffer;
extern _clCreateSubBuffer CLCreateSubBuffer;
extern _clEnqueueReadBuffer CLEnqueueReadBuffer;
extern _clEnqueueWriteBuffer CLEnqueueWriteBuffer;
extern _clEnqueueReadBufferRect CLEnqueueReadBufferRect;
extern _clEnqueueWriteBufferRect CLEnqueueWriteBufferRect;
extern _clEnqueueCopyBuffer CLEnqueueCopyBuffer;
extern _clEnqueueCopyBufferRect CLEnqueueCopyBufferRect;
extern _clEnqueueFillBuffer CLEnqueueFillBuffer;
extern _clEnqueueMapBuffer CLEnqueueMapBuffer;
extern _clReleaseBuffer CLReleaseBuffer;
extern _clCreateUserEvent CLCreateUserEvent;
extern _clSetUserEventStatus CLSetUserEventStatus;
extern _clWaitForEvents CLWaitForEvents;
extern _clGetEventInfo CLGetEventInfo;
extern _clSetEventCallback CLSetEventCallback;
extern _clRetainEvent CLRetainEvent;
extern _clReleaseEvent CLReleaseEvent;
extern _clEnqueueMarkerWithWaitList CLEnqueueMarkerWithWaitList;
extern _clEnqueueBarrierWithWaitList CLEnqueueBarrierWithWaitList;
extern _clGetEventProfilingInfo CLGetEventProfilingInfo;
extern _clSVMAlloc CLSVMAlloc;
extern _clSVMFree CLSVMFree;
extern _clEnqueueSVMFree CLEnqueueSVMFree;
extern _clEnqueueSVMMemcpy CLEnqueueSVMMemcpy;
extern _clEnqueueSVMMemFill CLEnqueueSVMMemFill;
extern _clEnqueueSVMMap CLEnqueueSVMMap;
extern _clEnqueueSVMUnmap CLEnqueueSVMUnmap;
extern _clEnqueueSVMMigrateMem CLEnqueueSVMMigrateMem;
extern _clCreateImage CLCreateImage;
extern _clGetSupportedImageFormats CLGetSupportedImageFormats;
extern _clEnqueueReadImage CLEnqueueReadImage;
extern _clEnqueueWriteImage CLEnqueueWriteImage;
extern _clEnqueueCopyImage CLEnqueueCopyImage;
extern _clEnqueueFillImage CLEnqueueFillImage;
extern _clEnqueueCopyImageToBuffer CLEnqueueCopyImageToBuffer;
extern _clEnqueueCopyBufferToImage CLEnqueueCopyBufferToImage;
extern _clEnqueueMapImage CLEnqueueMapImage;
extern _clGetImageInfo CLGetImageInfo;
extern _clCreatePipe CLCreatePipe;
extern _clGetPipeInfo CLGetPipeInfo;
extern _clRetainMemObject CLRetainMemObject;
extern _clReleaseMemObject CLReleaseMemObject;
extern _clEnqueueUnmapMemObject CLEnqueueUnmapMemObject;
extern _clEnqueueMigrateMemObjects CLEnqueueMigrateMemObjects;
extern _clSetMemObjectDestructorCallback CLSetMemObjectDestructorCallback;
extern _clCreateSamplerWithProperties CLCreateSamplerWithProperties;
extern _clRetainSampler CLRetainSampler;
extern _clReleaseSampler CLReleaseSampler;
extern _clGetSamplerInfo CLGetSamplerInfo;
extern _clCreateProgramWithSource CLCreateProgramWithSource;
extern _clCreateProgramWithIL CLCreateProgramWithIL;
extern _clCreateProgramWithBinary CLCreateProgramWithBinary;
extern _clCreateProgramWithBuiltInKernels CLCreateProgramWithBuiltInKernels;
extern _clRetainProgram CLRetainProgram;
extern _clReleaseProgram CLReleaseProgram;
extern _clSetProgramReleaseCallback CLSetProgramReleaseCallback;
extern _clSetProgramSpecializationConstant  CLSetProgramSpecializationConstant;
extern _clBuildProgram CLBuildProgram;
extern _clCompileProgram CLCompileProgram;
extern _clLinkProgram CLLinkProgram;
extern _clUnloadPlatformCompiler CLUnloadPlatformCompiler;
extern _clGetProgramInfo CLGetProgramInfo;
extern _clGetProgramBuildInfo CLGetProgramBuildInfo;
extern _clCreateKernel CLCreateKernel;
extern _clCreateKernelsInProgram CLCreateKernelsInProgram;
extern _clRetainKernel CLRetainKernel;
extern _clReleaseKernel CLReleaseKernel;
extern _clSetKernelArg CLSetKernelArg;
extern _clSetKernelArgSVMPointer CLSetKernelArgSVMPointer;
extern _clSetKernelExecInfo CLSetKernelExecInfo;
extern _clCloneKernel CLCloneKernel;
extern _clGetKernelInfo CLGetKernelInfo;
extern _clGetKernelWorkGroupInfo CLGetKernelWorkGroupInfo;
extern _clGetKernelSubGroupInfo CLGetKernelSubGroupInfo;
extern _clGetKernelArgInfo CLGetKernelArgInfo;
extern _clEnqueueNDRangeKernel CLEnqueueNDRangeKernel;
extern _clEnqueueNativeKernel CLEnqueueNativeKernel;
#endif