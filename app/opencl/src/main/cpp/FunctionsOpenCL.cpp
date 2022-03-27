#include <cstring>
#include <stdlib.h>
#include <android/log.h>
#include <cstdlib>
#include <malloc.h>
#include "FunctionsOpenCL.h"
#include "DataOpenCL.h"

_clGetPlatformIDs CLGetPlatformIDs;
_clGetPlatformInfo CLGetPlatformInfo;
_clGetDeviceIDs CLGetDeviceIDs;
_clGetDeviceInfo CLGetDeviceInfo;
_clGetDeviceAndHostTimer CLGetDeviceAndHostTimer;
_clGetHostTimer CLGetHostTimer;
_clCreateSubDevices CLCreateSubDevices;
_clRetainDevice CLRetainDevice;
_clReleaseDevice CLReleaseDevice;
_clCreateContext CLCreateContext;
_clCreateContextFromType CLCreateContextFromType;
_clRetainContext CLRetainContext;
_clReleaseContext CLReleaseContext;
_clGetContextInfo CLGetContextInfo;
_clCreateCommandQueueWithProperties CLCreateCommandQueueWithProperties;
_clSetDefaultDeviceCommandQueue CLSetDefaultDeviceCommandQueue;
_clRetainCommandQueue CLRetainCommandQueue;
_clReleaseCommandQueue CLReleaseCommandQueue;
_clFlush CLFlush;
_clFinish CLFinish;
_clGetCommandQueueInfo CLGetCommandQueueInfo;
_clCreateBuffer CLCreateBuffer;
_clCreateSubBuffer CLCreateSubBuffer;
_clReleaseBuffer CLReleaseBuffer;
_clEnqueueReadBuffer CLEnqueueReadBuffer;
_clEnqueueWriteBuffer CLEnqueueWriteBuffer;
_clEnqueueReadBufferRect CLEnqueueReadBufferRect;
_clEnqueueWriteBufferRect CLEnqueueWriteBufferRect;
_clEnqueueCopyBuffer CLEnqueueCopyBuffer;
_clEnqueueCopyBufferRect CLEnqueueCopyBufferRect;
_clEnqueueFillBuffer CLEnqueueFillBuffer;
_clEnqueueMapBuffer CLEnqueueMapBuffer;
_clCreateUserEvent CLCreateUserEvent;
_clSetUserEventStatus CLSetUserEventStatus;
_clWaitForEvents CLWaitForEvents;
_clGetEventInfo CLGetEventInfo;
_clSetEventCallback CLSetEventCallback;
_clRetainEvent CLRetainEvent;
_clReleaseEvent CLReleaseEvent;
_clEnqueueMarkerWithWaitList CLEnqueueMarkerWithWaitList;
_clEnqueueBarrierWithWaitList CLEnqueueBarrierWithWaitList;
_clGetEventProfilingInfo CLGetEventProfilingInfo;
_clSVMAlloc CLSVMAlloc;
_clSVMFree CLSVMFree;
_clEnqueueSVMFree CLEnqueueSVMFree;
_clEnqueueSVMMemcpy CLEnqueueSVMMemcpy;
_clEnqueueSVMMemFill CLEnqueueSVMMemFill;
_clEnqueueSVMMap CLEnqueueSVMMap;
_clEnqueueSVMUnmap CLEnqueueSVMUnmap;
_clEnqueueSVMMigrateMem CLEnqueueSVMMigrateMem;
_clCreateImage CLCreateImage;
_clGetSupportedImageFormats CLGetSupportedImageFormats;
_clEnqueueReadImage CLEnqueueReadImage;
_clEnqueueWriteImage CLEnqueueWriteImage;
_clEnqueueCopyImage CLEnqueueCopyImage;
_clEnqueueFillImage CLEnqueueFillImage;
_clEnqueueCopyImageToBuffer CLEnqueueCopyImageToBuffer;
_clEnqueueCopyBufferToImage CLEnqueueCopyBufferToImage;
_clEnqueueMapImage CLEnqueueMapImage;
_clGetImageInfo CLGetImageInfo;
_clCreatePipe CLCreatePipe;
_clGetPipeInfo CLGetPipeInfo;
_clRetainMemObject CLRetainMemObject;
_clReleaseMemObject CLReleaseMemObject;
_clEnqueueUnmapMemObject CLEnqueueUnmapMemObject;
_clEnqueueMigrateMemObjects CLEnqueueMigrateMemObjects;
_clSetMemObjectDestructorCallback CLSetMemObjectDestructorCallback;
_clCreateSamplerWithProperties CLCreateSamplerWithProperties;
_clRetainSampler CLRetainSampler;
_clReleaseSampler CLReleaseSampler;
_clGetSamplerInfo CLGetSamplerInfo;
_clCreateProgramWithSource CLCreateProgramWithSource;
_clCreateProgramWithIL CLCreateProgramWithIL;
_clCreateProgramWithBinary CLCreateProgramWithBinary;
_clCreateProgramWithBuiltInKernels CLCreateProgramWithBuiltInKernels;
_clRetainProgram CLRetainProgram;
_clReleaseProgram CLReleaseProgram;
_clSetProgramReleaseCallback CLSetProgramReleaseCallback;
_clSetProgramSpecializationConstant  CLSetProgramSpecializationConstant;
_clBuildProgram CLBuildProgram;
_clCompileProgram CLCompileProgram;
_clLinkProgram CLLinkProgram;
_clUnloadPlatformCompiler CLUnloadPlatformCompiler;
_clGetProgramInfo CLGetProgramInfo;
_clGetProgramBuildInfo CLGetProgramBuildInfo;
_clCreateKernel CLCreateKernel;
_clCreateKernelsInProgram CLCreateKernelsInProgram;
_clRetainKernel CLRetainKernel;
_clReleaseKernel CLReleaseKernel;
_clSetKernelArg CLSetKernelArg;
_clSetKernelArgSVMPointer CLSetKernelArgSVMPointer;
_clSetKernelExecInfo CLSetKernelExecInfo;
_clCloneKernel CLCloneKernel;
_clGetKernelInfo CLGetKernelInfo;
_clGetKernelWorkGroupInfo CLGetKernelWorkGroupInfo;
_clGetKernelSubGroupInfo CLGetKernelSubGroupInfo;
_clGetKernelArgInfo CLGetKernelArgInfo;
_clEnqueueNDRangeKernel CLEnqueueNDRangeKernel;
_clEnqueueNativeKernel CLEnqueueNativeKernel;
_notifyMessageContext notifyMessageContext;
void *Result[2];
char *dataFile;
unsigned char **dataBinaryFiles;
char *dataKernel;
size_t *dataSizeTKernel;
cl_uint dataIntKernel;
cl_ulong dataLongKernel;
cl_bitfield dataBitfieldKernel;
size_t *sizeFiles;
size_t numKernels;
cl_bool dataProgramPresent;
char *dataBuildProgram;
cl_build_status buildStatus;
cl_program_binary_type programBinaryType;
size_t sizeProgramVariables;
cl_platform_id *listPlatforms[10];
cl_device_id *listDevices[20000];
cl_context *listContexts[10];
cl_command_queue *listCommandQueue[100];
cl_mem *listBuffers[1000];
cl_buffer_region *listSubBuffersRegion[1000];
cl_event listEvents[100];
cl_mem *listImage[100];
cl_image_format *listImageFormat[100];
cl_image_desc *listImageDescriptor[100];
cl_mem *listPipes[100];
cl_sampler *listSamplers[100];
cl_program *listPrograms[100];
cl_kernel *listKernels[100];
jbyte *listDataMemory[100];
cl_int listNumbersDevices[20000];
cl_int listSizeBlocksMemory[100];
void *dataInfo[1];
void *dataListImageFormat[2];
void *dataFunctionNativeKernel[3];
void *dataImageDescriptor[9];
JavaVM *jvm;
jobject _callbackContext;
jobject _methodNofityContext;
jobject eventCallback;
jobject _callbackEvent;
jobject methodNotify;
jobject objectProgram;
jobject objectCallbackProgram;
jobject methodNotifyProgram;
cl_int positionCurrentDevice = 0;
jint positionCurrentContext = 0;
jint positionCurrentCommandQueue = 0;
jint positionCurrentBuffer = 0;
jint positionCurrentSubBuffer = 0;
jint positionCurrentBufferRegion = 0;
jint positionCurrentMemory;
jint positionCurrentEvent = 0;
jint positionCurrentImage = 0;
jint positionCurrentImageFormat = 0;
jint positionCurrentImageDescriptor = 0;
jint positionCurrentPipe = 0;
jint positionCurrentSampler = 0;
jint positionCurrentProgram = 0;
jint positionCurrentKernel = 0;
char *message;
cl_int state;

void **GetPlatformsIDs(cl_uint *numPlatforms) {
    state = CLGetPlatformIDs(NULL, NULL, numPlatforms);
    cl_platform_id  *platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * *numPlatforms);
    switch(state) {
        case CL_SUCCESS:
            state = CLGetPlatformIDs(*numPlatforms, platforms, NULL);
            switch(state) {
                case CL_SUCCESS:
                    processAddPlatforms(platforms);
                    break;
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 191);
                    strcpy(message, "It's impossible to get the id's of the platforms available on this android device, ");
                    strcat(message, "because in the CLGetPLatformIDs method the arguments platforms and num_platforms are ");
                    strcat(message, "assigned NULL");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 100);
                    strcpy(message, "The android device has run out of ram memory to get the id's of the platforms present");
                    break;
            }
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 191);
            strcpy(message, "It's impossible to get the id's of the platforms available on this android device, ");
            strcat(message, "because in the CLGetPLatformIDs method the arguments platforms and num_platforms are ");
            strcat(message, "assigned NULL");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "The android device has run out of ram memory to get the id's of the platforms present");
            break;
    }
    Result[0] = (void*)message;
    Result[1] = (void*)&state;
    return Result;
}
void **GetPlatformInfo(cl_int currentPlatform, cl_platform_info platformInfo) {
    cl_platform_id platform = *listPlatforms[currentPlatform];
    size_t sizeInfo;
    state = CLGetPlatformInfo(platform, platformInfo, NULL, NULL, &sizeInfo);
    switch(state) {
        case CL_SUCCESS:
            if (platformInfo != CL_PLATFORM_HOST_TIMER_RESOLUTION) {
                char *data = (char*)malloc(sizeof(char) * sizeInfo);
                state = CLGetPlatformInfo(platform, platformInfo, sizeInfo, data, NULL);
                dataInfo[0] = data;
            } else {
                cl_ulong timeStamp;
                state = CLGetPlatformInfo(platform, platformInfo, sizeInfo, &timeStamp, NULL);
                dataInfo[0] = &timeStamp;
            }
            switch (state) {
                case CL_INVALID_PLATFORM:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 33);
                    strcpy(message, "The current platform is not valid");
                    break;
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 281);
                    strcpy(message, "Check if the size provided to obtain the requested data of the current platform, ");
                    strcat(message, "is not less than the platform provided, also verify that the values of the size ");
                    strcat(message, "provided by the platform and the variable that will save the requested information ");
                    strcat(message, "from the platform are not NULL values");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 153);
                    strcpy(message, "While the requested information of the current platform was being processed, the ");
                    strcat(message, "android device ran out of available memory ram to complete the operation");
                    break;
            }
            break;
        case CL_INVALID_PLATFORM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 33);
            strcpy(message, "The current platform is not valid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 281);
            strcpy(message, "Check if the size provided to obtain the requested data of the current platform, ");
            strcat(message, "is not less than the platform provided, also verify that the values of the size ");
            strcat(message, "provided by the platform and the variable that will save the requested information ");
            strcat(message, "from the platform are not NULL values");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 153);
            strcpy(message, "While the requested information of the current platform was being processed, the ");
            strcat(message, "android device ran out of available memory ram to complete the operation");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetDeviceID(cl_int currentPlatform, cl_long DeviceType, cl_uint *numDevices) {
    state = CLGetDeviceIDs(*listPlatforms[currentPlatform], DeviceType, NULL, NULL, numDevices);
    cl_device_id *devices = (cl_device_id*)malloc(sizeof(cl_device_id) * *numDevices);
    switch(state) {
        case CL_SUCCESS:
            state = CLGetDeviceIDs(*listPlatforms[currentPlatform], DeviceType, *numDevices, devices, NULL);
            switch(state) {
                case CL_SUCCESS:
                    processAddDevices(devices, *numDevices);
                    break;
                case CL_INVALID_PLATFORM:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 34);
                    strcpy(message, "The provided platform is not valid");
                    break;
                case CL_INVALID_DEVICE_TYPE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 37);
                    strcpy(message, "The device type provided is not valid");
                    break;
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 131);
                    strcpy(message, "the \"num_devices\" and \"devices\" variables can not have null values at the same time,");
                    strcat(message, " check both arguments the CLGetDeviceIDs method");
                    break;
                case CL_DEVICE_NOT_FOUND:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 858);
                    strcpy(message, "The device you are trying to use can not use the OpenCL library, try to use another type ");
                    strcat(message, "of device such as Device.DEVICE_TYPE_CPU, Device.DEVICE_TYPE_GPU, ");
                    strcat(message, "Device.DEVICE_TYPE_DEFAULT, Device.DEVICE_TYPE_ACCELERATOR, Device.DEVICE_TYPE_CUSTOM or ");
                    strcat(message, "Device.DEVICE_TYPE_ALL, although any of these types of devices may not work on some devices, ");
                    strcat(message, "because if the gpu and cpu are integrated into the same chip, and assuming that your physical ");
                    strcat(message, "device (cell phone, tablet or computer) does not have more capacity to expand hardware, then ");
                    strcat(message, "only You can use the types Device.DEVICE_TYPE_GPU, Device.DEVICE_TYPE_DEFAULT and ");
                    strcat(message, "Device.DEVICE_TYPE_ALL, with which you can use the gpu by default, the other device types ");
                    strcat(message, "would not be accessible, it's just a matter of you researching the hardware of your physical ");
                    strcat(message, "device for you to use adequately the types of devices mentioned above");
                    break;
                case CL_OUT_OF_RESOURCES:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 119);
                    strcpy(message, "There was a problem during resource allocation requested by the OpenCL library for");
                    strcat(message, "device id of the device type provided");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 147);
                    strcpy(message, "The android device you are using has run out of ram memory to continue the requested");
                    strcat(message, " operation, close applications that you have open and try again");
                    break;
            }
            break;
        case CL_INVALID_PLATFORM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 34);
            strcpy(message, "The provided platform is not valid");
            break;
        case CL_INVALID_DEVICE_TYPE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 37);
            strcpy(message, "The device type provided is not valid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 131);
            strcpy(message, "the \"num_devices\" and \"devices\" variables can not have null values at the same time, check ");
            strcat(message, "both arguments the CLGetDeviceIDs method");
            break;
        case CL_DEVICE_NOT_FOUND:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 858);
            strcpy(message, "The device you are trying to use can not use the OpenCL library, try to use another type of ");
            strcat(message, "device such as Device.DEVICE_TYPE_CPU, Device.DEVICE_TYPE_GPU, Device.DEVICE_TYPE_DEFAULT, ");
            strcat(message, "Device.DEVICE_TYPE_ACCELERATOR, Device.DEVICE_TYPE_CUSTOM or Device.DEVICE_TYPE_ALL, although ");
            strcat(message, "any of these types of devices may not work on some devices, because if the gpu and cpu are ");
            strcat(message, "integrated into the same chip, and assuming that your physical device (cell phone, tablet or ");
            strcat(message, "computer) does not have more capacity to expand hardware, then only You can use the types ");
            strcat(message, "Device.DEVICE_TYPE_GPU, Device.DEVICE_TYPE_DEFAULT and Device.DEVICE_TYPE_ALL, with which you ");
            strcat(message, "can use the gpu by default, the other device types would not be accessible, it's just a matter ");
            strcat(message, "of you researching the hardware of your physical device for you to use adequately the types of ");
            strcat(message, "devices mentioned above");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 119);
            strcpy(message, "There was a problem during resource allocation requested by the OpenCL library for");
            strcat(message, "device id of the device type provided");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 147);
            strcpy(message, "The android device you are using has run out of ram memory to continue the requested");
            strcat(message, " operation, close applications that you have open and try again");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetDeviceInfo(cl_int currentDevice, cl_device_info DeviceInfo, cl_bool isDevicePartition, cl_int currentSubDevice) {
    cl_device_id device;
    size_t sizeDataInfoDevice;
    if (!isDevicePartition) device = listDevices[currentDevice][0];
    else device = listDevices[currentDevice][currentSubDevice];
    state = CLGetDeviceInfo(device, DeviceInfo, NULL, NULL, &sizeDataInfoDevice);
    switch(state) {
        case CL_SUCCESS:
            if (DeviceInfo == CL_DEVICE_TYPE || DeviceInfo == CL_DEVICE_MAX_MEM_ALLOC_SIZE || DeviceInfo == CL_DEVICE_SINGLE_FP_CONFIG ||
                DeviceInfo == CL_DEVICE_DOUBLE_FP_CONFIG || DeviceInfo == CL_DEVICE_GLOBAL_MEM_CACHE_SIZE ||
                DeviceInfo == CL_DEVICE_GLOBAL_MEM_SIZE || DeviceInfo == CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE ||
                DeviceInfo == CL_DEVICE_LOCAL_MEM_SIZE || DeviceInfo == CL_DEVICE_EXECUTION_CAPABILITIES ||
                DeviceInfo == CL_DEVICE_QUEUE_ON_HOST_PROPERTIES || DeviceInfo == CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES ||
                DeviceInfo == CL_DEVICE_PARTITION_AFFINITY_DOMAIN || DeviceInfo == CL_DEVICE_SVM_CAPABILITIES) {
                cl_ulong data1;
                state = CLGetDeviceInfo(device, DeviceInfo, sizeDataInfoDevice, &data1, NULL);
                dataInfo[0] = &data1;
            } else if (DeviceInfo == CL_DEVICE_VENDOR_ID || DeviceInfo == CL_DEVICE_MAX_COMPUTE_UNITS ||
                       DeviceInfo == CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS || DeviceInfo == CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR ||
                       DeviceInfo == CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT || DeviceInfo == CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT ||
                       DeviceInfo == CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG || DeviceInfo == CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT ||
                       DeviceInfo == CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE || DeviceInfo == CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF ||
                       DeviceInfo == CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR || DeviceInfo == CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT ||
                       DeviceInfo == CL_DEVICE_NATIVE_VECTOR_WIDTH_INT || DeviceInfo == CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG ||
                       DeviceInfo == CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT || DeviceInfo == CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE ||
                       DeviceInfo == CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF || DeviceInfo == CL_DEVICE_MAX_CLOCK_FREQUENCY ||
                       DeviceInfo == CL_DEVICE_ADDRESS_BITS || DeviceInfo == CL_DEVICE_MAX_READ_IMAGE_ARGS ||
                       DeviceInfo == CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS || DeviceInfo == CL_DEVICE_MAX_SAMPLERS ||
                       DeviceInfo == CL_DEVICE_IMAGE_PITCH_ALIGNMENT || DeviceInfo == CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT ||
                       DeviceInfo == CL_DEVICE_MAX_PIPE_ARGS || DeviceInfo == CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS ||
                       DeviceInfo == CL_DEVICE_PIPE_MAX_PACKET_SIZE || DeviceInfo == CL_DEVICE_MEM_BASE_ADDR_ALIGN ||
                       DeviceInfo == CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE || DeviceInfo == CL_DEVICE_MAX_CONSTANT_ARGS ||
                       DeviceInfo == CL_DEVICE_GLOBAL_MEM_CACHE_TYPE || DeviceInfo == CL_DEVICE_LOCAL_MEM_TYPE ||
                       DeviceInfo == CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE || DeviceInfo == CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE ||
                       DeviceInfo == CL_DEVICE_MAX_ON_DEVICE_QUEUES || DeviceInfo == CL_DEVICE_MAX_ON_DEVICE_EVENTS ||
                       DeviceInfo == CL_DEVICE_PARTITION_MAX_SUB_DEVICES || DeviceInfo == CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT ||
                       DeviceInfo == CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT ||
                       DeviceInfo == CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT || DeviceInfo == CL_DEVICE_MAX_NUM_SUB_GROUPS) {
                cl_uint data2;
                state = CLGetDeviceInfo(device, DeviceInfo, sizeDataInfoDevice, &data2, NULL);
                dataInfo[0] = &data2;
            } else if (DeviceInfo == CL_DEVICE_PARTITION_PROPERTIES || DeviceInfo == CL_DEVICE_PARTITION_TYPE) {
                cl_device_partition_property data3;
                state = CLGetDeviceInfo(device, DeviceInfo, sizeDataInfoDevice, &data3, NULL);
                dataInfo[0] = &data3;
            } else if (DeviceInfo == CL_DEVICE_MAX_WORK_GROUP_SIZE || DeviceInfo == CL_DEVICE_MAX_WORK_GROUP_SIZE ||
                       DeviceInfo == CL_DEVICE_IMAGE2D_MAX_WIDTH || DeviceInfo == CL_DEVICE_IMAGE2D_MAX_HEIGHT ||
                       DeviceInfo == CL_DEVICE_IMAGE3D_MAX_WIDTH || DeviceInfo == CL_DEVICE_IMAGE3D_MAX_HEIGHT ||
                       DeviceInfo == CL_DEVICE_IMAGE3D_MAX_DEPTH || DeviceInfo == CL_DEVICE_IMAGE_MAX_BUFFER_SIZE ||
                       DeviceInfo == CL_DEVICE_IMAGE_MAX_ARRAY_SIZE || DeviceInfo == CL_DEVICE_MAX_PARAMETER_SIZE ||
                       DeviceInfo == CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE || DeviceInfo == CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE ||
                       DeviceInfo == CL_DEVICE_PROFILING_TIMER_RESOLUTION || DeviceInfo == CL_DEVICE_PRINTF_BUFFER_SIZE) {
                size_t *data4 = (size_t*)malloc(sizeof(size_t) * sizeDataInfoDevice);
                state = CLGetDeviceInfo(device, DeviceInfo, sizeDataInfoDevice, data4, NULL);
                dataInfo[0] = data4;
            } else if (DeviceInfo == CL_DEVICE_IMAGE_SUPPORT || DeviceInfo == CL_DEVICE_ERROR_CORRECTION_SUPPORT ||
                       DeviceInfo == CL_DEVICE_ENDIAN_LITTLE || DeviceInfo == CL_DEVICE_AVAILABLE || DeviceInfo == CL_DEVICE_COMPILER_AVAILABLE ||
                       DeviceInfo == CL_DEVICE_LINKER_AVAILABLE || DeviceInfo == CL_DEVICE_PREFERRED_INTEROP_USER_SYNC ||
                       DeviceInfo == CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS) {
                cl_bool data5;
                state = CLGetDeviceInfo(device, DeviceInfo, sizeDataInfoDevice, &data5, NULL);
                dataInfo[0] = &data5;
            } else if (DeviceInfo == CL_DEVICE_BUILT_IN_KERNELS || DeviceInfo == CL_DEVICE_NAME || DeviceInfo == CL_DEVICE_VENDOR ||
                       DeviceInfo == CL_DEVICE_VERSION || DeviceInfo == CL_DEVICE_PROFILE || DeviceInfo == CL_DRIVER_VERSION ||
                       DeviceInfo == CL_DEVICE_OPENCL_C_VERSION || DeviceInfo == CL_DEVICE_EXTENSIONS) {
                char *data6 = (char*)malloc(sizeof(char) * sizeDataInfoDevice);
                state = CLGetDeviceInfo(device, DeviceInfo, sizeDataInfoDevice, data6, NULL);
                dataInfo[0] = data6;
            }
            switch(state) {
                case CL_INVALID_DEVICE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 36);
                    strcpy(message, "The device you provided is incorrect");
                    break;
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 220);
                    strcpy(message, "Check if the variable SizeDataDeviceSelected does not have a NULL value or 0, and also ");
                    strcat(message, "check that the previous variable and the deviceInformation->Information[Position] ");
                    strcat(message, "variable are not NULL value both at the same time");
                    break;
                case CL_OUT_OF_RESOURCES:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 93);
                    strcpy(message, "The requested information could not be processed, because the system has run out of resources");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 85);
                    strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                    break;
            }
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 42);
            strcpy(message, "The id of the device provided is incorrect");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 220);
            strcpy(message, "Check if the variable SizeDataDeviceSelected does not have a NULL value or 0, and also check that the ");
            strcat(message, "previous variable and the deviceInformation->Information[Position] variable are not NULL value both ");
            strcat(message, "at the same time");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetDeviceAndHostTimer(cl_device_id device, cl_ulong *deviceTimeStamp, cl_ulong *hostTimestamp) {
    state = CLGetDeviceAndHostTimer(device, deviceTimeStamp, hostTimestamp);
    switch(state) {
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 36);
            strcpy(message, "The device you provided is incorrect");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 56);
            strcpy(message, "The timestamp obtained from the host device is NULL or 0");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetHostTimer(cl_device_id device, cl_ulong *hostTimeStamp) {
    state = CLGetHostTimer(device, hostTimeStamp);
    switch(state) {
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 42);
            strcpy(message, "The id of the device provided is incorrect");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 56);
            strcpy(message, "The timestamp obtained from the host device is NULL or 0");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateSubDevices(cl_device_id deviceIn, cl_device_partition_property partitionProperty,
                           cl_device_partition_property affinityDomainType, cl_uint maxSubDevicesPartitions, cl_uint maxComputeUnits,
                           cl_uint *numDevices) {
    cl_device_partition_property *dataList;
    cl_device_partition_property numberUnitsComputesForDevice = 0;
    cl_device_partition_property fragmentUnitsComputesForDevices = 0;
    cl_int numberDevicesToBePartitioned;
    if (maxComputeUnits == 1) numberDevicesToBePartitioned = numberUnitsComputesForDevice = maxComputeUnits;
    else {
        numberUnitsComputesForDevice = 2;
        numberDevicesToBePartitioned = maxComputeUnits / numberUnitsComputesForDevice;
        if (numberDevicesToBePartitioned <= maxSubDevicesPartitions) {
            fragmentUnitsComputesForDevices = maxComputeUnits % numberUnitsComputesForDevice;
        } else {
            numberUnitsComputesForDevice = 4;
            numberDevicesToBePartitioned = maxComputeUnits / numberUnitsComputesForDevice;
            while(true) {
                if (numberDevicesToBePartitioned > maxSubDevicesPartitions) {
                    numberUnitsComputesForDevice *= 2;
                    numberDevicesToBePartitioned = maxComputeUnits / numberUnitsComputesForDevice;
                } else break;
            }
            fragmentUnitsComputesForDevices = maxComputeUnits % numberUnitsComputesForDevice;
        }
    }
    if (partitionProperty == CL_DEVICE_PARTITION_EQUALLY) {
        cl_device_partition_property data[] = { partitionProperty, numberUnitsComputesForDevice, 0 };
        dataList = data;
    } else if (partitionProperty == CL_DEVICE_PARTITION_BY_COUNTS) {
        cl_int partitionFragmentsUnitsCompute = 0;
        if (fragmentUnitsComputesForDevices > 0) partitionFragmentsUnitsCompute++;
        cl_int sizeDataList = numberDevicesToBePartitioned + partitionFragmentsUnitsCompute + 3;
        dataList = (cl_device_partition_property*)malloc(sizeof(cl_device_partition_property) * sizeDataList);
        size_t size = sizeDataList - 2;
        for (cl_int position = 0; position < size; position++) {
            if (position == 0) dataList[position] = partitionProperty;
            else dataList[position] = numberUnitsComputesForDevice;
        }
        if (partitionFragmentsUnitsCompute > 0) {
            dataList[size] = fragmentUnitsComputesForDevices;
            dataList[size + 1] = CL_DEVICE_PARTITION_BY_COUNTS_LIST_END;
            dataList[size + 2] = 0;
            numberDevicesToBePartitioned++;
        } else {
            dataList[size] = CL_DEVICE_PARTITION_BY_COUNTS_LIST_END;
            dataList[size + 1] = 0;
        }
    } else if (partitionProperty == CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN) {
        cl_device_partition_property data[] = { partitionProperty, affinityDomainType, 0 };
        numberDevicesToBePartitioned = maxSubDevicesPartitions;
        dataList = data;
    }
    state = CLCreateSubDevices(deviceIn, dataList, numberDevicesToBePartitioned, NULL, numDevices);
    cl_device_id *devicesPartitionsTemp = (cl_device_id*)malloc(sizeof(cl_device_id) * *numDevices);
    switch(state) {
        case CL_SUCCESS:
            state = CLCreateSubDevices(deviceIn, dataList, *numDevices, devicesPartitionsTemp, NULL);
            switch(state) {
                case CL_SUCCESS:
                    processAddDevicePartitions(devicesPartitionsTemp, *numDevices);
                    break;
                case CL_INVALID_DEVICE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 131);
                    strcpy(message, "The device provided to try to partition that device into several sub-devices is invalid, ");
                    strcat(message, "try another device and try again");
                    break;
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 186);
                    strcpy(message, "The list of properties used internally in the createPartition method is invalid or the values ");
                    strcat(message, "specified in it are incorrect or are not supported by the device you are trying to partition");
                    break;
                case CL_DEVICE_PARTITION_FAILED:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 146);
                    strcpy(message, "The device you are trying to partition can not be partitioned with the type of partition you ");
                    strcat(message, "have selected, although it is supported by the device");
                    break;
                case CL_INVALID_DEVICE_PARTITION_COUNT:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 379);
                    strcpy(message, "The current device can not be partitioned, since it has a single processing unit (which ");
                    strcat(message, "can be consulted with the getMaxComputeUnits method which is inside the current device), a ");
                    strcat(message, "processing unit is usually a simple unit of the GPU although it can also be that the ");
                    strcat(message, "microprocessor has a core, if your current device only has one processing unit you will not ");
                    strcat(message, "be able to partition it");
                    break;
                case CL_OUT_OF_RESOURCES:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 93);
                    strcpy(message, "The requested information could not be processed, because the system has run out of resources");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 85);
                    strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                    break;
                }
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 131);
            strcpy(message, "The device provided to try to partition that device into several sub-devices is invalid, ");
            strcat(message, "try another device and try again");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 186);
            strcpy(message, "The list of properties used internally in the createPartition method is invalid or the values ");
            strcat(message, "specified in it are incorrect or are not supported by the device you are trying to partition");
            break;
        case CL_DEVICE_PARTITION_FAILED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 146);
            strcpy(message, "The device you are trying to partition can not be partitioned with the type of partition you ");
            strcat(message, "have selected, although it is supported by the device");
            break;
        case CL_INVALID_DEVICE_PARTITION_COUNT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 379);
            strcpy(message, "The current device can not be partitioned, since it has a single processing unit (which can be ");
            strcat(message, "consulted with the getMaxComputeUnits method which is inside the current device), a processing ");
            strcat(message, "unit is usually a simple unit of the GPU although it can also be that the microprocessor has a ");
            strcat(message, "core, if your current device only has one processing unit you will not be able to partition it");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleaseDevice(cl_int currentDevice, cl_bool isSubDevice, cl_int currentSubDevice) {
    cl_device_id deviceSelected;
    if (isSubDevice) deviceSelected = listDevices[currentDevice][currentSubDevice];
    else deviceSelected = listDevices[currentDevice][0];
    state = CLReleaseDevice(deviceSelected);
    switch(state) {
        case CL_SUCCESS:
            listNumbersDevices[positionCurrentDevice]--;
            if (listNumbersDevices[positionCurrentDevice] == 0) listDevices[positionCurrentDevice] = NULL;
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 69);
            strcpy(message, "The input device that has been provided is incorrect or has a problem");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateContext(JNIEnv *env, cl_int *currentDevice, cl_int *currentSubDevice, cl_int currentPlatform, jobject callbackContext,
                        cl_bool *isDevicePartition, cl_int sizeListDevices) {
    cl_context_properties platform = (cl_context_properties)listPlatforms[currentPlatform];
    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, platform, 0 };
    cl_device_id *devices = (cl_device_id*)malloc(sizeof(cl_device_id) * sizeListDevices);
    cl_context context;
    for (cl_int position = 0; position < sizeListDevices; position++) {
        if (isDevicePartition[position]) devices[position] = listDevices[currentDevice[position]][currentSubDevice[position]];
        else devices[position] = listDevices[currentDevice[position]][0];
    }
    if (callbackContext == NULL) {
        context = CLCreateContext(properties, numberDevicesInCurrentPlatform, devices, NULL, NULL, (cl_int*)Result[1]);
    } else {
        jclass CallbackContext = env->FindClass("com/draico/asvappra/opencl/listeners/CallbackContext");
        jmethodID notify = env->GetMethodID(CallbackContext, "notify", "(Ljava/lang/String;[BLjava/lang/Integer;)V");
        jobject methodNofity = env->ToReflectedMethod(CallbackContext, notify, JNI_FALSE);
        _callbackContext = env->NewGlobalRef(callbackContext);
        _methodNofityContext = env->NewGlobalRef(methodNotify);
        void *dataInfoContext[] = { env, &callbackContext, &methodNofity };
        context = CLCreateContext(properties, numberDevicesInCurrentPlatform, devices, notifyCallbackContext, dataInfoContext,
                                  (cl_int*)Result[1]);
    }
    switch(*(cl_int*)Result[1]) {
        case CL_SUCCESS:
            processAddContext(context);
            break;
        case CL_INVALID_PLATFORM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 186);
            strcpy(message, "The platform you are using is invalid, or the platform that you specified in the property list is ");
            strcat(message, "not a valid platform, or also if the argument of the property list assigned a NULL value");
            break;
        case CL_INVALID_PROPERTY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 293);
            strcpy(message, "The list of properties has a problem, because the data you are using to create it is incorrect, you ");
            strcat(message, "must use CL_CONTEXT_PLATFORM followed by the value of the platform you are currently working with, ");
            strcat(message, "and the list must end with a 0, only that data are the ones you can enter in the property list");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 358);
            strcpy(message, "The device used is null or was declared as null in the corresponding argument of the ");
            strcat(message, "clCreateContext, also the number of input devices may have been specified as 0 and this causing an ");
            strcat(message, "error, also check if the event function pointer is NULL in your corresponding argument in the ");
            strcat(message, "clCreateContext method, if so make sure that the user_data argument is also NULL");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 35);
            strcpy(message, "The device you are using is invalid");
            break;
        case CL_DEVICE_NOT_AVAILABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 92);
            strcpy(message, "The device you are trying to use is not currently available (may be busy in another process)");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    return Result;
}
void **GetCreateContextFromType(JNIEnv *env, cl_int currentPlatform, jobject callbackNotify, cl_ulong deviceType) {
    cl_context_properties platform = (cl_context_properties)getPlatformsIDs(currentPlatform);
    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, platform, 0 };
    cl_context context;
    if (callbackNotify == NULL) context = CLCreateContextFromType(properties, deviceType, NULL, NULL, (cl_int*)Result[1]);
    else {
        jclass CallbackContext = env->FindClass("com/draico/asvappra/opencl/listeners/CallbackContext");
        jmethodID notify = env->GetMethodID(CallbackContext, "notify", "(Ljava/lang/String;[BLjava/lang/Integer;)V");
        jobject methodNofity = env->ToReflectedMethod(CallbackContext, notify, JNI_FALSE);
        _methodNofityContext = env->NewGlobalRef(methodNofity);
        _callbackContext = env->NewGlobalRef(callbackNotify);
        void *dataInfoContext[] { env, &callbackNotify, &methodNofity };
        context = CLCreateContextFromType(properties, deviceType, notifyCallbackContext, dataInfoContext, (cl_int*)Result[1]);
    }
    switch(*(cl_int*)Result[1]) {
        case CL_SUCCESS:
            processAddContext(context);
            break;
        case CL_INVALID_PLATFORM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 186);
            strcpy(message, "The platform you are using is invalid, or the platform that you specified in the property list is ");
            strcat(message, "not a valid platform, or also if the argument of the property list assigned a NULL value");
            break;
        case CL_INVALID_PROPERTY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 595);
            strcpy(message, "The list of properties has a problem, since it was armed with names of properties of the context ");
            strcat(message, "that are not valid names, the valid names or values are CL_CONTEXT_PLATFORM and ");
            strcat(message, "CL_CONTEXT_INTEROP_USER_SYNC, also check that the data corresponding to each one of the valid values ");
            strcat(message, "of the list corresponds to the value used, the value that is used immediately after ");
            strcat(message, "CL_CONTEXT_PLATFORM is the id of the platform on which you are working, and the value that ");
            strcat(message, "accompanies CL_CONTEXT_INTEROP_USER_SYNC is a boolean, which if true means that the user ");
            strcat(message, "will be who properly synchronize user data and opencl");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 194);
            strcpy(message, "The pointer to the event function was set to NULL in the argument corresponding to the ");
            strcat(message, "clCreateContextFromType method, but the userData argument of the previous method is not set to a ");
            strcat(message, "NULL value");
            break;
        case CL_DEVICE_NOT_FOUND: case CL_INVALID_DEVICE_TYPE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 481);
            strcpy(message, "You are trying to create a context using a type of device not supported on your physical device, ");
            strcat(message, "remember that you have to use any of the types of devices that are inside the Device class, which ");
            strcat(message, "are the following: Device.DEVICE_TYPE_DEFAULT, Device. DEVICE_TYPE_CPU, Device.DEVICE_TYPE_GPU, ");
            strcat(message, "Device.DEVICE_TYPE_ACCELERATOR, Device.DEVICE_TYPE_CUSTOM and Device.DEVICE_TYPE_ALL (on most mobile ");
            strcat(message, "devices you can only use the types Device.DEVICE_TYPE_DEFAULT and Device.DEVICE_TYPE_GPU)");
            break;
        case CL_DEVICE_NOT_AVAILABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 92);
            strcpy(message, "The device you are trying to use is not currently available (may be busy in another process)");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    return Result;
}
void **GetReleaseContext(cl_int currentContext) {
    state = CLReleaseContext(*listContexts[currentContext]);
    switch(state) {
        case CL_SUCCESS:
            processReleaseContext(currentContext);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 271);
            strcpy(message, "The context you are trying to create is not valid, try using other devices or device types, ");
            strcat(message, "depending on whether you are using clCreateContext or clCreateContextFromType, you can also ");
            strcat(message, "try to use another platform if you have more than 1 physical device in which is working");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateCommandQueueWithProperties(cl_int currentContext, cl_int currentDevice, cl_int currentSubDevice,
                                           cl_bool isDevicePartition, cl_queue_properties *properties) {
    cl_context context = *listContexts[currentContext];
    cl_device_id device;
    if (isDevicePartition) device = listDevices[currentDevice][currentSubDevice];
    else device = *listDevices[currentDevice];
    cl_command_queue commandQueue = CLCreateCommandQueueWithProperties(context, device, properties, (cl_int*)Result[1]);
    switch(*(cl_int*)Result[1]) {
        case CL_SUCCESS:
            processAddCommandQueue(commandQueue);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 169);
            strcpy(message, "The context you are using to try to create a CommandQueue type object, is not a valid context, use ");
            strcat(message, "another context and try again to create an object of type CommandQueue");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 114);
            strcpy(message, "The device you are using is not a valid device, use another device to try to create an object of ");
            strcat(message, "type CommandQueue");
            break;
        case CL_INVALID_VALUE: case CL_INVALID_QUEUE_PROPERTIES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 492);
            strcpy(message, "The value that you have entered in the hashmap using the data pair with the key ");
            strcat(message, "CommandQueue.QUEUE_PROPERTIES is incorrect, the possible values that you can add are any of the ");
            strcat(message, "following (from which you can make combinations in the same value):");
            strcat(message, "\nCommandQueue.QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE\nCommandQueue.QUEUE_PROFILING_ENABLE");
            strcat(message, "\nCommandQueue.QUEUE_ON_DEVICE\nCommandQueue.QUEUE_ON_DEVICE_DEFAULT\nYou can not use other ");
            strcat(message, "values than the ones mentioned above next to the CommandQueue key .QUEUE_PROPERTIES");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **setDefaultDeviceCommandQueue(cl_int currentContext, cl_int currentDevice, cl_int currentSubDevice, cl_int currentCommandQueue,
                                    cl_bool isDevicePartition) {
    cl_device_id device;
    cl_context _context = *listContexts[currentContext];
    cl_command_queue _commandQueue = *listCommandQueue[currentCommandQueue];
    if (isDevicePartition) device = listDevices[currentDevice][currentSubDevice];
    else device = *listDevices[currentDevice];
    state = CLSetDefaultDeviceCommandQueue(_context, device, _commandQueue);
    switch(state) {
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 169);
            strcpy(message, "The context you are using to try to create a CommandQueue type object, is not a valid context, use ");
            strcat(message, "another context and try again to create an object of type CommandQueue");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 114);
            strcpy(message, "The device you are using is invalid, try to create another device or use another one that has ");
            strcat(message, "already been created");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 116);
            strcpy(message, "The context you are using is invalid, try to create another context or use another one that ");
            strcat(message, "has already been created");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleaseCommandQueue(cl_int currentCommandQueue) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    state = CLReleaseCommandQueue(commandQueue);
    switch(state) {
        case CL_SUCCESS:
            processReleaseCommandQueue(currentCommandQueue);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 124);
            strcpy(message, "The object of the CommandQueue type that you are working with is invalid, try to use another ");
            strcat(message, "object of the CommandQueue type");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetFlushCommandQueue(cl_int currentCommandQueue) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    state = CLFlush(commandQueue);
    switch(state) {
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 120);
            strcpy(message, "You cannot use the flush method to execute the operation that is queued on the CommandQueue ");
            strcat(message, "object because it is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetFinishCommandQueue(cl_int currentCommandQueue) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    state = CLFinish(commandQueue);
    switch(state) {
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 121);
            strcpy(message, "You cannot use the finish method to execute the operation that is queued on the CommandQueue ");
            strcat(message, "object because it is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateBuffer(cl_int currentContext, size_t sizeBuffer, cl_int currentMemory, cl_mem_flags flagsBuffer) {
    cl_context _currentContext = *listContexts[currentContext];
    jbyte *bufferData = listDataMemory[currentMemory];
    cl_mem buffer = CLCreateBuffer(_currentContext, flagsBuffer, sizeBuffer, bufferData, &state);
    switch(state) {
        case CL_SUCCESS:
            processCreateBuffer(buffer);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 96);
            strcpy(message, "The context you are trying to use to create the buffer is invalid, use or create another context");
            break;
        case CL_INVALID_BUFFER_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 115);
            strcpy(message, "The size for the buffer you want to create is inadequate, since with a size of 0 bytes it is ");
            strcat(message, "impossible to create it");
            break;
        case CL_INVALID_HOST_PTR:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 128);
            strcpy(message, "The buffer cannot be created because you did not use the Buffer.BUFFER_USE_HOST_PTR flag or the ");
            strcat(message, "Buffer.BUFFER_COPY_HOST_PTR flag");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 327);
            strcpy(message, "The buffer could not be created, since the size you provide exceeds the maximum size allowed for ");
            strcat(message, "the device you used to create the context. To know the maximum size allowed for the buffer use the ");
            strcat(message, "getMaxMemAllocSize method of the device that I use to create the context, the size returned by ");
            strcat(message, "the previous method will be in bytes");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateBufferWithArrayPrimitive(cl_int currentContext, size_t sizeBuffer, void *data, cl_mem_flags flagsBuffer) {
    cl_context _currentContext = *listContexts[currentContext];
    cl_mem buffer = CLCreateBuffer(_currentContext, flagsBuffer, sizeBuffer, data, &state);
    switch(state) {
        case CL_SUCCESS:
            processCreateBuffer(buffer);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 96);
            strcpy(message, "The context you are trying to use to create the buffer is invalid, use or create another context");
            break;
        case CL_INVALID_BUFFER_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 115);
            strcpy(message, "The size for the buffer you want to create is inadequate, since with a size of 0 bytes it is ");
            strcat(message, "impossible to create it");
            break;
        case CL_INVALID_HOST_PTR:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 128);
            strcpy(message, "The buffer cannot be created because you did not use the Buffer.BUFFER_USE_HOST_PTR flag or the ");
            strcat(message, "Buffer.BUFFER_COPY_HOST_PTR flag");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 327);
            strcpy(message, "The buffer could not be created, since the size you provide exceeds the maximum size allowed for ");
            strcat(message, "the device you used to create the context. To know the maximum size allowed for the buffer use the ");
            strcat(message, "getMaxMemAllocSize method of the device that I use to create the context, the size returned by ");
            strcat(message, "the previous method will be in bytes");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateSubBuffer(cl_int currentBufferSrc, cl_mem_flags flagsBuffer, cl_buffer_create_type flagSubBuffer,
                          cl_int sizeBlockMemory) {
    cl_mem _currentBufferSrc = *listBuffers[currentBufferSrc];
    cl_buffer_region *bufferRegion = (cl_buffer_region*)malloc(sizeof(cl_buffer_region));
    bufferRegion->origin = 0;
    bufferRegion->size = sizeBlockMemory;
    cl_mem buffer = CLCreateSubBuffer(_currentBufferSrc, flagsBuffer, flagSubBuffer, bufferRegion, &state);
    switch(state) {
        case CL_SUCCESS:
            processCreateSubBuffer(buffer, bufferRegion[0]);
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 128);
            strcpy(message, "The buffer you have provided is invalid, and therefore the sub-buffer can not be created, ");
            strcat(message, "try using another buffer and try again");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 431);
            strcpy(message, "The buffer you are using to try to create a sub-buffer, was created with a different flag ");
            strcat(message, "than the flag you provided for the source buffer, verify if the source buffer was created ");
            strcat(message, "with any of the following flags:\nBuffer.BUFFER_READ_WRITE\nBuffer. Buffer_WRITE_ONLY\n");
            strcat(message, "Buffer.BUFFER_READ_ONLY\nBuffer.BUFFER_USE_HOST_PTR\nBuffer.BUFFER_ALLOC_HOST_PTR\n");
            strcat(message, "Buffer.BUFFER_HOST_WRITE_ONLY\nBuffer.BUFFER_HOST_READ_ONLY\nBuffer.BUFFER_HOST_NO_ACCESS");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 154);
            strcpy(message, "Could not allocate memory for the sub-buffer you are trying to create, check the flags you ");
            strcat(message, "have used and also check if the source buffer is a valid buffer");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueReadBuffer(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingRead,
                            size_t offsetRead, size_t sizeBlock) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem _currentBuffer = *listBuffers[currentBuffer];
    jbyte *dataBuffer = listDataMemory[currentBlockData];
    cl_uint sizeListEvent = 0;
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    if (listEvents != NULL) sizeListEvent = positionCurrentEvent;
    state = CLEnqueueReadBuffer(_currentCommandQueue, _currentBuffer, isBlockingRead, offsetRead, sizeBlock, dataBuffer, sizeListEvent,
                                listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 186);
            strcpy(message, "The object of the CommandQueue type that you are using is invalid, use another object of the ");
            strcat(message, "CommandQueue type and request again the enqueue reading of the buffer that you have indicated");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 153);
            strcpy(message, "The context that you are using to glue the reading of the buffer provided is incorrect, use");
            strcat(message, "another context and request the enqueue reading of the buffer");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 174);
            strcpy(message, "The memory assigned to the buffer you are using is incorrect, invalid, or has a problem, so use ");
            strcat(message, "another buffer to which the requested memory block has been assigned correctly");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 446);
            strcpy(message, "The size of the buffer and/or the specified offset is incorrect, since it is exceeding the size of ");
            strcat(message, "the memory block with which the buffer was being used to enqueue the buffer reading, you can know ");
            strcat(message, "the size of the memory block using the bufferData object that is inside the buffer and thus obtain ");
            strcat(message, "the length of the memory blocks available inside the bufferData object, it is enough the use the ");
            strcat(message, "length attribute with any of the available data arrays");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 173);
            strcpy(message, "Any of the events generated by the commands you have been using is not valid, to know which event ");
            strcat(message, "and which command was the cause of the error use the CheckStatusEvent class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 415);
            strcpy(message, "The offset that you are using to read the buffer has exceeded the limit allowed by the buffer, ");
            strcat(message, "since the buffer you are using is a sub-buffer, so that you can know the limit allowed by the ");
            strcat(message, "buffer you will have to use the getMemBaseAddressAlign method, the previous method finds it ");
            strcat(message, "within the device you used to create the context and the CommandQueue type object that you are ");
            strcat(message, "using to enqueue the sub-buffer reading");
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 182);
            strcpy(message, "You queued a read or write operation to a buffer, but blocked the read or write buffer, to know ");
            strcat(message, "which command container is the one that has the problem use the CheckStatusEvent class");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 146);
            strcpy(message, "There was a problem when allocating memory for the reading of the buffer that has been enqueue by ");
            strcat(message, "the CommandQueue type command that you have used");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 307);
            strcpy(message, "The buffer that you are using to queue the reading of it has a problem, since at the moment of ");
            strcat(message, "creating the buffer you did it using the Buffer.BUFFER_WRITE_ONLY or Buffer.BUFFER_HHOST_NO_ACCESS ");
            strcat(message, "flag, so to solve this problem is to use another buffer or create one new but not having the flags ");
            strcat(message, "mentioned above");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueWriteBuffer(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingWrite,
                             size_t offsetWrite, size_t sizeBlock) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem _currentBuffer = *listBuffers[currentBuffer];
    jbyte *dataBuffer = listDataMemory[currentBlockData];
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueWriteBuffer(_currentCommandQueue, _currentBuffer, isBlockingWrite, offsetWrite, sizeBlock, dataBuffer,
                                 sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 186);
            strcpy(message, "The object of the CommandQueue type that you are using is invalid, use another object of the ");
            strcat(message, "CommandQueue type and request again the enqueue writing of the buffer that you have indicated");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 153);
            strcpy(message, "The context that you are using to glue the writing of the buffer provided is incorrect, use another ");
            strcat(message, "context and request the enqueue writing of the buffer");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 174);
            strcpy(message, "The memory assigned to the buffer you are using is incorrect, invalid, or has problem, so use ");
            strcat(message, "another buffer to which the requested memory block has been assigned correctly");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 446);
            strcpy(message, "The size of the buffer and/or the specified offset is incorrect, since it is exceeding the size ");
            strcat(message, "of the memory block with which the buffer was being used to enqueue the buffer reading, you can ");
            strcat(message, "know the size of the memory block using the bufferDaata object that is inside the buffer and ");
            strcat(message, "thus obtain the length of the memory blocks available inside the bufferData object, it is ");
            strcat(message, "enough to use the length attribute with any of the available data arrays");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 173);
            strcpy(message, "Any of the events generated by the commands you have been using is not valid, to know wich event ");
            strcat(message, "and which command was the cause of the error use the CheckStatusEvent class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 436);
            strcpy(message, "The offset you are using to write the buffer has exceeded the limit allowed by the buffer, since ");
            strcat(message, "the buffer you are using is a sub-buffer and so that you know the allowed limit of the offset that ");
            strcat(message, "you can use with this buffer use the getMemBaseAddressAlign method , the previous method can be ");
            strcat(message, "used from the device you used to create the context and the CommandQueue type object that you are ");
            strcat(message, "using to enqueue the writing of the sub-buffer");
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 182);
            strcpy(message, "You queued a read or write operation to a buffer, but blocked the read or write buffer, to know ");
            strcat(message, "which command container is the one that has the problem use the CheckStatusEvent class");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 146);
            strcpy(message, "There was a problem when allocating memory for the writing of the buffer that has been enqueue ");
            strcat(message, "by the CommandQueue type command that you have used");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 307);
            strcpy(message, "The buffer that you are using to queue the writing of it has a problem, since at the moment of ");
            strcat(message, "creating the buffer you did it using the Buffer.BUFFER_WRITE_ONLY or Buffer.BUFFER_HOST_NO_ACCESS ");
            strcat(message, "flag, so to solve this problem is to use another buffer or create one new but not having the flags ");
            strcat(message, "mentioned above");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueReadBufferRect(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingRead,
                                const size_t *offsetXYZDataBlock, const size_t *offsetXYZBuffer, const size_t *regionRead,
                                size_t bytesPerDataInDataBlock, size_t sizeAreaDataBlock, size_t bytesPerDataInBuffer,
                                size_t sizeAreaBuffer) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem _currentBuffer = *listBuffers[currentBuffer];
    jbyte *dataBuffer = listDataMemory[currentBlockData];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    cl_uint sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueReadBufferRect(_currentCommandQueue, _currentBuffer, isBlockingRead, offsetXYZDataBlock, offsetXYZBuffer,
                                    regionRead, bytesPerDataInDataBlock, sizeAreaDataBlock, bytesPerDataInBuffer, sizeAreaBuffer,
                                    dataBuffer, sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 161);
            strcpy(message, "The object of the CommandQueue type that you are using to enqueue the buffer read operation is ");
            strcat(message, "invalid, use another object of the CommandQueue type and try again");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 154);
            strcpy(message, "The context that you used to create the object of type CommandQueue and the buffer is not the same ");
            strcat(message, "context that is associated with the existing event list");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 143);
            strcpy(message, "The buffer that you are using for the enqueue reading is invalid, use another buffer and try again ");
            strcat(message, "to perform the enqueue reading of the buffer");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 174);
            strcpy(message, "The bytesPerDataInDataBlock variable is not a multiple of the sizeAreaDataBlock variable or the ");
            strcat(message, "bytesPerDataInBuffer variable is not a multiple of the sizeAreaBuffer variable");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "Any of the events generated by the commands you have been using is not valid, to know which event ");
            strcat(message, "and which command was the cause of the error use the CheckStatusEvent class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 308);
            strcpy(message, "The buffer memory block or dataBlock is incorrectly formatted, you have to use the ");
            strcat(message, "getMemBaseAddressAlign method of the device you used to create the context in which you are working, ");
            strcat(message, "to correctly create the buffer memory block and also to dataBlock, also check if the buffer you are ");
            strcat(message, "using is not a subbuffer");
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 182);
            strcpy(message, "You queued a read or write operation to a buffer, but blocked the read or write buffer, to know ");
            strcat(message, "which command container is the one that has the problem use the CheckStatusEvent class");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 146);
            strcpy(message, "There was a problem when allocating memory for the reading of the buffer that has been enqueue by ");
            strcat(message, "the CommandQueue type command that you have used");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 589);
            strcpy(message, "The memory block that you used to create the buffer you are using to read or write data was ");
            strcat(message, "created using any of the following Memory.MEM_HOST_READ_ONLY, Memory.MEM_HOST_WRITE_ONLY or ");
            strcat(message, "Memory.MEM_HOST_NO_ACCESS flags, if you are trying to read the memory block and it is you created ");
            strcat(message, "it using the write-only flag or without access to the memory block, you have to use another memory ");
            strcat(message, "block with the correct permissions using the correct flags, the same would happen when you want to ");
            strcat(message, "write data in the memory block and you created it in the read-only mode or without access to the ");
            strcat(message, "memory block");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueWriteBufferRect(cl_int currentCommandQueue, cl_int currentBuffer, cl_int currentBlockData, cl_bool isBlockingWrite,
                                 const size_t *offsetXYZDataBlock, const size_t *offsetXYZBuffer, const size_t *regionWrite,
                                 size_t lengthColumnsPerRowInDataBlock, size_t sizeAreaDataBlock, size_t lengthColumnsPerRowInBuffer,
                                 size_t sizeAreaBuffer) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem _currentBuffer = *listBuffers[currentBuffer];
    jbyte *bufferData = listDataMemory[currentBlockData];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    cl_int sizeListEvent = 0;
    if (listEvents != 0) sizeListEvent = positionCurrentEvent;
    state = CLEnqueueWriteBufferRect(_currentCommandQueue, _currentBuffer, isBlockingWrite, offsetXYZDataBlock, offsetXYZBuffer,
                                     regionWrite, lengthColumnsPerRowInDataBlock, sizeAreaDataBlock, lengthColumnsPerRowInBuffer,
                                     sizeAreaBuffer, bufferData, sizeListEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 159);
            strcpy(message, "The object of the CommandQueue type that you are using to enqueue the buffer write operation is ");
            strcat(message, "invalid, use another object of the CommandQueue type and try again");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 154);
            strcpy(message, "The context that you used to create the object of type CommandQueue and the buffer is not the same ");
            strcat(message, "context that is associated with the existing event list");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 177);
            strcpy(message, "The buffer that you are using to write to the buffer has a problem, since the buffer is invalid, ");
            strcat(message, "so you will have to use another buffer and retry the enqueue write to the buffer");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 174);
            strcpy(message, "The bytesPerDataInDataBlock variable is not a multiple of the sizeAreaDataBlock variable or the ");
            strcat(message, "bytesPerDataInBuffer variable is not a multiple of the sizeAreaBuffer variable");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "Any of the events generated by the commands you have been using is not valid, to know which event ");
            strcat(message, "and which command was the cause of the error use the CheckStatusEvent class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 308);
            strcpy(message, "The buffer memory block or dataBlock is incorrectly formatted, you have to use the ");
            strcat(message, "getMemBaseAddressAlign method of the device you used to create the context in which you are working, ");
            strcat(message, "to correctly create the buffer memory block and also to dataBlock, also check if the buffer you are ");
            strcat(message, "using is not a subbuffer");
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 182);
            strcpy(message, "You queued a read or write operation to a buffer, but blocked the read or write buffer, to know ");
            strcat(message, "which command container is the one that has the problem use the CheckStatusEvent class");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 146);
            strcpy(message, "There was a problem when allocating memory for the writing of the buffer that has been enqueue by ");
            strcat(message, "the CommandQueue type command that you have used");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 589);
            strcpy(message, "The memory block that you used to create the buffer you are using to read or write data was created ");
            strcat(message, "using any of the following Memory.MEM_HOST_READ_ONLY, Memory.MEM_HOST_WRITE_ONLY or ");
            strcat(message, "Memory.MEM_HOST_NO_ACCESS flags, if you are trying to read the memory block and it is you created ");
            strcat(message, "it using the write-only flag or without access to the memory block, you have to use another block ");
            strcat(message, "with the correct permissions using the correct flags, the same would happen when you want to write ");
            strcat(message, "data in the memory block and you created it in the read-only mode or without access to the memory ");
            strcat(message, "block");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueCopyBuffer(cl_int currentCommandQueue, cl_int positionSrcBuffer, cl_int positionDstBuffer, size_t offsetSrcBuffer,
                            size_t offsetDstBuffer, size_t sizeBlock) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem srcBuffer = *listBuffers[positionSrcBuffer];
    cl_mem dstBuffer = *listBuffers[positionDstBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    cl_int sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueCopyBuffer(_currentCommandQueue, srcBuffer, dstBuffer, offsetSrcBuffer, offsetDstBuffer, sizeBlock, sizeListEvents,
                                listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 193);
            strcpy(message, "The CommandQueue type object that you are using to copy the contents of a buffer to another buffer ");
            strcat(message, "is invalid, so you must use another CommandQueue type object and try to cop the buffers again");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 237);
            strcpy(message, "The context you used to create one or both buffers is invalid, find which buffer is the one that ");
            strcat(message, "has an invalid context and use another buffer with a valid context and the try again to copy the ");
            strcat(message, "contents of one buffer to the other buffer");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 223);
            strcpy(message, "Some of the 2 buffers that you are using to make the copy is invalid, although it may also be that ");
            strcat(message, "both buffers are invalid, according to the possible causes it is now up to you to check if one or ");
            strcat(message, "both buffers have problems");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 271);
            strcpy(message, "The offset that you have specified for the operation of the copy of 2 buffers has a problem with ");
            strcat(message, "one or both buffers, check if the sum of the offset with the size of the copy block does not ");
            strcat(message, "exceed the size of the memory block of the source buffer as of destination buffer");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 273);
            strcpy(message, "The buffer memory block is incorrectly formatted, you have to use the getMemBaseAddressAlign ");
            strcat(message, "method of the device you used to create the context in which you are working, to correctly ");
            strcat(message, "create the buffer memory block, also check if the buffer you are using is not a subbuffer");
            break;
        case CL_MEM_COPY_OVERLAP:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 408);
            strcpy(message, "There was an error when copying the source buffer to the destination buffer, since both buffers ");
            strcat(message, "are the same buffer or sub-buffer. Also make sure that when using the offset of the source buffer ");
            strcat(message, "as the destination buffer, the destination buffer offset is not greater than or equal to the ");
            strcat(message, "origin buffer, the same would happen if the origin buffer offset is not greater than or equal to ");
            strcat(message, "the target buffer offset");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 348);
            strcpy(message, "There is a problem with one of the 2 buffers you are using to perform the data copy between the ");
            strcat(message, "source buffer and the destination buffer, since that buffer has a problem with the memory block, ");
            strcat(message, "identify which is the problematic buffer or if both have the same problem replace them with ");
            strcat(message, "other buffers that the memory block has been assigned correctly");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueCopyBufferRect(cl_int currentCommandQueue, cl_int positionSrcBuffer, cl_int positionDstBuffer,
                                const size_t *offsetSrcBufferXYZ, const size_t *offsetDstBufferXYZ, const size_t *regionCopy,
                                size_t lengthColumnsPerRowInSrcBuffer, size_t sizeAreaSrcBuffer,
                                size_t lengthColumnsPerRowInDstBuffer, size_t sizeAreaDstBuffer) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem srcBuffer = *listBuffers[positionSrcBuffer];
    cl_mem dstBuffer = *listBuffers[positionDstBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueCopyBufferRect(_currentCommandQueue, srcBuffer, dstBuffer, offsetSrcBufferXYZ, offsetDstBufferXYZ, regionCopy,
                                    lengthColumnsPerRowInSrcBuffer, sizeAreaSrcBuffer, lengthColumnsPerRowInDstBuffer,
                                    sizeAreaDstBuffer, sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 193);
            strcpy(message, "The CommandQueue type object that you are using to copy the contents of a buffer to another ");
            strcat(message, "buffer is invalid, so you must use another CommandQueue type object and try to copy the buffers ");
            strcat(message, "again");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 237);
            strcpy(message, "The context you used to create one or both buffers is invalid, find which buffer is the one that ");
            strcat(message, "has an invalid context and use another buffer with a valid context and then try again to copy the ");
            strcat(message, "contents of one buffer to the other buffer");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 223);
            strcpy(message, "Some of the 2 buffers that you are using to make the copy is invalid, although it may also be that ");
            strcat(message, "both buffers are invalid, according to the possible causes it is now up to you to check if one or ");
            strcat(message, "both buffers have problems");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 181);
            strcpy(message, "The lengthColumnsPerRowInSrcBuffer variable is not a multiple of sizeAreaSrcBuffer or the ");
            strcat(message, "lengthColumnsPerRowInDstBuffer variable is not a multiple of the sizeAreaDstBuffer variable");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 273);
            strcpy(message, "The buffer memory block is incorrectly formatted, you have to use the getMemBaseAddressAlign ");
            strcat(message, "method of the device you used to create the context in which you are working, to correctly ");
            strcat(message, "create the buffer memory block, also check if the buffer you are using is not a subbuffer");
            break;
        case CL_MEM_COPY_OVERLAP:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 408);
            strcpy(message, "There was an error when copying the source buffer to the destination buffer, since both buffers ");
            strcat(message, "are the same buffer or sub-buffer. Also make sure that when using the offset of the source buffer ");
            strcat(message, "as the destination buffer, the destination buffer offset is not greater than or equal to the ");
            strcat(message, "origin buffer, the same would happen if the origin buffer offset is not greater than or equal to ");
            strcat(message, "the target buffer offset");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 348);
            strcpy(message, "There is a problem with one of the 2 buffers you are using to perform the data copy between the ");
            strcat(message, "source buffer and the destination buffer, since that buffer has a problem with the memory block, ");
            strcat(message, "identify which is the problematic buffer or if both have the same problem replace them with other ");
            strcat(message, "buffers that the memory block has been assigned correctly");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueFillBuffer(cl_int currentCommandQueue, cl_int currentBuffer, const void *patternFill, size_t sizeFillPattern,
                            size_t offsetFill, size_t sizeAreaFillInBuffer) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem _currentBuffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueFillBuffer(_currentCommandQueue, _currentBuffer, patternFill, sizeFillPattern, offsetFill, sizeAreaFillInBuffer,
                                sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 232);
            strcpy(message, "The CommandQueue type object that you are using to enqueue the buffer fill with a pattern is ");
            strcat(message, "incorrect, so to solve this problem you will have to use another object of the CommandQueue type ");
            strcat(message, "and try to use the fillBuffer method again");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 47);
            strcpy(message, "The context in which you are working is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 153);
            strcpy(message, "The buffer you are using to fill it with the pattern you have chosen is invalid, so you must use ");
            strcat(message, "another buffer and try again using the fillBuffer method");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 198);
            strcpy(message, "There is a problem with the fillBuffer method, and that is that the offset of the fill pattern as ");
            strcat(message, "well as the size of the fill pattern are not multiples of the size of the memory block of the ");
            strcat(message, "buffer");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 273);
            strcpy(message, "The buffer memory block is incorrectly formatted, you have to use the getMemBaseAddressAlign ");
            strcat(message, "method of the device you used to create the context in which you are working, to correctly ");
            strcat(message, "create the buffer memory block, also check if the buffer you are using is not a subbuffer");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 203);
            strcpy(message, "The memory block of the buffer that you are using to fill it with a design pattern was not ");
            strcat(message, "correctly assigned, so you will have to use another buffer to which the memory block has been ");
            strcat(message, "correctly assigned");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueMapBuffer(cl_int currentCommandQueue, cl_int currentBuffer, cl_bool isBlockingMap, cl_map_flags flagsMapBuffer,
                           size_t offsetMap, size_t sizeMap) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem _currentBuffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    CLEnqueueMapBuffer(_currentCommandQueue, _currentBuffer, isBlockingMap, flagsMapBuffer, offsetMap, sizeMap, sizeListEvents,
                       listEvents, event, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 175);
            strcpy(message, "The object of the CommandQueue type that you are using to map the buffer you are using is invalid, ");
            strcat(message, "so the only solution is that you use another object of the CommandQueue type");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 36);
            strcpy(message, "The context you are using is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 98);
            strcpy(message, "The buffer you are using is invalid, so you must use another buffer to try again to map the buffer");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 205);
            strcpy(message, "The offset and the size of the mapping that you are providing to map the buffer exceed the size of ");
            strcat(message, "the memory block that has the buffer, another possible problem is that the size of the buffer ");
            strcat(message, "mapping is 0");
            break;
        case CL_INVALID_EVENT_WAIT_LIST: case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 273);
            strcpy(message, "The buffer memory block is incorrectly formatted, you have to use the getMemBaseAddressAlign method ");
            strcat(message, "of the device you used to create the context in which you are working, to correctly create the ");
            strcat(message, "buffer memory block, also check if the buffer you are using is not a subbuffer");
            break;
        case CL_MAP_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 258);
            strcpy(message, "You can not map the buffer since at the time of its creation the Buffer.BUFFER_USER_HOST_PTR flag ");
            strcat(message, "or the Buffer.BUFFER_ALLOC_HOST_PTR flag was used, so the solution is to use another buffer that ");
            strcat(message, "has been created without using any of the flags mentioned above");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 169);
            strcpy(message, "The buffer can not be mapped since the memory block of the buffer was not correctly assigned, so ");
            strcat(message, "use another buffer to which the memory block has been assigned correctly");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 623);
            strcpy(message, "You can not map the buffer with the Buffer.MAP_BUFFER_READ flag because the buffer was created ");
            strcat(message, "with the Buffer.BUFFER_WRITE_ONLY flag, or if you used the Buffer.MAP_BUFFER_WRITE flag you ");
            strcat(message, "will not be able to map the buffer, if the buffer was created with the Buffer.BUFFER_READ_ONLY ");
            strcat(message, "flag , also must take into consideration that for both cases both for reading or writing the ");
            strcat(message, "buffer mapping avoid a buffer that has been created with the Buffer.BUFFER_HOST_NO_ACCESS ");
            strcat(message, "flag, also another cause of the problem is because it used the flag ");
            strcat(message, "Buffer.MAP_WRITE_INVALIDATE_REGION in a buffer with the memory block with writing problems");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueUnmapBuffer(cl_int currentCommandQueue, cl_int currentBuffer) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem buffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueUnmapMemObject(commandQueue, buffer, &buffer, positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 82);
            strcpy(message, "You cannot use the unmapBuffer method because the context you are using is invalid");
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 94);
            strcpy(message, "You cannot use the unmapBuffer method because the CommandQueue object you are using is invalid");
            break;
        case CL_INVALID_MEM_OBJECT: case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 81);
            strcpy(message, "You cannot use the unmapBuffer method because the buffer you are using is invalid");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueMigrationBuffer(cl_int currentCommandQueue, cl_int *listBuffersMigration, cl_int sizeList,
                                 cl_int flagsMigrationBuffers) {
    cl_command_queue  commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem *_listBuffers = (cl_mem*)malloc(sizeof(cl_mem) * sizeList);
    cl_uint sizeListEvents = positionCurrentEvent;
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    for (jint position = 0; position < sizeList; position++) {
        _listBuffers[position] = *listBuffers[listBuffersMigration[position]];
    }
    state = CLEnqueueMigrateMemObjects(commandQueue, sizeList, _listBuffers, flagsMigrationBuffers, sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 196);
            strcpy(message, "The CommandQueue object you are using to perform the migration of the current buffer and the list ");
            strcat(message, "of buffers you provided is invalid, and therefore the migration of the buffers cannot be performed");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 143);
            strcpy(message, "The context you used to create the buffers or the CommandQueue object is invalid and therefore ");
            strcat(message, "the migration of the buffers cannot be performed");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 207);
            strcpy(message, "The buffer you are using to perform the migration of the buffers or, at least one of the buffers ");
            strcat(message, "in the list of buffers you provided is invalid, and therefore the migration of the buffers cannot ");
            strcat(message, "be performed");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "Buffers migration cannot be performed because the flag you provided for buffers migration is invalid");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 223);
            strcpy(message, "The migration of the buffers cannot be performed because the size of the event list does not ");
            strcat(message, "correspond to the number of events in the event list, or the event list was set with a null value ");
            strcat(message, "and the list size is larger to 0");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 123);
            strcpy(message, "The migration of the buffers could not be performed because there was a problem in the allocation ");
            strcat(message, "of memory for the buffers");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetBufferDestructorCallback(JNIEnv *env, cl_int currentBuffer, jobject callbackBuffer, jobject buffer) {
    cl_mem _buffer = *listBuffers[currentBuffer];
    if (callbackBuffer == NULL) state = CLSetMemObjectDestructorCallback(_buffer, NULL, NULL);
    else {
        jclass CallbackBuffer = env->GetObjectClass(callbackBuffer);
        jmethodID notify = env->GetMethodID(CallbackBuffer, "notify", "(Lcom/draico/asvappra/opencl/memory/buffer/Buffer;)V");
        jobject notifyBufferMethod = env->ToReflectedMethod(CallbackBuffer, notify, JNI_FALSE);
        void *data[] = { env, &buffer, &callbackBuffer, &notifyBufferMethod };
        void *dataUser = { data };
        state = CLSetMemObjectDestructorCallback(_buffer, notifyCallbackBuffer, dataUser);
    }
    switch(state) {
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 54);
            strcpy(message, "The buffer you used to set a CallbackBuffer is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 87);
            strcpy(message, "A CallbackBuffer cannot be set to the buffer, because the CallbackBuffer method is null");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleaseBuffer(cl_int currentBuffer, cl_bool isSubBuffer, cl_int currentBufferRegion) {
    cl_mem _currentBuffer = *listBuffers[currentBuffer];
    state = CLReleaseBuffer(_currentBuffer);
    switch(state) {
        case CL_SUCCESS:
            processReleaseBuffer(currentBuffer, isSubBuffer, currentBufferRegion);
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 98);
            strcpy(message, "The buffer you are using is invalid, so you must use another buffer to try ");
            strcat(message, "again to map the buffer");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetSVMAlloc(cl_int currentContext, cl_svm_mem_flags flagsMemory, size_t sizeBlock, cl_uint numberBytesPerData) {
    cl_context _currentContext = *listContexts[currentContext];
    jbyte *memory = (jbyte*)CLSVMAlloc(_currentContext, flagsMemory, sizeBlock, numberBytesPerData);
    if (memory != NULL) {
        jsize sizeBuffer = sizeof(memory);
        listDataMemory[positionCurrentMemory] = memory;
        listSizeBlocksMemory[positionCurrentMemory] = sizeBlock;
        state = CL_SUCCESS;
    } else {
        message = NULL;
        message = (char*)malloc(sizeof(char) * 608);
        strcpy(message, "There was a problem when you tried to create the memory block, the problem may have been caused by ");
        strcat(message, "different reasons: The context is invalid, you used the Memory.MEMORY_SVM_FINE_GRAIN_BUFFER flag ");
        strcat(message, "without using the Memory.MEMORY_SVM_ATOMICS flag or if you used the Memory.MEMORY_SVM_ATOMICS flag ");
        strcat(message, "without using the Memory.MEMORY_SVM_FINE_GRAIN_BUFFER flag, the device you are currently using is ");
        strcat(message, "not compatible with the Memory.MEMORY_SVM_FINE_GRAIN_BUFFER and Memory.MEMORY_SVM_ATOMICS flags, ");
        strcat(message, "the size that you specified for the block exceeds the maximum size allowed by the getMaxMemApp ");
        strcat(message, "method using the device");
        state = -1;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetSVMFree(cl_int currentContext, cl_int currentSVMMemory) {
    if (listDataMemory != NULL) {
        cl_context context = *listContexts[currentContext];
        jbyte *dataBlockMemory = listDataMemory[currentSVMMemory];
        CLSVMFree(context, dataBlockMemory);
        listDataMemory[positionCurrentMemory] = NULL;
        listSizeBlocksMemory[positionCurrentMemory] = NULL;
        if (positionCurrentMemory > 0) positionCurrentMemory--;
        state = CL_SUCCESS;
    } else {
        message = NULL;
        message = (char*)malloc(sizeof(char) * 55);
        strcpy(message, "There is no available memory block that can be released");
        state = -1;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueSVMFree(JNIEnv *env, jobject commandQueue, cl_uint numberBlocksMemory, cl_int *listMemory, cl_int *sizeBlocksMemory,
                         cl_int *flagsBlocksMemory, jobject callbackMemory) {
    jclass CommandQueue = env->GetObjectClass(commandQueue);
    jfieldID currentCommandQueue = env->GetFieldID(CommandQueue, "currentCommandQueue", "I");
    jint _currentCommandQueue = env->GetIntField(commandQueue, currentCommandQueue);
    cl_command_queue _commandQueue = *listCommandQueue[_currentCommandQueue];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    cl_int lengthListEvents = positionCurrentEvent;
    jbyte **listMemoryData = (jbyte**)malloc(sizeof((jbyte*)malloc(sizeof(jbyte))) * numberBlocksMemory);
    for (int position = 0; position < numberBlocksMemory; position++) listMemoryData[position] = listDataMemory[listMemory[position]];
    if (callbackMemory == NULL) {
        state = CLEnqueueSVMFree(_commandQueue, numberBlocksMemory, (void**)&listMemoryData, NULL, NULL, lengthListEvents, listEvents,
                                 event);
    } else {
        jclass CallbackMemory = env->FindClass("com/draico/asvappra/opencl/listeners/CallbackMemory");
        jmethodID notify = env->GetMethodID(CallbackMemory, "notify",
                                            "(Lcom/draico/asvappra/opencl/CommandQueue;[Lcom/draico/asvappra/opencl/memory/Memory;)V");
        jobject methodNotify = env->ToReflectedMethod(CallbackMemory, notify, JNI_FALSE);
        void *dataInfo[] = { env, &callbackMemory, &methodNotify, &commandQueue, listMemory, sizeBlocksMemory, flagsBlocksMemory };
        state = CLEnqueueSVMFree(_commandQueue, numberBlocksMemory, (void**)listMemoryData, notifyCallbackMemory, dataInfo,
                                 lengthListEvents, listEvents, event);
    }
    switch(state) {
        case CL_SUCCESS:
            processSVMFree(listMemory, numberBlocksMemory);
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 118);
            strcpy(message, "The object of the CommandQueue type that you are using is invalid, use or instance another object ");
            strcat(message, "of type CommandQueue");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 180);
            strcpy(message, "You have provided a list of empty memory blocks or you placed a null value instead, the list of ");
            strcat(message, "memory blocks must contain valid memory blocks, so this value can not be set as null");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueSVMMemcpy(cl_int currentCommandQueue, cl_bool isBlockingCopy, cl_int positionDstMemory, cl_int positionSrcMemory) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    jbyte *dstMemory = listDataMemory[positionDstMemory];
    jbyte *srcMemory = listDataMemory[positionSrcMemory];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    size_t sizeBlock = sizeof(srcMemory);
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueSVMMemcpy(_currentCommandQueue, isBlockingCopy, dstMemory, srcMemory, sizeBlock, sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 214);
            strcpy(message, "The CommandQueue type object that you are using to enqueue the process of copying srcBlockMemory ");
            strcat(message, "to current Memory is invalid, so you will have to use another CommandQueue type object and try ");
            strcat(message, "again to make the copy");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 196);
            strcpy(message, "The context you are using to make the copy of the srcBlockMemory memory block to the current Memory ");
            strcat(message, "memory block is invalid, so you must use another context and try again to copy the memory ");
            strcat(message, "blocks");
            break;
        case CL_INVALID_EVENT_WAIT_LIST: case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 53);
            strcpy(message, "The variables srcMemory and dstMemory can not be null");
            break;
        case CL_MEM_COPY_OVERLAP:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 105);
            strcpy(message, "You are trying to copy an exact copy of the contents of the srcBlockMemory memory block to ");
            strcat(message, "current Memory");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;

    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueSVMMemFill(cl_int currentCommandQueue, cl_int currentMemory, const void *pattern, size_t sizePattern,
                            size_t sizeBlockToFill) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    jbyte *blockMemory = listDataMemory[currentMemory];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueSVMMemFill(_currentCommandQueue, blockMemory, pattern, sizePattern, sizeBlockToFill, sizeListEvents, listEvents,
                                event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 90);
            strcpy(message, "The CommandQueue object that you are using in the enqueueFillBlockMemory method is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 172);
            strcpy(message, "The context that you are using to fill the given memory block is incorrect, so you must use another ");
            strcat(message, "context and try again to fill the memory block with the pattern provided");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 72);
            strcpy(message, "The sizePattern variable is not multiple of the sizeBlockToFill variable");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueSVMMap(cl_int currentCommandQueue, cl_int currentMemory, cl_bool isBlockingMap, cl_int sizeBlockMap,
                        cl_map_flags flagsMap) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    jbyte *dataMemory = listDataMemory[currentMemory];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueSVMMap(_currentCommandQueue, isBlockingMap, flagsMap, dataMemory, sizeBlockMap, sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 91);
            strcpy(message, "There is a problem in the enqueueMapMemory method, since the CommandQueue object is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 159);
            strcpy(message, "The context that you are using to enqueue the mapping of the memory block is invalid, you must use ");
            strcat(message, "another context and retry the mapping of the memory block");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 160);
            strcpy(message, "The block of memory you want to map is null, or the size of the memory block mapping is 0, or the ");
            strcat(message, "flag you are using to try to map the memory block is not valid");
            break;
        case CL_INVALID_EVENT_WAIT_LIST: case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueSVMUnmap(cl_int currentCommandQueue, cl_int currentMemory) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    jbyte *dataMemory = listDataMemory[currentMemory];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueSVMUnmap(_currentCommandQueue, dataMemory, sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "There is a problem in the enqueueUnmapMemory method, since the CommandQueue object is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 137);
            strcpy(message, "The context you are using in the memory block mappings cancellation operation is incorrect, so you ");
            strcat(message, "use another context and try again");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 90);
            strcpy(message, "The memory block has a problem when executing the enqueueUnmapMemory method");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueSVMMigrateMem(cl_int currentCommandQueue, cl_uint numberBlocksMemory, cl_int *currentListMemory,
                               cl_mem_migration_flags flagsMigrateMemory) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    jbyte **dataMemory = (jbyte**)malloc(sizeof((jbyte*)malloc(sizeof(jbyte))) * numberBlocksMemory);
    size_t sizeBlocks[numberBlocksMemory];
    for (jint position = 0; position < numberBlocksMemory; position++) dataMemory[position] = listDataMemory[currentListMemory[position]];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    size_t sizeListEvents = 0;
    if (listEvents != NULL) sizeListEvents = positionCurrentEvent;
    state = CLEnqueueSVMMigrateMem(_currentCommandQueue, numberBlocksMemory, (const void**)dataMemory, sizeBlocks, flagsMigrateMemory,
                                   sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 88);
            strcpy(message, "The CommandQueue object that you are using in the enqueueMigrateMemory method is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 184);
            strcpy(message, "The context you are using to perform the migration of the memory blocks provided is invalid, so ");
            strcat(message, "you must use another context and try again to perform the migration of the memory blocks");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 361);
            strcpy(message, "The number of memory blocks you want to migrate is 0 or the list of memory blocks to which a ");
            strcat(message, "migration will be applied was set to a null value, or the flag you use to perform the migration of ");
            strcat(message, "the data blocks is invalid, so you will have to use the Memory.MIGRATE_MEM_OBJECT_HOST flag or the ");
            strcat(message, "Memory.MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED flag or combine both flags");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateEvent(cl_int currentContext) {
    cl_context _currentContext = *listContexts[currentContext];
    cl_event event = CLCreateUserEvent(_currentContext, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 83);
            strcpy(message, "Could not create a new event, since the context in which you are working is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetEventStatus(cl_int currentEvent, cl_int eventStatus) {
    cl_event event = listEvents[currentEvent];
    state = CLSetUserEventStatus(event, eventStatus);
    switch(state) {
        case CL_INVALID_EVENT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 69);
            strcpy(message, "The event to which you want to change the execution status is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 170);
            strcpy(message, "There is a problem with the setEventStatus method of the event you are currently using, to get more ");
            strcat(message, "detailed information about the problem use the CheckStatusEvents class");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 276);
            strcpy(message, "You have previously changed the status of the event execution using the setEventStatus method of the ");
            strcat(message, "current event, you have to wait for the process that is monitoring the event to end, and while the ");
            strcat(message, "event is working you can monitor its status with the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetWaitForEvents(cl_int numEvents, cl_int *positionsListEvents) {
    cl_event *events = (cl_event*)malloc(sizeof(cl_event) * numEvents);
    for (cl_int position = 0; position < numEvents; position++) events[position] = listEvents[positionsListEvents[position]];
    state = CLWaitForEvents(numEvents, events);
    switch(state) {
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 60);
            strcpy(message, "The number of events provided is 0 or the event list is null");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 135);
            strcpy(message, "There is a problem with the waitForEvents method because the events in the event list you provided ");
            strcat(message, "were not created in the same context");
            break;
        case CL_INVALID_EVENT: case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "The list of events generated by the commands that you have enqueued has invalid events, to know ");
            strcat(message, "what they are and what problem they have use the CheckStatusEvents class");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEventInfo(cl_int currentEvent, cl_event_info infoEvent, cl_int *dataInfoEvent) {
    cl_event event = listEvents[currentEvent];
    size_t sizeDataInfoEvent;
    state = CLGetEventInfo(event, infoEvent, NULL, NULL, &sizeDataInfoEvent);
    switch(state) {
        case CL_SUCCESS:
            dataInfoEvent = (cl_int*)malloc(sizeof(cl_int) * sizeDataInfoEvent);
            state = CLGetEventInfo(event, infoEvent, sizeDataInfoEvent, dataInfoEvent, NULL);
            switch(state) {
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 131);
                    strcpy(message, "You can not use the getCommandType or getExecutionStatus method, since the event does not ");
                    strcat(message, "have information about the requested data");
                    break;
                case CL_INVALID_EVENT:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 123);
                    strcpy(message, "Unable to obtain event information using the getCommandType and getExecutionStatus method, ");
                    strcat(message, "since the current event is invalid");
                    break;
                case CL_OUT_OF_RESOURCES:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 93);
                    strcpy(message, "The requested information could not be processed, because the system has run out of resources");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 85);
                    strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                    break;
            }
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 131);
            strcpy(message, "You can not use the getCommandType or getExecutionStatus method, since the event does not have ");
            strcat(message, "information about the requested data");
            break;
        case CL_INVALID_EVENT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 123);
            strcpy(message, "Unable to obtain event information using the getCommandType and getExecutionStatus method, since the ");
            strcat(message, "current event is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetEventCallback(JNIEnv *env, jobject event, cl_int executionStatus, jobject callbackEvent) {
    jclass Event = env->GetObjectClass(event);
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jint _currentEvent = env->GetIntField(event, currentEvent);
    cl_event _event = listEvents[_currentEvent];
    if (callbackEvent == NULL) state = CLSetEventCallback(_event, executionStatus, NULL, NULL);
    else {
        jclass CallbackEvent = env->GetObjectClass(callbackEvent);
        jmethodID notify = env->GetMethodID(CallbackEvent, "notify",
                                            "(Lcom/draico/asvappra/opencl/listeners/Event;Ljava/lang/Integer;)V");
        jobject _methodNofity = env->ToReflectedMethod(CallbackEvent, notify, JNI_FALSE);
        eventCallback = env->NewGlobalRef(event);
        _callbackEvent = env->NewGlobalRef(callbackEvent);
        methodNotify = env->NewGlobalRef(_methodNofity);
        void *dataInfo[] { env, &eventCallback, &_callbackEvent, &methodNotify };
        state = CLSetEventCallback(_event, executionStatus, notifyCallbackEvent, dataInfo);
    }
    switch(state) {
        case CL_INVALID_EVENT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 86);
            strcpy(message, "The current event is not a valid event and therefore a executionStatus can not be set");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 447);
            strcpy(message, "The values that can be used for the value of the variable executionStatus setEventCallback method ");
            strcat(message, "are:\nEvent.COMMAND_EXECUTION_STATUS_COMPLETE\nEvent.COMMAND_EXECUTION_STATUS_RUNNING\n");
            strcat(message, "Event.COMMAND_EXECUTION_STATUS_SUBMITTED\nEvent.COMMAND_EXECUTION_STATUS_QUEUED\n");
            strcat(message, "Event.COMMAND_EXECUTION_STATUS_WAIT\nThe latter value can serve as a reference to distinguish the ");
            strcat(message, "error of an event with the error of another event, whose value is a negative number");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetRetainEvent(cl_int currentEvent) {
    cl_event event = listEvents[currentEvent];
    state = CLRetainEvent(event);
    switch(state) {
        case CL_INVALID_EVENT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 28);
            strcpy(message, "The current event is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleaseEvent(cl_int currentEvent) {
    cl_event event = listEvents[currentEvent];
    state = CLReleaseEvent(event);
    switch(state) {
        case CL_SUCCESS:
            processReleaseEvent(currentEvent);
            break;
        case CL_INVALID_EVENT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 28);
            strcpy(message, "The current event is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueMarkerWithWaitList(cl_int currentCommandQueue, cl_int *positionWaitListEvents, cl_int sizeListEvents) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    cl_event *waitListEvents = (cl_event*)malloc(sizeof(cl_event) * sizeListEvents);
    for (cl_int position = 0; position < sizeListEvents; position++) {
        waitListEvents[position] = listEvents[positionWaitListEvents[position]];
    }
    state = CLEnqueueMarkerWithWaitList(_currentCommandQueue, sizeListEvents, waitListEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 91);
            strcpy(message, "The CommandQueue object that you are using in the markerWithWaitListEvent method is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 36);
            strcpy(message, "The context you are using is invalid");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 101);
            strcpy(message, "There is a problem with the markerWithWaitListEvent method because the list of events you provided ");
            strcat(message, "has invalid events");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueBarrierWithWaitList(cl_int currentCommandQueue, cl_int *positionWaitListEvents, cl_int sizeListEvents) {
    cl_command_queue _currentCommandQueue = *listCommandQueue[currentCommandQueue];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    cl_event *waitListEvents = (cl_event*)malloc(sizeof(cl_event) * sizeListEvents);
    for (cl_int position = 0; position < sizeListEvents; position++) {
        waitListEvents[position] = listEvents[positionWaitListEvents[position]];
    }
    state = CLEnqueueBarrierWithWaitList(_currentCommandQueue, sizeListEvents, waitListEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 88);
            strcpy(message, "The CommandQueue object that I provide in the barrierWithWaitListEvent method is invalid");
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 36);
            strcpy(message, "The context you are using is invalid");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 118);
            strcpy(message, "There is a problem with the barrierWithWaitListEvent method because the list of events you provided ");
            strcat(message, "has invalid events");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEventProfilingInfo(cl_int currentEvent, cl_profiling_info profilingInfo, cl_ulong *infoRequest) {
    cl_event event = listEvents[currentEvent];
    size_t sizeInfoRequest;
    state = CLGetEventProfilingInfo(event, profilingInfo, NULL, NULL, &sizeInfoRequest);
    switch(state) {
        case CL_SUCCESS:
            infoRequest = (cl_ulong*)malloc(sizeof(cl_ulong) * sizeInfoRequest);
            state = CLGetEventProfilingInfo(event, profilingInfo, sizeInfoRequest, infoRequest, NULL);
            switch(state) {
                case CL_PROFILING_INFO_NOT_AVAILABLE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 670);
                    strcpy(message, "The possible errors when using the getProfilingCommandQueue, getProfilingCommandSubmit, ");
                    strcat(message, "getProfilingCommandStart, getProfilingCommandEnd and getProfilingCommandComplete methods ");
                    strcat(message, "may be due to the fact that at the time of creating the CommandQueue, you did not use ");
                    strcat(message, "the CommandQueue flag.QUEUE_PROFILING_ENABLE to enqueue some action. Another possible error ");
                    strcat(message, "of any of the previous methods is that the operation that is related to the event you ");
                    strcat(message, "are using has not completed its work by doing, another possible error is that the event ");
                    strcat(message, "you are using is an event created by the createEvent method, so that is not related to ");
                    strcat(message, "any action that has been enqueue with any CommandQueue");
                    break;
                case CL_INVALID_EVENT:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 137);
                    strcpy(message, "At least one event in the list of events is invalid, so that you know what is causing the ");
                    strcat(message, "problem you must use the CheckStatusEvent class");
                    break;
                case CL_OUT_OF_RESOURCES:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 93);
                    strcpy(message, "The requested information could not be processed, because the system has run out of resources");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 85);
                    strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                    break;
            }
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetSupportedImageFormats(cl_int currentContext, cl_int typeAccessImage, cl_int typeImage) {
    cl_context context = *listContexts[currentContext];
    dataListImageFormat[0] = NULL;
    dataListImageFormat[1] = NULL;
    cl_uint sizeList;
    state = CLGetSupportedImageFormats(context, typeAccessImage, typeImage, NULL, NULL, &sizeList);
    switch(state) {
        case CL_SUCCESS: {
            cl_image_format *listTemp = (cl_image_format*)malloc(sizeof(cl_image_format) * sizeList);
            state = CLGetSupportedImageFormats(context, typeAccessImage, typeImage, sizeList, listTemp, NULL);
            switch (state) {
                case CL_SUCCESS:
                    dataListImageFormat[0] = listTemp;
                    dataListImageFormat[1] = &sizeList;
                    processAddListImageFormats(listTemp, sizeList);
                    break;
                case CL_INVALID_CONTEXT:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 132);
                    strcpy(message, "You cannot get the list of available image formats that opencl supports on this device, ");
                    strcat(message, "because the context you are using is invalid");
                    break;
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 316);
                    strcpy(message, "You cannot get the list of available image formats that opencl supports on this device, ");
                    strcat(message, "because the type of image access or the type of image you specified is invalid, or opencl ");
                    strcat(message, "cannot determine the list of image formats available on this device, because the type of ");
                    strcat(message, "image you provided is not supported by the device");
                    break;
                case CL_OUT_OF_RESOURCES:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 93);
                    strcpy(message, "The requested information could not be processed, because the system has run out of resources");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 85);
                    strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                    break;

            }
        }
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 132);
            strcpy(message, "You cannot get the list of available image formats that opencl supports on this device, because ");
            strcat(message, "the context you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 316);
            strcpy(message, "You cannot get the list of available image formats that opencl supports on this device, because ");
            strcat(message, "the type of image access or the type of image you specified is invalid, or opencl cannot determine ");
            strcat(message, "the list of image formats available on this device, because the type of image you provided is not ");
            strcat(message, "supported by the device");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateImage(cl_int currentContext, cl_mem_flags typeAccessImage, cl_int currentImageFormat, cl_int currentImageDescriptor,
                      jint currentDataImage, jbyte *dataImage) {
    cl_context context = *listContexts[currentContext];
    cl_image_format imageFormat = *listImageFormat[currentImageFormat];
    cl_image_desc imageDescriptor = *listImageDescriptor[currentImageDescriptor];
    cl_mem *image = (cl_mem*)malloc(sizeof(cl_mem));
    *image = CLCreateImage(context, typeAccessImage, &imageFormat, &imageDescriptor, dataImage, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddImageToListImages(*image);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 78);
            strcpy(message, "You cannot create an Image object because the context you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 555);
            strcpy(message, "The Image object cannot be created because the access type for the image is invalid, you can only ");
            strcat(message, "use any of the following values:\nImage.IMAGE_READ_WRITE\nImage.Image_WRITE_ONLY\n");
            strcat(message, "Image.IMAGE_READ_ONLY\nAnother possible problem is that the buffer you are using to create the image ");
            strcat(message, "it has been created with the Buffer.BUFFER_WRITE_ONLY flag, and the type of access to the image you ");
            strcat(message, "have set is Image.IMAGE_READ_WRITE or Image.IMAGE_READ_ONLY, you must ensure that the flags of both the ");
            strcat(message, "buffer and the Image object you are trying to create have compatible flags");
            break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 126);
            strcpy(message, "You cannot create the Image object because the storage sequence of the 3D or 2D image is not linear ");
            strcat(message, "in the buffer you provided");
            break;
        case CL_INVALID_IMAGE_DESCRIPTOR:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 94);
            strcpy(message, "You cannot create the Image object because the ImageDescriptor object you are using is invalid");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 305);
            strcpy(message, "You cannot create the Image object because the dimensions that you specified in the ImageDescriptor ");
            strcat(message, "object as such as width, height and depth exceed the maximum values allowed by the device you used to ");
            strcat(message, "create the context, which you used to create ");
            strcat(message, "the buffer you are using to try to create the image object");
            break;
        case CL_INVALID_HOST_PTR:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 196);
            strcpy(message, "You cannot create the Image object because you set the Image.IMAGE_USE_HOST_PTR and/or ");
            strcat(message, "Image.IMAGE_COPY_HOST_PTR flag and the buffer was created without using the ");
            strcat(message, "Buffer.BUFFER_USE_HOST_PTR flag");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 125);
            strcpy(message, "You cannot create the Image object because the ImageFormat object was created in a format that is ");
            strcat(message, "not supported by the device");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 118);
            strcpy(message, "You cannot create the Image object because no device present in the context can support working ");
            strcat(message, "with the Images object");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 104);
            strcpy(message, "You cannot create the Image object because there was a problem allocating memory to the new ");
            strcat(message, "Image object");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueReadImage(cl_int currentCommandQueue, cl_int currentImage, cl_bool isBlockingRead, const size_t *origin,
                           const size_t *region, size_t numberBytesPerRow, size_t numberBytesPerLayer, cl_int currentBuffer) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem image = *listImage[currentImage];
    cl_mem buffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueReadImage(commandQueue, image, isBlockingRead, origin, region, numberBytesPerRow, numberBytesPerLayer, &buffer,
                               positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 82);
            strcpy(message, "You cannot read the image because the CommandQueue object you are using is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 70);
            strcpy(message, "You cannot read the image because the context you are using is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 69);
            strcpy(message, "You cannot read the image because the buffer you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 771);
            strcpy(message, "You cannot read the image because the origin and region arrays do not have correct data, the ");
            strcat(message, "origin array data represents the offset X, Y and Z of the image according to their type, for the 1D ");
            strcat(message, "image types the offset Y and Z must be be 0, for an arrangement of the type of 1D image the offset ");
            strcat(message, "Y must be set with a 1 always and Z with a 0, for 2D image types the offset of Z must always be a 0, ");
            strcat(message, "and for the 2D image type arrays the Z offset specifies the index in the 2D image array, for the 3D ");
            strcat(message, "image types the offset X, Y and Z must be specified. The region array is used to specify the values ");
            strcat(message, "width, height and depth of the image, if the image type is 1D or 2D the depth must be specified with ");
            strcat(message, "a value of 1, and the value of the height for 1D image types must always be 1");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 292);
            strcpy(message, "You cannot read the image because the event list has invalid events, use the CheckStatusEvents ");
            strcat(message, "class to check if the events belong to the same context, and eliminate the events that are no longer ");
            strcat(message, "necessary, as well as separate the events from ");
            strcat(message, "different contexts and check their status current");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 247);
            strcpy(message, "You cannot read the image because its dimensions such as width, height and depth, as well as ");
            strcat(message, "calculations for the number of bytes per line and/or the number of bytes per layer are not supported ");
            strcat(message, "by the device associated with the CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 149);
            strcpy(message, "You cannot read the image because the format selected in the ImageFormat class is not supported by ");
            strcat(message, "the device associated with the CommandQueue object");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 120);
            strcpy(message, "You cannot read the image because the buffer memory block could not be accessed or the buffer ");
            strcat(message, "memory block has a problem");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 359);
            strcpy(message, "You cannot read the image because the device associated with the CommandQueue object does not ");
            strcat(message, "support images, to verify this use the getImageSupport method of the device associated with the ");
            strcat(message, "CommandQueue object, another possible problem is that the buffer has been created with using the ");
            strcat(message, "Buffer.BUFFER_HOST_READ_ONLY flags and / or Buffer.BUFFER_HOST_NO_ACCESS");
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 194);
            strcpy(message, "You cannot read the image because some of the events in the event list have a problem with the ");
            strcat(message, "process with which it is related, use the CheckStatusEvents class to check the problem and solve it");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueWriteImage(cl_int currentCommandQueue, cl_int currentImage, cl_bool isBlockingRead, const size_t *origin,
                            const size_t *region, size_t numberBytesPerRow, size_t numberBytesPerLayer, cl_int currentBuffer) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem image = *listImage[currentImage];
    cl_mem buffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueWriteImage(commandQueue, image, isBlockingRead, origin, region, numberBytesPerRow, numberBytesPerLayer, &buffer,
                                positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 83);
            strcpy(message, "You cannot write the image because the CommandQueue object you are using is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 71);
            strcpy(message, "You cannot write the image because the context you are using is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 70);
            strcpy(message, "You cannot write the image because the buffer you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 772);
            strcpy(message, "You cannot write the image because the origin and region arrays do not have correct data, the ");
            strcat(message, "origin array data represents the offset X, Y and Z of the image according to their type, for the ");
            strcat(message, "1D image types the offset Y and Z must be be 0, for an arrangement of the type of 1D image the ");
            strcat(message, "offset Y must be set with a 1 always and Z with a 0, for 2D image types the offset of Z must always ");
            strcat(message, "be a 0, and for the 2D image type arrays the Z offset specifies the index in the 2D image array, for ");
            strcat(message, "the 3D image types the offset X, Y and Z must be specified. The region array is used to specify the ");
            strcat(message, "values width, height and depth of the image, if the image type is 1D or 2D the depth must be ");
            strcat(message, "specified with a value of 1, and the value of the height for 1D image types must always be 1");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 293);
            strcpy(message, "You cannot write the image because the event list has invalid events, use the CheckStatusEvents ");
            strcat(message, "class to check if the events belong to the same context, and eliminate the events that are no longer ");
            strcat(message, "necessary, as well as separate the events from different contexts and check their status current");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 248);
            strcpy(message, "You cannot write the image because its dimensions such as width, height and depth, as well as ");
            strcat(message, "calculations for the number of bytes per line and/or the number of bytes per layer are not ");
            strcat(message, "supported by the device associated with the CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 150);
            strcpy(message, "You cannot write the image because the format selected in the ImageFormat class is not supported ");
            strcat(message, "by the device associated with the CommandQueue object");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 121);
            strcpy(message, "You cannot write the image because the buffer memory block could not be accessed or the buffer ");
            strcat(message, "memory block has a problem");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 360);
            strcpy(message, "You cannot write the image because the device associated with the CommandQueue object does not ");
            strcat(message, "support images, to verify this use the getImageSupport method of the device associated with the ");
            strcat(message, "CommandQueue object, another possible problem is that the buffer has been created with using the ");
            strcat(message, "Buffer.BUFFER_HOST_READ_ONLY flags and / or Buffer.BUFFER_HOST_NO_ACCESS");
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 195);
            strcpy(message, "You cannot write the image because some of the events in the event list have a problem with the ");
            strcat(message, "process with which it is related, use the CheckStatusEvents class to check the problem and solve it");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueCopyImage(cl_int currentCommandQueue, cl_int currentImageSrc, cl_int currentImageDst, const size_t *srcOrigin,
                           const size_t *dstOrigin, const size_t *region) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem imageSrc = *listImage[currentImageSrc];
    cl_mem imageDst = *listImage[currentImageDst];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    cl_uint sizeListEvents = positionCurrentEvent;
    state = CLEnqueueCopyImage(commandQueue, imageSrc, imageDst, srcOrigin, dstOrigin, region, sizeListEvents, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 114);
            strcpy(message, "You cannot copy the source image to the destination image because the CommandQueue object you are ");
            strcat(message, "using is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 158);
            strcpy(message, "You cannot copy the source image to the destination image because the context I use to create the ");
            strcat(message, "images and also to create the CommandQueue object is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 113);
            strcpy(message, "You cannot copy the source image to the destination image because either of the Image objects or ");
            strcat(message, "both are invalid");
            break;
        case CL_IMAGE_FORMAT_MISMATCH:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 178);
            strcpy(message, "You cannot copy the source image to the destination image because the Image objects, both the ");
            strcat(message, "destination image and the source image, were not created with the same type of image");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 1217);
            strcpy(message, "You cannot copy the source image to the destination image because the values you specified in both ");
            strcat(message, "the srcOrigin and dstOrigin arrays represent the offset for the X, Y, and Z positions respectively in ");
            strcat(message, "both arrays, and for the image types ImageDescriptor.TYPE_IMAGE1D and ImageDescriptor.TYPE_IMAGE1D_BUFFER");
            strcat(message, " the Y and Z values of both offset must always be 0, for the image types ");
            strcat(message, "ImageDescriptor.TYPE_IMAGE1D_ARRAY the values Y and Z must always be 1, for the image type ");
            strcat(message, "ImageDescriptor.TYPE_IMAGE2D_ARRAY the Z value is used to change the index value and thus change ");
            strcat(message, "from one layer to another layer, the values of the region array are used to adjust the values of ");
            strcat(message, "width, height and depth, for the image types ImageDescriptor.TYPE_IMAGE1D and ");
            strcat(message, "ImageDescriptor.TYPE_IMAGE1D_BUFFER the value of the height and depth must be set with value 1, for ");
            strcat(message, "the image type ImageDescriptor.TYPE_IMAGE1D_ARRAY and ImageDescriptor.TYPE_IMAGE2D the value depth ");
            strcat(message, "must always be set to 1. You should also keep in mind that the sum of the values that you adjust ");
            strcat(message, "in both the srcOrigin and dstOrigin offset arrays as well as in the region fix must not exceed ");
            strcat(message, "the size of the buffer memory block that is related to your image object");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 1);
            strcpy(message, "You cannot copy the source image to the destination image because the list of events has events ");
            strcat(message, "that belong to other contexts than the one you are currently working on, there may also be events ");
            strcat(message, "that the operation to which they are linked have already been completed or that there has been an ");
            strcat(message, "error in any of those operations that are linked to the events in the event list, to correct any of ");
            strcat(message, "the aforementioned problems use the CheckStatusEvents class");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 222);
            strcpy(message, "You cannot copy the source image to the destination image because the values that for the region ");
            strcat(message, "array that are width, height and depth exceed the limit allowed by the device that is associated ");
            strcat(message, "with the CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 268);
            strcpy(message, "You cannot copy the source image to the destination image because the format you used to create ");
            strcat(message, "the ImageFormat object with which you created the destination Image object or the Source Image ");
            strcat(message, "object is not supported by the device associated with the CommandQueue object");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 267);
            strcpy(message, "You cannot copy the source image to the destination image because the buffer associated with the ");
            strcat(message, "Source Image object or the buffer associated with the Destination Image object does not have a ");
            strcat(message, "large enough memory block to copy the source image to the destination image");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 255);
            strcpy(message, "You cannot copy the source image to the destination image because the device associated with the ");
            strcat(message, "CommandQueue object does not support handling Image objects, to verify it use the getImageSupport ");
            strcat(message, "method of the device associated with the CommandQueue object");
            break;
        case CL_MEM_COPY_OVERLAP:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 148);
            strcpy(message, "You cannot copy the source image to the destination image because the Image source object and the ");
            strcat(message, "Image destination object are the same Image object");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueFillImage(cl_int currentCommandQueue, cl_int currentImage, const void *fillColor, const size_t *origin,
                           const size_t *region) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem image = *listImage[currentImage];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueFillImage(commandQueue, image, fillColor, origin, region, positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 103);
            strcpy(message, "You cannot perform color fill the Image object because the CommandQueue object you are using is ");
            strcat(message, "invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 91);
            strcpy(message, "You cannot perform color fill the Image object because the context you are using is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 112);
            strcpy(message, "You cannot perform color fill the Image object because the buffer you used to create the Image ");
            strcat(message, "object is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 874);
            strcpy(message, "You cannot perform fill the color of the Image object because the values you provided to the ");
            strcat(message, "origin array (offset x, y and z) and the region array (width, height and depth) exceed the size of the ");
            strcat(message, "memory block of the buffer you used to create the Image object, another possible problem is that ");
            strcat(message, "offset values and dimension values do not match the type of image you set when creating the Image ");
            strcat(message, "object, if you used image types ImageDescriptor.TYPE_IMAGE1D or ImageDescriptor.TYPE_IMAGE1D_BUFFER ");
            strcat(message, "offset values y and z must always be set to 0, and the height and depth values must always be set ");
            strcat(message, "to 1, if you use the image type ImageDescriptor.TYPE_IMAGE1D_ARRAY the offset value z must always be ");
            strcat(message, "0 and the depth value must always be if set to 1, for image type ImageDescriptor.TYPE_IMAGE2D the ");
            strcat(message, "offset z must always be set to 0 and the depth value must always be set to one");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 391);
            strcpy(message, "You cannot perform fill the Image object because the event list has events that have already ");
            strcat(message, "completed the operation that is linked to those events, or there may also be events that cannot be ");
            strcat(message, "processed because the operation that is linked to those events has a problem , or the event list has ");
            strcat(message, "events that belong to another context, to solve the above problems use the CheckStatusEvents class");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 215);
            strcpy(message, "You cannot perform fill the color of the Image object because the dimensions you provided in the ");
            strcat(message, "region array or when creating the Image object are not supported by the device associated with the ");
            strcat(message, "CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 211);
            strcpy(message, "You cannot perform color fill the Image object because the format you specified in the ");
            strcat(message, "imageChannelOrder and imageChannelDataType variables are not supported by the device associated with ");
            strcat(message, "the CommandQueue object");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 149);
            strcpy(message, "You cannot fill the color of the Image object because there is a problem with the memory block ");
            strcat(message, "of the buffer that you used ");
            strcat(message, "to create the Image object");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueCopyImageToBuffer(cl_int currentCommandQueue, cl_int currentImage, cl_int currentBuffer, const size_t *origin,
                                   const size_t *region, size_t offsetCopy) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem image = *listImage[currentImage];
    cl_mem buffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueCopyImageToBuffer(commandQueue, image, buffer, origin, region, offsetCopy, positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 111);
            strcpy(message, "You cannot perform copy the Image object to the buffer you provided, because the CommandQueue object ");
            strcat(message, "is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 99);
            strcpy(message, "You cannot perform copy the Image object to the buffer you provided, because the context is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 104);
            strcpy(message, "You cannot perform copy the Image object to the buffer you provided, because the Image object is ");
            strcat(message, "invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 877);
            strcpy(message, "You cannot copy the Image object to the buffer you provided, because the size of the image to be ");
            strcat(message, "copied according to the type of image that you specified in the ImageDescriptor object, and the ");
            strcat(message, "values of the origin and region arrays exceeds the size of the memory block of the buffer, another ");
            strcat(message, "possible problem is that the origin and region arrays have incorrect values according to the type of ");
            strcat(message, "image that you have set in the ImageDescriptor object, for example, for the image types ");
            strcat(message, "ImageDescriptor.TYPE_IMAGE1D and ImageDescriptor.TYPE_IMAGE1D_BUFFER the positions 1 and 2 of the ");
            strcat(message, "original array must be set to 0 and the positions 1 and 2 of the region array must be set to 1, ");
            strcat(message, "for the image type ImageDescriptor.TYPE_IMAGE1D_ARRAY and ImageDescriptor.TYPE_IMAGE2D the position 2 ");
            strcat(message, "of the source array must be set to 0, as well as the position 2 of the region array must be set to 1");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 506);
            strcpy(message, "You cannot copy the Image object to the buffer you provided, because there is a problem with the ");
            strcat(message, "list of events since the operation of any of the events has been completed and it is necessary to ");
            strcat(message, "remove those events from the list, there may also be events that will have problems because the ");
            strcat(message, "operation to which they are linked had a problem, there may also be events that do not belong to the ");
            strcat(message, "same context as the CommandQueue object, to correct the above problems you have to use the ");
            strcat(message, "CheckStatusEvents class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 282);
            strcpy(message, "You cannot copy the Image object to the buffer that you provided, because the buffer was not ");
            strcat(message, "created correctly, since the size of the buffer memory block is not multiple of the number of ");
            strcat(message, "bytes returned by the getMemBaseAddressAlign method of the device you used to create the buffer");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 238);
            strcpy(message, "You cannot copy the Image object to the buffer you provided, because the dimensions you provided ");
            strcat(message, "(such as height, width and depth, etc.) to the ImageDescriptor object are not supported by the ");
            strcat(message, "device associated with the CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 236);
            strcpy(message, "You cannot copy the Image object to the buffer you provided, because the format you set for the ");
            strcat(message, "imageChannelOrder variable and for the imageChannelDataType variable are not supported by the ");
            strcat(message, "device associated with the CommandQueue object");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 137);
            strcpy(message, "You cannot copy the Image object to the buffer you provided, because the buffer memory block is ");
            strcat(message, "not large enough to copy the Image object");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 262);
            strcpy(message, "You cannot copy the Image object to the buffer you provided because the device associated with ");
            strcat(message, "the CommandQueue object does not support the Images objects, this can be checked using the ");
            strcat(message, "getImageSupport method of the device associated with the CommandQueue object");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueCopyBufferToImage(cl_int currentCommandQueue, cl_int currentImage, cl_int currentBuffer, const size_t *origin,
                                   const size_t *region, size_t offsetCopy) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem image = *listImage[currentImage];
    cl_mem buffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueCopyBufferToImage(commandQueue, buffer, image, offsetCopy, origin, region, positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 119);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the CommandQueue object ");
            strcat(message, "you are using is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 107);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the context you are using ");
            strcat(message, "is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 106);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the buffer you are using ");
            strcat(message, "is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 1002);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the width, height, depth, ");
            strcat(message, "etc. data that you provided when you created the ImageDescriptor object, as well as the data that ");
            strcat(message, "you adjusted in the origin and region arrays exceed the size of the memory block of the buffer, ");
            strcat(message, "as well as the dimensions of the Image object, another possible problem is that the values you ");
            strcat(message, "provided in the origin and region arrays are incorrect for the type of image you have set in the ");
            strcat(message, "ImageDescriptor object that you used to create the Image object that is using to make the copy ");
            strcat(message, "of the buffer to the Image object, if the type of image you set was ImageDescriptor.TYPE_IMAGE1D or ");
            strcat(message, "ImageDescriptor.TYPE_IMAGE1D_BUFFER then position 1 and 2 of the origin array must be set to 0, and ");
            strcat(message, "the same positions of the region array must if set to 1, if you set the image type ");
            strcat(message, "ImageDescriptor.TYPE_IMAGE1D_ARRAY then position 2 of the original array must be 0 and the same ");
            strcat(message, "position of region arrangement must be in 1");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the event list contains ");
            strcat(message, "events that have already completed the operation to which the events are linked, there may also be ");
            strcat(message, "events that present problems because the operation to the events are linked had a problem, there ");
            strcat(message, "may also be events that belong to another context, to solve the above problems use the ");
            strcat(message, "CheckStatusEvents class");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 219);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the buffer block size ");
            strcat(message, "is not multiplied by the number of bytes returned by the getMemBaseAddressAlign method of the ");
            strcat(message, "device used to create the buffer");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 324);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the dimensions you used ");
            strcat(message, "to create the ImageDescriptor object that you then used to create the Image object you are ");
            strcat(message, "using, as well as the dimensions you provided for the region array are not supported by the ");
            strcat(message, "device associated with the CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 326);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the format and type of ");
            strcat(message, "data you set in the variables imageChannelOrder and imageChannelDataType, when you created the ");
            strcat(message, "ImageFormat object that you then used to create the Image object, are not supported by the ");
            strcat(message, "device associated with the object CommandQueue");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 182);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the buffer associated with ");
            strcat(message, "the Image object does not have a memory block large enough to complete the operation");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 274);
            strcpy(message, "You cannot copy the contents of the buffer to the Image object because the device associated with ");
            strcat(message, "the CommandQueue object does not support working with the Images object, you can check it using ");
            strcat(message, "the getImageSupport method of the device associated with the CommandQueue object");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueMapImage(cl_int currentCommandQueue, cl_int currentImage, const size_t *origin, const size_t *region,
                          size_t *numberBytesPerRow, size_t *numberBytesPerLayer, cl_bool isBlockingMap, cl_map_flags flagMap) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem image = *listImage[currentImage];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    CLEnqueueMapImage(commandQueue, image, isBlockingMap, flagMap, origin, region, numberBytesPerRow, numberBytesPerLayer,
                      positionCurrentEvent, listEvents, event, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 74);
            strcpy(message, "You cannot map the Image object because the CommandQueue object is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 101);
            strcpy(message, "You cannot map the Image object because the context used to create the CommandQueue object is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 67);
            strcpy(message, "You cannot map the Image object because the Image object is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 1003);
            strcpy(message, "You cannot map the Image object because the values specified in the origin and region arrays ");
            strcat(message, "exceed the size of the memory block of the buffer you used to create the Image object that you ");
            strcat(message, "want to map, another possible problem is that the values  of the origin and region arrays do not ");
            strcat(message, "were properly provided for the type of image that you specified in the ImageDescriptor object, ");
            strcat(message, "which you used to create the Image object you want to map, for the image types ");
            strcat(message, "ImageDescriptor.TYPE_IMAGE1D and ImageDescriptor.TYPE_IMAGE1D_BUFFER positions 1 and 2 of the ");
            strcat(message, "origin array must be set to 0, while positions 1 and 2 of the region array must be set to 1, ");
            strcat(message, "for image type ImageDescriptor.TYPE_IMAGE1D_ARRAY and ImageDescriptor.TYPE_IMAGE2D position 2 of ");
            strcat(message, "the original array must be set to 0 and position 2 of the array region must be set to 1, ");
            strcat(message, "remembering that the origin array is used to specify the offset in X, Y and Z, while e The ");
            strcat(message, "region arrangement is used to specify the Width, Height and Depth dimensions");
            break;
        case CL_INVALID_EVENT_WAIT_LIST: case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 354);
            strcpy(message, "You cannot map the Image object because there is a problem with the list of events, since there ");
            strcat(message, "may be events that the operation to which they are linked has ended or that operation has ");
            strcat(message, "presented a problem, there may also be events that belong to other contexts in the list of ");
            strcat(message, "events, to correct the above problems you must use the CheckStatusEvent class");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 275);
            strcpy(message, "You cannot map the Image object because the dimensions used such as width, height, depth, etc., ");
            strcat(message, "are not supported by the device associated with the CommandQueue object, to verify it you ");
            strcat(message, "must use the getImageSupport method of the device associated with the CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 265);
            strcpy(message, "You cannot map the Image object because the image format that you set in the imageChannelOrder ");
            strcat(message, "and imageChannelDataType variables of the ImageFormat object, which you used to create the Image ");
            strcat(message, "object is not supported by device associated with the CommandQueue object");
            break;
        case CL_MAP_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 142);
            strcpy(message, "You cannot map the Image object because the buffer you used to create the Image object was not ");
            strcat(message, "created with the Buffer.BUFER_USE_HOST_PTR flag");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 148);
            strcpy(message, "You cannot map the Image object because the buffer memory block that you used to create the ");
            strcat(message, "Image object is not large enough to map the Image object");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 186);
            strcpy(message, "You cannot map the Image object because you created the buffer with any of the following ");
            strcat(message, "flags:\nBuffer.BUFFER_HOST_READ_ONLY\nBuffer.BUFFER_HOST_WRITE_ONLY\nBuffer.BUFFER_HOST_NO_ACCESS");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueUnmapImage(cl_int currentCommandQueue, cl_int currentImage, cl_int currentBuffer) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem image = *listImage[currentImage];
    cl_mem buffer = *listBuffers[currentBuffer];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueUnmapMemObject(commandQueue, image, &buffer, positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 79);
            strcpy(message, "You cannot use the unmapImage method because the CommandQueue object is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 110);
            strcpy(message, "You cannot use the unmapImage method because the context you used to create the CommandQueue ");
            strcat(message, "object is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 72);
            strcpy(message, "You cannot use the unmapImage method because the Image object is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 98);
            strcpy(message, "You cannot use the unmapImage method because the buffer used to create the Image object is invalid");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 324);
            strcpy(message, "You cannot use the unmapImage method because there is a problem with the list of events, since one or more events have ");
            strcat(message, "completed the operation to which they are related, these operations may also have a problem or the events in the list ");
            strcat(message, "belong to others contexts, use the CheckStateEvents class to correct the above problems");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueMigrateImage(cl_int currentCommandQueue, cl_int sizeList, cl_int *listImagesToMigrate,
                              cl_mem_migration_flags flagMigrate) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_mem *_listImagesToMigrate = (cl_mem*)malloc(sizeof(cl_mem) * sizeList);
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    for (jint position = 0; position < sizeList; position++) {
        _listImagesToMigrate[position] = *listImage[listImagesToMigrate[position]];
    }
    state = CLEnqueueMigrateMemObjects(commandQueue, sizeList, _listImagesToMigrate, flagMigrate, positionCurrentEvent, listEvents,
                                       event);
    switch(state) {
        case CL_SUCCESS:
            processAddEventToListEvent(event[0]);
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 87);
            strcpy(message, "You cannot perform migrate the Image objects because the CommandQueue object is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 118);
            strcpy(message, "You cannot perform migrate the Image objects because the context you used to create the ");
            strcat(message, "CommandQueue object is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 90);
            strcpy(message, "You cannot perform migrate the Image objects because some of the Image objects are invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 182);
            strcpy(message, "You cannot perform the migration because the flag you provided is invalid, you can only use one ");
            strcat(message, "of the following flags:\nImage.MIGRATE_IMAGE_HOST\nImage.MIGRATE_IMAGE_CONTENT_UNDEFINED");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 300);
            strcpy(message, "You cannot perform migrate the Image objects because the list of events has events that are in ");
            strcat(message, "conflict or that have already completed the operation to which the events are linked, there may ");
            strcat(message, "also be events that belong to different contexts, to solve the problems above use the ");
            strcat(message, "CheckStatusEvents class");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 220);
            strcpy(message, "You cannot perform the migration of the Image objects because the memory blocks of the buffers ");
            strcat(message, "of each Image object do not have enough memory to perform the migration to the device associated ");
            strcat(message, "with the CommandQueue object");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleaseImage(cl_int currentImage, cl_int currentImageFormat, cl_int currentImageDescriptor) {
    cl_mem image = *listImage[currentImage];
    state = CLReleaseMemObject(image);
    switch(state) {
        case CL_SUCCESS:
            processReleaseImage(currentImage, currentImageFormat, currentImageDescriptor);
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 57);
            strcpy(message, "You cannot release the Image object because it is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreatePipe(cl_int currentContext, cl_mem_flags typeAccess, cl_uint packetSize, cl_uint numberPacket) {
    cl_context context = *listContexts[currentContext];
    cl_mem pipe = CLCreatePipe(context, typeAccess, packetSize, numberPacket, NULL, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddPipe(pipe);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 77);
            strcpy(message, "You cannot create the Pipe object because the context you provided is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 170);
            strcpy(message, "You cannot create the Pipe object because the type of access you provided is invalid, you can ");
            strcat(message, "only use the following values:\nPipe.PIPE_READ_WRITE\nPipe.PIPE_HOST_NO_ACCESS");
            break;
        case CL_INVALID_PIPE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 403);
            strcpy(message, "You cannot create the Pipe object because the size of the packages that you provided that the ");
            strcat(message, "Pipe object will handle exceeds the size that is allowed by all the devices that were created ");
            strcat(message, "with the context that you provided, so you can know the maximum size of the packages that you ");
            strcat(message, "can support a Pipe object, use the getPipeMaxPacketSize method of any device that was created ");
            strcat(message, "in the context you provided");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 105);
            strcpy(message, "You cannot create the Pipe object because there was a problem allocating memory to create the ");
            strcat(message, "Pipe object");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleasePipe(cl_int currentPipe) {
    cl_mem pipe = *listPipes[currentPipe];
    state = CLReleaseMemObject(pipe);
    switch(state) {
        case CL_SUCCESS:
            processReleasePipe(currentPipe);
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 83);
            strcpy(message, "You cannot release the Pipe object because the Pipe object you are using is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateSample(cl_int currentContext, cl_bool isNormalizedCoords, cl_int addressingMode, cl_int filterMode) {
    cl_context context = *listContexts[currentContext];
    cl_sampler_properties properties[] = { CL_SAMPLER_NORMALIZED_COORDS, (cl_sampler_properties)isNormalizedCoords,
                                           CL_SAMPLER_ADDRESSING_MODE, (cl_sampler_properties)addressingMode,
                                           CL_SAMPLER_FILTER_MODE, (cl_sampler_properties)filterMode, 0  };
    cl_sampler sampler = CLCreateSamplerWithProperties(context, properties, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddSampler(sampler);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 79);
            strcpy(message, "You cannot create the Sample object because the context you provided is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 434);
            strcpy(message, "You cannot create the Sampler object because the values of the addressingMode variables and the ");
            strcat(message, "filterMode variable are incorrect, you can only use the following values for the addressingMode ");
            strcat(message, "variable:\nSample.ADDRESS_NONE\nSample.ADDRESS_CLAMP_TO_EDGE\nSample.ADDRESS_CLAMP\n");
            strcat(message, "Sample.ADDRESS_REPEAT\nSample.ADDRESS_MIRRORED_REPEAT\nFor the filterMode variable you can ");
            strcat(message, "only use the following values:\nSample.FILTER_NEAREST\nSample.FILTER_LINEAR");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot create the Sample object because any device that has been created with the context ");
            strcat(message, "you provided does not support the use of the Images object, to verify it use the getImageSupport ");
            strcat(message, "method of any of those devices");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message,
    Result[1] = &state;
    return Result;
}
void **GetReleaseSample(cl_int currentSample) {
    cl_sampler sampler = *listSamplers[currentSample];
    state = CLReleaseSampler(sampler);
    switch(state) {
        case CL_SUCCESS:
            processReleaseSampler(currentSample);
            break;
        case CL_INVALID_SAMPLER:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 59);
            strcpy(message, "You cannot release the Sample object because it is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateProgramWithSource(cl_int currentContext, cl_uint numberFiles, const char **filesSources,
                                  const size_t *sizeNameFilesSources) {
    cl_context context = *listContexts[currentContext];
    cl_program program = CLCreateProgramWithSource(context, numberFiles, filesSources, sizeNameFilesSources, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddProgram(program);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 80);
            strcpy(message, "You cannot create the Program object because the context you provided is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 103);
            strcpy(message, "You cannot create the Program object because some of the paths of the source code files are ");
            strcat(message, "set to null");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateProgramWithIL(cl_int currentContext, jsize sizeArray, jbyte *dataProgramSPIRV) {
    cl_context context = *listContexts[currentContext];
    cl_program program = CLCreateProgramWithIL(context, dataProgramSPIRV, sizeArray, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddProgram(program);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 80);
            strcpy(message, "You cannot create the Program object because the context you provided is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 317);
            strcpy(message, "You cannot create the Program object because the data array you provided was not created using ");
            strcat(message, "the SPIRV intermediate language, first design a program using the SPIRV intermediate language, ");
            strcat(message, "compile it with CMake and the generated library(s), then you will have to open them and extract ");
            strcat(message, "their content to a binary array");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateProgramWithBinaries(cl_int currentContext, cl_int *_listDevices, cl_int *listSubDevices, jint sizeListDevices, size_t *sizeFilesBinaries,
                                    const unsigned char **filesBinaries, cl_int *isLoadFileSuccessfulOverDevice) {
    cl_context context = *listContexts[currentContext];
    cl_device_id *__listDevices = (cl_device_id*)malloc(sizeof(cl_device_id) * sizeListDevices);
    for (jint position = 0; position < sizeListDevices; position++) {
        __listDevices[position] = listDevices[_listDevices[position]][listSubDevices[position]];
    }
    cl_program program = CLCreateProgramWithBinary(context, sizeListDevices, __listDevices, sizeFilesBinaries, filesBinaries, isLoadFileSuccessfulOverDevice,
                                                   &state);
    switch(state) {
        case CL_SUCCESS:
            processAddProgram(program);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 80);
            strcpy(message, "You cannot create the Program object because the context you provided is invalid");
            break;
        case CL_INVALID_VALUE: case CL_INVALID_BINARY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 645);
            strcpy(message, "You cannot create the Program object because some of the binary files that you provided are ");
            strcat(message, "corrupted, or are not supported by any of the devices in the list of devices you provided, will ");
            strcat(message, "be saved in the stateFilesBinaries list the states of the binary files that could and that ");
            strcat(message, "could not be processed on devices in the device list, where the value of 0 represents ");
            strcat(message, "binary files that could be processed correctly and another value other than 0 represents ");
            strcat(message, "binary files that could not be processed, with the list of resulting states you will determine ");
            strcat(message, "which binary file has problems with respect to the device that was assigned internally by opencl");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 136);
            strcpy(message, "You cannot create the Program object because the device list contains devices that were not ");
            strcat(message, "used to create the context that you provided");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateProgramWithBuiltInKernels(cl_int currentContext, cl_uint numDevices, cl_int *devicesList, cl_int *listSubDevices,
                                          const char *listKernelNames) {
    cl_context context = *listContexts[currentContext];
    cl_device_id *devices = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);
    for (jint position = 0; position < numDevices; position++) {
        devices[position] = listDevices[devicesList[position]][listSubDevices[position]];
    }
    cl_program program = CLCreateProgramWithBuiltInKernels(context, numDevices, devices, listKernelNames, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddProgram(program);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 80);
            strcpy(message, "You cannot create the Program object because the context you provided is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 192);
            strcpy(message, "You cannot create the Program object because in the list of kernel names there is at least one ");
            strcat(message, "kernel name that is not supported by any of the devices that you have provided in the device list");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 148);
            strcpy(message, "You cannot create the Program object because the list of devices you provided contains devices ");
            strcat(message, "that were not used to create the context you provided.");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleaseProgram(cl_int currentProgram) {
    cl_program program = *listPrograms[currentProgram];
    state = CLReleaseProgram(program);
    switch(state) {
        case CL_SUCCESS: processReleaseProgram(currentProgram); break;
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot release the Program object because it is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetProgramReleaseCallback(JNIEnv *env, jobject program, jobject callbackProgram) {
    jclass Program = env->GetObjectClass(program);
    jfieldID currentProgram = env->GetFieldID(Program, "currentProgram", "I");
    jint _currentProgram = env->GetIntField(program, currentProgram);
    cl_program _program = *listPrograms[_currentProgram];
    jclass CallbackProgram = env->GetObjectClass(callbackProgram);
    jmethodID notify = env->GetMethodID(CallbackProgram, "notify", "(Lcom/draico/asvappra/opencl/Program;)V");
    jobject methodNotify = env->ToReflectedMethod(CallbackProgram, notify, JNI_FALSE);
    objectProgram = env->NewGlobalRef(program);
    objectCallbackProgram = env->NewGlobalRef(callbackProgram);
    methodNotifyProgram = env->NewGlobalRef(methodNotify);
    void *dataUser[] = { env, &program, &callbackProgram, &methodNotify };
    state = CLSetProgramReleaseCallback(_program, notifyCallbackProgram, dataUser);
    switch(state) {
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 127);
            strcpy(message, "You cannot adjust the Callback to monitor the release of the Program object because the Program ");
            strcat(message, "object you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 124);
            strcpy(message, "You cannot adjust the Callback to monitor the release of the Program object because the callback ");
            strcat(message, "you provided is set o null");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetProgramSpecializationConstant(cl_int currentProgram, cl_uint specializationID, size_t sizeData, const void *data) {
    cl_program program = *listPrograms[currentProgram];
    state = CLSetProgramSpecializationConstant(program, specializationID, sizeData, data);
    switch(state) {
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 110);
            strcpy(message, "You cannot use the setProgramSpecializationConstant method because the Program object you are ");
            strcat(message, "using is invalid");
            break;
        case CL_INVALID_SPEC_ID:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 171);
            strcpy(message, "You cannot use the SetProgramSpecializationConstant method of the Program object you are using, ");
            strcat(message, "because the value you provided for the specializationID variable is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot use the SetProgramSpecializationConstant method of the Program object you are using, ");
            strcat(message, "because the data size does not match the data you provided in the dataSpecialization variable");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetBuildProgram(JNIEnv *env, cl_int currentProgram, cl_uint numDevices, jint *_listDevices, jint *listSubDevices,
                       const char *buildOptions, jobject program, jobject callbackBuildProgram) {
    cl_program _program = *listPrograms[currentProgram];
    cl_device_id *__listDevices = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);
    for (jint position = 0; position < numDevices; position++) {
        __listDevices[position] = listDevices[_listDevices[position]][listSubDevices[position]];
    }
    if (callbackBuildProgram == NULL) state = CLBuildProgram(_program, numDevices, __listDevices, buildOptions, NULL, NULL);
    else {
        jclass CallbackBuildProgram = env->GetObjectClass(callbackBuildProgram);
        jmethodID notify = env->GetMethodID(CallbackBuildProgram, "notify", "(Lcom/draico/asvappra/opencl/Program;)V");
        jobject methodNotify = env->ToReflectedMethod(CallbackBuildProgram, notify, JNI_FALSE);
        objectProgram = env->NewGlobalRef(program);
        objectCallbackProgram = env->NewGlobalRef(callbackBuildProgram);
        methodNotifyProgram = env->NewGlobalRef(methodNotify);
        void *dataUser[] = { env, &program, &callbackBuildProgram, &methodNotify };
        state = CLBuildProgram(_program, numDevices, __listDevices, buildOptions, notifyCallbackProgram, dataUser);
    }
    switch(state) {
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 80);
            strcpy(message, "You cannot build the program because the Program object you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 61);
            strcpy(message, "You cannot build the program because the device list is empty");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 264);
            strcpy(message, "You cannot build the program because the device list contains devices that were not used to ");
            strcat(message, "create the Program object using the context you provided in the methods:\ncreateProgramWithSource\n");
            strcat(message, "createProgramWithIL\ncreateProgramWithBinary\ncreateProgramWithBuiltInKernels");
            break;
        case CL_INVALID_BINARY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 612);
            strcpy(message, "You cannot build the program because the list of devices you provided has devices that are not ");
            strcat(message, "related to the Program object, another possible problem is that the binary file you provided when ");
            strcat(message, "you created the Program object with the createProgramWithBinary method is not an executable binary, ");
            strcat(message, "when you decide create a Program object with the createProgramWithBinary method, make sure that ");
            strcat(message, "the binary file is executable since it is not allowed to load library files directly, if you plan ");
            strcat(message, "to work with compiled libraries you must do so from the executable file that you load from the ");
            strcat(message, "createProgramWithBinary method");
            break;
        case CL_INVALID_BUILD_OPTIONS:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 387);
            strcpy(message, "You cannot build the program because the options you specified to build the program are invalid, ");
            strcat(message, "if you created the Program object using the createProgramWithBinary method then the options to ");
            strcat(message, "build the program must be null, separate one option from another using a white space, and only ");
            strcat(message, "you can use the options that are within the Program class some options need you to provide ");
            strcat(message, "some data");
            break;
        case CL_COMPILER_NOT_AVAILABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot build the program because the compiler is not available, you can check it using ");
            strcat(message, "the getCompilerAvailable method of any of the devices you used to create the context (which ");
            strcat(message, "you used to create the Program object), or of any of the devices you have provided to When ");
            strcat(message, "trying to build the program, this problem appears when you created the Program object using the ");
            strcat(message, "createProgramWithSource method, since it is necessary for the compiler to be available to ");
            strcat(message, "compile the source code of the c language that you provided in that method to create the ");
            strcat(message, "Program object you are currently using");
            break;
        case CL_BUILD_PROGRAM_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 459);
            strcpy(message, "You cannot build the program because there is a problem with the code you provided either from ");
            strcat(message, "the c language, or from the SPIRV api, or from the executable binary file, or another type of ");
            strcat(message, "problem, check the program's construction log using the getBuildLog method, for this you have ");
            strcat(message, "to provide the CallbackBuildProgram to the buildProgram method and use the getBuildLog method ");
            strcat(message, "of the Program object that is inside the callback so that it can solve the problem");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 408);
            strcpy(message, "You cannot build the program because the construction process cannot end (check the program's ");
            strcat(message, "building log to solve this problem), another problem is that you have attached a Kernel object ");
            strcat(message, "to the program, another possible problem is that the Program object you are using currently has ");
            strcat(message, "not been created using any of the following methods:\ncreateProgramWithSource\n");
            strcat(message, "createProgramWithIL\ncreateProgramWithBinary");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCompileProgram(JNIEnv *env, cl_int currentProgram, cl_uint numDevices, jint *_listDevices, jint *listSubDevices,
                         jint sizeListPrograms, jint *_listPrograms, const char *compileOptions, cl_uint numHeaderFiles,
                         const char **headerFiles, jobject program, jobject callbackCompileProgram) {
    cl_program _program = *listPrograms[currentProgram];
    cl_device_id *__listDevices = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);
    cl_program *__listPrograms = (cl_program*)malloc(sizeof(cl_program) * sizeListPrograms);
    for (jint position = 0; position < numDevices; position++) {
        __listDevices[position] = listDevices[_listDevices[position]][listSubDevices[position]];
    }
    for (jint position = 0; position < sizeListPrograms; position++) {
        __listPrograms[position] = *listPrograms[_listPrograms[position]];
    }
    if (callbackCompileProgram == NULL) {
        state = CLCompileProgram(_program, numDevices, __listDevices, compileOptions, numHeaderFiles, __listPrograms, headerFiles,
                                 NULL, NULL);
    } else {
        jclass CallbackCompileProgram = env->GetObjectClass(callbackCompileProgram);
        jmethodID notify = env->GetMethodID(CallbackCompileProgram, "notify", "(Lcom/draico/asvappra/opencl/Program;)V");
        jobject methodNotify = env->ToReflectedMethod(CallbackCompileProgram, notify, JNI_FALSE);
        objectProgram = env->NewGlobalRef(program);
        objectCallbackProgram = env->NewGlobalRef(callbackCompileProgram);
        methodNotifyProgram = env->NewGlobalRef(methodNotify);
        void *dataUser[] = { env, &program, &callbackCompileProgram, &methodNotify };
        state = CLCompileProgram(_program, numDevices, __listDevices, compileOptions, numHeaderFiles, __listPrograms, headerFiles,
                                 notifyCallbackProgram, dataUser);
    }
    switch(state) {
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 82);
            strcpy(message, "You cannot compile the program because the Program object you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 158);
            strcpy(message, "You cannot compile the program because there is a problem with the list of programs, or with the ");
            strcat(message, "list of devices or the list of header files that you provided");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 237);
            strcpy(message, "You cannot compile the program because the list of devices you provided contains at least one ");
            strcat(message, "invalid device, or that device does not belong to the context you used to create the Program ");
            strcat(message, "object you are using to try to compile the program");
            break;
        case CL_INVALID_COMPILER_OPTIONS:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 152);
            strcpy(message, "You cannot compile the program because the compilation options you provided are incorrect, if ");
            strcat(message, "you use more than 1 option use a whitespace to separate it");
            break;
        case CL_INVALID_OPERATION: case CL_COMPILE_PROGRAM_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 207);
            strcpy(message, "You cannot compile the program because the compiler is not available at the moment, since another ");
            strcat(message, "previous process is still using it, so you must wait for that process to finish compiling the ");
            strcat(message, "current program");
            break;
        case CL_COMPILER_NOT_AVAILABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 219);
            strcpy(message, "You cannot compile the current program because the compiler is not available in the list of ");
            strcat(message, "devices you provided, you can check it using the getCompilerAvailable method of the devices in ");
            strcat(message, "the list of devices you provided");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetLinkProgram(JNIEnv *env, cl_int currentContext, cl_uint numDevices, jint *_listDevices, jint *listSubDevices,
                      const char *linkOptions, cl_uint numPrograms, jint *_listPrograms, jobject program,
                      jobject callbackLinkPrograms) {
    cl_context context = *listContexts[currentContext];
    cl_device_id *__listDevices = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);
    cl_program *__listPrograms = (cl_program*)malloc(sizeof(cl_program) * numPrograms);
    for (jint position = 0; position < numDevices; position++) {
        __listDevices[position] = listDevices[_listDevices[position]][listSubDevices[position]];
    }
    for (jint position = 0; position < numPrograms; position++) {
        __listPrograms[position] = *listPrograms[_listPrograms[position]];
    }
    cl_program _program;
    if (callbackLinkPrograms == NULL) {
        _program = CLLinkProgram(context, numDevices, __listDevices, linkOptions, numPrograms, __listPrograms, NULL, NULL, &state);
    } else {
        jclass CallbackLinkProgram = env->GetObjectClass(callbackLinkPrograms);
        jmethodID notify = env->GetMethodID(CallbackLinkProgram, "notify", "(Lcom/draico/asvappra/opencl/Program;)V");
        jobject methodNotify = env->ToReflectedMethod(CallbackLinkProgram, notify, JNI_FALSE);
        objectProgram = env->NewGlobalRef(program);
        objectCallbackProgram = env->NewGlobalRef(callbackLinkPrograms);
        methodNotifyProgram = env->NewGlobalRef(methodNotify);
        void *dataUser[] = { env, &program, &callbackLinkPrograms, &methodNotify };
        _program = CLLinkProgram(context, numDevices, __listDevices, linkOptions, numPrograms, __listPrograms, notifyCallbackProgram,
                                 dataUser, &state);
    }
    switch(state) {
        case CL_SUCCESS:
            processAddProgram(_program);
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 71);
            strcpy(message, "You cannot link the program because the context you provided is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot link the program because there is a problem with the device lists and/or the program list");
            break;
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 174);
            strcpy(message, "You cannot link the program because the program list contains an invalid Program object, also the ");
            strcat(message, "problem may be that the Program object you are currently using has a problem");
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 171);
            strcpy(message, "You cannot link the program because the list of devices you provided contains devices that do not ");
            strcat(message, "belong to the list of devices you used to create the context you provided");
            break;
        case CL_INVALID_LINKER_OPTIONS:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 377);
            strcpy(message, "You cannot link the current program because the linking options are incorrect, the only options ");
            strcat(message, "available are the following (you can use more than 1 separating them with a whitespace):\n");
            strcat(message, "Program.CreateLibrary\nProgram.EnableLinkOptions\nProgram.DenormAreZero\nProgram.NoSignedZeros\n");
            strcat(message, "Program.UnsafeMathOptimizations\nProgram.FiniteMathOnly\nProgram.FastRelaxedMath\n");
            strcat(message, "Program.NoSubgroupIFP");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 688);
            strcpy(message, "You cannot link the program because the list of devices you provided contains at least 1 device ");
            strcat(message, "that is still running a compilation job, or building another program, and you will have to wait for ");
            strcat(message, "that process to finish or use another device that is not busy , another possible problem that the ");
            strcat(message, "list of Program objects you provided as well as the Program object you are using do not contain a ");
            strcat(message, "c code, SPIRV or the binary data of an executable program, which have been created using the ");
            strcat(message, "createProgramWithSource, createProgramWithIL or createProgramWithBinary methods , having first ");
            strcat(message, "compiled the code before performing the program binding operation, which will generate an ");
            strcat(message, "executable program");
            break;
        case CL_LINKER_NOT_AVAILABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot link the program because the list of devices provided by at least one device that ");
            strcat(message, "does not have the linker available, you can check it using the getLinkerAvailable method of the ");
            strcat(message, "device list to determine which of the devices are the ones that do not have the linker available");
            break;
        case CL_LINK_PROGRAM_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 134);
            strcpy(message, "You cannot link the program because there is a problem with at least one of the Programs objects ");
            strcat(message, "that you provided in the program list");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **UnloadPlatformCompiler(cl_int currentPlatform) {
    cl_platform_id platform = *listPlatforms[currentPlatform];
    state = CLUnloadPlatformCompiler(platform);
    switch(state) {
        case CL_INVALID_PLATFORM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "The compiler of the platform to which it was assigned cannot be released by using a Program ");
            strcat(message, "object using the buildProgram or compileProgram method, because the provided platform is invalid");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetProgramInfo(cl_int currentProgram, cl_program_info nameInfo, size_t *sizeData) {
    cl_program program = *listPrograms[currentProgram];
    state = CLGetProgramInfo(program, nameInfo, NULL, NULL, sizeData);
    switch(state) {
        case CL_SUCCESS: {
                switch(nameInfo) {
                    case CL_PROGRAM_SOURCE: case CL_PROGRAM_IL: case CL_PROGRAM_KERNEL_NAMES:
                        dataFile = (char*)malloc(sizeof(char) * *sizeData);
                        state = CLGetProgramInfo(program, nameInfo, *sizeData, dataFile, NULL);
                        break;
                    case CL_PROGRAM_BINARY_SIZES:
                        *sizeData = *sizeData / sizeof(size_t);
                        sizeFiles = (size_t*)malloc(sizeof(size_t) * *sizeData);
                        state = CLGetProgramInfo(program, nameInfo, *sizeData * sizeof(size_t), sizeFiles, NULL);
                        break;
                    case CL_PROGRAM_BINARIES:
                        {
                            *sizeData = *sizeData / sizeof(size_t);
                            jsize numberFiles = *sizeData;
                            jsize sizeTotalFiles = 0;
                            jsize sizeArray = 0;
                            for (jint position = 0; position < numberFiles; position++) {
                                sizeTotalFiles += sizeFiles[position];
                                if (sizeFiles[position] > sizeArray) sizeArray = sizeFiles[position];
                            }
                            dataBinaryFiles = (unsigned char**)malloc(sizeof((unsigned char*)malloc(sizeof(unsigned char) * sizeArray)) * *sizeData);
                            state = CLGetProgramInfo(program, nameInfo, sizeTotalFiles, dataBinaryFiles, NULL);
                        }
                        break;
                    case CL_PROGRAM_NUM_KERNELS:
                        state = CLGetProgramInfo(program, nameInfo, sizeof(size_t), &numKernels, NULL);
                        break;
                    case CL_PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT: case CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT:
                        state = CLGetProgramInfo(program, nameInfo, sizeof(cl_bool), &dataProgramPresent, NULL);
                        break;
                }
                switch (state) {
                    case CL_INVALID_VALUE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 160);
                        strcpy(message, "You cannot get the value requested by the Program object you are using because the ");
                        strcat(message, "name of the information you are using to request the information is incorrect");
                        break;
                    case CL_INVALID_PROGRAM:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 92);
                        strcpy(message, "You cannot get the value requested by the Program object you are using because it is ");
                        strcat(message, "invalid");
                        break;
                    case CL_INVALID_PROGRAM_EXECUTABLE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 380);
                        strcpy(message, "You cannot get the number of kernels or kernel names because you have not yet ");
                        strcat(message, "compiled and linked the program to create a new Program object that is executable, ");
                        strcat(message, "if you use the buildProgram method a new Program object will not be created, because ");
                        strcat(message, "the same Program object it will be executable or a library all depends on the build ");
                        strcat(message, "options that you assign to the buildProgram method");
                        break;
                    case CL_OUT_OF_RESOURCES:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 93);
                        strcpy(message, "The requested information could not be processed, because the system has run out of ");
                        strcat(message, "resources");
                        break;
                    case CL_OUT_OF_HOST_MEMORY:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 85);
                        strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                        break;
                }
            }
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 160);
            strcpy(message, "You cannot get the value requested by the Program object you are using because the name of the ");
            strcat(message, "information you are using to request the information is incorrect");
            break;
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 92);
            strcpy(message, "You cannot get the value requested by the Program object you are using because it is invalid");
            break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 380);
            strcpy(message, "You cannot get the number of kernels or kernel names because you have not yet compiled and linked ");
            strcat(message, "the program to create a new Program object that is executable, if you use the buildProgram method ");
            strcat(message, "a new Program object will not be created, because the same Program object it will be executable ");
            strcat(message, "or a library all depends on the build options that you assign to the buildProgram method");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetProgramBuildInfo(cl_int currentProgram, cl_int currentDevice, cl_int currentSubDevice, cl_program_build_info nameInfo,
                           size_t *sizeData) {
    cl_program program = *listPrograms[currentProgram];
    cl_device_id device = listDevices[currentDevice][currentSubDevice];
    state = CLGetProgramBuildInfo(program, device, nameInfo, NULL, NULL, sizeData);
    switch(state) {
        case CL_SUCCESS: {
                switch(nameInfo) {
                    case CL_PROGRAM_BUILD_OPTIONS: case CL_PROGRAM_BUILD_LOG: {
                            dataBuildProgram = NULL;
                            dataBuildProgram = (char*)malloc(sizeof(char) * *sizeData);
                            state = CLGetProgramBuildInfo(program, device, nameInfo, *sizeData, dataBuildProgram, NULL);
                        }
                        break;
                    case CL_PROGRAM_BUILD_STATUS:
                        state = CLGetProgramBuildInfo(program, device, nameInfo, sizeof(cl_build_status), &buildStatus, NULL);
                        break;
                    case CL_PROGRAM_BINARY_TYPE:
                        state = CLGetProgramBuildInfo(program, device, nameInfo, sizeof(cl_program_binary_type), &programBinaryType,
                                                      NULL);
                        break;
                    case CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE:
                        state = CLGetProgramBuildInfo(program, device, nameInfo, sizeof(size_t), &sizeProgramVariables, NULL);
                        break;
                }
                switch(state) {
                    case CL_INVALID_DEVICE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 165);
                        strcpy(message, "You cannot obtain the requested information about the program construction process ");
                        strcat(message, "because the device you provided is not related to the Program object you are using");
                        break;
                    case CL_INVALID_VALUE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 157);
                        strcpy(message, "You cannot obtain the requested information about the program construction process ");
                        strcat(message, "because the name of the requested information you have provided is invalid");
                        break;
                    case CL_INVALID_PROGRAM:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 131);
                        strcpy(message, "You cannot get the requested information about the program construction process ");
                        strcat(message, "because the Program object you are using is invalid");
                        break;
                    case CL_OUT_OF_RESOURCES:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 93);
                        strcpy(message, "The requested information could not be processed, because the system has run out of ");
                        strcat(message, "resources");
                        break;
                    case CL_OUT_OF_HOST_MEMORY:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 85);
                        strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                        break;
                }
            }
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 165);
            strcpy(message, "You cannot obtain the requested information about the program construction process because the ");
            strcat(message, "device you provided is not related to the Program object you are using");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 157);
            strcpy(message, "You cannot obtain the requested information about the program construction process because the ");
            strcat(message, "name of the requested information you have provided is invalid");
            break;
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 131);
            strcpy(message, "You cannot get the requested information about the program construction process because the ");
            strcat(message, "Program object you are using is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateKernel(cl_int currentProgram, const char *kernelName) {
    cl_program program = *listPrograms[currentProgram];
    cl_kernel kernel = CLCreateKernel(program, kernelName, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddKernel(kernel);
            break;
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 87);
            strcpy(message, "You cannot create the kernel because because the Program object you provided is invalid");
            break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 130);
            strcpy(message, "You cannot create the kernel because because the Program object you provided was not created ");
            strcat(message, "correctly using an executable program");
            break;
        case CL_INVALID_KERNEL_NAME:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot create the kernel because because the name of the kernel you provided is not found ");
            strcat(message, "within the binaries of the Program object which also provided");
            break;
        case CL_INVALID_KERNEL_DEFINITION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 265);
            strcpy(message, "You cannot create the kernel because because the methods that you defined as kernels using the ");
            strcat(message, "__kernel identifier, the arguments number and its types are not the same for all devices, which ");
            strcat(message, "you used to build the program that you provided through the Program object");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 80);
            strcpy(message, "You cannot create the kernel because the kernel name you provided is set to null");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCreateKernelsInProgram(cl_int currentProgram, cl_uint *sizeArray) {
    cl_program program = *listPrograms[currentProgram];
    cl_kernel *listKernels = NULL;
    state = CLCreateKernelsInProgram(program, NULL, NULL, sizeArray);
    switch(state) {
        case CL_SUCCESS:
            listKernels = (cl_kernel*)malloc(sizeof(cl_kernel) * *sizeArray);
            state = CLCreateKernelsInProgram(program, *sizeArray, listKernels, NULL);
            switch(state) {
                case CL_SUCCESS:
                    {
                        jsize sizeList = *sizeArray;
                        for (jint position = 0; position < sizeList; position++) {
                            processAddKernel(listKernels[position]);
                        }
                    }
                    break;
                case CL_INVALID_PROGRAM:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 84);
                    strcpy(message, "You cannot create the kernel list because the Program object you provided is invalid");
                    break;
                case CL_INVALID_PROGRAM_EXECUTABLE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 197);
                    strcpy(message, "You cannot create the kernel list because the Program object you provided contains one ");
                    strcat(message, "or more executable binaries that were not created correctly for all devices associated ");
                    strcat(message, "with the Program object");
                    break;
                case CL_INVALID_VALUE:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 122);
                    strcpy(message, "You cannot create the kernel list because there is a problem in determining the number ");
                    strcat(message, "of kernels in the executable binary");
                    break;
                case CL_OUT_OF_RESOURCES:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 93);
                    strcpy(message, "The requested information could not be processed, because the system has run out of resources");
                    break;
                case CL_OUT_OF_HOST_MEMORY:
                    message = NULL;
                    message = (char*)malloc(sizeof(char) * 85);
                    strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                    break;
            }
            break;
        case CL_INVALID_PROGRAM:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 84);
            strcpy(message, "You cannot create the kernel list because the Program object you provided is invalid");
            break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 197);
            strcpy(message, "You cannot create the kernel list because the Program object you provided contains one or more ");
            strcat(message, "executable binaries that were not created correctly for all devices associated with the Program object");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 122);
            strcpy(message, "You cannot create the kernel list because there is a problem in determining the number of kernels ");
            strcat(message, "in the executable binary");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetReleaseKernel(cl_int currentKernel) {
    cl_kernel kernel = *listKernels[currentKernel];
    state = CLReleaseKernel(kernel);
    switch(state) {
        case CL_SUCCESS: processReleaseKernel(currentKernel); break;
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 73);
            strcpy(message, "You cannot release the kernel because the kernel you are using is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetKernelArg(cl_int currentKernel, cl_uint argumentIndex, size_t argumentSize, const void *value) {
    cl_kernel kernel = *listKernels[currentKernel];
    state = CLSetKernelArg(kernel, argumentIndex, argumentSize, value);
    switch(state) {
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 103);
            strcpy(message, "You cannot adjust the arguments to the kernel you are using because the kernel you are using ");
            strcat(message, "is invalid");
            break;
        case CL_INVALID_ARG_INDEX:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 187);
            strcpy(message, "You cannot adjust the arguments to the kernel you are using because the index of the argument ");
            strcat(message, "that is being used internally is not the same index of the type of argument that you provided");
            break;
        case CL_INVALID_ARG_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 117);
            strcpy(message, "You cannot adjust the arguments to the kernel you are using because the value of the argument ");
            strcat(message, "you provided is invalid");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 121);
            strcpy(message, "You cannot adjust the arguments to the kernel you are using because you are using an invalid ");
            strcat(message, "buffer, image, pipe or sample object");
            break;
        case CL_INVALID_SAMPLER: case CL_INVALID_DEVICE_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 121);
            strcpy(message, "You cannot adjust the arguments to the kernel you are using because you are using an invalid ");
            strcat(message, "Sample object as an argument");
            break;
        case CL_INVALID_ARG_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 195);
            strcpy(message, "You cannot adjust the arguments to the kernel you are using because the size of the data type ");
            strcat(message, "provided internally does not correspond to the size of the data type that you provided as an argument");
            break;
        case CL_MAX_SIZE_RESTRICTION_EXCEEDED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 142);
            strcpy(message, "You cannot adjust the arguments to the kernel you are using because the size of the type of data ");
            strcat(message, "you provided exceeds the maximum allowed size");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetKernelArgumentsSVMPointer(cl_int currentKernel, cl_uint argumentIndex, const void *value) {
    cl_kernel kernel = *listKernels[currentKernel];
    state = CLSetKernelArgSVMPointer(kernel, argumentIndex, value);
    switch(state) {
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 97);
            strcpy(message, "You cannot use the setKernelArgumentSVMPointer method because the kernel you are using is invalid");
            break;
        case CL_INVALID_ARG_INDEX:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 98);
            strcpy(message, "You cannot use the setKernelArgumentSVMPointer method because the index of the argument is invalid");
            break;
        case CL_INVALID_ARG_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 98);
            strcpy(message, "You cannot use the setKernelArgumentSVMPointer method because the value of the argument is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **SetKernelExecInfo(cl_int currentKernel, cl_uint execInfo, size_t sizeData, const void *data) {
    cl_kernel kernel = *listKernels[currentKernel];
    state = CLSetKernelExecInfo(kernel, execInfo, sizeData, data);
    switch(state) {
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 90);
            strcpy(message, "You cannot use the setKernelExecInfoSVM method because the kernel you are using is invalid");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 169);
            strcpy(message, "You cannot use the setKernelExecInfoSVM method because the value you provided for the execInfo ");
            strcat(message, "variable is invalid, or some of the arguments you provided are set to null");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 377);
            strcpy(message, "You cannot use the setKernelExecInfoSVM method because the value you provided for the execInfo ");
            strcat(message, "variable is Kernel.EXEC_INFO_SVM_FINE_GRAIN_SYSTEM and if the value you set in the Boolean or ");
            strcat(message, "Boolean argument is true, then a problem may occur because not all devices in the context you ");
            strcat(message, "used to create the Program object do not support the fine-grain system SVM allocations feature");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetCloneKernel(cl_int currentKernel) {
    cl_kernel kernel = *listKernels[currentKernel];
    cl_kernel kernelCloned = CLCloneKernel(kernel, &state);
    switch(state) {
        case CL_SUCCESS:
            processAddKernel(kernelCloned);
            break;
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 63);
            strcpy(message, "You cannot clone the kernel you are using because it is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetKernelInfo(cl_int currentKernel, cl_kernel_info infoName) {
    cl_kernel kernel = *listKernels[currentKernel];
    size_t sizeData;
    state = CLGetKernelInfo(kernel, infoName, NULL, NULL, &sizeData);
    switch(state) {
        case CL_SUCCESS:
            {
                dataKernel = NULL;
                dataIntKernel = 0;
                switch(infoName) {
                    case CL_KERNEL_FUNCTION_NAME: case CL_KERNEL_ATTRIBUTES:
                        dataKernel = (char*)malloc(sizeof(char) * sizeData);
                        state = CLGetKernelInfo(kernel, infoName, sizeData, dataKernel, NULL);
                        break;
                    case CL_KERNEL_NUM_ARGS:
                        state = CLGetKernelInfo(kernel, infoName, sizeData, &dataIntKernel, NULL);
                }
                switch(state) {
                    case CL_INVALID_VALUE: case CL_INVALID_KERNEL:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 84);
                        strcpy(message, "You cannot get the requested information because the kernel you are using is invalid");
                        break;
                    case CL_OUT_OF_RESOURCES:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 93);
                        strcpy(message, "The requested information could not be processed, because the system has run out of ");
                        strcat(message, "resources");
                        break;
                    case CL_OUT_OF_HOST_MEMORY:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 85);
                        strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                        break;
                }
            }
            break;
        case CL_INVALID_VALUE: case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 84);
            strcpy(message, "You cannot get the requested information because the kernel you are using is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetKernelWorkGroupInfo(cl_int currentKernel, cl_int currentDevice, cl_int currentSubDevice, cl_kernel_work_group_info nameInfo) {
    cl_kernel kernel = *listKernels[currentKernel];
    cl_device_id device = listDevices[currentDevice][currentSubDevice];
    size_t sizeData;
    state = CLGetKernelWorkGroupInfo(kernel, device, nameInfo, NULL, NULL, &sizeData);
    switch(state) {
        case CL_SUCCESS:
            {
                dataSizeTKernel = NULL;
                dataLongKernel = 0;
                switch(nameInfo) {
                    case CL_KERNEL_GLOBAL_WORK_SIZE: case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
                        dataSizeTKernel = (size_t*)malloc(sizeof(size_t) * 3);
                        state = CLGetKernelWorkGroupInfo(kernel, device, nameInfo, sizeData, dataSizeTKernel, NULL);
                        break;
                    case CL_KERNEL_WORK_GROUP_SIZE: case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
                        dataSizeTKernel = (size_t*)malloc(sizeof(size_t));
                        state = CLGetKernelWorkGroupInfo(kernel, device, nameInfo, sizeData, dataSizeTKernel, NULL);
                        break;
                    case CL_KERNEL_LOCAL_MEM_SIZE: case CL_KERNEL_PRIVATE_MEM_SIZE:
                        state = CLGetKernelWorkGroupInfo(kernel, device, nameInfo, sizeData, &dataLongKernel, NULL);
                        break;
                }
                switch(state) {
                    case CL_INVALID_DEVICE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 290);
                        strcpy(message, "You cannot obtain the requested information from the kernel because the device you ");
                        strcat(message, "provided does not belong to the list of devices that you used to create, compile, ");
                        strcat(message, "link or build the program you used to create the kernel you are currently using, or ");
                        strcat(message, "if the provided device is invalid or null");
                        break;
                    case CL_INVALID_VALUE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 222);
                        strcpy(message, "You cannot get the requested information from the kernel because you are trying to ");
                        strcat(message, "use the getGlobalWorkSize method, since an not custom device is being used ");
                        strcat(message, "internally and the kernel you are using is not a built-in kernel");
                        break;
                    case CL_INVALID_KERNEL:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 100);
                        strcpy(message, "You cannot get the requested information from the kernel because the kernel you are ");
                        strcat(message, "using is invalid");
                        break;
                    case CL_OUT_OF_RESOURCES:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 93);
                        strcpy(message, "The requested information could not be processed, because the system has run out ");
                        strcat(message, "of resources");
                        break;
                    case CL_OUT_OF_HOST_MEMORY:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 85);
                        strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                        break;
                }
            }
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 387);
            strcpy(message, "You cannot get the requested information from the kernel because the device that was used ");
            strcat(message, "internally does not belong to the list of devices that was used to create, compile, link or ");
            strcat(message, "build the program you used to create the kernel you are currently using, or if the device ");
            strcat(message, "used internally is invalid or null, but there are more devices present in the list of devices ");
            strcat(message, "of the Program object");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 222);
            strcpy(message, "You cannot get the requested information from the kernel because you are trying to use the ");
            strcat(message, "getGlobalWorkSize method, since an not custom device is being used internally and the kernel ");
            strcat(message, "you are using is not a built-in kernel");
            break;
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot get the requested information from the kernel because the kernel you are using is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetKernelSubGroupInfo(cl_int currentKernel, cl_int currentDevice, cl_int currentSubDevice, cl_kernel_sub_group_info nameInfo,
                             size_t sizeDataInputInBytes, const void *dataInput, size_t *sizeData) {
    cl_kernel kernel = *listKernels[currentKernel];
    cl_device_id device = listDevices[currentDevice][currentSubDevice];
    state = CLGetKernelSubGroupInfo(kernel, device, nameInfo, sizeDataInputInBytes, dataInput, NULL, NULL, sizeData);
    switch(state) {
        case CL_SUCCESS:
            {
                dataSizeTKernel = NULL;
                switch(nameInfo) {
                    case CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE: case CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE:
                    case CL_KERNEL_MAX_NUM_SUB_GROUPS: case CL_KERNEL_COMPILE_NUM_SUB_GROUPS:
                        dataSizeTKernel = (size_t*)malloc(sizeof(size_t));
                        break;
                    case CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT:
                        dataSizeTKernel = (size_t*)malloc(sizeof(size_t) * (*sizeData / sizeof(size_t)));
                        break;
                }
                state = CLGetKernelSubGroupInfo(kernel, device, nameInfo, sizeDataInputInBytes, dataInput, *sizeData, dataSizeTKernel, NULL);
                switch(state) {
                    case CL_INVALID_DEVICE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 272);
                        strcpy(message, "The device you have provided not belong to the list of devices that you used to ");
                        strcat(message, "create the Program object that you used to create the kernel you are using, so the ");
                        strcat(message, "device not belong to the list of devices that you used when you compiled, linked and ");
                        strcat(message, "built the object Program");
                        break;
                    case CL_INVALID_VALUE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 165);
                        strcpy(message, "You cannot get the information about the kernel sub groups you are using, because ");
                        strcat(message, "the size in bytes of the information you provided could not be calculated correctly");
                        break;
                    case CL_INVALID_KERNEL:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 103);
                        strcpy(message, "You cannot get the information about the kernel sub groups you are using, because ");
                        strcat(message, "the kernel is invalid");
                        break;
                    case CL_OUT_OF_RESOURCES:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 93);
                        strcpy(message, "The requested information could not be processed, because the system has run out ");
                        strcat(message, "of resources");
                        break;
                    case CL_OUT_OF_HOST_MEMORY:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 85);
                        strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
                        break;
                }
            }
            break;
        case CL_INVALID_DEVICE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 272);
            strcpy(message, "The device you have provided not belong to the list of devices that you used to create the ");
            strcat(message, "Program object that you used to create the kernel you are using, so the device not belong to ");
            strcat(message, "the list of devices that you used when you compiled, linked and built the object Program");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 165);
            strcpy(message, "You cannot get the information about the kernel sub groups you are using, because the size in ");
            strcat(message, "bytes of the information you provided could not be calculated correctly");
            break;
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 103);
            strcpy(message, "You cannot get the information about the kernel sub groups you are using, because the kernel ");
            strcat(message, "is invalid");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetKernelArgInfo(cl_int currentKernel, cl_uint argumentIndex, cl_kernel_arg_info nameInfo) {
    cl_kernel kernel = *listKernels[currentKernel];
    size_t sizeData;
    state = CLGetKernelArgInfo(kernel, argumentIndex, nameInfo, NULL, NULL, &sizeData);
    switch(state) {
        case CL_SUCCESS:
            {
                switch(nameInfo) {
                    case CL_KERNEL_ARG_ADDRESS_QUALIFIER: case CL_KERNEL_ARG_ACCESS_QUALIFIER:
                        state = CLGetKernelArgInfo(kernel, argumentIndex, nameInfo, sizeData, &dataIntKernel, NULL);
                        break;
                    case CL_KERNEL_ARG_TYPE_NAME: case CL_KERNEL_ARG_NAME:
                        dataKernel = NULL;
                        dataKernel = (char*)malloc(sizeof(char) *  sizeData);
                        state = CLGetKernelArgInfo(kernel, argumentIndex, nameInfo, sizeData, dataKernel, NULL);
                        break;
                    case CL_KERNEL_ARG_TYPE_QUALIFIER:
                        state = CLGetKernelArgInfo(kernel, argumentIndex, nameInfo, sizeData, &dataBitfieldKernel, NULL);
                        break;
                }
                switch(state) {
                    case CL_INVALID_ARG_INDEX:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 137);
                        strcpy(message, "You cannot get the requested information from the argument because the index you ");
                        strcat(message, "provided for a specific argument is not a valid argument");
                        break;
                    case CL_INVALID_VALUE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 270);
                        strcpy(message, "You cannot obtain the requested information from the argument because the name ");
                        strcat(message, "of the information (which is handled internally) is incorrect, and also the size ");
                        strcat(message, "of the information does not correspond to the type of information requested (which ");
                        strcat(message, "is also handled internally)");
                        break;
                    case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 127);
                        strcpy(message, "You cannot get the requested information from the argument because the information ");
                        strcat(message, "is not available in the kernel you are using");
                        break;
                    case CL_INVALID_KERNEL:
                        message = NULL;
                        message = (char*)malloc(sizeof(char) * 102);
                        strcpy(message, "You cannot get the requested information from the argument because the kernel you ");
                        strcat(message, "are using is invalid");
                        break;
                }
            }
            break;
        case CL_INVALID_ARG_INDEX:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 137);
            strcpy(message, "You cannot get the requested information from the argument because the index you provided for ");
            strcat(message, "a specific argument is not a valid argument");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 270);
            strcpy(message, "You cannot obtain the requested information from the argument because the name of the information ");
            strcat(message, "(which is handled internally) is incorrect, and also the size of the information does not ");
            strcat(message, "correspond to the type of information requested (which is also handled internally)");
            break;
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 127);
            strcpy(message, "You cannot get the requested information from the argument because the information is not available");
            strcat(message, "in the kernel you are using");
            break;
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 102);
            strcpy(message, "You cannot get the requested information from the argument because the kernel you are using ");
            strcat(message, "is invalid");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueNDRangeKernel(cl_int currentKernel, cl_int currentCommandQueue, cl_uint workNumberDimensions,
                               const size_t *globalWorkOffset, const size_t *globalWorkSize, const size_t *localWorkSize) {
    cl_kernel kernel = *listKernels[currentKernel];
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    state = CLEnqueueNDRangeKernel(commandQueue, kernel, workNumberDimensions, globalWorkOffset, globalWorkSize, localWorkSize,
                                   positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS: processAddEventToListEvent(event[0]); break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 241);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because the CommandQueue object you ");
            strcat(message, "provided was created with a device that does not belong to the list of devices used to create ");
            strcat(message, "the program you used to create the kernel you are using");
            break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 111);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because the CommandQueue object you ");
            strcat(message, "provided is invalid");
            break;
        case CL_INVALID_KERNEL:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 115);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because the kernel that the kernel you ");
            strcat(message, "are using is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 270);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because the context you used to create ");
            strcat(message, "the CommandQueue object you provided, as well as the context you used to create the Program ");
            strcat(message, "object, which you used to create the kernel you are using, are not the same context");
            break;
        case CL_INVALID_KERNEL_ARGS:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 290);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because you have not yet adjusted the ");
            strcat(message, "list of arguments that have a method that you defined with the __kernel qualifier, to adjust ");
            strcat(message, "the arguments use the setKernelArguments, setKernelArgumentsSVMPointer and ");
            strcat(message, "setKernelExecInfoSVM methods");
            break;
        case CL_INVALID_WORK_DIMENSION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 286);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because the number of dimensions that ");
            strcat(message, "you specified in the workNumberDimensions variable is incorrect, you can only use 1 to specify ");
            strcat(message, "that it will work unidimensionally, 2 to work two-dimensionally and 3 to work three-dimensionally");
            break;
        case CL_INVALID_GLOBAL_WORK_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 628);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method, because at least 1 of the values in the ");
            strcat(message, "globalWorkSize array is greater than the values returned by the getMaxWorkItemSizes method, of any of ");
            strcat(message, "the devices you used to create the Program object, which you used to create the kernel you are using, ");
            strcat(message, "where the first position of the globalWorkSize array must be less than or equal to the first position ");
            strcat(message, "of the array returned by the getMaxWorkItemSizes method, and the following values of the globalWorkSize ");
            strcat(message, "array must be less than or equal to the value of the 3rd position of the array returned by the ");
            strcat(message, "getMaxWorkItemSizes method");
            break;
        case CL_INVALID_GLOBAL_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 844);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method, because the sum of the values corresponding ");
            strcat(message, "to the positions of the globalWorkOffset and globalWorkSize arrays, are greater than the values ");
            strcat(message, "returned by the getMaxWorkItemDimensions method of any of the devices you used to create the ");
            strcat(message, "Program object, the which one you used to create the kernel you are using, the 0 positions of both ");
            strcat(message, "the globalWorkOffset array and the globalWorkSize array must be less than or equal to the value of ");
            strcat(message, "the first position of the array returned by the getMaxWorkItemDimensions method, and the following ");
            strcat(message, "data from the globalWorkOffset arrays as of the globalWorkSize array the sum of the values of the ");
            strcat(message, "corresponding positions of both arrays must be less than or equal to the value of the 3rd position ");
            strcat(message, "of the array returned by the getMaxWorkItemDimensions method");
            break;
        case CL_INVALID_WORK_GROUP_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 659);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method, because the localWorkSize array does not ");
            strcat(message, "match the values you specified in the source code that you used to create the Program object (which ");
            strcat(message, "you used to create the kernel you are using), another possible problem is that the multiplication ");
            strcat(message, "of all data in the localWorkSize array exceeds the value returned by the getWorkGroupSize method of ");
            strcat(message, "the kernel you are using, another possible problem is that the Program object has compiled it using ");
            strcat(message, "the cl-uniform-work-group-size compilation option but the GlobalWorkSize array data is not multiples ");
            strcat(message, "of the localWorkSize data of the same positions in both arrays");
            break;
        case CL_INVALID_WORK_ITEM_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 360);
            strcpy(message, "You cannot run the kernel with the NDRange Kernel method because positions 0 and the last data ");
            strcat(message, "position of the localWorkSize array must be less than or equal to the data in positions 0 and 2 ");
            strcat(message, "of the array returned by the getMaxWorkItemSizes method of any of the devices that you used to ");
            strcat(message, "create the Program object that you used to create the kernel you are using");
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 418);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because the buffer you are using in the ");
            strcat(message, "program cannot be used with the device associated with the CommandQueue object, since the buffer ");
            strcat(message, "was created with the context you used to create the Program object which you used to create the ");
            strcat(message, "kernel you are using, so you will have to use the context you used to create the CommandQueue ");
            strcat(message, "object so you can create the buffer");
            break;
        case CL_INVALID_IMAGE_SIZE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 270);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because you specified at least one Image ");
            strcat(message, "object as an argument in a function that has the __kernel qualifier, and the size of that Image ");
            strcat(message, "object is not supported by the device associated with the CommandQueue object");
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 386);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because you are using at least one Image ");
            strcat(message, "type argument in a function with the __kernel qualifier, and the format of this Image object is ");
            strcat(message, "not supported by the device associated with the CommandQueue object, so you must use another ");
            strcat(message, "CommandQueue object that was created in the same context that you used to create those Image ");
            strcat(message, "objects");
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 293);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because you are using at least one argument ");
            strcat(message, "of the Image or buffer type in the function with the __kernel qualifier, and there was a problem ");
            strcat(message, "with the allocation of memory to instantiate the Image and/or Buffer objects as kernel arguments");
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 570);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because there is a problem in the list of ");
            strcat(message, "events, since there may be events that have completed the process to which they are linked, there ");
            strcat(message, "could also be a problem in any of the processes linked to the events that are in conflict, ");
            strcat(message, "another possible problem is that all events in the event list do not belong to the same context, ");
            strcat(message, "to correct the above problems, use the CheckStatusEvent class to solve the above problems, as well ");
            strcat(message, "as to eliminate the events that have already been completed. processes linked to events");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 313);
            strcpy(message, "You cannot run the kernel with the NDRangeKernel method because you are using memory blocks as ");
            strcat(message, "arguments in functions that have the __kernel qualifier, and the devices associated with the ");
            strcat(message, "context used to create the CommandQueue object, cannot work with memory blocks defined as ");
            strcat(message, "kernel arguments what are you using");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
void **GetEnqueueNativeKernel(JNIEnv *env, cl_int currentCommandQueue, void *listArguments, size_t sizeListArguments,
                              cl_int sizeListMemObjects, const cl_mem *listMemObjects, const void **listMemObjectsAddress,
                              jobject functionCustom, jobjectArray argumentsFunctionCustom) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    cl_event *event = (cl_event*)malloc(sizeof(cl_event));
    dataFunctionNativeKernel[0] = env;
    dataFunctionNativeKernel[1] = &functionCustom;
    dataFunctionNativeKernel[2] = &argumentsFunctionCustom;
    state = CLEnqueueNativeKernel(commandQueue, functionNativeKernel, listArguments, sizeListArguments, sizeListMemObjects,
                                  listMemObjects, listMemObjectsAddress, positionCurrentEvent, listEvents, event);
    switch(state) {
        case CL_SUCCESS: processAddEventToListEvent(*event); break;
        case CL_INVALID_COMMAND_QUEUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 146);
            strcpy(message, "You cannot execute the custom function using the kernel nativeKernel method you are using, because ");
            strcat(message, "the CommandQueue object you provided is invalid");
            break;
        case CL_INVALID_CONTEXT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 288);
            strcpy(message, "You cannot execute the custom function using the kernel nativeKernel method you are using, because ");
            strcat(message, "the context you used to create the CommandQueue object you provided and the events in the event ");
            strcat(message, "list do not belong to the same event, to correct this problem use the class CheckStatusEvents");
            break;
        case CL_INVALID_VALUE:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 146);
            strcpy(message, "You cannot execute the custom function using the kernel nativeKernel method you are using, because ");
            strcat(message, "the list of arguments you provided has problems");
            break;
        case CL_INVALID_OPERATION:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 341);
            strcpy(message, "You cannot execute the custom function using the nativeKernel method of the kernel you are using, ");
            strcat(message, "because the device associated with the CommandQueue object you provided cannot execute the ");
            strcat(message, "nativeKernel method, use a device that is related to the kernel you are using, to create an ");
            strcat(message, "object CommandQueue that can execute the nativeKernel method");
            break;
        case CL_INVALID_MEM_OBJECT:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 215);
            strcpy(message, "You cannot execute the custom function using the kernel nativeKernel method you are using, because ");
            strcat(message, "Image, Buffer and Pipe object that are invalid both in the custom function and in the list of ");
            strcat(message, "arguments you provided");
            break;
        case CL_OUT_OF_RESOURCES:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 93);
            strcpy(message, "The requested information could not be processed, because the system has run out of resources");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            message = NULL;
            message = (char*)malloc(sizeof(char) * 85);
            strcpy(message, "The requested operation could not be completed, because the system ram memory ran out");
            break;
    }
    Result[0] = message;
    Result[1] = &state;
    return Result;
}
cl_platform_id getPlatformsIDs(cl_int position) { return *listPlatforms[position]; }
void addImageFormatToListImageFormat(cl_int imageChannelOrder, cl_int imageChannelDataType) {
    cl_image_format *imageFormat = (cl_image_format*)malloc(sizeof(cl_image_format));
    imageFormat->image_channel_order = imageChannelOrder;
    imageFormat->image_channel_data_type = imageChannelDataType;
    *listImageFormat[positionCurrentImage] = imageFormat[0];
}
void addImageDescriptorToListImageDescriptor(cl_mem_object_type imageType, size_t imageWidth, size_t imageHeight, size_t imageDepth,
                                             size_t numberImages, size_t numberBytesPerRow, size_t numberBytesPerLayer,
                                             cl_int currentBuffer, cl_bool addBuffer) {
    cl_mem buffer = NULL;
    if (addBuffer) buffer = *listBuffers[currentBuffer];
    listImageDescriptor[positionCurrentImageDescriptor] = (cl_image_desc*)malloc(sizeof(cl_image_desc));
    listImageDescriptor[positionCurrentImageDescriptor]->image_type = imageType;
    listImageDescriptor[positionCurrentImageDescriptor]->image_width = imageWidth;
    listImageDescriptor[positionCurrentImageDescriptor]->image_height = imageHeight;
    listImageDescriptor[positionCurrentImageDescriptor]->image_depth = imageDepth;
    listImageDescriptor[positionCurrentImageDescriptor]->image_array_size = numberImages;
    listImageDescriptor[positionCurrentImageDescriptor]->image_row_pitch = numberBytesPerRow;
    listImageDescriptor[positionCurrentImageDescriptor]->image_slice_pitch = numberBytesPerLayer;
    listImageDescriptor[positionCurrentImageDescriptor]->num_mip_levels = 0;
    listImageDescriptor[positionCurrentImageDescriptor]->num_samples = 0;
    listImageDescriptor[positionCurrentImageDescriptor]->mem_object = buffer;
}
void **getListImageFormatSupportByTypImage() { return dataListImageFormat; }
void getDataImageDescriptorSelected(cl_int currentImageDescriptor) {
    dataImageDescriptor[0] = &listImageDescriptor[currentImageDescriptor]->image_type;
    dataImageDescriptor[1] = &listImageDescriptor[currentImageDescriptor]->image_width;
    dataImageDescriptor[2] = &listImageDescriptor[currentImageDescriptor]->image_height;
    dataImageDescriptor[3] = &listImageDescriptor[currentImageDescriptor]->image_depth;
    dataImageDescriptor[4] = &listImageDescriptor[currentImageDescriptor]->image_array_size;
    dataImageDescriptor[5] = &listImageDescriptor[currentImageDescriptor]->image_row_pitch;
    dataImageDescriptor[6] = &listImageDescriptor[currentImageDescriptor]->image_slice_pitch;
    dataImageDescriptor[7] = &listImageDescriptor[currentImageDescriptor]->num_mip_levels;
    dataImageDescriptor[8] = &listImageDescriptor[currentImageDescriptor]->num_samples;
}
void processAddPlatforms(cl_platform_id *platforms) {
    size_t numberPlatforms = sizeof(platforms) / sizeof(cl_platform_id);
    for (cl_int position = 0; position < numberPlatforms; position++) {
        *listPlatforms[position] = platforms[position];
    }
}
void processAddDevices(cl_device_id *devices, cl_int numberDevices) {
    listDevices[positionCurrentDevice] = devices;
    listNumbersDevices[positionCurrentDevice] = numberDevices;
    for (jint position = 0; position < numberDevicesInCurrentPlatform; position++) {
        CLRetainDevice(devices[position]);
    }
}
void processAddDevicePartitions(cl_device_id *listDevicePartitions, cl_int numberPartitions) {
    cl_int position;
    size_t sizeListDevices = positionCurrentDevice;
    listDevices[position] = listDevicePartitions;
    listNumbersDevices[position] = numberPartitions;
    for (position = 0; position < numberPartitions; position++) {
        CLRetainDevice(listDevicePartitions[position]);
    }
}
void processAddContext(cl_context *context) {
    listContexts[positionCurrentContext] = context;
}
void processReleaseContext(cl_int currentContext) {
    listContexts[positionCurrentContext] = NULL;
    if (positionCurrentContext > 0) positionCurrentContext--;
}
void processAddCommandQueue(cl_command_queue commandQueue) {
    *listCommandQueue[positionCurrentCommandQueue] = commandQueue;
}
void processReleaseCommandQueue(cl_int currentCommandQueue) {
    listCommandQueue[positionCurrentCommandQueue] = NULL;
    if (positionCurrentCommandQueue > 0) positionCurrentCommandQueue--;
}
void processCreateBuffer(cl_mem buffer) {
    *listBuffers[positionCurrentBuffer] = buffer;
}
void processCreateSubBuffer(cl_mem buffer, cl_buffer_region bufferRegion) {
    jsize sizeListBuffer = positionCurrentBuffer;
    cl_mem *listBufferTemp = (cl_mem*)malloc(sizeof(cl_mem) * sizeListBuffer);
    *listBuffers[positionCurrentBuffer] = buffer;
    *listSubBuffersRegion[positionCurrentBufferRegion] = bufferRegion;
}
void processReleaseBuffer(cl_int currentBuffer, cl_bool isSubBuffer, cl_int currentBufferRegion) {
    listBuffers[positionCurrentBuffer] = NULL;
    listSubBuffersRegion[positionCurrentBufferRegion] = NULL;
    if (positionCurrentBuffer > 0) positionCurrentBuffer--;
    if (positionCurrentBufferRegion > 0) positionCurrentBufferRegion--;
}
void processSVMFree(cl_int *listMemory, cl_int numberBlocksMemory) {
    for (jint position = 0; position < numberBlocksMemory; position++) {
        listDataMemory[listMemory[position]] = NULL;
        listSizeBlocksMemory[listMemory[position]] = NULL;
    }
    positionCurrentMemory -= numberBlocksMemory;
}
void processAddEventToListEvent(cl_event event) {
    listEvents[positionCurrentEvent] = event;
}
void processReleaseEvent(cl_int currentEvent) {
    cl_event event = NULL;
    listEvents[currentEvent] = event;
}
void processAddImageToListImages(cl_mem image) {
    *listImage[positionCurrentImage] = image;
}
void processAddListImageFormats(cl_image_format *list, cl_int sizeList) {
    for (jint position = 0; position < sizeList; position++) {
        *listImageFormat[positionCurrentImageFormat + position] = list[position];
    }
    if (positionCurrentImageFormat + sizeList < 100) positionCurrentImageFormat += sizeList;
    else {
        jint size = 100 - positionCurrentImageFormat;
        jint finalSize = sizeList - size;
        for (jint position = 0; position < size; position++) *listImageFormat[positionCurrentImageFormat + position] = list[position];
        positionCurrentImageFormat = 0;
        for (jint position = size; position < sizeList; position++, positionCurrentImageFormat++)
            *listImageFormat[positionCurrentImageFormat] = list[position];
    }
}
void processReleaseImage(cl_int currentImage, cl_int currentImageFormat, cl_int currentImageDescriptor) {
    listImage[positionCurrentImage] = NULL;
    listImageFormat[positionCurrentImageFormat] = NULL;
    listImageDescriptor[positionCurrentImageDescriptor] = NULL;
    if (positionCurrentImage > 0) positionCurrentImage--;
    if (positionCurrentImageFormat > 0) positionCurrentImageFormat--;
    if (positionCurrentImageDescriptor > 0) positionCurrentImageDescriptor--;
}
void processAddPipe(cl_mem pipe) {
    *listPipes[positionCurrentPipe] = pipe;
}
void processReleasePipe(cl_int currentPipe) {
    listPipes[positionCurrentPipe] = NULL;
    if (positionCurrentPipe > 0) positionCurrentPipe--;
}
void processAddSampler(cl_sampler sampler) {
    *listSamplers[positionCurrentSampler] = sampler;
}
void processReleaseSampler(cl_int currentSample) {
    listSamplers[positionCurrentSampler] = NULL;
    if (positionCurrentSampler > 0) positionCurrentSampler--;
}
void processAddProgram(cl_program program) {
    *listPrograms[positionCurrentProgram] = program;
}
void processReleaseProgram(cl_int currentProgram) {
    listPrograms[positionCurrentProgram] = NULL;
    if (positionCurrentProgram > 0) positionCurrentProgram--;
}
void processAddKernel(cl_kernel kernel) {
    *listKernels[positionCurrentKernel] = kernel;
}
void processReleaseKernel(cl_int currentKernel) {
    listKernels[positionCurrentKernel] = NULL;
    if (positionCurrentKernel > 0) positionCurrentKernel--;
}
void CL_CALLBACK notifyCallbackContext(const char *errorInfo, const void *privateInfo, size_t cb, void *userData) {
    void **dataInfo = (void**)userData;
    JNIEnv *env = (JNIEnv*)dataInfo[0];
    jclass Integer = env->FindClass("java/lang/Integer");
    jclass Method = env->FindClass("java/lang/reflect/Method");
    jclass Object = env->FindClass("java/lang/Object");
    jmethodID constructorInteger = env->GetMethodID(Integer, "<init>", "(I)V");
    jmethodID invoke = env->GetMethodID(Method, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    jobjectArray arguments = env->NewObjectArray(3, Object, NULL);
    jstring _errorInfo = env->NewStringUTF(errorInfo);
    jbyte *dataBinary = (jbyte*)privateInfo;
    jbyteArray _dataBinary = env->NewByteArray(cb);
    jobject _cb = env->NewObject(Integer, constructorInteger, (jint)cb);
    env->SetByteArrayRegion(_dataBinary, 0, cb, dataBinary);
    env->SetObjectArrayElement(arguments, 0, _errorInfo);
    env->SetObjectArrayElement(arguments, 1, _dataBinary);
    env->SetObjectArrayElement(arguments, 2, _cb);
    env->CallObjectMethod(_methodNofityContext, invoke, _callbackContext, arguments);
    env->DeleteGlobalRef(_methodNofityContext);
    env->DeleteGlobalRef(_callbackContext);
}
void CL_CALLBACK notifyCallbackEvent(cl_event __eventCallback, cl_int statusCommandExecution, void *userData) {
    void **dataInfo = (void**)userData;
    JNIEnv *env = (JNIEnv*)dataInfo[0];
    jvm->AttachCurrentThread(&env, NULL);
    jclass Method = env->FindClass("java/lang/reflect/Method");
    jclass Integer = env->FindClass("java/lang/Integer");
    jclass Object = env->FindClass("java/lang/Object");
    jmethodID constructorInteger = env->GetMethodID(Integer, "<init>", "(I)V");
    jmethodID invoke = env->GetMethodID(Method, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    jobjectArray arguments = env->NewObjectArray(2, Object, NULL);
    jobject integer = env->NewObject(Integer, constructorInteger, (jint)statusCommandExecution);
    env->SetObjectArrayElement(arguments, 0, eventCallback);
    env->SetObjectArrayElement(arguments, 1, integer);
    env->CallObjectMethod(methodNotify, invoke, _callbackEvent, arguments);
    env->DeleteGlobalRef(eventCallback);
    env->DeleteGlobalRef(methodNotify);
    env->DeleteGlobalRef(_callbackEvent);
}
void CL_CALLBACK notifyCallbackMemory(cl_command_queue _commandQueue, cl_uint numBlocksMemory, void **listDataMemory, void *userData) {
    void **dataInfo = (void**)userData;
    JNIEnv *env = (JNIEnv*)dataInfo[0];
    jobject callbackMemory = *(jobject*)dataInfo[1];
    jobject methodNotify = *(jobject*)dataInfo[2];
    jobject commandQueue = *(jobject*)dataInfo[3];
    jint *positionBlockMemory = (jint*)dataInfo[4];
    jint *sizeBlocksMemory = (jint*)dataInfo[5];
    jint *flagsMemory = (jint*)dataInfo[6];
    jclass HashMap = env->FindClass("java/util/HashMap");
    jclass Method = env->FindClass("java/lang/reflect/Method");
    jclass Object = env->FindClass("java/lang/Object");
    jclass Memory = env->FindClass("com/draico/asvappra/opencl/memory/Memory");
    jclass CommandQueue = env->FindClass("com/draico/asvappra/opencl/CommandQueue");
    jmethodID invoke = env->GetMethodID(Method, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    jmethodID putHashMap = env->GetMethodID(HashMap, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jmethodID constructorHashMap = env->GetMethodID(HashMap, "<init>", "()V");
    jfieldID currentContextCommandQueue = env->GetFieldID(CommandQueue, "currentContext", "Lcom/draico/asvappra/opencl/Context;");
    jfieldID currentContextMemory = env->GetFieldID(Memory, "currentContext", "Lcom/draico/asvappra/opencl/Context;");
    jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
    jfieldID sizeBlockMemory = env->GetFieldID(Memory, "sizeBlockMemory", "I");
    jfieldID flagsBlockMemory = env->GetFieldID(Memory, "flagsBlockMemory", "I");
    jfieldID dataMemory = env->GetFieldID(Memory, "dataMemory", "Ljava/util/HashMap;");
    jfieldID byteArrayMemory = env->GetStaticFieldID(Memory, "byteArrayMemory", "Ljava/lang/String;");
    jstring _byteArrayMemory = (jstring)env->GetStaticObjectField(Memory, byteArrayMemory);
    jobject context = env->GetObjectField(commandQueue, currentContextCommandQueue);
    jobjectArray blocksMemory = env->NewObjectArray(numBlocksMemory, Memory, NULL);
    jbyte **listDataBlocksMemory = (jbyte**)listDataMemory;
    for (jint position = 0; position < numBlocksMemory; position++) {
        jobject memory = env->AllocObject(Memory);
        jobject hashMap = env->NewObject(HashMap, constructorHashMap);
        jint _currentMemory = positionBlockMemory[position];
        jint _flagsMemory = flagsMemory[position];
        jbyte *dataBlockMemory = listDataBlocksMemory[position];
        jsize _sizeBlockMemory = sizeBlocksMemory[position];
        jbyteArray _dataBlockMemory = env->NewByteArray(_sizeBlockMemory);
        env->SetByteArrayRegion(_dataBlockMemory, 0, _sizeBlockMemory, dataBlockMemory);
        env->CallObjectMethod(hashMap, putHashMap, _byteArrayMemory, _dataBlockMemory);
        env->SetIntField(memory, currentMemory, _currentMemory);
        env->SetIntField(memory, flagsBlockMemory, _flagsMemory);
        env->SetIntField(memory, sizeBlockMemory, _sizeBlockMemory);
        env->SetObjectField(memory, currentContextMemory, context);
        env->SetObjectField(memory, dataMemory, hashMap);
        env->SetObjectArrayElement(blocksMemory, position, memory);
    }
    jobjectArray arguments = env->NewObjectArray(2, Object, NULL);
    env->SetObjectArrayElement(arguments, 0, commandQueue);
    env->SetObjectArrayElement(arguments, 1, blocksMemory);
    env->CallObjectMethod(methodNotify, invoke, callbackMemory, arguments);
}
void CL_CALLBACK notifyCallbackBuffer(cl_mem buffer, void *userData) {
    void **data = (void**)userData;
    JNIEnv *env = (JNIEnv*)data[0];
    jobject _buffer = *(jobject*)data[1];
    jobject callbackBuffer = *(jobject*)data[2];
    jobject notifyBufferMethod = *(jobject*)data[3];
    jclass Method = env->FindClass("java/lang/reflect/Method");
    jclass Object = env->FindClass("java/lang/Object");
    jmethodID invoke = env->GetMethodID(Method, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    jobjectArray arguments = env->NewObjectArray(1, Object, _buffer);
    env->CallObjectMethod(notifyBufferMethod, invoke, callbackBuffer, arguments);
}
void CL_CALLBACK notifyCallbackProgram(cl_program program, void *userData) {
    void **data = (void**)userData;
    JNIEnv *env = (JNIEnv*)data[0];
    jvm->AttachCurrentThread(&env, NULL);
    jclass Method = env->FindClass("java/lang/reflect/Method");
    jclass Object = env->FindClass("java/lang/Object");
    jmethodID invoke = env->GetMethodID(Method, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    jobjectArray arguments = env->NewObjectArray(1, Object, objectProgram);
    env->CallObjectMethod(methodNotifyProgram, invoke, objectCallbackProgram, arguments);
    env->DeleteGlobalRef(objectProgram);
    env->DeleteGlobalRef(objectCallbackProgram);
    env->DeleteGlobalRef(methodNotifyProgram);
}
void CL_CALLBACK functionNativeKernel(void *data) {
    JNIEnv *env = (JNIEnv*)dataFunctionNativeKernel[0];
    jobject functionCustom = *(jobject*)dataFunctionNativeKernel[1];
    jobjectArray argumentsFunctionCustom = *(jobjectArray*)dataFunctionNativeKernel[2];
    jclass Method = env->FindClass("java/lang/reflect/Method");
    jclass FunctionNativeKernel = env->FindClass("com/draico/asvappra/opencl/functioncustom/FunctionNativeKernel");
    jmethodID functionNativeKernel = env->GetMethodID(FunctionNativeKernel, "functionNativeKernel", "([Ljava/lang/Object;)V");
    jmethodID invoke = env->GetMethodID(Method, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    jobject _functionNativeKernel = env->ToReflectedMethod(FunctionNativeKernel, functionNativeKernel, JNI_FALSE);
    env->CallObjectMethod(_functionNativeKernel, invoke, functionCustom, argumentsFunctionCustom);
}