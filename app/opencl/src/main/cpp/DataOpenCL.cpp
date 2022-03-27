#include <cstring>
#include <malloc.h>
#include <android/log.h>
#include "DataOpenCL.h"

cl_float VersionOpenCL;
cl_uint numberPlatforms;
cl_uint numberDevicesInCurrentPlatform;
cl_int sizeArrayKernelArguments = 0;
cl_int numberSVMPointer = 0;
jint dataDeviceNumber;
jlong dataPlatformNumber;
jlong dataDeviceNumberLong;
jboolean dataDeviceState;
jobject dataPlatform;
jobject dataDevice;
jbyte *dataByte;
jchar *dataChar;
jboolean *dataBoolean;
jshort *dataShort;
jint *dataInt;
jfloat *dataFloat;
jlong *dataLong;
cl_mem dataBufferKernelArguments[1000];
void *dataKernelArguments[1000];
int *dataExtraTypeIntKernelArguments[1000];
cl_bool *dataExtraTypeBoolKernelArguments[1000];
jobject *dataExtraTypeObjectKernelArguments[1000];
int *typeDataKernelArguments = NULL;
int *sizeArraysKernelArguments = NULL;
int **sizeArraysDataKernelArguments = NULL;
char **scriptKernel = NULL;

void *getPlatformInfo(JNIEnv *env, jobject platform, char *platformInfo) {
    jclass Platform = env->GetObjectClass(platform);
    jfieldID currentPlatform = env->GetFieldID(Platform, "currentPlatform", "I");
    jint _currentPlatform = env->GetIntField(platform, currentPlatform);
    void **Result;
    void *data;
    if (strcmp(platformInfo, "getName") == 0) {
        Result = GetPlatformInfo(_currentPlatform, CL_PLATFORM_NAME);
        if (*(jint*)Result[1] == CL_SUCCESS) dataPlatform = env->NewStringUTF((char*)dataInfo[0]);
        else dataPlatform = env->NewStringUTF("None");
        data = &dataPlatform;
    } else if (strcmp(platformInfo, "getProfile") == 0) {
        Result = GetPlatformInfo(_currentPlatform, CL_PLATFORM_PROFILE);
        if (*(jint*)Result[1] == CL_SUCCESS) dataPlatform = env->NewStringUTF((char*)dataInfo[0]);
        else dataPlatform = env->NewStringUTF("None");
        data = &dataPlatform;
    } else if (strcmp(platformInfo, "getVersion") == 0) {
        Result = GetPlatformInfo(_currentPlatform, CL_PLATFORM_VERSION);
        if (*(jint*)Result[1] == CL_SUCCESS) dataPlatform = env->NewStringUTF((char*)dataInfo[0]);
        else dataPlatform = env->NewStringUTF("None");
        data = &dataPlatform;
    } else if (strcmp(platformInfo, "getVendor") == 0) {
        Result = GetPlatformInfo(_currentPlatform, CL_PLATFORM_VENDOR);
        if (*(jint*)Result[1] == CL_SUCCESS) dataPlatform = env->NewStringUTF((char*)dataInfo[0]);
        else dataPlatform = env->NewStringUTF("None");
        data = &dataPlatform;
    } else if (strcmp(platformInfo, "getExtensions") == 0) {
        Result = GetPlatformInfo(_currentPlatform, CL_PLATFORM_EXTENSIONS);
        if (*(jint*)Result[1] == CL_SUCCESS) dataPlatform = env->NewStringUTF((char*)dataInfo[0]);
        else dataPlatform = env->NewStringUTF("None");
        data = &dataPlatform;
    } else if (strcmp(platformInfo, "getHostTimerResolution") == 0) {
        if (VersionOpenCL >= 2.1) {
            Result = GetPlatformInfo(_currentPlatform, CL_PLATFORM_HOST_TIMER_RESOLUTION);
            if (*(jint*)Result[1] == CL_SUCCESS) dataPlatformNumber = *(cl_ulong*)dataInfo[0];
            else dataPlatformNumber = 0;
        } else dataPlatformNumber = 0;
        data = &dataPlatformNumber;
    }
    return data;
}
void setVersionOpenCL(JNIEnv *env, jstring versionOpenCL) {
    jclass String = env->FindClass("java/lang/String");
    jclass Pattern = env->FindClass("java/util/regex/Pattern");
    jclass Float = env->FindClass("java/lang/Float");
    jclass File = env->FindClass("java/io/File");
    jmethodID split = env->GetMethodID(String, "split", "(Ljava/lang/String;)[Ljava/lang/String;");
    jmethodID parseFloat = env->GetStaticMethodID(Float, "parseFloat", "(Ljava/lang/String;)F");
    jmethodID quote = env->GetStaticMethodID(Pattern, "quote", "(Ljava/lang/String;)Ljava/lang/String;");
    jstring Space = env->NewStringUTF(" ");
    jstring _space = (jstring)env->CallStaticObjectMethod(Pattern, quote, Space);
    jobjectArray ContentVersionOpenCL = (jobjectArray)env->CallObjectMethod(versionOpenCL, split, _space);
    jstring Version = (jstring)env->GetObjectArrayElement(ContentVersionOpenCL, 1);
    VersionOpenCL = env->CallStaticFloatMethod(Float, parseFloat, Version);
}
jstring getTypeRound(JNIEnv *env, cl_device_fp_config typeRound) {
    jstring TypeRound;
    switch(typeRound) {
        case CL_FP_DENORM: TypeRound = env->NewStringUTF("FP_DENORM"); break;
        case CL_FP_INF_NAN: TypeRound = env->NewStringUTF("FP_INF_NAN"); break;
        case CL_FP_ROUND_TO_NEAREST: TypeRound = env->NewStringUTF("FP_ROUND_TO_NEAREST"); break;
        case CL_FP_ROUND_TO_ZERO: TypeRound = env->NewStringUTF("FP_ROUND_TO_ZERO"); break;
        case CL_FP_ROUND_TO_INF: TypeRound = env->NewStringUTF("ROUNT_TO_INF"); break;
        case CL_FP_FMA: TypeRound = env->NewStringUTF("FP_FMA"); break;
        case CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT: TypeRound = env->NewStringUTF("FP_CORRECTLY_ROUNDED_DIVIDE_SQRT"); break;
        case CL_FP_SOFT_FLOAT: TypeRound = env->NewStringUTF("FP_SOFT_FLOAT"); break;
        default: TypeRound = env->NewStringUTF("None");
    }
    return TypeRound;
}
jstring getCommandQueueProperties(JNIEnv *env, cl_command_queue_properties commandQueueProperties) {
    jstring queueOnHostProperties;
    switch(commandQueueProperties) {
        case CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE: queueOnHostProperties = env->NewStringUTF("QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE"); break;
        case CL_QUEUE_PROFILING_ENABLE: queueOnHostProperties = env->NewStringUTF("QUEUE_PROFILING_ENABLE"); break;
        case CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
            queueOnHostProperties = env->NewStringUTF("QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | QUEUE_PROFILING_ENABLE");
            break;
        default: queueOnHostProperties = env->NewStringUTF("None");
    }
    if (VersionOpenCL >= 2) {
        switch(commandQueueProperties) {
            case CL_QUEUE_ON_DEVICE: queueOnHostProperties = env->NewStringUTF("QUEUE_ON_DEVICE"); break;
            case CL_QUEUE_ON_DEVICE_DEFAULT: queueOnHostProperties = env->NewStringUTF("QUEUE_ON_DEVICE_DEFAULT"); break;
            default: queueOnHostProperties = env->NewStringUTF("None");
        }
    }
    return queueOnHostProperties;
}
void *getDataDeviceSelected(JNIEnv *env, jobject device, const char *message) {
    jclass Device = env->GetObjectClass(device);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID currentSubDevice = env->GetFieldID(Device, "currentSubDevice", "I");
    jfieldID isDevicePartition = env->GetFieldID(Device, "isDevicePartition", "Z");
    jint _currentDevice = env->GetIntField(device, currentDevice);
    jint _currentSubDevice = env->GetIntField(device, currentSubDevice);
    jint _isDevicePartition = env->GetBooleanField(device, isDevicePartition);
    void *data;
    void **Result;
    if (strcmp(message, "getType") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_TYPE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) {
            switch (*(cl_device_type*)dataInfo[0]) {
                case CL_DEVICE_TYPE_CPU: dataDevice = env->NewStringUTF("CPU"); break;
                case CL_DEVICE_TYPE_GPU: dataDevice = env->NewStringUTF("GPU"); break;
                case CL_DEVICE_TYPE_ACCELERATOR: dataDevice = env->NewStringUTF("Accelerator"); break;
                case CL_DEVICE_TYPE_DEFAULT: dataDevice = env->NewStringUTF("Default"); break;
                case CL_DEVICE_TYPE_CUSTOM: dataDevice = env->NewStringUTF("Custom"); break;
                case CL_DEVICE_TYPE_ALL: dataDevice = env->NewStringUTF("All"); break;
            }
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getVendorID") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_VENDOR_ID, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxComputeUnits") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_COMPUTE_UNITS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxWorkItemDimensions") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxWorkItemSizes") == 0) {
        jintArray Data = env->NewIntArray(3);
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_WORK_ITEM_SIZES, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) env->SetIntArrayRegion(Data, 0, 3, (jint*)dataInfo[0]);
        else {
            jint temp[] = { 0, 0, 0 };
            env->SetIntArrayRegion(Data, 0, 3, temp);
        }
        dataDevice = Data;
        data = &dataDevice;
    } else if (strcmp(message, "getMaxWorkGroupSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_WORK_GROUP_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredVectorWidthChar") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredVectorWidthShort") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredVectorWidthInt") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredVectorWidthLong") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredVectorWidthFloat") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredVectorWidthDouble") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredVectorWidthHalf") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getNativeVectorWidthChar") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getNativeVectorWidthShort") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getNativeVectorWidthInt") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getNativeVectorWidthLong") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getNativeVectorWidthFloat") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getNativeVectorWidthDouble") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getNativeVectorWidthHalf") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxClockFrequency") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_CLOCK_FREQUENCY, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getAddressBits") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_ADDRESS_BITS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxMemAllocSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_MEM_ALLOC_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumberLong = *(cl_ulong*)dataInfo[0];
        else dataDeviceNumberLong = 0;
        data = &dataDeviceNumberLong;
    } else if (strcmp(message, "getImageSupport") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE_SUPPORT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
        else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    } else if (strcmp(message, "getMaxReadImageArgs") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_READ_IMAGE_ARGS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxWriteImageArgs") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxReadWriteImageArgs") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getILVersion") == 0) {
        if (VersionOpenCL >= 2.1) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IL_VERSION, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
            else dataDevice = env->NewStringUTF("None");
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getImage2DMaxWidth") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE2D_MAX_WIDTH, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImage2DMaxHeight") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE2D_MAX_HEIGHT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImage3DMaxWidth") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE3D_MAX_WIDTH, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImage3DMaxHeight") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE3D_MAX_HEIGHT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImage3DMaxDepth") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE3D_MAX_DEPTH, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImageMaxBufferSize") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImageMaxArraySize") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxSamplers") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_SAMPLERS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImagePitchAlignment") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE_PITCH_ALIGNMENT, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getImageBaseAddressAlignment") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxPipeArgs") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_PIPE_ARGS, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPipeMaxActiveReservations") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPipeMaxPacketSize") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PIPE_MAX_PACKET_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxParameterSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_PARAMETER_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMemBaseAddressAlign") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MEM_BASE_ADDR_ALIGN, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getSingleFPConfig") == 0 || strcmp(message, "getDoubleFPConfig") == 0) {
        cl_device_info deviceInfo;
        if (strcmp(message, "getSingleFPConfig") == 0) deviceInfo = CL_DEVICE_SINGLE_FP_CONFIG;
        else if (strcmp(message, "getDoubleFPConfig") == 0) deviceInfo = CL_DEVICE_DOUBLE_FP_CONFIG;
        Result = GetDeviceInfo(_currentDevice, deviceInfo, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = getTypeRound(env, *(cl_device_fp_config*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getGlobalMemCacheType") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) {
            switch (*(cl_device_mem_cache_type*)dataInfo[0]) {
                case CL_NONE: dataDevice = env->NewStringUTF("None"); break;
                case CL_READ_ONLY_CACHE: dataDevice = env->NewStringUTF("READ_ONLY_CACHE"); break;
                case CL_READ_WRITE_CACHE: dataDevice = env->NewStringUTF("READ_WRITE_CACHE"); break;
            }
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getGlobalMemCachelineSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getGlobalMemCacheSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumberLong = *(cl_ulong*)dataInfo[0];
        else dataDeviceNumberLong = 0;
        data = &dataDeviceNumberLong;
    } else if (strcmp(message, "getGlobalMemSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_GLOBAL_MEM_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumberLong = *(cl_ulong*)dataInfo[0];
        else dataDeviceNumberLong = 0;
        data = &dataDeviceNumberLong;
    } else if (strcmp(message, "getMaxConstantBufferSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumberLong = *(cl_ulong*)dataInfo[0];
        else dataDeviceNumberLong = 0;
        data = &dataDeviceNumberLong;
    } else if (strcmp(message, "getMaxConstantArgs") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_CONSTANT_ARGS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxGlobalVariableSize") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getGlobalVariablePreferredTotalSize") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getLocalMemType") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_LOCAL_MEM_TYPE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) {
            switch(*(cl_device_local_mem_type*)dataInfo[0]) {
                case CL_NONE: dataDevice = env->NewStringUTF("None"); break;
                case CL_LOCAL: dataDevice = env->NewStringUTF("LOCAL"); break;
                case CL_GLOBAL: dataDevice = env->NewStringUTF("GLOBAL"); break;
            }
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getLocalMemSize") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_LOCAL_MEM_SIZE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumberLong = *(cl_ulong*)dataInfo[0];
        else dataDeviceNumberLong = 0;
        data = &dataDeviceNumberLong;
    } else if (strcmp(message, "getErrorCorrectionSupport") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_ERROR_CORRECTION_SUPPORT, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
        else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    } else if (strcmp(message, "getProfilingTimerResolution") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PROFILING_TIMER_RESOLUTION, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
        else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getEndianLittle") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_ENDIAN_LITTLE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
        else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    } else if (strcmp(message, "getAvailable") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_AVAILABLE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
        else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    } else if (strcmp(message, "getCompilerAvailable") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_COMPILER_AVAILABLE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
        else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    } else if (strcmp(message, "getLinkerAvailable") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_LINKER_AVAILABLE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
            else dataDeviceState = JNI_FALSE;
        } else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    } else if (strcmp(message, "getExecutionCapabilities") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_EXECUTION_CAPABILITIES, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) {
            switch (*(cl_device_exec_capabilities*)dataInfo[0]) {
                case CL_EXEC_KERNEL: dataDevice = env->NewStringUTF("EXECUTION_KERNEL"); break;
                case CL_EXEC_NATIVE_KERNEL: dataDevice = env->NewStringUTF("EXECUTION_NATIVE_KERNEL"); break;
                default: dataDevice = env->NewStringUTF("None");
            }
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getQueueOnHostProperties") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = getCommandQueueProperties(env, *(cl_command_queue_properties*)dataInfo[0]);
            else dataDevice = env->NewStringUTF("None");
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getQueueOnDeviceProperties") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = getCommandQueueProperties(env, *(cl_command_queue_properties*)dataInfo[0]);
            else dataDevice = env->NewStringUTF("None");
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getQueueOnDevicePreferredSize") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getQueueOnDeviceMaxSize") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxOnDeviceQueues") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_ON_DEVICE_QUEUES, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxOnDeviceEvents") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_ON_DEVICE_EVENTS, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getBuildInKernels") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_BUILT_IN_KERNELS, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
            else dataDevice = env->NewStringUTF("None");
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getName") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_NAME, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getVendor") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_VENDOR, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getDriverVersion") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DRIVER_VERSION, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getProfile") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PROFILE, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getVersion") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_VERSION, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getOpenCL_C_1Version") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_OPENCL_C_VERSION, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getExtensions") == 0) {
        Result = GetDeviceInfo(_currentDevice, CL_DEVICE_EXTENSIONS, _isDevicePartition, _currentSubDevice);
        if (*(jint*)Result[1] == CL_SUCCESS) dataDevice = env->NewStringUTF((char*)dataInfo[0]);
        else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getPrintfBufferSize") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PRINTF_BUFFER_SIZE, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(size_t*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredInteropUserSync") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
            else dataDeviceState = JNI_FALSE;
        } else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    } else if (strcmp(message, "getPartitionMaxSubDevices") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPartitionProperties") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PARTITION_PROPERTIES, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) {
                switch (*(cl_device_partition_property*)dataInfo[0]) {
                    case CL_DEVICE_PARTITION_EQUALLY: dataDevice = env->NewStringUTF("PARTITION_EQUALLY"); break;
                    case CL_DEVICE_PARTITION_BY_COUNTS: dataDevice = env->NewStringUTF("PARTITION_BY_COUNTS"); break;
                    case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN: dataDevice = env->NewStringUTF("PARTITION_BY_AFFINITY_DOMAIN"); break;
                    default: dataDevice = env->NewStringUTF("None");
                }
            } else dataDevice = env->NewStringUTF("None");
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getPartitionAffinityDomain") == 0 || strcmp(message, "getPartitionType") == 0) {
        cl_device_info deviceInfo;
        if (VersionOpenCL >= 1.2) {
            if (strcmp(message, "getPartitionAffinityDomain") == 0) deviceInfo = CL_DEVICE_PARTITION_AFFINITY_DOMAIN;
            else if (strcmp(message, "getPartitionType") == 0) deviceInfo = CL_DEVICE_PARTITION_TYPE;
            Result = GetDeviceInfo(_currentDevice, deviceInfo, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) {
                switch (*(cl_device_partition_property*)dataInfo[0]) {
                    case CL_DEVICE_AFFINITY_DOMAIN_NUMA: dataDevice = env->NewStringUTF("PARTITION_AFFINITY_DOMAIN_NUMA"); break;
                    case CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE: dataDevice = env->NewStringUTF("PARTITION_AFFINITY_DOMAIN_L4_CACHE"); break;
                    case CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE: dataDevice = env->NewStringUTF("PARTITION_AFFINITY_DOMAIN_L3_CACHE"); break;
                    case CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE: dataDevice = env->NewStringUTF("PARTITION_AFFINITY_DOMAIN_L2_CACHE"); break;
                    case CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE: dataDevice = env->NewStringUTF("PARTITION_AFFINITY_DOMAIN_L1_CACHE"); break;
                    case CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE: dataDevice = env->NewStringUTF("PARTITION__AFFINITY_DOMAIN_NEXT_PARTITIONABLE"); break;
                    default: dataDevice = env->NewStringUTF("None");
                }
            } else dataDevice = env->NewStringUTF("None");
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getReferenceCount") == 0) {
        if (VersionOpenCL >= 1.2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_REFERENCE_COUNT, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getSVMCapabilities") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_SVM_CAPABILITIES, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) {
                switch (*(cl_device_svm_capabilities*)dataInfo[0]) {
                    case CL_DEVICE_SVM_COARSE_GRAIN_BUFFER: dataDevice = env->NewStringUTF("SVM_COARSE_GRAIN_BUFFER"); break;
                    case CL_DEVICE_SVM_FINE_GRAIN_BUFFER: dataDevice = env->NewStringUTF("SVM_FINE_GRAIN_BUFFER"); break;
                    case CL_DEVICE_SVM_FINE_GRAIN_SYSTEM: dataDevice = env->NewStringUTF("SVM_FINE_GRAIN_SYSTEM"); break;
                    case CL_DEVICE_SVM_ATOMICS: dataDevice = env->NewStringUTF("SVM_ATOMICS"); break;
                    default: dataDevice = env->NewStringUTF("None");
                }
            } else dataDevice = env->NewStringUTF("None");
        } else dataDevice = env->NewStringUTF("None");
        data = &dataDevice;
    } else if (strcmp(message, "getPreferredPlatformAtomicAlignment") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredGlobalAtomicAlignment") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getPreferredLocalAtomicAlignment") == 0) {
        if (VersionOpenCL >= 2) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getMaxNumSubGroups") == 0) {
        if (VersionOpenCL >= 2.1) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_MAX_NUM_SUB_GROUPS, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceNumber = *(cl_uint*)dataInfo[0];
            else dataDeviceNumber = 0;
        } else dataDeviceNumber = 0;
        data = &dataDeviceNumber;
    } else if (strcmp(message, "getSubGroupIndependentForwardProgress") == 0) {
        if (VersionOpenCL >= 2.1) {
            Result = GetDeviceInfo(_currentDevice, CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS, _isDevicePartition, _currentSubDevice);
            if (*(jint*)Result[1] == CL_SUCCESS) dataDeviceState = *(cl_bool*)dataInfo[0];
            else dataDeviceState = JNI_FALSE;
        } else dataDeviceState = JNI_FALSE;
        data = &dataDeviceState;
    }
    return data;
}
void showMessageError(JNIEnv *env, const char *Message) {
    jstring message = env->NewStringUTF(Message);
    jclass MessageError = env->FindClass("com/draico/asvappra/opencl/tools/MessageError");
    jmethodID _showMessageError = env->GetStaticMethodID(MessageError, "showMessage", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(MessageError, _showMessageError, message);
}
jobject newEvent(JNIEnv *env, jobject object, jobject commandQueue) {
    jclass Event = env->FindClass("com/draico/asvappra/opencl/listeners/Event");
    jclass Buffer = env->FindClass("com/draico/asvappra/opencl/memory/buffer/Buffer");
    jclass Context = env->FindClass("com/draico/asvappra/opencl/Context");
    jfieldID currentCommandQueue = env->GetFieldID(Event, "currentCommandQueue", "Lcom/draico/asvappra/opencl/CommandQueue;");
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jfieldID currentContext = env->GetFieldID(Event, "currentContext", "Lcom/draico/asvappra/opencl/Context;");
    jfieldID isSetStatusEvent = env->GetFieldID(Event, "isSetStatusEvent", "Z");
    jobject context;
    if (object != NULL) {
        if (env->IsInstanceOf(object, Buffer)) {
            jfieldID currentContextBuffer = env->GetFieldID(Buffer, "currentContext", "Lcom/draico/asvappra/opencl/Context;");
            context = env->GetObjectField(object, currentContextBuffer);
        } else if (env->IsInstanceOf(object, Event)) context = env->GetObjectField(object, currentContext);
        else if (env->IsInstanceOf(object, Context)) context = object;
    } else {
        jclass CommandQueue = env->GetObjectClass(commandQueue);
        jfieldID currentContextCommandQueue = env->GetFieldID(CommandQueue, "currentContext", "Lcom/draico/asvappra/opencl/Context;");
        context = env->GetObjectField(commandQueue, currentContextCommandQueue);
    }
    jobject event = env->AllocObject(Event);
    env->SetObjectField(event, currentCommandQueue, commandQueue);
    env->SetObjectField(event, currentContext, context);
    env->SetIntField(event, currentEvent, positionCurrentEvent);
    env->SetBooleanField(event, isSetStatusEvent, JNI_FALSE);
    positionCurrentEvent++;
    return event;
}
void **getProfilingEvent(JNIEnv *env, jobject event, cl_profiling_info  profileInfo, cl_ulong *profileEvent) {
    jclass Event = env->GetObjectClass(event);
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jint _currentEvent = env->GetIntField(event, currentEvent);
    void **Result = GetEventProfilingInfo(_currentEvent, profileInfo, profileEvent);
    return Result;
}
cl_mem createBuffer(cl_int currentContext, void *data, cl_int sizeData) {
    cl_context context = *listContexts[currentContext];
    cl_int result;
    cl_mem buffer = CLCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeData, data, &result);
    if (result != CL_SUCCESS) return NULL;
    return buffer;
}
void writeBuffer(cl_int currentCommandQueue, cl_mem *buffer, void *data, cl_int sizeData) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    CLEnqueueWriteBuffer(commandQueue, *buffer, CL_FALSE, 0, sizeData, data, NULL, NULL, NULL);
}
void readBuffer(cl_int currentCommandQueue, cl_mem buffer, void *data, cl_int sizeData) {
    cl_command_queue commandQueue = *listCommandQueue[currentCommandQueue];
    CLEnqueueReadBuffer(commandQueue, buffer, CL_FALSE, 0, sizeData, data, NULL, NULL, NULL);
    CLFlush(commandQueue);
    CLFinish(commandQueue);
}
void executeCommandQueue(JNIEnv *env, jobject commandQueue) {
    jclass CommandQueue = env->GetObjectClass(commandQueue);
    jmethodID finish = env->GetMethodID(CommandQueue, "finish", "()V");
    jmethodID flush = env->GetMethodID(CommandQueue, "flush", "()V");
    env->CallVoidMethod(commandQueue, flush);
    env->CallVoidMethod(commandQueue, finish);
}
void executeKernel(JNIEnv *env, jobject commandQueue, jint _currentBlockMemory, jint _sizeBlockMemory, jint _flagsBlockMemory,
                   jint _flagsMapMemory, jint typeData) {
    jclass CommandQueue = env->FindClass("com/draico/asvappra/opencl/CommandQueue");
    jclass Context = env->FindClass("com/draico/asvappra/opencl/Context");
    jclass Memory = env->FindClass("com/draico/asvappra/opencl/memory/Memory");
    jclass Program = env->FindClass("com/draico/asvappra/opencl/Program");
    jclass Kernel = env->FindClass("com/draico/asvappra/opencl/Kernel");
    jclass HashMap = env->FindClass("java/util/HashMap");
    jclass Integer = env->FindClass("java/lang/Integer");
    jclass Object = env->FindClass("java/lang/Object");
    jclass String = env->FindClass("java/lang/String");
    jfieldID contextCommandQueue = env->GetFieldID(CommandQueue, "currentContext", "Lcom/draico/asvappra/opencl/Context;");
    jfieldID devicesContext = env->GetFieldID(Context, "devices", "[Lcom/draico/asvappra/opencl/Device;");
    jfieldID sizeBlockMemory = env->GetFieldID(Memory, "sizeBlockMemory", "I");
    jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
    jfieldID flagsBlockMemory = env->GetFieldID(Memory, "flagsBlockMemory", "I");
    jfieldID flagsMapMemory = env->GetFieldID(Memory, "flagsMapMemory", "I");
    jfieldID contextMemory = env->GetFieldID(Memory, "currentContext", "Lcom/draico/asvappra/opencl/Context;");
    jfieldID dataMemory = env->GetFieldID(Memory, "dataMemory", "Ljava/util/HashMap;");
    jfieldID STD = env->GetStaticFieldID(Program, "STD", "Ljava/lang/String;");
    jfieldID CL2_0 = env->GetStaticFieldID(Program, "CL2_0", "Ljava/lang/String;");
    jfieldID SinglePrecisionConstant = env->GetStaticFieldID(Program, "SinglePrecisionConstant", "Ljava/lang/String;");
    jmethodID createProgramWithSourceAssets = env->GetStaticMethodID(Program, "createProgramWithSourceAssets",
                                                                     "(Lcom/draico/asvappra/opencl/Context;)Lcom/draico/asvappra/opencl/Program;");
    jmethodID buildProgram = env->GetMethodID(Program, "buildProgram",
                                              "([Lcom/draico/asvappra/opencl/Device;Ljava/lang/String;Lcom/draico/asvappra/opencl/listeners/CallbackBuildProgram;)V");
    jmethodID createKernel = env->GetStaticMethodID(Kernel, "createKernel", "(Lcom/draico/asvappra/opencl/Program;Ljava/lang/String;)Lcom/draico/asvappra/opencl/Kernel;");
    jmethodID setKernelArguments = env->GetMethodID(Kernel, "setKernelArguments", "(Lcom/draico/asvappra/opencl/CommandQueue;[Ljava/lang/Object;)V");
    jmethodID setKernelArgumentsSVMPointer = env->GetMethodID(Kernel, "setKernelArgumentsSVMPointer", "(Lcom/draico/asvappra/opencl/CommandQueue;[Ljava/lang/Object;)V");
    jmethodID NDRangeKernel = env->GetMethodID(Kernel, "NDRangeKernel", "(Lcom/draico/asvappra/opencl/CommandQueue;I[I[I[I)Lcom/draico/asvappra/opencl/listeners/Event;");
    jmethodID getArgumentsKernel = env->GetMethodID(Kernel, "getArgumentsKernel", "(Lcom/draico/asvappra/opencl/CommandQueue;)[Ljava/lang/Object;");
    jmethodID constructorHashMap = env->GetMethodID(HashMap, "<init>", "()V");
    jmethodID concat = env->GetMethodID(String, "concat", "(Ljava/lang/String;)Ljava/lang/String;");
    jmethodID constructorInteger = env->GetMethodID(Integer, "<init>", "(I)V");
    jobject _contextCommandQueue = env->GetObjectField(commandQueue, contextCommandQueue);
    jobjectArray _devicesContext = (jobjectArray)env->GetObjectField(_contextCommandQueue, devicesContext);
    jstring _STD = (jstring)env->GetStaticObjectField(Program, STD);
    jstring _CL2_0 = (jstring)env->GetStaticObjectField(Program, CL2_0);
    jstring _SinglePrecisionConstant = (jstring)env->GetStaticObjectField(Program, SinglePrecisionConstant);
    jstring space = env->NewStringUTF(" ");
    jstring buildOptions = _STD;
    buildOptions = (jstring)env->CallObjectMethod(buildOptions, concat, _CL2_0);
    buildOptions = (jstring)env->CallObjectMethod(buildOptions, concat, space);
    buildOptions = (jstring)env->CallObjectMethod(buildOptions, concat, _SinglePrecisionConstant);
    jobject program = env->CallStaticObjectMethod(Program, createProgramWithSourceAssets, _contextCommandQueue);
    jobject callbackBuildProgram = NULL;
    env->CallVoidMethod(program, buildProgram, _devicesContext, buildOptions, callbackBuildProgram);
    jobjectArray listBlocksMemory = env->NewObjectArray(2, Object, NULL);
    jobject __sizeBlockMemory = env->NewObject(Integer, constructorInteger, _sizeBlockMemory);
    jobject memoryBlock = env->AllocObject(Memory);
    jobject _dataMemory = env->NewObject(HashMap, constructorHashMap);
    jobject positionMemoryArgument = env->NewObject(Integer, constructorInteger, 0);
    env->SetIntField(memoryBlock, currentMemory, _currentBlockMemory);
    env->SetIntField(memoryBlock, sizeBlockMemory, _sizeBlockMemory);
    env->SetIntField(memoryBlock, flagsBlockMemory, _flagsBlockMemory);
    env->SetIntField(memoryBlock, flagsMapMemory, _flagsMapMemory);
    env->SetObjectField(memoryBlock, dataMemory, _dataMemory);
    env->SetObjectField(memoryBlock, contextMemory, _contextCommandQueue);
    env->SetObjectArrayElement(listBlocksMemory, 0, memoryBlock);
    env->SetObjectArrayElement(listBlocksMemory, 1, positionMemoryArgument);
    jintArray globalWorkOffset = env->NewIntArray(3);
    jintArray globalWorkSize = env->NewIntArray(3);
    jintArray localWorkSize = env->NewIntArray(3);
    jint _globalWorkOffset[3] = { 0, 0, 0 };
    jint _globalWorkSize[3] = { 10, 10, 10 };
    jint _localWorkSize[3] = { 10, 10, 10 };
    env->SetIntArrayRegion(globalWorkOffset, 0, 3, _globalWorkOffset);
    env->SetIntArrayRegion(globalWorkSize, 0, 3, _globalWorkSize);
    env->SetIntArrayRegion(localWorkSize, 0, 3, _localWorkSize);
    if (typeData == -1) {
        jstring nameKernel = env->NewStringUTF("clearDataBlockMemory");
        jobject clearDataBlockMemory = env->CallStaticObjectMethod(Kernel, createKernel, program, nameKernel);
        jobjectArray listArguments = env->NewObjectArray(2, Object, NULL);
        env->SetObjectArrayElement(listArguments, 0, memoryBlock);
        env->SetObjectArrayElement(listArguments, 1, __sizeBlockMemory);
        env->CallVoidMethod(clearDataBlockMemory, setKernelArguments, commandQueue, listArguments);
        env->CallVoidMethod(clearDataBlockMemory, setKernelArgumentsSVMPointer, commandQueue, listBlocksMemory);
        env->CallObjectMethod(clearDataBlockMemory, NDRangeKernel, commandQueue, 3, globalWorkOffset, globalWorkSize, localWorkSize);
        executeCommandQueue(env, commandQueue);
    } else {
        jsize sizeDataType;
        switch(typeData) {
            case 0: case 1: case 2: sizeDataType = 1; break;
            case 3: sizeDataType = 2; break;
            case 4: case 5: sizeDataType = 4; break;
            case 6: case 7: sizeDataType = 8; break;
        }
        jstring nameKernel = env->NewStringUTF("loadDataBlockMemory");
        jobject loadDataBlockMemory = env->CallStaticObjectMethod(Kernel, createKernel, program, nameKernel);
        jobject _typeData = env->NewObject(Integer, constructorInteger, typeData);
        jint sizeBlock = _sizeBlockMemory / sizeDataType;
        jbyteArray dataBlockMemory = env->NewByteArray(_sizeBlockMemory);
        jbyteArray _byteArray = env->NewByteArray(_sizeBlockMemory);
        jintArray _intArray = env->NewIntArray(sizeBlock);
        jshortArray _shortArray = env->NewShortArray(sizeBlock);
        jfloatArray _floatArray = env->NewFloatArray(sizeBlock);
        jlongArray _longArray = env->NewLongArray(sizeBlock);
        env->SetByteArrayRegion(dataBlockMemory, 0, _sizeBlockMemory, listDataMemory[_currentBlockMemory]);
        jobjectArray listArguments2 = env->NewObjectArray(8, Object, NULL);
        env->SetObjectArrayElement(listArguments2, 0, dataBlockMemory);
        env->SetObjectArrayElement(listArguments2, 1, _byteArray);
        env->SetObjectArrayElement(listArguments2, 2, _shortArray);
        env->SetObjectArrayElement(listArguments2, 3, _intArray);
        env->SetObjectArrayElement(listArguments2, 4, _floatArray);
        env->SetObjectArrayElement(listArguments2, 5, _longArray);
        env->SetObjectArrayElement(listArguments2, 6, __sizeBlockMemory);
        env->SetObjectArrayElement(listArguments2, 7, _typeData);
        env->CallVoidMethod(loadDataBlockMemory, setKernelArguments, commandQueue, listArguments2);
        env->CallVoidMethod(loadDataBlockMemory, setKernelArgumentsSVMPointer, commandQueue, listBlocksMemory);
        env->CallObjectMethod(loadDataBlockMemory, NDRangeKernel, commandQueue, 3, globalWorkOffset, globalWorkSize, localWorkSize);
        executeCommandQueue(env, commandQueue);
        jobjectArray listArguments = (jobjectArray)env->CallObjectMethod(loadDataBlockMemory, getArgumentsKernel, commandQueue);
        _byteArray = (jbyteArray)env->GetObjectArrayElement(listArguments, 1);
        _shortArray = (jshortArray)env->GetObjectArrayElement(listArguments, 2);
        _intArray = (jintArray)env->GetObjectArrayElement(listArguments, 3);
        _floatArray = (jfloatArray)env->GetObjectArrayElement(listArguments, 4);
        _longArray = (jlongArray)env->GetObjectArrayElement(listArguments, 5);
        dataByte = env->GetByteArrayElements(_byteArray, NULL);
        dataShort = env->GetShortArrayElements(_shortArray, NULL);
        dataInt = env->GetIntArrayElements(_intArray, NULL);
        dataFloat = env->GetFloatArrayElements(_floatArray, NULL);
        dataLong = env->GetLongArrayElements(_longArray, NULL);
    }
}