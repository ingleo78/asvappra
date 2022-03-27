#include "neuralnetworks.h"

extern "C" {
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Model_newModel(JNIEnv *env, jclass Model) {
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    if (currentPositionModel >= 100) {
        char *message = (char*)malloc(sizeof(char) * 112);
        strcpy(message, "You cannot create the NeuralNetworkModel object because you have exceeded the allowed limit of 100Model objects");
        showMessageError(env, message);
        return NULL;
    }
    void **Result = createNeuralNetworkModel();
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jobject neuralNetworkModel = env->AllocObject(Model);
        env->SetIntField(neuralNetworkModel, currentModel, currentPositionModel);
        env->SetBooleanField(neuralNetworkModel, isCreated, JNI_TRUE);
        currentPositionModel++;
        if (currentPositionModel == 100) currentPositionModel = 0;
        return neuralNetworkModel;
    } else {
        showMessageError(env, (char*)Result[0]);
        return NULL;
    }
}
JNICALL jlong Java_com_draico_asvappra_neuralnetworks_Model_getDefaultLoopTimeout(JNIEnv *env, jclass Model) {
    jlong timeout = -1;
#if __ANDROID_API__ >= 30
    timeout = (jlong)ANeuralNetworks_getDefaultLoopTimeout();
#else
    char *message = (char*)malloc(sizeof(char) * 167);
    strcpy(message, "You cannot use the getDefaultLoopTimeout method because the version of the operating system that your android ");
    strcat(message, "device has installed is lower than the Android 11 version");
    showMessageError(env, message);
#endif
    return timeout;
}
JNICALL jlong Java_com_draico_asvappra_neuralnetworks_Model_getMaximumLoopTimeout(JNIEnv *env, jclass Model) {
    jlong timeout = -1;
#if __ANDROID_API__ >= 30
    timeout = (jlong)ANeuralNetworks_getMaximumLoopTimeout();
#else
    char *message = (char*)malloc(sizeof(char) * 167);
    strcpy(message, "You cannot use the getMaximumLoopTimeout method because the version of the operating system that your android ");
    strcat(message, "device has installed is lower than the Android 11 version");
    showMessageError(env, message);
#endif
    return timeout;
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_finish(JNIEnv *env, jobject model) {
    jclass Model = env->GetObjectClass(model);
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 119);
        strcpy(message, "You cannot use the finish method of the Model object you are using because it was not created using the ");
        strcat(message, "newModel method");
        showMessageError(env, message);
        return;
    }
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 93);
        strcpy(message, "You cannot use the finish method of the Model type object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    void **Result = finishNeuralNetworkModel(_currentModel);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_delete(JNIEnv *env, jobject model) {
    jclass Model = env->GetObjectClass(model);
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 127);
        strcpy(message, "You cannot use the delete method of the Model object you are using because this object was not created ");
        strcat(message, "with the newModel method");
        showMessageError(env, message);
        return;
    }
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 98);
        strcpy(message, "You cannot use the delete method to delete the object of type Model because this object is invalid");
        showMessageError(env, message);
        return;
    }
    deleteNeuralNetworkModel(_currentModel);
    env->SetIntField(model, currentModel, -1);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_addOperand(JNIEnv *env, jobject model, jobject operandType) {
    if (operandType == NULL) {
        char *message = (char*)malloc(sizeof(char) * 134);
        strcpy(message, "You cannot use the addOperand method of the Model object you are using because the OperandType object you have provided ");
        strcat(message, "is set to null");
        showMessageError(env, message);
        return;
    }
    jclass Model = env->GetObjectClass(model);
    jclass OperandType = env->GetObjectClass(OperandType);
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jfieldID isCreatedOperandType = env->GetFieldID(OperandType, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    jboolean _isCreatedOperandType = env->GetBooleanField(operandType, isCreatedOperandType);
    if (!_isCreated || !_isCreatedOperandType) {
        char *message = (char*)malloc(sizeof(char) * 243);
        strcpy(message, "You cannot use the addOperand method of the Model object because the Model type object was not created with the Model ");
        strcat(message, "method, or the OperandType object of the type OperandType that you provided was not created with the newOperandType ");
        strcat(message, "method");
        showMessageError(env, message);
        return;
    }
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jfieldID currentOperandType = env->GetFieldID(OperandType, "currentOperandType", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    jint _currentOperandType = env->GetIntField(operandType, currentOperandType);
    if (_currentModel == -1 || _currentOperandType == -1) {
        char *message = (char*)malloc(sizeof(char) * 146);
        strcpy(message, "You cannot use the addOperand method of the Model object because the object of type Model and/or the OperandType object ");
        strcat(message, "you provided are invalid");
        showMessageError(env, message);
        return;
    }
    void **Result = addOperandModel(_currentModel, _currentOperandType);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_addOperation(JNIEnv *env, jobject model, jint typeOperation, jintArray inputs, jintArray outputs) {
    if (inputs == NULL || outputs == NULL) {
        char *message = (char*)malloc(sizeof(char) * 145);
        strcpy(message, "You cannot use the addOperation method of the Model type object because one or both of the arrays you provided (inputs or ");
        strcat(message, "outputs) is set to null");
        showMessageError(env, message);
        return;
    }
    jsize sizeInputs = env->GetArrayLength(inputs);
    jsize sizeOutputs = env->GetArrayLength(outputs);
    if (sizeInputs == 0 || sizeOutputs == 0) {
        char *message = (char*)malloc(sizeof(char) * 145);
        strcpy(message, "You cannot use the addOperation method of the Model type object you are using because the array inputs or the array outputs ");
        strcat(message, "you provided is empty");
        showMessageError(env, message);
        return;
    }
    jclass Model = env->GetObjectClass(model);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 118);
        strcpy(message, "You cannot use the addOperation method of the Model type object you are using was not created with the newModel method");
        showMessageError(env, message);
        return;
    }
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 99);
        strcpy(message, "You cannot use the addOperation method of the Model type object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    jfieldID OPERATION_TYPE_ADD = env->GetStaticFieldID(Model, "OPERATION_TYPE_ADD", "I");
    jfieldID OPERATION_TYPE_AVERAGE_POOL_2D = env->GetStaticFieldID(Model, "OPERATION_TYPE_AVERAGE_POOL_2D", "I");
    jfieldID OPERATION_TYPE_CONCATENATION = env->GetStaticFieldID(Model, "OPERATION_TYPE_CONCATENATION", "I");
    jfieldID OPERATION_TYPE_CONV_2D = env->GetStaticFieldID(Model, "OPERATION_TYPE_CONV_2D", "I");
    jfieldID OPERATION_TYPE_DEPTHWISE_CONV_2D = env->GetStaticFieldID(Model, "OPERATION_TYPE_DEPTHWISE_CONV_2D", "I");
    jfieldID OPERATION_TYPE_DEPTH_TO_SPACE = env->GetStaticFieldID(Model, "OPERATION_TYPE_DEPTH_TO_SPACE", "I");
    jfieldID OPERATION_TYPE_DEQUANTIZE = env->GetStaticFieldID(Model, "OPERATION_TYPE_DEQUANTIZE", "I");
    jfieldID OPERATION_TYPE_EMBEDDING_LOOKUP = env->GetStaticFieldID(Model, "OPERATION_TYPE_EMBEDDING_LOOKUP", "I");
    jfieldID OPERATION_TYPE_FLOOR = env->GetStaticFieldID(Model, "OPERATION_TYPE_FLOOR", "I");
    jfieldID OPERATION_TYPE_FULLY_CONNECTED = env->GetStaticFieldID(Model, "OPERATION_TYPE_FULLY_CONNECTED", "I");
    jfieldID OPERATION_TYPE_HASTABLE_LOOKUP = env->GetStaticFieldID(Model, "OPERATION_TYPE_HASTABLE_LOOKUP", "I");
    jfieldID OPERATION_TYPE_L2_NORMALIZATION = env->GetStaticFieldID(Model, "OPERATION_TYPE_L2_NORMALIZATION", "I");
    jfieldID OPERATION_TYPE_L2_POOL_2D = env->GetStaticFieldID(Model, "OPERATION_TYPE_L2_POOL_2D", "I");
    jfieldID OPERATION_TYPE_LOCAL_RESPONSE_NORMALIZATION = env->GetStaticFieldID(Model, "OPERATION_TYPE_LOCAL_RESPONSE_NORMALIZATION", "I");
    jfieldID OPERATION_TYPE_LOGISTIC = env->GetStaticFieldID(Model, "OPERATION_TYPE_LOGISTIC", "I");
    jfieldID OPERATION_TYPE_LSH_PROJECTION = env->GetStaticFieldID(Model, "OPERATION_TYPE_LSH_PROJECTION", "I");
    jfieldID OPERATION_TYPE_LSTM = env->GetStaticFieldID(Model, "OPERATION_TYPE_LSTM", "I");
    jfieldID OPERATION_TYPE_MAX_POOL_2D = env->GetStaticFieldID(Model, "OPERATION_TYPE_MAX_POOL_2D", "I");
    jfieldID OPERATION_TYPE_MUL = env->GetStaticFieldID(Model, "OPERATION_TYPE_MUL", "I");
    jfieldID OPERATION_TYPE_RELU = env->GetStaticFieldID(Model, "OPERATION_TYPE_RELU", "I");
    jfieldID OPERATION_TYPE_RELU1 = env->GetStaticFieldID(Model, "OPERATION_TYPE_RELU1", "I");
    jfieldID OPERATION_TYPE_RELU6 = env->GetStaticFieldID(Model, "OPERATION_TYPE_RELU6", "I");
    jfieldID OPERATION_TYPE_RESHAPE = env->GetStaticFieldID(Model, "OPERATION_TYPE_RESHAPE", "I");
    jfieldID OPERATION_TYPE_RESIZE_BILINEAR = env->GetStaticFieldID(Model, "OPERATION_TYPE_RESIZE_BILINEAR", "I");
    jfieldID OPERATION_TYPE_RNN = env->GetStaticFieldID(Model, "OPERATION_TYPE_RNN", "I");
    jfieldID OPERATION_TYPE_SOFTMAX = env->GetStaticFieldID(Model, "OPERATION_TYPE_SOFTMAX", "I");
    jfieldID OPERATION_TYPE_SPACE_TO_DEPTH = env->GetStaticFieldID(Model, "OPERATION_TYPE_SPACE_TO_DEPTH", "I");
    jfieldID OPERATION_TYPE_SVDF = env->GetStaticFieldID(Model, "OPERATION_TYPE_SVDF", "I");
    jfieldID OPERATION_TYPE_TANH = env->GetStaticFieldID(Model, "OPERATION_TYPE_TANH", "I");
    jfieldID OPERATION_TYPE_BATCH_TO_SPACE_ND = env->GetStaticFieldID(Model, "OPERATION_TYPE_BATCH_TO_SPACE_ND", "I");
    jfieldID OPERATION_TYPE_DIV = env->GetStaticFieldID(Model, "OPERATION_TYPE_DIV", "I");
    jfieldID OPERATION_TYPE_MEAN = env->GetStaticFieldID(Model, "OPERATION_TYPE_MEAN", "I");
    jfieldID OPERATION_TYPE_PAD = env->GetStaticFieldID(Model, "OPERATION_TYPE_PAD", "I");
    jfieldID OPERATION_TYPE_SPACE_TO_BATCH_ND = env->GetStaticFieldID(Model, "OPERATION_TYPE_SPACE_TO_BATCH_ND", "I");
    jfieldID OPERATION_TYPE_SQUEEZE = env->GetStaticFieldID(Model, "OPERATION_TYPE_SQUEEZE", "I");
    jfieldID OPERATION_TYPE_STRIDED_SLICE = env->GetStaticFieldID(Model, "OPERATION_TYPE_STRIDED_SLICE", "I");
    jfieldID OPERATION_TYPE_SUB = env->GetStaticFieldID(Model, "OPERATION_TYPE_SUB", "I");
    jfieldID OPERATION_TYPE_TRANSPOSE = env->GetStaticFieldID(Model, "OPERATION_TYPE_TRANSPOSE", "I");
    jfieldID OPERATION_TYPE_ABS = env->GetStaticFieldID(Model, "OPERATION_TYPE_ABS", "I");
    jfieldID OPERATION_TYPE_ARGMAX = env->GetStaticFieldID(Model, "OPERATION_TYPE_ARGMAX", "I");
    jfieldID OPERATION_TYPE_ARGMIN = env->GetStaticFieldID(Model, "OPERATION_TYPE_ARGMIN", "I");
    jfieldID OPERATION_TYPE_AXIS_ALIGNED_BBOX_TRANSFORM = env->GetStaticFieldID(Model, "OPERATION_TYPE_AXIS_ALIGNED_BBOX_TRANSFORM", "I");
    jfieldID OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_LSTM = env->GetStaticFieldID(Model, "OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_LSTM", "I");
    jfieldID OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_RNN = env->GetStaticFieldID(Model, "OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_RNN", "I");
    jfieldID OPERATION_TYPE_BOX_WITH_NMS_LIMIT = env->GetStaticFieldID(Model, "OPERATION_TYPE_BOX_WITH_NMS_LIMIT", "I");
    jfieldID OPERATION_TYPE_CAST = env->GetStaticFieldID(Model, "OPERATION_TYPE_CAST", "I");
    jfieldID OPERATION_TYPE_CHANNEL_SHUFFLE = env->GetStaticFieldID(Model, "OPERATION_TYPE_CHANNEL_SHUFFLE", "I");
    jfieldID OPERATION_TYPE_DETECTION_POSTPROCESSING = env->GetStaticFieldID(Model, "OPERATION_TYPE_DETECTION_POSTPROCESSING", "I");
    jfieldID OPERATION_TYPE_EQUAL = env->GetStaticFieldID(Model, "OPERATION_TYPE_EQUAL", "I");
    jfieldID OPERATION_TYPE_EXP = env->GetStaticFieldID(Model, "OPERATION_TYPE_EXP", "I");
    jfieldID OPERATION_TYPE_EXPAND_DIMS = env->GetStaticFieldID(Model, "OPERATION_TYPE_EXPAND_DIMS", "I");
    jfieldID OPERATION_TYPE_GATHER = env->GetStaticFieldID(Model, "OPERATION_TYPE_GATHER", "I");
    jfieldID OPERATION_TYPE_GENERATE_PROPOSALS = env->GetStaticFieldID(Model, "OPERATION_TYPE_GENERATE_PROPOSALS", "I");
    jfieldID OPERATION_TYPE_GREATER = env->GetStaticFieldID(Model, "OPERATION_TYPE_GREATER", "I");
    jfieldID OPERATION_TYPE_GREATER_EQUAL = env->GetStaticFieldID(Model, "OPERATION_TYPE_GREATER_EQUAL", "I");
    jfieldID OPERATION_TYPE_GROUPED_CONV_2D = env->GetStaticFieldID(Model, "OPERATION_TYPE_GROUPED_CONV_2D", "I");
    jfieldID OPERATION_TYPE_HEATMAP_MAX_KEYPOINT = env->GetStaticFieldID(Model, "OPERATION_TYPE_HEATMAP_MAX_KEYPOINT", "I");
    jfieldID OPERATION_TYPE_INSTANCE_NORMALIZATION = env->GetStaticFieldID(Model, "OPERATION_TYPE_INSTANCE_NORMALIZATION", "I");
    jfieldID OPERATION_TYPE_LESS = env->GetStaticFieldID(Model, "OPERATION_TYPE_LESS", "I");
    jfieldID OPERATION_TYPE_LESS_EQUAL = env->GetStaticFieldID(Model, "OPERATION_TYPE_LESS_EQUAL", "I");
    jfieldID OPERATION_TYPE_LOG = env->GetStaticFieldID(Model, "OPERATION_TYPE_LOG", "I");
    jfieldID OPERATION_TYPE_LOGICAL_AND = env->GetStaticFieldID(Model, "OPERATION_TYPE_LOGICAL_AND", "I");
    jfieldID OPERATION_TYPE_LOGICAL_NOT = env->GetStaticFieldID(Model, "OPERATION_TYPE_LOGICAL_NOT", "I");
    jfieldID OPERATION_TYPE_LOGICAL_OR = env->GetStaticFieldID(Model, "OPERATION_TYPE_LOGICAL_OR", "I");
    jfieldID OPERATION_TYPE_LOG_SOFTMAX = env->GetStaticFieldID(Model, "OPERATION_TYPE_LOG_SOFTMAX", "I");
    jfieldID OPERATION_TYPE_MAXIMUM = env->GetStaticFieldID(Model, "OPERATION_TYPE_MAXIMUM", "I");
    jfieldID OPERATION_TYPE_MINIMUM = env->GetStaticFieldID(Model, "OPERATION_TYPE_MINIMUM", "I");
    jfieldID OPERATION_TYPE_NEG = env->GetStaticFieldID(Model, "OPERATION_TYPE_NEG", "I");
    jfieldID OPERATION_TYPE_NOT_EQUAL = env->GetStaticFieldID(Model, "OPERATION_TYPE_NOT_EQUAL", "I");
    jfieldID OPERATION_TYPE_PAD_V2 = env->GetStaticFieldID(Model, "OPERATION_TYPE_PAD_V2", "I");
    jfieldID OPERATION_TYPE_POW = env->GetStaticFieldID(Model, "OPERATION_TYPE_POW", "I");
    jfieldID OPERATION_TYPE_PRELU = env->GetStaticFieldID(Model, "OPERATION_TYPE_PRELU", "I");
    jfieldID OPERATION_TYPE_QUANTIZE = env->GetStaticFieldID(Model, "OPERATION_TYPE_QUANTIZE", "I");
    jfieldID OPERATION_TYPE_QUANTIZED_16BIT_LSTM = env->GetStaticFieldID(Model, "OPERATION_TYPE_QUANTIZED_16BIT_LSTM", "I");
    jfieldID OPERATION_TYPE_RANDOM_MULTINOMIAL = env->GetStaticFieldID(Model, "OPERATION_TYPE_RANDOM_MULTINOMIAL", "I");
    jfieldID OPERATION_TYPE_REDUCE_ALL = env->GetStaticFieldID(Model, "OPERATION_TYPE_REDUCE_ALL", "I");
    jfieldID OPERATION_TYPE_REDUCE_ANY = env->GetStaticFieldID(Model, "OPERATION_TYPE_REDUCE_ANY", "I");
    jfieldID OPERATION_TYPE_REDUCE_MAX = env->GetStaticFieldID(Model, "OPERATION_TYPE_REDUCE_MAX", "I");
    jfieldID OPERATION_TYPE_REDUCE_MIN = env->GetStaticFieldID(Model, "OPERATION_TYPE_REDUCE_MIN", "I");
    jfieldID OPERATION_TYPE_REDUCE_PROD = env->GetStaticFieldID(Model, "OPERATION_TYPE_REDUCE_PROD", "I");
    jfieldID OPERATION_TYPE_REDUCE_SUM = env->GetStaticFieldID(Model, "OPERATION_TYPE_REDUCE_SUM", "I");
    jfieldID OPERATION_TYPE_ROI_ALIGN = env->GetStaticFieldID(Model, "OPERATION_TYPE_ROI_ALIGN", "I");
    jfieldID OPERATION_TYPE_ROI_POOLING = env->GetStaticFieldID(Model, "OPERATION_TYPE_ROI_POOLING", "I");
    jfieldID OPERATION_TYPE_RSQRT = env->GetStaticFieldID(Model, "OPERATION_TYPE_RSQRT", "I");
    jfieldID OPERATION_TYPE_SELECT = env->GetStaticFieldID(Model, "OPERATION_TYPE_SELECT", "I");
    jfieldID OPERATION_TYPE_SIN = env->GetStaticFieldID(Model, "OPERATION_TYPE_SIN", "I");
    jfieldID OPERATION_TYPE_SLICE = env->GetStaticFieldID(Model, "OPERATION_TYPE_SLICE", "I");
    jfieldID OPERATION_TYPE_SPLIT = env->GetStaticFieldID(Model, "OPERATION_TYPE_SPLIT", "I");
    jfieldID OPERATION_TYPE_SQRT = env->GetStaticFieldID(Model, "OPERATION_TYPE_SQRT", "I");
    jfieldID OPERATION_TYPE_TILE = env->GetStaticFieldID(Model, "OPERATION_TYPE_TILE", "I");
    jfieldID OPERATION_TYPE_TOPK_V2 = env->GetStaticFieldID(Model, "OPERATION_TYPE_TOPK_V2", "I");
    jfieldID OPERATION_TYPE_TRANSPOSE_CONV_2D = env->GetStaticFieldID(Model, "OPERATION_TYPE_TRANSPOSE_CONV_2D", "I");
    jfieldID OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_LSTM = env->GetStaticFieldID(Model, "OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_LSTM", "I");
    jfieldID OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_RNN = env->GetStaticFieldID(Model, "OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_RNN", "I");
    jfieldID OPERATION_TYPE_RESIZE_NEAREST_NEIGHBOR = env->GetStaticFieldID(Model, "OPERATION_TYPE_RESIZE_NEAREST_NEIGHBOR", "I");
    jfieldID OPERATION_TYPE_QUANTIZED_LSTM = env->GetStaticFieldID(Model, "OPERATION_TYPE_QUANTIZED_LSTM", "I");
    jfieldID OPERATION_TYPE_IF = env->GetStaticFieldID(Model, "OPERATION_TYPE_IF", "I");
    jfieldID OPERATION_TYPE_WHILE = env->GetStaticFieldID(Model, "OPERATION_TYPE_WHILE", "I");
    jfieldID OPERATION_TYPE_ELU = env->GetStaticFieldID(Model, "OPERATION_TYPE_ELU", "I");
    jfieldID OPERATION_TYPE_HARD_SWISH = env->GetStaticFieldID(Model, "OPERATION_TYPE_HARD_SWISH", "I");
    jfieldID OPERATION_TYPE_FILL = env->GetStaticFieldID(Model, "OPERATION_TYPE_FILL", "I");
    jfieldID OPERATION_TYPE_RANK = env->GetStaticFieldID(Model, "OPERATION_TYPE_RANK", "I");
    jint _OPERATION_TYPE_ADD = env->GetStaticIntField(Model, OPERATION_TYPE_ADD);
    jint _OPERATION_TYPE_AVERAGE_POOL_2D = env->GetStaticIntField(Model, OPERATION_TYPE_AVERAGE_POOL_2D);
    jint _OPERATION_TYPE_CONCATENATION = env->GetStaticIntField(Model, OPERATION_TYPE_CONCATENATION);
    jint _OPERATION_TYPE_CONV_2D = env->GetStaticIntField(Model, OPERATION_TYPE_CONV_2D);
    jint _OPERATION_TYPE_DEPTHWISE_CONV_2D = env->GetStaticIntField(Model, OPERATION_TYPE_DEPTHWISE_CONV_2D);
    jint _OPERATION_TYPE_DEPTH_TO_SPACE = env->GetStaticIntField(Model, OPERATION_TYPE_DEPTH_TO_SPACE);
    jint _OPERATION_TYPE_DEQUANTIZE = env->GetStaticIntField(Model, OPERATION_TYPE_DEQUANTIZE);
    jint _OPERATION_TYPE_EMBEDDING_LOOKUP = env->GetStaticIntField(Model, OPERATION_TYPE_EMBEDDING_LOOKUP);
    jint _OPERATION_TYPE_FLOOR = env->GetStaticIntField(Model, OPERATION_TYPE_FLOOR);
    jint _OPERATION_TYPE_FULLY_CONNECTED = env->GetStaticIntField(Model, OPERATION_TYPE_FULLY_CONNECTED);
    jint _OPERATION_TYPE_HASTABLE_LOOKUP = env->GetStaticIntField(Model, OPERATION_TYPE_HASTABLE_LOOKUP);
    jint _OPERATION_TYPE_L2_NORMALIZATION = env->GetStaticIntField(Model, OPERATION_TYPE_L2_NORMALIZATION);
    jint _OPERATION_TYPE_L2_POOL_2D = env->GetStaticIntField(Model, OPERATION_TYPE_L2_POOL_2D);
    jint _OPERATION_TYPE_LOCAL_RESPONSE_NORMALIZATION = env->GetStaticIntField(Model, OPERATION_TYPE_LOCAL_RESPONSE_NORMALIZATION);
    jint _OPERATION_TYPE_LOGISTIC = env->GetStaticIntField(Model, OPERATION_TYPE_LOGISTIC);
    jint _OPERATION_TYPE_LSH_PROJECTION = env->GetStaticIntField(Model, OPERATION_TYPE_LSH_PROJECTION);
    jint _OPERATION_TYPE_LSTM = env->GetStaticIntField(Model, OPERATION_TYPE_LSTM);
    jint _OPERATION_TYPE_MAX_POOL_2D = env->GetStaticIntField(Model, OPERATION_TYPE_MAX_POOL_2D);
    jint _OPERATION_TYPE_MUL = env->GetStaticIntField(Model, OPERATION_TYPE_MUL);
    jint _OPERATION_TYPE_RELU = env->GetStaticIntField(Model, OPERATION_TYPE_RELU);
    jint _OPERATION_TYPE_RELU1 = env->GetStaticIntField(Model, OPERATION_TYPE_RELU1);
    jint _OPERATION_TYPE_RELU6 = env->GetStaticIntField(Model, OPERATION_TYPE_RELU6);
    jint _OPERATION_TYPE_RESHAPE = env->GetStaticIntField(Model, OPERATION_TYPE_RESHAPE);
    jint _OPERATION_TYPE_RESIZE_BILINEAR = env->GetStaticIntField(Model, OPERATION_TYPE_RESIZE_BILINEAR);
    jint _OPERATION_TYPE_RNN = env->GetStaticIntField(Model, OPERATION_TYPE_RNN);
    jint _OPERATION_TYPE_SOFTMAX = env->GetStaticIntField(Model, OPERATION_TYPE_SOFTMAX);
    jint _OPERATION_TYPE_SPACE_TO_DEPTH = env->GetStaticIntField(Model, OPERATION_TYPE_SPACE_TO_DEPTH);
    jint _OPERATION_TYPE_SVDF = env->GetStaticIntField(Model, OPERATION_TYPE_SVDF);
    jint _OPERATION_TYPE_TANH = env->GetStaticIntField(Model, OPERATION_TYPE_TANH);
    jint _OPERATION_TYPE_BATCH_TO_SPACE_ND = env->GetStaticIntField(Model, OPERATION_TYPE_BATCH_TO_SPACE_ND);
    jint _OPERATION_TYPE_DIV = env->GetStaticIntField(Model, OPERATION_TYPE_DIV);
    jint _OPERATION_TYPE_MEAN = env->GetStaticIntField(Model, OPERATION_TYPE_MEAN);
    jint _OPERATION_TYPE_PAD = env->GetStaticIntField(Model, OPERATION_TYPE_PAD);
    jint _OPERATION_TYPE_SPACE_TO_BATCH_ND = env->GetStaticIntField(Model, OPERATION_TYPE_SPACE_TO_BATCH_ND);
    jint _OPERATION_TYPE_SQUEEZE = env->GetStaticIntField(Model, OPERATION_TYPE_SQUEEZE);
    jint _OPERATION_TYPE_STRIDED_SLICE = env->GetStaticIntField(Model, OPERATION_TYPE_STRIDED_SLICE);
    jint _OPERATION_TYPE_SUB = env->GetStaticIntField(Model, OPERATION_TYPE_SUB);
    jint _OPERATION_TYPE_TRANSPOSE = env->GetStaticIntField(Model, OPERATION_TYPE_TRANSPOSE);
    jint _OPERATION_TYPE_ABS = env->GetStaticIntField(Model, OPERATION_TYPE_ABS);
    jint _OPERATION_TYPE_ARGMAX = env->GetStaticIntField(Model, OPERATION_TYPE_ARGMAX);
    jint _OPERATION_TYPE_ARGMIN = env->GetStaticIntField(Model, OPERATION_TYPE_ARGMIN);
    jint _OPERATION_TYPE_AXIS_ALIGNED_BBOX_TRANSFORM = env->GetStaticIntField(Model, OPERATION_TYPE_AXIS_ALIGNED_BBOX_TRANSFORM);
    jint _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_LSTM = env->GetStaticIntField(Model, OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_LSTM);
    jint _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_RNN = env->GetStaticIntField(Model, OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_RNN);
    jint _OPERATION_TYPE_BOX_WITH_NMS_LIMIT = env->GetStaticIntField(Model, OPERATION_TYPE_BOX_WITH_NMS_LIMIT);
    jint _OPERATION_TYPE_CAST = env->GetStaticIntField(Model, OPERATION_TYPE_CAST);
    jint _OPERATION_TYPE_CHANNEL_SHUFFLE = env->GetStaticIntField(Model, OPERATION_TYPE_CHANNEL_SHUFFLE);
    jint _OPERATION_TYPE_DETECTION_POSTPROCESSING = env->GetStaticIntField(Model, OPERATION_TYPE_DETECTION_POSTPROCESSING);
    jint _OPERATION_TYPE_EQUAL = env->GetStaticIntField(Model, OPERATION_TYPE_EQUAL);
    jint _OPERATION_TYPE_EXP = env->GetStaticIntField(Model, OPERATION_TYPE_EXP);
    jint _OPERATION_TYPE_EXPAND_DIMS = env->GetStaticIntField(Model, OPERATION_TYPE_EXPAND_DIMS);
    jint _OPERATION_TYPE_GATHER = env->GetStaticIntField(Model, OPERATION_TYPE_GATHER);
    jint _OPERATION_TYPE_GENERATE_PROPOSALS = env->GetStaticIntField(Model, OPERATION_TYPE_GENERATE_PROPOSALS);
    jint _OPERATION_TYPE_GREATER = env->GetStaticIntField(Model, OPERATION_TYPE_GREATER);
    jint _OPERATION_TYPE_GREATER_EQUAL = env->GetStaticIntField(Model, OPERATION_TYPE_GREATER_EQUAL);
    jint _OPERATION_TYPE_GROUPED_CONV_2D = env->GetStaticIntField(Model, OPERATION_TYPE_GROUPED_CONV_2D);
    jint _OPERATION_TYPE_HEATMAP_MAX_KEYPOINT = env->GetStaticIntField(Model, OPERATION_TYPE_HEATMAP_MAX_KEYPOINT);
    jint _OPERATION_TYPE_INSTANCE_NORMALIZATION = env->GetStaticIntField(Model, OPERATION_TYPE_INSTANCE_NORMALIZATION);
    jint _OPERATION_TYPE_LESS = env->GetStaticIntField(Model, OPERATION_TYPE_LESS);
    jint _OPERATION_TYPE_LESS_EQUAL = env->GetStaticIntField(Model, OPERATION_TYPE_LESS_EQUAL);
    jint _OPERATION_TYPE_LOG = env->GetStaticIntField(Model, OPERATION_TYPE_LOG);
    jint _OPERATION_TYPE_LOGICAL_AND = env->GetStaticIntField(Model, OPERATION_TYPE_LOGICAL_AND);
    jint _OPERATION_TYPE_LOGICAL_NOT = env->GetStaticIntField(Model, OPERATION_TYPE_LOGICAL_NOT);
    jint _OPERATION_TYPE_LOGICAL_OR = env->GetStaticIntField(Model, OPERATION_TYPE_LOGICAL_OR);
    jint _OPERATION_TYPE_LOG_SOFTMAX = env->GetStaticIntField(Model, OPERATION_TYPE_LOG_SOFTMAX);
    jint _OPERATION_TYPE_MAXIMUM = env->GetStaticIntField(Model, OPERATION_TYPE_MAXIMUM);
    jint _OPERATION_TYPE_MINIMUM = env->GetStaticIntField(Model, OPERATION_TYPE_MINIMUM);
    jint _OPERATION_TYPE_NEG = env->GetStaticIntField(Model, OPERATION_TYPE_NEG);
    jint _OPERATION_TYPE_NOT_EQUAL = env->GetStaticIntField(Model, OPERATION_TYPE_NOT_EQUAL);
    jint _OPERATION_TYPE_PAD_V2 = env->GetStaticIntField(Model, OPERATION_TYPE_PAD_V2);
    jint _OPERATION_TYPE_POW = env->GetStaticIntField(Model, OPERATION_TYPE_POW);
    jint _OPERATION_TYPE_PRELU = env->GetStaticIntField(Model, OPERATION_TYPE_PRELU);
    jint _OPERATION_TYPE_QUANTIZE = env->GetStaticIntField(Model, OPERATION_TYPE_QUANTIZE);
    jint _OPERATION_TYPE_QUANTIZED_16BIT_LSTM = env->GetStaticIntField(Model, OPERATION_TYPE_QUANTIZED_16BIT_LSTM);
    jint _OPERATION_TYPE_RANDOM_MULTINOMIAL = env->GetStaticIntField(Model, OPERATION_TYPE_RANDOM_MULTINOMIAL);
    jint _OPERATION_TYPE_REDUCE_ALL = env->GetStaticIntField(Model, OPERATION_TYPE_REDUCE_ALL);
    jint _OPERATION_TYPE_REDUCE_ANY = env->GetStaticIntField(Model, OPERATION_TYPE_REDUCE_ANY);
    jint _OPERATION_TYPE_REDUCE_MAX = env->GetStaticIntField(Model, OPERATION_TYPE_REDUCE_MAX);
    jint _OPERATION_TYPE_REDUCE_MIN = env->GetStaticIntField(Model, OPERATION_TYPE_REDUCE_MIN);
    jint _OPERATION_TYPE_REDUCE_PROD = env->GetStaticIntField(Model, OPERATION_TYPE_REDUCE_PROD);
    jint _OPERATION_TYPE_REDUCE_SUM = env->GetStaticIntField(Model, OPERATION_TYPE_REDUCE_SUM);
    jint _OPERATION_TYPE_ROI_ALIGN = env->GetStaticIntField(Model, OPERATION_TYPE_ROI_ALIGN);
    jint _OPERATION_TYPE_ROI_POOLING = env->GetStaticIntField(Model, OPERATION_TYPE_ROI_POOLING);
    jint _OPERATION_TYPE_RSQRT = env->GetStaticIntField(Model, OPERATION_TYPE_RSQRT);
    jint _OPERATION_TYPE_SELECT = env->GetStaticIntField(Model, OPERATION_TYPE_SELECT);
    jint _OPERATION_TYPE_SIN = env->GetStaticIntField(Model, OPERATION_TYPE_SIN);
    jint _OPERATION_TYPE_SLICE = env->GetStaticIntField(Model, OPERATION_TYPE_SLICE);
    jint _OPERATION_TYPE_SPLIT = env->GetStaticIntField(Model, OPERATION_TYPE_SPLIT);
    jint _OPERATION_TYPE_SQRT = env->GetStaticIntField(Model, OPERATION_TYPE_SQRT);
    jint _OPERATION_TYPE_TILE = env->GetStaticIntField(Model, OPERATION_TYPE_TILE);
    jint _OPERATION_TYPE_TOPK_V2 = env->GetStaticIntField(Model, OPERATION_TYPE_TOPK_V2);
    jint _OPERATION_TYPE_TRANSPOSE_CONV_2D = env->GetStaticIntField(Model, OPERATION_TYPE_TRANSPOSE_CONV_2D);
    jint _OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_LSTM = env->GetStaticIntField(Model, OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_LSTM);
    jint _OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_RNN = env->GetStaticIntField(Model, OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_RNN);
    jint _OPERATION_TYPE_RESIZE_NEAREST_NEIGHBOR = env->GetStaticIntField(Model, OPERATION_TYPE_RESIZE_NEAREST_NEIGHBOR);
    jint _OPERATION_TYPE_QUANTIZED_LSTM = env->GetStaticIntField(Model, OPERATION_TYPE_QUANTIZED_LSTM);
    jint _OPERATION_TYPE_IF = env->GetStaticIntField(Model, OPERATION_TYPE_IF);
    jint _OPERATION_TYPE_WHILE = env->GetStaticIntField(Model, OPERATION_TYPE_WHILE);
    jint _OPERATION_TYPE_ELU = env->GetStaticIntField(Model, OPERATION_TYPE_ELU);
    jint _OPERATION_TYPE_HARD_SWISH = env->GetStaticIntField(Model, OPERATION_TYPE_HARD_SWISH);
    jint _OPERATION_TYPE_FILL = env->GetStaticIntField(Model, OPERATION_TYPE_FILL);
    jint _OPERATION_TYPE_RANK = env->GetStaticIntField(Model, OPERATION_TYPE_RANK);
    jint _typeOperation = typeOperation;
    if ((_typeOperation & _OPERATION_TYPE_ADD) == _OPERATION_TYPE_ADD) _typeOperation -= _OPERATION_TYPE_ADD;
    if ((_typeOperation & _OPERATION_TYPE_AVERAGE_POOL_2D) == _OPERATION_TYPE_AVERAGE_POOL_2D) _typeOperation -= _OPERATION_TYPE_AVERAGE_POOL_2D;
    if ((_typeOperation & _OPERATION_TYPE_CONCATENATION) == _OPERATION_TYPE_CONCATENATION) _typeOperation -= _OPERATION_TYPE_CONCATENATION;
    if ((_typeOperation & _OPERATION_TYPE_CONV_2D) == _OPERATION_TYPE_CONV_2D) _typeOperation -= _OPERATION_TYPE_CONV_2D;
    if ((_typeOperation & _OPERATION_TYPE_DEPTHWISE_CONV_2D) == _OPERATION_TYPE_DEPTHWISE_CONV_2D) _typeOperation -= _OPERATION_TYPE_DEPTHWISE_CONV_2D;
    if ((_typeOperation & _OPERATION_TYPE_DEPTH_TO_SPACE) == _OPERATION_TYPE_DEPTH_TO_SPACE) _typeOperation -= _OPERATION_TYPE_DEPTH_TO_SPACE;
    if ((_typeOperation & _OPERATION_TYPE_DEQUANTIZE) == _OPERATION_TYPE_DEQUANTIZE) _typeOperation -= _OPERATION_TYPE_DEQUANTIZE;
    if ((_typeOperation & _OPERATION_TYPE_EMBEDDING_LOOKUP) == _OPERATION_TYPE_EMBEDDING_LOOKUP) _typeOperation -= _OPERATION_TYPE_EMBEDDING_LOOKUP;
    if ((_typeOperation & _OPERATION_TYPE_FLOOR) == _OPERATION_TYPE_FLOOR) _typeOperation -= _OPERATION_TYPE_FLOOR;
    if ((_typeOperation & _OPERATION_TYPE_FULLY_CONNECTED) == _OPERATION_TYPE_FULLY_CONNECTED) _typeOperation -= _OPERATION_TYPE_FULLY_CONNECTED;
    if ((_typeOperation & _OPERATION_TYPE_HASTABLE_LOOKUP) == _OPERATION_TYPE_HASTABLE_LOOKUP) _typeOperation -= _OPERATION_TYPE_HASTABLE_LOOKUP;
    if ((_typeOperation & _OPERATION_TYPE_L2_NORMALIZATION) == _OPERATION_TYPE_L2_NORMALIZATION) _typeOperation -= _OPERATION_TYPE_L2_NORMALIZATION;
    if ((_typeOperation & _OPERATION_TYPE_L2_POOL_2D) == _OPERATION_TYPE_L2_POOL_2D) _typeOperation -= _OPERATION_TYPE_L2_POOL_2D;
    if ((_typeOperation & _OPERATION_TYPE_LOCAL_RESPONSE_NORMALIZATION) == _OPERATION_TYPE_LOCAL_RESPONSE_NORMALIZATION) _typeOperation -= _OPERATION_TYPE_LOCAL_RESPONSE_NORMALIZATION;
    if ((_typeOperation & _OPERATION_TYPE_LOGISTIC) == _OPERATION_TYPE_LOGISTIC) _typeOperation -= _OPERATION_TYPE_LOGISTIC;
    if ((_typeOperation & _OPERATION_TYPE_LSH_PROJECTION) == _OPERATION_TYPE_LSH_PROJECTION) _typeOperation -= _OPERATION_TYPE_LSH_PROJECTION;
    if ((_typeOperation & _OPERATION_TYPE_LSTM) == _OPERATION_TYPE_LSTM) _typeOperation -= _OPERATION_TYPE_LSTM;
    if ((_typeOperation & _OPERATION_TYPE_MAX_POOL_2D) == _OPERATION_TYPE_MAX_POOL_2D) _typeOperation -= _OPERATION_TYPE_MAX_POOL_2D;
    if ((_typeOperation & _OPERATION_TYPE_MUL) == _OPERATION_TYPE_MUL) _typeOperation -= _OPERATION_TYPE_MAX_POOL_2D;
    if ((_typeOperation & _OPERATION_TYPE_RELU) == _OPERATION_TYPE_RELU) _typeOperation -= _OPERATION_TYPE_RELU;
    if ((_typeOperation & _OPERATION_TYPE_RELU1) == _OPERATION_TYPE_RELU1) _typeOperation -= _OPERATION_TYPE_RELU1;
    if ((_typeOperation & _OPERATION_TYPE_RELU6) == _OPERATION_TYPE_RELU6) _typeOperation -= _OPERATION_TYPE_RELU6;
    if ((_typeOperation & _OPERATION_TYPE_RESHAPE) == _OPERATION_TYPE_RESHAPE) _typeOperation -= _OPERATION_TYPE_RESHAPE;
    if ((_typeOperation & _OPERATION_TYPE_RESIZE_BILINEAR) == _OPERATION_TYPE_RESIZE_BILINEAR) _typeOperation -= _OPERATION_TYPE_RESIZE_BILINEAR;
    if ((_typeOperation & _OPERATION_TYPE_RNN) == _OPERATION_TYPE_RNN) _typeOperation -= _OPERATION_TYPE_RNN;
    if ((_typeOperation & _OPERATION_TYPE_SOFTMAX) == _OPERATION_TYPE_SOFTMAX) _typeOperation -= _OPERATION_TYPE_SOFTMAX;
    if ((_typeOperation & _OPERATION_TYPE_SPACE_TO_DEPTH) == _OPERATION_TYPE_SPACE_TO_DEPTH) _typeOperation -= _OPERATION_TYPE_SPACE_TO_DEPTH;
    if ((_typeOperation & _OPERATION_TYPE_SVDF) == _OPERATION_TYPE_SVDF) _typeOperation -= _OPERATION_TYPE_SVDF;
    if ((_typeOperation & _OPERATION_TYPE_TANH) == _OPERATION_TYPE_TANH) _typeOperation -= _OPERATION_TYPE_TANH;
    if ((_typeOperation & _OPERATION_TYPE_BATCH_TO_SPACE_ND) == _OPERATION_TYPE_BATCH_TO_SPACE_ND) _typeOperation -= _OPERATION_TYPE_BATCH_TO_SPACE_ND;
    if ((_typeOperation & _OPERATION_TYPE_DIV) == _OPERATION_TYPE_DIV) _typeOperation -= _OPERATION_TYPE_DIV;
    if ((_typeOperation & _OPERATION_TYPE_MEAN) == _OPERATION_TYPE_MEAN) _typeOperation -= _OPERATION_TYPE_MEAN;
    if ((_typeOperation & _OPERATION_TYPE_PAD) == _OPERATION_TYPE_PAD) _typeOperation -= _OPERATION_TYPE_PAD;
    if ((_typeOperation & _OPERATION_TYPE_SPACE_TO_BATCH_ND) == _OPERATION_TYPE_SPACE_TO_BATCH_ND) _OPERATION_TYPE_SPACE_TO_BATCH_ND;
    if ((_typeOperation & _OPERATION_TYPE_SQUEEZE) == _OPERATION_TYPE_SQUEEZE) _typeOperation -= _OPERATION_TYPE_SQUEEZE;
    if ((_typeOperation & _OPERATION_TYPE_STRIDED_SLICE) == _OPERATION_TYPE_STRIDED_SLICE) _typeOperation -= _OPERATION_TYPE_STRIDED_SLICE;
    if ((_typeOperation & _OPERATION_TYPE_SUB) == _OPERATION_TYPE_SUB) _typeOperation -= _OPERATION_TYPE_SUB;
    if ((_typeOperation & _OPERATION_TYPE_TRANSPOSE) == _OPERATION_TYPE_TRANSPOSE) _typeOperation -= _OPERATION_TYPE_TRANSPOSE;
    if ((_typeOperation & _OPERATION_TYPE_ABS) == _OPERATION_TYPE_ABS) _typeOperation -= _OPERATION_TYPE_ABS;
    if ((_typeOperation & _OPERATION_TYPE_ARGMAX) == _OPERATION_TYPE_ARGMAX) _typeOperation -= _OPERATION_TYPE_ARGMAX;
    if ((_typeOperation & _OPERATION_TYPE_ARGMIN) == _OPERATION_TYPE_ARGMIN) _typeOperation -= _OPERATION_TYPE_ARGMIN;
    if ((_typeOperation & _OPERATION_TYPE_AXIS_ALIGNED_BBOX_TRANSFORM) == _OPERATION_TYPE_AXIS_ALIGNED_BBOX_TRANSFORM) _typeOperation -= _OPERATION_TYPE_AXIS_ALIGNED_BBOX_TRANSFORM;
    if ((_typeOperation & _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_LSTM) == _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_LSTM) _typeOperation -= _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_LSTM;
    if ((_typeOperation & _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_RNN) == _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_RNN) _typeOperation  -= _OPERATION_TYPE_BIDIRECTIONAL_SEQUENCE_RNN;
    if ((_typeOperation & _OPERATION_TYPE_BOX_WITH_NMS_LIMIT) == _OPERATION_TYPE_BOX_WITH_NMS_LIMIT) _typeOperation -= _OPERATION_TYPE_BOX_WITH_NMS_LIMIT;
    if ((_typeOperation & _OPERATION_TYPE_CAST) == _OPERATION_TYPE_CAST) _typeOperation -= _OPERATION_TYPE_CAST;
    if ((_typeOperation & _OPERATION_TYPE_CHANNEL_SHUFFLE) == _OPERATION_TYPE_CHANNEL_SHUFFLE) _typeOperation -= _OPERATION_TYPE_CHANNEL_SHUFFLE;
    if ((_typeOperation & _OPERATION_TYPE_DETECTION_POSTPROCESSING) == _OPERATION_TYPE_DETECTION_POSTPROCESSING) _typeOperation -= _OPERATION_TYPE_DETECTION_POSTPROCESSING;
    if ((_typeOperation & _OPERATION_TYPE_EQUAL) == _OPERATION_TYPE_EQUAL) _typeOperation -= _OPERATION_TYPE_EQUAL;
    if ((_typeOperation & _OPERATION_TYPE_EXP) == _OPERATION_TYPE_EXP) _typeOperation -= _OPERATION_TYPE_EXP;
    if ((_typeOperation & _OPERATION_TYPE_EXPAND_DIMS) == _OPERATION_TYPE_EXPAND_DIMS) _typeOperation -= _OPERATION_TYPE_EXPAND_DIMS;
    if ((_typeOperation & _OPERATION_TYPE_GATHER) == _OPERATION_TYPE_GATHER) _typeOperation -= _OPERATION_TYPE_GATHER;
    if ((_typeOperation & _OPERATION_TYPE_GENERATE_PROPOSALS) == _OPERATION_TYPE_GENERATE_PROPOSALS) _typeOperation -= _OPERATION_TYPE_GENERATE_PROPOSALS;
    if ((_typeOperation & _OPERATION_TYPE_GREATER) == _OPERATION_TYPE_GREATER) _typeOperation -= _OPERATION_TYPE_GREATER;
    if ((_typeOperation & _OPERATION_TYPE_GREATER_EQUAL) == _OPERATION_TYPE_GREATER_EQUAL) _typeOperation -= _OPERATION_TYPE_GREATER_EQUAL;
    if ((_typeOperation & _OPERATION_TYPE_GROUPED_CONV_2D) == _OPERATION_TYPE_GROUPED_CONV_2D) _typeOperation -= _OPERATION_TYPE_GROUPED_CONV_2D;
    if ((_typeOperation & _OPERATION_TYPE_HEATMAP_MAX_KEYPOINT) == _OPERATION_TYPE_HEATMAP_MAX_KEYPOINT) _typeOperation -= _OPERATION_TYPE_HEATMAP_MAX_KEYPOINT;
    if ((_typeOperation & _OPERATION_TYPE_INSTANCE_NORMALIZATION) == _OPERATION_TYPE_INSTANCE_NORMALIZATION) _typeOperation -= _OPERATION_TYPE_INSTANCE_NORMALIZATION;
    if ((_typeOperation & _OPERATION_TYPE_LESS) == _OPERATION_TYPE_LESS) _typeOperation -= _OPERATION_TYPE_LESS;
    if ((_typeOperation & _OPERATION_TYPE_LESS_EQUAL) == _OPERATION_TYPE_LESS_EQUAL) _typeOperation -= _OPERATION_TYPE_LESS_EQUAL;
    if ((_typeOperation & _OPERATION_TYPE_LOG) == _OPERATION_TYPE_LOG) _typeOperation -= _OPERATION_TYPE_LOG;
    if ((_typeOperation & _OPERATION_TYPE_LOGICAL_AND) == _OPERATION_TYPE_LOGICAL_AND) _typeOperation -= _OPERATION_TYPE_LOGICAL_AND;
    if ((_typeOperation & _OPERATION_TYPE_LOGICAL_NOT) == _OPERATION_TYPE_LOGICAL_NOT) _typeOperation -= _OPERATION_TYPE_LOGICAL_NOT;
    if ((_typeOperation & _OPERATION_TYPE_LOGICAL_OR) == _OPERATION_TYPE_LOGICAL_OR) _typeOperation -= _OPERATION_TYPE_LOGICAL_OR;
    if ((_typeOperation & _OPERATION_TYPE_LOG_SOFTMAX) == _OPERATION_TYPE_LOG_SOFTMAX) _typeOperation -= _OPERATION_TYPE_LOG_SOFTMAX;
    if ((_typeOperation & _OPERATION_TYPE_MAXIMUM) == _OPERATION_TYPE_MAXIMUM) _typeOperation -= _OPERATION_TYPE_MAXIMUM;
    if ((_typeOperation & _OPERATION_TYPE_MINIMUM) == _OPERATION_TYPE_MINIMUM) _typeOperation -= _OPERATION_TYPE_MINIMUM;
    if ((_typeOperation & _OPERATION_TYPE_NEG) == _OPERATION_TYPE_NEG) _typeOperation -= _OPERATION_TYPE_NEG;
    if ((_typeOperation & _OPERATION_TYPE_NOT_EQUAL) == _OPERATION_TYPE_NOT_EQUAL) _typeOperation -= _OPERATION_TYPE_NOT_EQUAL;
    if ((_typeOperation & _OPERATION_TYPE_PAD_V2) == _OPERATION_TYPE_PAD_V2) _typeOperation -= _OPERATION_TYPE_PAD_V2;
    if ((_typeOperation & _OPERATION_TYPE_POW) == _OPERATION_TYPE_POW) _typeOperation -= _OPERATION_TYPE_POW;
    if ((_typeOperation & _OPERATION_TYPE_PRELU) == _OPERATION_TYPE_PRELU) _typeOperation -= _OPERATION_TYPE_PRELU;
    if ((_typeOperation & _OPERATION_TYPE_QUANTIZE) == _OPERATION_TYPE_QUANTIZE) _typeOperation -= _OPERATION_TYPE_QUANTIZE;
    if ((_typeOperation & _OPERATION_TYPE_QUANTIZED_16BIT_LSTM) == _OPERATION_TYPE_QUANTIZED_16BIT_LSTM) _typeOperation -= _OPERATION_TYPE_QUANTIZED_16BIT_LSTM;
    if ((_typeOperation & _OPERATION_TYPE_RANDOM_MULTINOMIAL) == _OPERATION_TYPE_RANDOM_MULTINOMIAL) _typeOperation -= _OPERATION_TYPE_RANDOM_MULTINOMIAL;
    if ((_typeOperation & _OPERATION_TYPE_REDUCE_ALL) == _OPERATION_TYPE_REDUCE_ALL) _typeOperation -= _OPERATION_TYPE_REDUCE_ALL;
    if ((_typeOperation & _OPERATION_TYPE_REDUCE_ANY) == _OPERATION_TYPE_REDUCE_ANY) _typeOperation -= _OPERATION_TYPE_REDUCE_ANY;
    if ((_typeOperation & _OPERATION_TYPE_REDUCE_MAX) == _OPERATION_TYPE_REDUCE_MAX) _typeOperation -= _OPERATION_TYPE_REDUCE_MAX;
    if ((_typeOperation & _OPERATION_TYPE_REDUCE_MIN) == _OPERATION_TYPE_REDUCE_MIN) _typeOperation -= _OPERATION_TYPE_REDUCE_MIN;
    if ((_typeOperation & _OPERATION_TYPE_REDUCE_PROD) == _OPERATION_TYPE_REDUCE_PROD) _typeOperation -= _OPERATION_TYPE_REDUCE_PROD;
    if ((_typeOperation & _OPERATION_TYPE_REDUCE_SUM) == _OPERATION_TYPE_REDUCE_SUM) _typeOperation -= _OPERATION_TYPE_REDUCE_SUM;
    if ((_typeOperation & _OPERATION_TYPE_ROI_ALIGN) ==  _OPERATION_TYPE_ROI_ALIGN) _typeOperation -= _OPERATION_TYPE_ROI_ALIGN;
    if ((_typeOperation & _OPERATION_TYPE_ROI_POOLING) == _OPERATION_TYPE_ROI_POOLING) _typeOperation -= _OPERATION_TYPE_ROI_POOLING;
    if ((_typeOperation & _OPERATION_TYPE_RSQRT) == _OPERATION_TYPE_RSQRT) _typeOperation -= _OPERATION_TYPE_RSQRT;
    if ((_typeOperation & _OPERATION_TYPE_SELECT) == _OPERATION_TYPE_SELECT) _typeOperation -= _OPERATION_TYPE_SELECT;
    if ((_typeOperation & _OPERATION_TYPE_SIN) == _OPERATION_TYPE_SIN) _typeOperation -= _OPERATION_TYPE_SIN;
    if ((_typeOperation & _OPERATION_TYPE_SLICE) == _OPERATION_TYPE_SLICE) _typeOperation -= _OPERATION_TYPE_SLICE;
    if ((_typeOperation & _OPERATION_TYPE_SPLIT) == _OPERATION_TYPE_SPLIT) _typeOperation -= _OPERATION_TYPE_SPLIT;
    if ((_typeOperation & _OPERATION_TYPE_SQRT) == _OPERATION_TYPE_SQRT) _typeOperation -= _OPERATION_TYPE_SQRT;
    if ((_typeOperation & _OPERATION_TYPE_TILE) == _OPERATION_TYPE_TILE) _typeOperation -= _OPERATION_TYPE_TILE;
    if ((_typeOperation & _OPERATION_TYPE_TOPK_V2) == _OPERATION_TYPE_TOPK_V2) _typeOperation -= _OPERATION_TYPE_TOPK_V2;
    if ((_typeOperation & _OPERATION_TYPE_TRANSPOSE_CONV_2D) == _OPERATION_TYPE_TRANSPOSE_CONV_2D) _typeOperation -= _OPERATION_TYPE_TRANSPOSE_CONV_2D;
    if ((_typeOperation & _OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_LSTM) == _OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_LSTM) _typeOperation -= _OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_LSTM;
    if ((_typeOperation & _OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_RNN) ==_OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_RNN) _typeOperation -= _OPERATION_TYPE_UNIDIRECTIONAL_SEQUENCE_RNN;
    if ((_typeOperation & _OPERATION_TYPE_RESIZE_NEAREST_NEIGHBOR) == _OPERATION_TYPE_RESIZE_NEAREST_NEIGHBOR) _typeOperation -= _OPERATION_TYPE_RESIZE_NEAREST_NEIGHBOR;
    if ((_typeOperation & _OPERATION_TYPE_QUANTIZED_LSTM) == _OPERATION_TYPE_QUANTIZED_LSTM) _typeOperation -= _OPERATION_TYPE_QUANTIZED_LSTM;
    if ((_typeOperation & _OPERATION_TYPE_IF) ==_OPERATION_TYPE_IF) _typeOperation -= _OPERATION_TYPE_WHILE;
    if ((_typeOperation & _OPERATION_TYPE_WHILE) == _OPERATION_TYPE_WHILE) _typeOperation -= _OPERATION_TYPE_WHILE;
    if ((_typeOperation & _OPERATION_TYPE_ELU) == _OPERATION_TYPE_ELU) _typeOperation -= _OPERATION_TYPE_ELU;
    if ((_typeOperation & _OPERATION_TYPE_HARD_SWISH) == _OPERATION_TYPE_HARD_SWISH) _typeOperation -= _OPERATION_TYPE_HARD_SWISH;
    if ((_typeOperation & _OPERATION_TYPE_FILL) == _OPERATION_TYPE_FILL) _typeOperation -= _OPERATION_TYPE_FILL;
    if ((_typeOperation & _OPERATION_TYPE_RANK) == _OPERATION_TYPE_RANK) _typeOperation -= _OPERATION_TYPE_RANK;
    if (_typeOperation != 0) {
        char *message = (char*)malloc(sizeof(char) * 313);
        strcpy(message, "You cannot use the addOperation method of the Model object you are currently using, because the operationType variable does ");
        strcat(message, "not have any of the allowed values, so you can only choose one or more of the static integer values of the Model class, ");
        strcat(message, "according to the function that want it to be done on the Model object");
        showMessageError(env, message);
        return;
    }
    jint *dataInputs = env->GetIntArrayElements(inputs, NULL);
    jint *dataOutputs = env->GetIntArrayElements(outputs, NULL);
    void **Result = addOperationModel(_currentModel, typeOperation, sizeInputs, dataInputs, sizeOutputs, dataOutputs);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_getSupportedOperationsForDevices(JNIEnv *env, jobject model, jobjectArray devices, jbooleanArray isSupportedOps) {
    if (devices == NULL) {
        char *message = (char*)malloc(sizeof(char) * 146);
        strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because the array devices ");
        strcat(message, "you provided is set to null");
        showMessageError(env, message);
        return;
    }
    jsize sizeDevices = env->GetArrayLength(devices);
    if (sizeDevices == 0) {
        char *message = (char*)malloc(sizeof(char) * 140);
        strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because the array devices ");
        strcat(message, "you provided is empty");
        showMessageError(env, message);
        return;
    }
    jclass Model = env->GetObjectClass(model);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 111);
        strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because is invalid");
        showMessageError(env, message);
        return;
    }
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 138);
        strcpy(message, "You cannot use the getSupportedOperationsForDevices method because the Model object you are using was not created with ");
        strcat(message, "the newModel method");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 29
    jclass Device = env->FindClass("com/draico/asvappra/neuralnetworks/Device");
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    for (jint position = 0; position < sizeDevices; position++) {
        jobject device = env->GetObjectArrayElement(devices, position);
        if (device == NULL) {
            char *message = (char*)malloc(sizeof(char) * 162);
            strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because the devices ");
            strcat(message, "array you provided contains at least a null value");
            showMessageError(env, message);
            return;
        }
        jint _currentDevice = env->GetIntField((jobject)device, currentDevice);
        jboolean _isCreated = env->GetBooleanField((jobject)device, isCreated);
        if (_currentDevice == -1) {
            char *message = (char*)malloc(sizeof(char) * 168);
            strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because the array ");
            strcat(message, "devices you provided contains at least one invalid device");
            showMessageError(env, message);
            return;
        }
        if (!_isCreated) {
            char *message = (char*)malloc(sizeof(char) * 218);
            strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because the devices ");
            strcat(message, "array you provided contains at least one device that was not created with the Device.getDevices( ) method");
            showMessageError(env, message);
            return;
                }
            }
            for (jint position1 = 0; position1 < (sizeDevices - 1); position1++) {
                jobject device1 = env->GetObjectArrayElement(devices, position1);
                for (jint position2 = position1 + 1; position2 < sizeDevices; position2++) {
                    jobject device2 = env->GetObjectArrayElement(devices, position2);
                    if (env->IsSameObject(device1, device2)) {
                        char *message = (char*)malloc(sizeof(char) * 169);
                        strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because the array ");
                        strcat(message, "devices you provided contains at least 2 identical devices");
                        showMessageError(env, message);
                        return;
                    }
                }
            }
            ANeuralNetworksDevice _devicesList[sizeDevices];
            for (jint position = 0; position < sizeDevices; position++) {
                jobject device = env->GetObjectArrayElement(devices, position);
                jint _currentDevice = env->GetIntField(device, currentDevice);
                _devicesList[position] = devicesList[_currentDevice];
            }
            jboolean *devicesSupported;
            ANeuralNetworksModel *_model = modelList[_currentModel];
            int result = ANeuralNetworksModel_getSupportedOperationsForDevices(_model, _devicesList, (uint32_t)sizeDevices, devicesSupported);
            if (result == ANEURALNETWORKS_NO_ERROR) {
                isSupportedOps = env->NewBooleanArray(sizeDevices);
                env->SetBooleanArrayRegion(isSupportedOps, 0, sizeDevices, devicesSupported);
            } else {
                char *message = (char*)malloc(sizeof(char) * 150);
                strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because ");
                findErrorType(result, message);
                showMessageError(env, message);
                return;
            }
#else
    char *message = (char*)malloc(sizeof(char) * 238);
    strcpy(message, "You cannot use the getSupportedOperationsForDevices method of the Model object you are using because the version of the ");
    strcat(message, "android operating system that your android device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_identifyInputsAndOutputs(JNIEnv *env, jobject model, jintArray inputs, jintArray outputs) {
    if (inputs == NULL || outputs == NULL) {
        char *message = (char*)malloc(sizeof(char) * 135);
        strcpy(message, "You cannot use the identifyInputsAndOutputs method because the input and output arrays provided by at least one of them ");
        strcat(message, "are set to null");
        showMessageError(env, message);
        return;
    }
    jsize sizeInputs = env->GetArrayLength(inputs);
    jsize sizeOutputs = env->GetArrayLength(outputs);
    if (sizeInputs == 0 || sizeOutputs == 0) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the identifyInputsAndOutputs method because the input and output arrays provided by at least one of the ");
        strcat(message, "arrays is empty");
        showMessageError(env, message);
        return;
    }
    jclass Model = env->GetObjectClass(model);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 111);
        strcpy(message, "You cannot use the identifyInputsAndOutputs method because the arrays the Model object you are using is invalid");
        showMessageError(env, message);
        return;
    }
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 130);
        strcpy(message, "You cannot use the identifyInputsAndOutputs method because the Model object you are using was not created with the ");
        strcat(message, "newModel method");
        showMessageError(env, message);
        return;
    }
    jint *dataInputs = env->GetIntArrayElements(inputs, NULL);
    jint *dataOutputs = env->GetIntArrayElements(outputs, NULL);
    void **Result = identifyInputsAndOutputs(_currentModel, sizeInputs, sizeOutputs, dataInputs, dataOutputs);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_relaxComputationFloat32toFloat16(JNIEnv *env, jobject model, jboolean allow) {
    jclass Model = env->GetObjectClass(model);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 108);
        strcpy(message, "You cannot use the relaxComputationFloat32toFloat16 method because the Model object you are using is invalid");
        showMessageError(env, message);
        return;
    }
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 138);
        strcpy(message, "You cannot use the relaxComputationFloat32toFloat16 method because the Model object you are using was not created with ");
        strcat(message, "the newModel method");
        showMessageError(env, message);
        return;
    }
    void **Result = relaxComputationFloat32toFloat16(_currentModel, allow);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_setOperandSymmPerChannelQuantParams(JNIEnv *env, jobject model, jobject symmPerChannelQuantParams,
                                                       jint indexOperand) {
    if (symmPerChannelQuantParams == NULL) {
        char *message = (char*)malloc(sizeof(char) * 168);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method of the Model object you are using because the ");
        strcat(message, "SymmPerChannelQuantParams object you provided is set to null");
        showMessageError(env, message);
        return;
    }
    jclass Model = env->GetObjectClass(model);
    jclass SymmPerChannelQuantParams = env->GetObjectClass(symmPerChannelQuantParams);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 132);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method of the Model object you are using because that Model object is ");
        strcat(message, "invalid");
        showMessageError(env, message);
        return;
    }
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 141);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method because the Model object you are using was not created ");
        strcat(message, "with the newModel method");
        showMessageError(env, message);
        return;
    }
    jfieldID channelDim = env->GetFieldID(SymmPerChannelQuantParams, "channelDim", "I");
    jfieldID scaleCount = env->GetFieldID(SymmPerChannelQuantParams, "scaleCount", "I");
    jfieldID scales = env->GetFieldID(SymmPerChannelQuantParams, "scales", "[F");
    jint _channelDim = env->GetIntField(symmPerChannelQuantParams, channelDim);
    jint _scaleCount = env->GetIntField(symmPerChannelQuantParams, scaleCount);
    jfloatArray _scales = (jfloatArray)env->GetObjectField(symmPerChannelQuantParams, scales);
    jfloat *dataScales = env->GetFloatArrayElements(_scales, NULL);
    if (_scales == NULL) {
        char *message = (char*)malloc(sizeof(char) * 154);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method because the scales array of the SymmPerChannelQuantParams ");
        strcat(message, "object you provided is set to null");
        showMessageError(env, message);
        return;
    }
    jsize sizeScales = env->GetArrayLength(_scales);
    if (sizeScales == 0) {
        char *message = (char*)malloc(sizeof(char) * 148);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method because the scales array of the SymmPerChannelQuantParams ");
        strcat(message, "object you provided is empty");
        showMessageError(env, message);
        return;
    }
    if (_channelDim <= 0 || _scaleCount <= 0 || indexOperand <= 0) {
        char *message = (char*)malloc(sizeof(char) * 235);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method of the Model object you are using because the channelDim or");
        strcat(message, "indexOperand or scaleCount values of the SymmPerChannelQuantParams object you provided are equal to or less than 0");
        showMessageError(env, message);
        return;
    }
    if (sizeScales != _channelDim) {
        char *message = (char*)malloc(sizeof(char) * 266);
        strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method of the Model object you are using because the size of the ");
        strcat(message, "scales array is different from the number of channels you provided in the channelDim variable of the ");
        strcat(message, "SymmPerChannelQuantParams object you provided");
        showMessageError(env, message);
        return;
    }
    for (jint position = 0; position < sizeScales; position++) {
        if (dataScales[position] <= 0) {
            char *message = (char*)malloc(sizeof(char) * 207);
            strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method of the Model object you are using because the scale ");
            strcat(message, "array values are less than or equal to 0 of the SymmPerChannelQuantParams object you provided");
            showMessageError(env, message);
            return;
        }
    }
#if __ANDROID_API__ >= 29
    ANeuralNetworksSymmPerChannelQuantParams *params = (ANeuralNetworksSymmPerChannelQuantParams*)malloc(sizeof(ANeuralNetworksSymmPerChannelQuantParams));
            params->channelDim = (uint32_t)_channelDim;
            params->scaleCount = (uint32_t)_scaleCount;
            params->scales = dataScales;
            void **Result = setOperandSymmPerChannelQuantParams(_currentModel, (int32_t)indexOperand, params);
            if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 217);
    strcpy(message, "You cannot use the setOperandSymmPerChannelQuantParams method of the Model object you are using because the android ");
    strcat(message, "version that your android device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
    return;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_setOperandValue(JNIEnv *env, jobject model, jint indexOperand, jobject value) {
    if (value == NULL) {
        char *message = (char*)malloc(sizeof(char) * 128);
        strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the value object you provided is ");
        strcat(message, "set to null");
        showMessageError(env, message);
        return;
    }
    jclass Model = env->GetObjectClass(model);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 111);
        strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the Model object is invalid");
        showMessageError(env, message);
        return;
    }
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 121);
        strcpy(message, "You cannot use the setOperandValue method because the Model object you are using was not created with the newModel method");
        showMessageError(env, message);
        return;
    }
    jclass Object = env->FindClass("java/lang/Object");
    jclass Class = env->FindClass("java/lang/Class");
    jclass Integer = env->FindClass("java/lang/Integer");
    jclass Short = env->FindClass("java/lang/Short");
    jclass Long = env->FindClass("java/lang/Long");
    jclass Float = env->FindClass("java/lang/Float");
    jclass Character = env->FindClass("java/lang/Character");
    jclass Byte = env->FindClass("java/lang/Byte");
    jmethodID getClass = env->GetMethodID(Object, "getClass", "()Ljava/lang/Class;");
    jmethodID getName = env->GetMethodID(Class, "getName", "()Ljava/lang/String;");
    jmethodID isArray = env->GetMethodID(Class, "isArray", "()Z");
    jmethodID intValue = env->GetMethodID(Integer, "intValue", "()I");
    jmethodID shortValue = env->GetMethodID(Short, "shortValue", "()S");
    jmethodID longValue = env->GetMethodID(Long, "longValue", "()J");
    jmethodID floatValue = env->GetMethodID(Float, "floatValue", "()F");
    jmethodID charValue = env->GetMethodID(Character, "charValue", "()C");
    jmethodID byteValue = env->GetMethodID(Byte, "byteValue", "()B");
    jobject objectClass = env->CallObjectMethod(value, getClass);
    jstring nameClass = (jstring)env->CallObjectMethod(objectClass, getName);
    jboolean _isArray = env->CallBooleanMethod(objectClass, isArray);
    const char *_nameClass = env->GetStringUTFChars(nameClass, NULL);
    void **Result;
    if (_isArray)  {
        jobjectArray dataValue = (jobjectArray)value;
        jsize sizeArray = env->GetArrayLength(dataValue);
        if (strcmp(_nameClass, "[I") == 0) {
            if (sizeArray < (128 / sizeof(jint))) {
                char *message = (char*)malloc(sizeof(char) * 153);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type int you ");
                strcat(message, "provided is less than 32 data in length");
                showMessageError(env, message);
                return;
            }
            jint *data = env->GetIntArrayElements((jintArray)dataValue, NULL);
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jint) * sizeArray);
        } else if (strcmp(_nameClass, "[S") == 0) {
            if (sizeArray < (128 / sizeof(jshort))) {
                char *message = (char*)malloc(sizeof(char) * 155);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type short you ");
                strcat(message, "provided is less than 64 data in length");
                showMessageError(env, message);
                return;
            }
            jshort *data = env->GetShortArrayElements((jshortArray)dataValue, NULL);
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jshort) * sizeArray);
        } else if (strcmp(_nameClass, "[J") == 0) {
            if (sizeArray < (128 / sizeof(jlong))) {
                char *message = (char*)malloc(sizeof(char) * 154);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type long you ");
                strcat(message, "provided is less than 16 data in length");
                showMessageError(env, message);
                return;
            }
            jlong *data = env->GetLongArrayElements((jlongArray)dataValue, NULL);
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jlong) * sizeArray);
        } else if (strcmp(_nameClass, "[F") == 0) {
            if (sizeArray < (128 / sizeof(jfloat))) {
                char *message = (char*)malloc(sizeof(char) * 155);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type float you ");
                strcat(message, "provided is less than 32 data in length");
                showMessageError(env, message);
                return;
            }
            jfloat *data = env->GetFloatArrayElements((jfloatArray)dataValue, NULL);
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jfloat) * sizeArray);
        } else if (strcmp(_nameClass, "[C") == 0) {
            if (sizeArray < 128) {
                char *message = (char*)malloc(sizeof(char) * 155);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type char you ");
                strcat(message, "provided is less than 128 data in length");
                showMessageError(env, message);
                return;
            }
            char *data = (char*)env->GetCharArrayElements((jcharArray)dataValue, NULL);
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(char) * sizeArray);
        } else if (strcmp(_nameClass, "[B") == 0) {
            if (sizeArray < 128) {
                char *message = (char*)malloc(sizeof(char) * 155);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type byte you ");
                strcat(message, "provided is less than 128 data in length");
                showMessageError(env, message);
                return;
            }
            jbyte *data = env->GetByteArrayElements((jbyteArray)dataValue, NULL);
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jbyte) * sizeArray);
        } else if (strcmp(_nameClass, "[Ljava.lang.Integer;") == 0) {
            if (sizeArray < (128 /sizeof(jint))) {
                char *message = (char*)malloc(sizeof(char) * 157);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type Integer ");
                strcat(message, "you provided is less than 32 data in length");
                showMessageError(env, message);
                return;
            }
            jint *data = (jint*)malloc(sizeof(jint) * sizeArray);
            for (jint position = 0; position < sizeArray; position++) {
                jobject number = env->GetObjectArrayElement(dataValue, position);
                data[position] = env->CallIntMethod(number, intValue);
            }
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jint) * sizeArray);
        } else if (strcmp(_nameClass, "[Ljava.lang.Short;") == 0) {
            if (sizeArray < (128 /  sizeof(jshort))) {
                char *message = (char*)malloc(sizeof(char) * 155);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type Short you ");
                strcat(message, "provided is less than 64 data in length");
                showMessageError(env, message);
                return;
            }
            jshort *data = (jshort*)malloc(sizeof(jshort) * sizeArray);
            for (jint position = 0; position < sizeArray; position++) {
                jobject number = env->GetObjectArrayElement(dataValue, position);
                data[position] = env->CallShortMethod(number, shortValue);
            }
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jshort) * sizeArray);
        } else if (strcmp(_nameClass, "[Ljava.lang.Long;") == 0) {
            if (sizeArray < (28 / sizeof(jlong))) {
                char *message = (char*)malloc(sizeof(char) * 154);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type Long you ");
                strcat(message, "provided is less than 16 data in length");
                showMessageError(env, message);
                return;
            }
            jlong *data = (jlong*)malloc(sizeof(jlong) * sizeArray);
            for (jint position = 0; position < sizeArray; position++) {
                jobject number = env->GetObjectArrayElement(dataValue, position);
                data[position] = env->CallLongMethod(number, longValue);
            }
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jlong) * sizeArray);
        } else if (strcmp(_nameClass, "[Ljava.lang.Float;") == 0) {
            if (sizeArray < (128 / sizeof(jfloat))) {
                char *message = (char*)malloc(sizeof(char) * 155);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type Float ");
                strcat(message, "you provided is less than 32 data in length");
                showMessageError(env, message);
                return;
            }
            jfloat *data = (jfloat*)malloc(sizeof(jfloat) * sizeArray);
            for (jint position = 0; position < sizeArray; position++) {
                jobject number = env->GetObjectArrayElement(dataValue, position);
                data[position] = env->CallFloatMethod(number, floatValue);
            }
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jfloat) * sizeArray);
        } else if (strcmp(_nameClass, "[Ljava.lang.Character;") == 0) {
            if (sizeArray < 128) {
                char *message = (char*)malloc(sizeof(char) * 160);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type Character ");
                strcat(message, "you provided is less than 128 data in length");
                showMessageError(env, message);
                return;
            }
            char *data = (char*)malloc(sizeof(char) * sizeArray);
            for (jint position = 0; position < sizeArray; position++) {
                jobject number = env->GetObjectArrayElement(dataValue, position);
                data[position] = (char)env->CallCharMethod(number, charValue);
            }
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(char) * sizeArray);
        } else if (strcmp(_nameClass, "[Ljava.lang.Byte;") == 0) {
            if (sizeArray < 128) {
                char *message = (char*)malloc(sizeof(char) * 100);
                strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the array of the type Byte you ");
                strcat(message, "provided is less than 128 data in length");
                showMessageError(env, message);
                return;
            }
            jbyte *data = (jbyte*)malloc(sizeof(jbyte) * sizeArray);
            for (jint position = 0; position < sizeArray; position++) {
                jobject number = env->GetObjectArrayElement(dataValue, position);
                data[position] = env->CallByteMethod(number, byteValue);
            }
            Result = setOperandValue(_currentModel, indexOperand, data, sizeof(jbyte) * sizeArray);
        } else {
            char *message = (char*)malloc(sizeof(char) * 261);
            strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because the value object can contain a ");
            strcat(message, "one-dimensional array of any of the primitive types int, short, long, float, char, byte and of the Integer, Short, ");
            strcat(message, "Long, Float, Character and Byte");
            showMessageError(env, message);
            return;
        }
    } else {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the setOperandValue method of the Model object you are using because you can only enter data arrays of ");
        strcat(message, "the primitive types int, short, long, float, double, byte and char, as well as the Integer, Short, Long, Float, Double, ");
        strcat(message, "Byte and Character");
        showMessageError(env, message);
        return;
    }
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_setOperandValueFromMemory(JNIEnv *env, jobject model, jobject memory, jint indexOperand, jint offset,
                                             jint size) {
    if (memory == NULL) {
        char *message = (char*)malloc(sizeof(char) * 105);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because the Memory object you provided is set to null");
        showMessageError(env, message);
        return;
    }
    jclass Memory = env->GetObjectClass(memory);
    jclass Model = env->GetObjectClass(model);
    jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jfieldID isCreatedMemory = env->GetFieldID(Memory, "isCreated", "Z");
    jfieldID isCreatedModel = env->GetFieldID(Model, "isCreated", "Z");
    jint _currentMemory = env->GetIntField(memory, currentMemory);
    jint _currentModel = env->GetIntField(model, currentModel);
    if (_currentMemory == -1 || _currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 140);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because the Memory object you provided and/or the Model object you ");
        strcat(message, "are using are invalid");
        showMessageError(env, message);
        return;
    }
    jboolean _isCreatedMemory = env->GetBooleanField(memory, isCreatedMemory);
    jboolean _isCreatedModel = env->GetBooleanField(model, isCreatedModel);
    if (!_isCreatedMemory || !_isCreatedModel) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because the Memory object was not created with the newMemory ");
        strcat(message, "method or with the newMemoryFromFileDescriptor method, also the Model object you are using may not have been created ");
        strcat(message, "with the newModel method");
        showMessageError(env, message);
        return;
    }
    if (indexOperand < 0) {
        char *message = (char*)malloc(sizeof(char) * 120);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because the value of the indexOperand variable cannot be less than 0");
        showMessageError(env, message);
        return;
    }
    if (offset < 0 || size <= 0) {
        char *message = (char*)malloc(sizeof(char) * 162);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because the offset is less than 0, also check if the value of the ");
        strcat(message, "size variable is not less than or equal to 0");
        showMessageError(env, message);
        return;
    }
    jfieldID sizeMemory = env->GetFieldID(Memory, "size", "I");
    jfieldID offsetMemory = env->GetFieldID(Memory, "offset", "I");
    jint _sizeMemory = env->GetIntField(memory, sizeMemory);
    jint _offsetMemory = env->GetIntField(memory, offsetMemory);
    if (offset < _offsetMemory) {
        char *message = (char*)malloc(sizeof(char) * 152);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because the offset you provided is less than the offset you ");
        strcat(message, "provided when creating the Memory object");
        showMessageError(env, message);
        return;
    }
    if ((offset + size) > _sizeMemory) {
        char *message = (char*)malloc(sizeof(char) * 179);
        strcpy(message, "You cannot use the setOperandValueFromMemory method because the sum of the values of the offset and size variables are ");
        strcat(message, "greater than the size of the Memory object that you provided");
        showMessageError(env, message);
        return;
    }
    void **Result = setOperandValueFromMemory(_currentModel, _currentMemory, indexOperand, offset, size);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Model_setOperandValueFromModel(JNIEnv *env, jobject model, jobject modelSrc, jint indexOperand) {
    if (modelSrc == NULL) {
        char *message = (char*)malloc(sizeof(char) * 103);
        strcpy(message, "You cannot use the setOperandValueFromModel method because the Model object you provided is set to null");
        showMessageError(env, message);
        return;
    }
    jclass Model = env->GetObjectClass(model);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jint _currentModel = env->GetIntField(model, currentModel);
    jint currentModelSrc = env->GetIntField(modelSrc, currentModel);
    if (_currentModel == -1 || currentModelSrc == -1) {
        char *message = (char*)malloc(sizeof(char) * 134);
        strcpy(message, "You cannot use the setOperandValueFromModel method because the Model object you provided or the Model object you are ");
        strcat(message, "using are invalid");
        showMessageError(env, message);
        return;
    }
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jboolean _isCreatedModel = env->GetBooleanField(model, isCreated);
    jboolean isCreatedModelSrc = env->GetBooleanField(modelSrc, isCreated);
    if (!_isCreatedModel || !isCreatedModelSrc) {
        char *message = (char*)malloc(sizeof(char) * 164);
        strcpy(message, "You cannot use the setOperandValueFromModel method because the Model object you provided or the Model object you are ");
        strcat(message, "using were not created with the newModel method");
        showMessageError(env, message);
        return;
    }
    if (indexOperand < 0) {
        char *message = (char*)malloc(sizeof(char) * 112);
        strcpy(message, "You cannot use the setOperandValueFromModel method because the indexOperand variable you provided is less than 0");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 30
    void **Result = setOperandValueFromModel(_currentModel, currentModelSrc, indexOperand);
            if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 172);
    strcpy(message, "You cannot use the setOperandValueFromModel method because the android version that your android device has installed ");
    strcat(message, "is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Compilation_newCompilation(JNIEnv *env, jclass Compilation, jobject model) {
    if (model == NULL) {
        char *message = (char*)malloc(sizeof(char) * 93);
        strcpy(message, "You cannot use the newCompilation method because the Model object you provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass Model = env->GetObjectClass(model);
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jfieldID isCreated = env->GetFieldID(Model, "isCreated", "Z");
    jint _currentModel = env->GetIntField(model, currentModel);
    jboolean _isCreated = env->GetBooleanField(model, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 119);
        strcpy(message, "You cannot use the newCompilation method because the Model object you provided was not created with the newModel method");
        showMessageError(env, message);
        return NULL;
    }
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 89);
        strcpy(message, "You cannot use the newCompilation method because the Model object you provided is invalid");
        showMessageError(env, message);
        return NULL;
    }
    void **Result = createNeuralNetworkCompilation(_currentModel);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) {
        showMessageError(env, (char*)Result[0]);
        return NULL;
    } else {
        jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
        jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
        jobject compilation = env->AllocObject(Compilation);
        env->SetIntField(compilation, currentCompilation, currentPositionCompilation);
        env->SetBooleanField(compilation, isCreated, JNI_TRUE);
        currentPositionCompilation++;
        return compilation;
    }
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Compilation_newCompilationForDevices(JNIEnv *env, jclass Compilation, jobject model,
                                                               jobjectArray devices) {
    if (model == NULL || devices == NULL) {
        char *message = (char*)malloc(sizeof(char) * 129);
        strcpy(message, "You cannot use the newCompilationForDevices method because the Model object and/or the devices array you provided are ");
        strcat(message, "set to null");
        showMessageError(env, message);
        return NULL;
    }
    jsize sizeDevices = env->GetArrayLength(devices);
    if (sizeDevices == 0) {
        char *message = (char*)malloc(sizeof(char) * 85);
        strcpy(message, "You cannot use the newCompilationForDevices method because the array devices is empty");
        showMessageError(env, message);
        return NULL;
    }
    jclass Device = env->FindClass("com/draico/asvappra/neuralnetworks/Device");
    jclass Model = env->GetObjectClass(model);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID currentModel = env->GetFieldID(Model, "currentModel", "I");
    jfieldID isCreatedDevice = env->GetFieldID(Device, "isCreated", "Z");
    jfieldID isCreatedModel = env->GetFieldID(Model, "isCreated", "Z");
    jint _currentModel = env->GetIntField(model, currentModel);
    jboolean _isCreatedModel = env->GetBooleanField(model, isCreatedModel);
    if (_currentModel == -1) {
        char *message = (char*)malloc(sizeof(char) * 99);
        strcpy(message, "You cannot use the newCompilationForDevices method because the Model object you provided is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreatedModel) {
        char *message = (char*)malloc(sizeof(char) * 129);
        strcpy(message, "You cannot use the newCompilationForDevices method because the Model object you provided was not created with the ");
        strcat(message, "newModel method");
        showMessageError(env, message);
        return NULL;
    }
    jint *positionDevices = (jint*)malloc(sizeof(jint) * sizeDevices);
    if (sizeDevices > 1) {
        for (jint position1 = 0; position1 < (sizeDevices -1); position1++) {
            jobject device1 = env->GetObjectArrayElement(devices, position1);
            for (jint position2 = position1 + 1; position2 < sizeDevices; position2++) {
                jobject device2 = env->GetObjectArrayElement(devices, position2);
                if (env->IsSameObject(device1, device2)) {
                    char *message = (char*)malloc(sizeof(char) * 126);
                    strcpy(message, "You cannot use the newCompilationForDevices method because the device array you provided contains at least 2 ");
                    strcat(message, "identical devices");
                    showMessageError(env, message);
                    return NULL;
                }
                jint _currentDevice1 = env->GetIntField(device1, currentDevice);
                jint _currentDevice2 = env->GetIntField(device2, currentDevice);
                if (_currentDevice1 == -1 || _currentDevice2 == -1) {
                    char *message = (char*)malloc(sizeof(char) * 123);
                    strcpy(message, "You cannot use the newCompilationForDevices method because the device array you provided contains at least 1 ");
                    strcat(message, "invalid device");
                    showMessageError(env, message);
                    return NULL;
                }
                jboolean _isCreatedDevice1 = env->GetBooleanField(device1, isCreatedDevice);
                jboolean _isCreatedDevice2 = env->GetBooleanField(device2, isCreatedDevice);
                if (!_isCreatedDevice1 || !_isCreatedDevice2) {
                    char *message = (char*)malloc(sizeof(char) * 184);
                    strcpy(message, "You cannot use the newCompilationForDevices method because the device array you provided contains at least 1 ");
                    strcat(message, "device that was not created using the getDevices method of the Device class");
                    showMessageError(env, message);
                    return NULL;
                }
            }
        }
    } else {
        jobject device = env->GetObjectArrayElement(devices, 0);
        jint _currentDevice = env->GetIntField(device, currentDevice);
        jboolean _isCreated = env->GetBooleanField(device, isCreatedDevice);
        if (_currentDevice == -1) {
            char *message = (char*)malloc(sizeof(char) * 139);
            strcpy(message, "You cannot use the newCompilationForDevices method because the array devices you provided only contains 1 device and ");
            strcat(message, "this device is invalid");
            showMessageError(env, message);
            return NULL;
        }
        if (!_isCreated) {
            char *message = (char*)malloc(sizeof(char) * 191);
            strcpy(message, "You cannot use the newCompilationForDevices method because the devices array you provided only contains 1 device and ");
            strcat(message, "this device was not created with the getDevices method of the Device class");
            showMessageError(env, message);
            return NULL;
        }
    }
    for (jint position = 0; position < sizeDevices; position++) {
        jobject device = env->GetObjectArrayElement(devices, position);
        positionDevices[position] = env->GetIntField(device, currentDevice);
    }
#if __ANDROID_API__ >= 29
    void **Result = createForDevices(_currentModel, positionDevices, sizeDevices);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
        jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
        jobject compilation = env->AllocObject(Compilation);
        env->SetIntField(compilation, currentCompilation, currentPositionCompilation);
        env->SetBooleanField(compilation, isCreated, JNI_TRUE);
        currentPositionCompilation++;
        if (currentPositionCompilation == 100) currentPositionCompilation = 0;
        return compilation;
    } else {
        showMessageError(env, (char*)Result[0]);
        return NULL;
    }
#else
    char *message = (char*)malloc(sizeof(char) * 172);
    strcpy(message, "You cannot use the newCompilationForDevices method because the android version that your android device has installed ");
    strcat(message, "is lower than the Q version better known as android 10");
    showMessageError(env, message);
    return NULL;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Compilation_finish(JNIEnv *env, jobject compilation) {
    jclass Compilation = env->GetObjectClass(compilation);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 94);
        strcpy(message, "You cannot use the finish method of the Compilation object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    jboolean _isCreated = env->GetBooleanField(compilation, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 174);
        strcpy(message, "You cannot use the finish method of the Compilation object you are using because it was not created with the ");
        strcat(message, "newCompilation method or with the newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    void **Result = finishCompilation(_currentCompilation);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Compilation_delete(JNIEnv *env, jobject compilation) {
    jclass Compilation = env->GetObjectClass(compilation);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreated = env->GetBooleanField(compilation, isCreated);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 91);
        strcpy(message, "You cannot use the delete method of the Compilation object you are using because is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) *174);
        strcpy(message, "You cannot use the delete method of the Compilation object you are using because it was not created with the ");
        strcat(message, "newCompilation method or with the newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    deleteCompilation(_currentCompilation);
    env->SetIntField(compilation, currentCompilation, -1);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Compilation_setCaching(JNIEnv *env, jobject compilation, jobject cacheDir, jbyteArray token) {
    if (cacheDir == NULL || token == NULL) {
        char *message = (char*)malloc(sizeof(char) * 161);
        strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because the File cacheDir object and/or ");
        strcat(message, "the token array you provided are set to null");
        showMessageError(env, message);
        return;
    }
    jsize sizeToken = env->GetArrayLength(token);
    if (sizeToken != 32) {
        char *message = (char*)malloc(sizeof(char) * 157);
        strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because the token array that provided this ");
        strcat(message, "is empty or it's size is not 32 bytes");
        showMessageError(env, message);
        return;
    }
    jclass Compilation = env->GetObjectClass(compilation);
    jclass File = env->GetObjectClass(cacheDir);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreated = env->GetBooleanField(compilation, isCreated);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 99);
        strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 179);
        strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because it was not created with the ");
        strcat(message, "newCompilation method or with the newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    jmethodID getAbsolutePath = env->GetMethodID(File, "getAbsolutePath", "()Ljava/lang/String;");
    jmethodID exists = env->GetMethodID(File, "exists", "()Z");
    jmethodID isDirectory = env->GetMethodID(File, "isDirectory", "()Z");
    jboolean _exists = env->CallBooleanMethod(cacheDir, exists);
    jboolean _isDirectory = env->CallBooleanMethod(cacheDir, isDirectory);
    if (!_exists) {
        char *message = (char*)malloc(sizeof(char) * 161);
        strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because the address you provided in the ");
        strcat(message, "cacheDir object of type File does not exist");
        showMessageError(env, message);
        return;
    }
    if (!_isDirectory) {
        char *message = (char*)malloc(sizeof(char) * 165);
        strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because the address you provided in the ");
        strcat(message, "cacheDir object of type File is not a directory");
        showMessageError(env, message);
        return;
    }
    jstring _cacheDir = (jstring)env->CallObjectMethod(cacheDir, getAbsolutePath);
    const char *__cacheDir = env->GetStringUTFChars(_cacheDir, NULL);
    jbyte *dataToken = env->GetByteArrayElements(token, NULL);
#if __ANDROID_API__ >= 29
    void **Result = setCaching(_currentCompilation, __cacheDir, dataToken);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 202);
    strcpy(message, "You cannot use the setCaching method of the Compilation object you are using because the version of android that is ");
    strcat(message, "installed on your android device is less than the Q version better known as android 10");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Compilation_setPreference(JNIEnv *env, jobject compilation, jint preference) {
    jclass Compilation = env->GetObjectClass(compilation);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreated = env->GetBooleanField(compilation, isCreated);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 101);
        strcpy(message, "You cannot use the setPreference method of the Compilation object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 165);
        strcpy(message, "You cannot use the setPreference method of the Compilation object you are using because it was not created with the ");
        strcat(message, "newCompilation or newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    jfieldID PREFER_LOW_POWER = env->GetStaticFieldID(Compilation, "PREFER_LOW_POWER", "I");
    jfieldID PREFER_FAST_SINGLE_ANSWER = env->GetStaticFieldID(Compilation, "PREFER_FAST_SINGLE_ANSWER", "I");
    jfieldID PREFER_SUSTAINED_SPEED = env->GetStaticFieldID(Compilation, "PREFER_SUSTAINED_SPEED", "I");
    jint _PREFER_LOW_POWER = env->GetStaticIntField(Compilation, PREFER_LOW_POWER);
    jint _PREFER_FAST_SINGLE_ANSWER = env->GetStaticIntField(Compilation, PREFER_FAST_SINGLE_ANSWER);
    jint _PREFER_SUSTAINED_SPEED = env->GetStaticIntField(Compilation, PREFER_SUSTAINED_SPEED);
    jint _preference = preference;
    jint counter = 0;
    if ((_preference & _PREFER_LOW_POWER) == _PREFER_LOW_POWER) { _preference -= _PREFER_LOW_POWER; counter++; }
    if ((_preference & _PREFER_FAST_SINGLE_ANSWER) == _PREFER_FAST_SINGLE_ANSWER) { _preference -= _PREFER_FAST_SINGLE_ANSWER; counter++; }
    if ((_preference & _PREFER_SUSTAINED_SPEED) == _PREFER_SUSTAINED_SPEED) { _preference -= _PREFER_SUSTAINED_SPEED; counter++; }
    if (_preference != 0 || counter > 1) {
        char *message = (char*)malloc(sizeof(char) * 281);
        strcpy(message, "You cannot use the setPreference method of the Compilation object you are using because the value of the preference ");
        strcat(message, "variable you provided can only take one of the following values:\nCompilation.PREFER_LOW_POWE\n");
        strcat(message, "Compilation.PREFER_FAST_SINGLE_ANSWER\nCompilation.PREFER_SUSTAINED_SPEED");
        showMessageError(env, message);
        return;
    }
    void **Result = setPreferenceCompilation(_currentCompilation, preference);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Compilation_setPriority(JNIEnv *env, jobject compilation, jint priority) {
    jclass Compilation = env->GetObjectClass(compilation);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreate = env->GetBooleanField(compilation, isCreated);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 99);
        strcpy(message, "You cannot use the serPriority method of the Compilation object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreate) {
        char *message = (char*)malloc(sizeof(char) * 163);
        strcpy(message, "You cannot use the serPriority method of the Compilation object you are using because it was not created with the ");
        strcat(message, "newCompilation or newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    jfieldID PRIORITY_LOW = env->GetStaticFieldID(Compilation, "PRIORITY_LOW", "I");
    jfieldID PRIORITY_MEDIUM = env->GetStaticFieldID(Compilation, "PRIORITY_MEDIUM", "I");
    jfieldID PRIORITY_HIGH = env->GetStaticFieldID(Compilation, "PRIORITY_HIGH", "I");
    jfieldID PRIORITY_DEFAULT = env->GetStaticFieldID(Compilation, "PRIORITY_DEFAULT", "I");
    jint _PRIORITY_LOW = env->GetStaticIntField(Compilation, PRIORITY_LOW);
    jint _PRIORITY_MEDIUM = env->GetStaticIntField(Compilation, PRIORITY_MEDIUM);
    jint _PRIORITY_HIGH = env->GetStaticIntField(Compilation, PRIORITY_HIGH);
    jint _PRIORITY_DEFAULT = env->GetStaticIntField(Compilation, PRIORITY_DEFAULT);
    jint _priority = priority;
    jint count = 0;
    if ((_priority & _PRIORITY_LOW) == _PRIORITY_LOW)  { _priority -= _PRIORITY_LOW; count++; }
    if ((_priority & _PRIORITY_MEDIUM) == _PRIORITY_MEDIUM) { _priority -= _PRIORITY_MEDIUM; count++; }
    if ((_priority & _PRIORITY_HIGH) == _PRIORITY_HIGH) { _priority -= _PRIORITY_HIGH; count++; }
    if ((_priority & _PRIORITY_DEFAULT) == _PRIORITY_DEFAULT)  { _priority -= _PRIORITY_DEFAULT; count++; }
    if (_priority != 0 || count > 1) {
        char *message = (char*)malloc(sizeof(char) * 286);
        strcpy(message, "You cannot use the serPriority method of the Compilation object you are using because the value of the priority ");
        strcat(message, "variable you provided can only be one of the following values:\nCompilation.PRIORITY_LOW\nCompilation.PRIORITY_MEDIUM\n");
        strcat(message, "Compilation.PRIORITY_HIGH\nCompilation.PRIORITY_DEFAULT");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 30
    void **Result = setPriority(_currentCompilation, priority);
            if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 199);
    strcpy(message, "You cannot use the serPriority method of the Compilation object you are using because the android version that your ");
    strcat(message, "android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Compilation_setTimeout(JNIEnv *env, jobject compilation, jlong duration) {
    jclass Compilation = env->GetObjectClass(compilation);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreated = env->GetBooleanField(compilation, isCreated);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 98);
        strcpy(message, "You cannot use the setTimeout method of the Compilation object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 162);
        strcpy(message, "You cannot use the setTimeout method of the Compilation object you are using because it was not created with the ");
        strcat(message, "newCompilation or newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    if (duration < 0) {
        char *message = (char*)malloc(sizeof(char) * 147);
        strcpy(message, "You cannot use the setTimeout method of the Compilation object you are using because the value of the duration variable ");
        strcat(message, "you provided is less than 0");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 30
    void **Result = setTimeout(_currentCompilation, duration);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Rsult[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 201);
    strcpy(message, "You cannot use the setTimeout method of the Compilation object you are using because the version of android that your ");
    strcat(message, "android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Execution_newExecution(JNIEnv *env, jclass Execution, jobject compilation) {
    if (compilation == NULL) {
        char *message = (char*)malloc(sizeof(char) * 97);
        strcpy(message, "You cannot use the newExecution method because the Compilation object you provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass Compilation = env->GetObjectClass(compilation);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreated = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreated = env->GetBooleanField(compilation, isCreated);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 93);
        strcpy(message, "You cannot use the newExecution method because the Compilation object you provided is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 157);
        strcpy(message, "You cannot use the newExecution method because the Compilation object you provided was not created with the ");
        strcat(message, "newCompilation or newCompilationForDevices method");
        showMessageError(env, message);
        return NULL;
    }
    void **Result = createExecution(_currentCompilation);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jobject execution = env->AllocObject(Execution);
        jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
        jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
        env->SetIntField(execution, currentExecution, currentPositionExecution);
        env->SetBooleanField(execution, isCreated, JNI_TRUE);
        currentPositionExecution++;
        if (currentPositionExecution == 100) currentPositionExecution = 0;
        return execution;
    } else {
        showMessageError(env, (char*)Result[0]);
        return NULL;
    }
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_burstCompute(JNIEnv *env, jobject execution, jobject burst) {
    if (burst == NULL) {
        char *message = (char*)malloc(sizeof(char) * 129);
        strcpy(message, "You cannot use the burstCompute method of the Execution object you are using because the Burst object you provided is ");
        strcat(message, "set to null");
        showMessageError(env, message);
        return;
    }
    jclass Burst = env->GetObjectClass(burst);
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentBurst = env->GetFieldID(Burst, "currentBurst", "I");
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreatedBurst = env->GetFieldID(Burst, "isCreated", "Z");
    jfieldID isCreatedExecution = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentBurst = env->GetIntField(burst, currentBurst);
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreatedBurst = env->GetBooleanField(burst, isCreatedBurst);
    jboolean _isCreatedExecution = env->GetBooleanField(execution, isCreatedExecution);
    if (_currentBurst == -1 || _currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the burstCompute method of the Execution object you are using because the Burst object you provided is ");
        strcat(message, "invalid or the Execution object you are using is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedBurst || !_isCreatedExecution) {
        char *message = (char*)malloc(sizeof(char) * 241);
        strcpy(message, "You cannot use the burstCompute method of the Execution object you are using because the Burst object you provided ");
        strcat(message, "was not created with the newBurt method and/or the Execution object you are using was not created with the ");
        strcat(message, "newExecution method");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 29
    void **Result = burstCompute(_currentExecution, _currentBurst);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 201);
    strcpy(message, "You cannot use the burstCompute method of the Execution object you are using because the version of android that your ");
    strcat(message, "android device has installed is lower than the version Q better known as android 10");
    showMessageError(env, message);
    return;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_compute(JNIEnv *env, jobject execution) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 93);
        strcpy(message, "You cannot use the compute method of the execution object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 127);
        strcpy(message, "You cannot use the compute method of the execution object you are using because it was not created with the ");
        strcat(message, "newExecution method");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 29
    void **Result = compute(_currentExecution);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 100);
    strcpy(message, "You cannot use the compute method of the Execution object you are using because the android version that your ");
    strcat(message, "android device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_delete(JNIEnv *env, jobject execution) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 99);
        strcpy(message, "You cannot use the delete method to delete the Execution object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 133);
        strcpy(message, "You cannot use the delete method to delete the Execution object you are using because it was not created with the ");
        strcat(message, "newExecution method");
        showMessageError(env, message);
        return;
    }
    ANeuralNetworksExecution_free(executionList[_currentExecution]);
    env->SetIntField(execution, currentExecution, -1);
}
JNICALL jlong Java_com_draico_asvappra_neuralnetworks_Execution_getDuration(JNIEnv *env, jobject execution, jint durationType) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 97);
        strcpy(message, "You cannot use the getDuration method of the Execution object you are using because it is invalid");
        showMessageError(env, message);
        return 0;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 131);
        strcpy(message, "You cannot use the getDuration method of the Execution object you are using because it was not created with the ");
        strcat(message, "newExecution method");
        showMessageError(env, message);
        return 0;
    }
    jfieldID DURATION_ON_HARDWARE = env->GetStaticFieldID(Execution, "DURATION_ON_HARDWARE", "I");
    jfieldID DURATION_IN_DRIVER = env->GetStaticFieldID(Execution, "DURATION_IN_DRIVER", "I");
    jfieldID FENCED_DURATION_ON_HARDWARE = env->GetStaticFieldID(Execution, "FENCED_DURATION_ON_HARDWARE", "I");
    jfieldID FENCED_DURATION_IN_DRIVER = env->GetStaticFieldID(Execution, "FENCED_DURATION_IN_DRIVER", "I");
    jint _DURATION_ON_HARDWARE = env->GetStaticIntField(Execution, DURATION_ON_HARDWARE);
    jint _DURATION_IN_DRIVER = env->GetStaticIntField(Execution, DURATION_IN_DRIVER);
    jint _FENCED_DURATION_ON_HARDWARE = env->GetStaticIntField(Execution, FENCED_DURATION_ON_HARDWARE);
    jint _FENCED_DURATION_IN_DRIVER = env->GetStaticIntField(Execution, FENCED_DURATION_IN_DRIVER);
    jint _durationType = durationType;
    jint count = 0;
    if ((_durationType & _DURATION_ON_HARDWARE) == _DURATION_ON_HARDWARE) { _durationType -= _DURATION_ON_HARDWARE; count++; }
    if ((_durationType & _DURATION_IN_DRIVER) == _DURATION_IN_DRIVER) { _durationType -= _DURATION_IN_DRIVER; count++; }
    if ((_durationType & _FENCED_DURATION_ON_HARDWARE) == _FENCED_DURATION_ON_HARDWARE) { _durationType -= _FENCED_DURATION_ON_HARDWARE; count++; }
    if ((_durationType & _FENCED_DURATION_IN_DRIVER) == _FENCED_DURATION_IN_DRIVER) { _durationType -= _FENCED_DURATION_IN_DRIVER; count++; }
    if (_durationType != 0 || count > 1) {
        char *message = (char*)malloc(sizeof(char) * 292);
        strcpy(message, "You cannot use the getDuration method of the Execution object you are using because the value of the durationType ");
        strcat(message, "variable can only have any of the following values:\nExecution.DURATION_ON_HARDWARE\nExecution.DURATION_IN_DRIVER\n");
        strcat(message, "Execution.FENCED_DURATION_ON_HARDWARE\nExecution.DURATION_IN_DRIVER");
        showMessageError(env, message);
        return 0;
    }
    jlong duration = 0;
#if __ANDROID_API__ >= 29
    void **Result = getDuration(_currentExecution, durationType, &duration);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 100);
    strcpy(message, "You cannot use the getDuration method of the Execution object you are using because the android version that your ");
    strcat(message, "android device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
    return duration;
}
JNICALL jintArray Java_com_draico_asvappra_neuralnetworks_Execution_getOutputOperandDimensions(JNIEnv *env, jobject execution, jint indexOutput) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 112);
        strcpy(message, "You cannot use the getOutputOperandDimensions method of the Execution object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 146);
        strcpy(message, "You cannot use the getOutputOperandDimensions method of the Execution object you are using because it was not created ");
        strcat(message, "with the newExecution method");
        showMessageError(env, message);
        return NULL;
    }
    if (indexOutput < 0) {
        char *message = (char*)malloc(sizeof(char) * 151);
        strcpy(message, "You cannot use the getOutputOperandDimensions method of the Execution object you are using because the value of the ");
        strcat(message, "indexOutput variable is less than 0");
        showMessageError(env, message);
        return NULL;
    }
    jintArray data = NULL;
#if __ANDROID_API__ >= 29
    jmethodID getOutputOperandRank = env->GetMethodID(Execution, "getOutputOperandRank", "()I");
    jint sizeOutput = env->CallIntMethod(execution, getOutputOperandRank, indexOutput);
    if (sizeOutput == 0) return data;
    jint *dataArray = (jint*)malloc(sizeof(jint) * sizeOutput);
    void **Result = getOutputOperandDimensions(_currentExecution, indexOutput, dataArray);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jintArray data = env->NewIntArray(sizeOutput);
        env->SetIntArrayRegion(data, 0, sizeOutput, dataArray);
    } else showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 212);
    strcpy(message, "You cannot use the getOutputOperandDimensions method of the Execution object you are using because the android version ");
    strcat(message, "that your android device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
    return data;
}
JNICALL jint Java_com_draico_asvappra_neuralnetworks_Execution_getOutputOperandRank(JNIEnv *env, jobject execution, jint indexOutput) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 106);
        strcpy(message, "You cannot use the getOutputOperandRank method of the Execution object you are using because it is invalid");
        showMessageError(env, message);
        return 0;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 140);
        strcpy(message, "You cannot use the getOutputOperandRank method of the Execution object you are using because it was not created ");
        strcat(message, "with the newExecution method");
        showMessageError(env, message);
        return 0;
    }
    if (indexOutput < 0) {
        char *message = (char*)malloc(sizeof(char) * 137);
        strcpy(message, "You cannot use the getOutputOperandRank method of the Execution object you are using because the value of the ");
        strcat(message, "indexOutput variable is less than 0");
        showMessageError(env, message);
        return 0;
    }
    jint sizeRank = 0;
#if __ANDROID_API__ >= 29
    void **Result = getOutputOperandRank(_currentExecution, indexOutput, &sizeRank);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 205);
    strcpy(message, "You cannot use the getOutputOperandRank method of the Execution object you are using because the version of android ");
    strcat(message, "that your android device has installed is lower than android Q better known as android 10");
    showMessageError(env, message);
#endif
    return sizeRank;
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_setInput(JNIEnv *env, jobject execution, jobject operandType, jobject data, jint indexInput) {
    if (operandType == NULL || data == NULL) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the setInput method of the Execution object you are using because the operandType object and/or the data ");
        strcat(message, "object you provided are set to null");
        showMessageError(env, message);
        return;
    }
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 94);
        strcpy(message, "You cannot use the setInput method of the Execution object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 128);
        strcpy(message, "You cannot use the setInput method of the Execution object you are using because it was not created with the ");
        strcat(message, "newExecution method");
        showMessageError(env, message);
        return;
    }
    jclass OperandType = env->GetObjectClass(operandType);
    jfieldID currentOperandType = env->GetFieldID(OperandType, "currentOperandType", "I");
    jfieldID isCreatedOperandType = env->GetFieldID(OperandType, "isCreated", "Z");
    jint _currentOperandType = env->GetIntField(operandType, currentOperandType);
    jboolean _isCreatedOperandType = env->GetBooleanField(operandType, isCreatedOperandType);
    if (_currentOperandType == -1) {
        char *message = (char*)malloc(sizeof(char) * 127);
        strcpy(message, "You cannot use the setInput method of the Execution object you are using because the operandType object you provided ");
        strcat(message, "is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedOperandType) {
        char *message = (char*)malloc(sizeof(char) * 163);
        strcpy(message, "You cannot use the setInput method of the Execution object you are using because the operandType object you provided ");
        strcat(message, "was not created with the newOperandType method");
        showMessageError(env, message);
        return;
    }
    if (indexInput < 0) {
        char *message = (char*)malloc(sizeof(char) * 132);
        strcpy(message, "You cannot use the setInput method of the Execution object you are using because the value of the indexInput variable is ");
        strcat(message, "less than 0");
        showMessageError(env, message);
        return;
    }
    jclass Object = env->FindClass("java/lang/Object");
    jclass Class = env->FindClass("java/lang/Class");
    jclass Integer = env->FindClass("java/lang/Integer");
    jclass Short = env->FindClass("java/lang/Short");
    jclass Long = env->FindClass("java/lang/Long");
    jclass Float = env->FindClass("java/lang/Float");
    jclass Character = env->FindClass("java/lang/Character");
    jclass Byte = env->FindClass("java/lang/Byte");
    jmethodID getClass = env->GetMethodID(Object, "getClass", "()Ljava/lang/Class;");
    jmethodID getName = env->GetMethodID(Class, "getName", "()Ljava/lang/String;");
    jmethodID isArray = env->GetMethodID(Class, "isArray", "()Z");
    jmethodID intValue = env->GetMethodID(Integer, "intValue", "()I");
    jmethodID shortValue = env->GetMethodID(Short, "shortValue", "()S");
    jmethodID longValue = env->GetMethodID(Long, "longValue", "()J");
    jmethodID floatValue = env->GetMethodID(Float, "floatValue", "()F");
    jmethodID byteValue = env->GetMethodID(Byte, "byteValue", "()B");
    jmethodID charValue = env->GetMethodID(Character, "charValue", "()C");
    jobject objectClass = env->CallObjectMethod(data, getClass);
    jstring nameClass = (jstring)env->CallObjectMethod(objectClass, getName);
    jboolean _isArray = env->CallBooleanMethod(objectClass, isArray);
    const char *_nameClass = env->GetStringUTFChars(nameClass, NULL);
    void **Result;
    if (!_isArray) {
        if (strcmp(_nameClass, "I") == 0 || strcmp(_nameClass, "Ljava.lang.Integer;") == 0) {
            jint number = env->CallIntMethod((jobject)data, intValue);
            Result = setInput(_currentExecution, _currentOperandType, &number, indexInput, sizeof(jint));
        } else if (strcmp(_nameClass, "S") == 0 || strcmp(_nameClass, "Ljava.lang.Short;") == 0) {
            jshort number = env->CallShortMethod((jobject)data, shortValue);
            Result = setInput(_currentExecution, _currentOperandType, &number, indexInput, sizeof(jshort));
        } else if (strcmp(_nameClass, "J") == 0 || strcmp(_nameClass, "Ljava.lang.Long;") == 0) {
            jlong number = env->CallLongMethod((jobject)data, longValue);
            Result = setInput(_currentExecution, _currentOperandType, &number, indexInput, sizeof(jlong));
        } else if (strcmp(_nameClass, "F") == 0 || strcmp(_nameClass, "Ljava.lang.Float;") == 0) {
            jfloat number = env->CallFloatMethod((jobject)data, floatValue);
            Result = setInput(_currentExecution, _currentOperandType, &number, indexInput, sizeof(jfloat));
        } else if (strcmp(_nameClass, "B") == 0 || strcmp(_nameClass, "Ljava.lang.Byte;") == 0) {
            jbyte number = env->CallByteMethod((jobject)data, byteValue);
            Result = setInput(_currentExecution, _currentOperandType, &number, indexInput, sizeof(jbyte));
        } else if (strcmp(_nameClass, "C") == 0 || strcmp(_nameClass, "Ljava.lang.Character;") == 0) {
            char number = (char)env->CallCharMethod((jobject)data, charValue);
            Result = setInput(_currentExecution, _currentOperandType, &number, indexInput, sizeof(char));
        } else if (strcmp(_nameClass, "Ljava.lang.String;") == 0) {
            char *_data = (char*)env->GetStringUTFChars((jstring)data, NULL);
            jsize sizeData = env->GetStringLength((jstring)data);
            Result = setInput(_currentExecution, _currentOperandType, _data, indexInput, sizeof(char) * sizeData);
        } else {
            char *message = (char*)malloc(sizeof(char) * 288);
            strcpy(message, "You cannot use the setInput method of the Execution object you are using because in the value of the data object ");
            strcat(message, "you provided it can only contain data of the primitive type int, short, long, float, byte, char or of the object ");
            strcat(message, "types Integer, Short, Long , Float, Byte, Character and String");
            showMessageError(env, message);
            return;
        }
    } else {
        jobjectArray _data = (jobjectArray)data;
        jsize sizeData = env->GetArrayLength(_data);
        if (strcmp(_nameClass, "[I") == 0) {
            jint *numbers = env->GetIntArrayElements((jintArray)_data, NULL);
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jint) * sizeData);
        } else if (strcmp(_nameClass, "[S") == 0) {
            jshort *numbers = env->GetShortArrayElements((jshortArray)_data, NULL);
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jshort) * sizeData);
        } else if (strcmp(_nameClass, "[J") == 0) {
            jlong *numbers = env->GetLongArrayElements((jlongArray)_data, NULL);
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jlong) * sizeData);
        } else if (strcmp(_nameClass, "[F") == 0) {
            jfloat *numbers = env->GetFloatArrayElements((jfloatArray)_data, NULL);
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jfloat) * sizeData);
        } else if (strcmp(_nameClass, "[B") == 0) {
            jbyte *numbers = env->GetByteArrayElements((jbyteArray)_data, NULL);
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jbyte) * sizeData);
        } else if (strcmp(_nameClass, "[C") == 0) {
            char *numbers = (char*)env->GetCharArrayElements((jcharArray)_data, NULL);
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(char) * sizeData);
        } else if (strcmp(_nameClass, "[Ljava.lang.Integer;") == 0) {
            jint *numbers = (jint*)malloc(sizeof(jint) * sizeData);
            for (jint position = 0; position < sizeData; position++) {
                jobject number = env->GetObjectArrayElement(_data, position);
                numbers[position] = env->CallIntMethod(number, intValue);
            }
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jint) * sizeData);
        } else if (strcmp(_nameClass, "[Ljava.lang.Short;") == 0) {
            jshort *numbers = (jshort*)malloc(sizeof(jshort) * sizeData);
            for (jint position = 0; position < sizeData; position++) {
                jobject number = env->GetObjectArrayElement(_data, position);
                numbers[position] = env->CallShortMethod(number, shortValue);
            }
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jshort) * sizeData);
        } else if (strcmp(_nameClass, "[Ljava.lang.Long;") == 0) {
            jlong *numbers = (jlong*)malloc(sizeof(jlong) *sizeData);
            for (jint position = 0; position < sizeData; position++) {
                jobject number = env->GetObjectArrayElement(_data, position);
                numbers[position] = env->CallLongMethod(number, longValue);
            }
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jlong) * sizeData);
        } else if (strcmp(_nameClass, "[Ljava.lang.Float;") == 0) {
            jfloat *numbers = (jfloat*)malloc(sizeof(jfloat) * sizeData);
            for (jint position = 0; position < sizeData; position++) {
                jobject number = env->GetObjectArrayElement(_data, position);
                numbers[position] = env->CallFloatMethod(number, floatValue);
            }
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jfloat) * sizeData);
        } else if (strcmp(_nameClass, "[Ljava.lang.Byte;") == 0) {
            jbyte *numbers = (jbyte*)malloc(sizeof(jbyte) * sizeData);
            for (jint position = 0; position < sizeData; position++) {
                jobject number = env->GetObjectArrayElement(_data, position);
                numbers[position] = env->CallByteMethod(number, byteValue);
            }
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(jbyte) * sizeData);
        } else if (strcmp(_nameClass, "[Ljava.lang.Character;") == 0) {
            char *numbers = (char*)malloc(sizeof(char) * sizeData);
            for (jint position = 0; position < sizeData; position++) {
                jobject number = env->GetObjectArrayElement(_data, position);
                numbers[position] = (char)env->CallCharMethod(number, charValue);
            }
            Result = setInput(_currentExecution, _currentOperandType, numbers, indexInput, sizeof(char) * sizeData);
        } else {
            char *message = (char*)malloc(sizeof(char) * 244);
            strcpy(message, "You cannot use the setInput method of the Execution object you are using because the data object you provided only ");
            strcat(message, "supports arrays of the following data types:\\nint, short, long, float, byte, char, Integer, Short, Long, Float, ");
            strcat(message, "Byte and Character");
            showMessageError(env, message);
            return;
        }
    }
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_setInputFromMemory(JNIEnv *env, jobject execution, jobject operandType, jobject memory,
                                          jint indexInput, jint offset, jint size) {
    if (operandType == NULL || memory == NULL) {
        char *message = (char*)malloc(sizeof(char) * 167);
        strcpy(message, "You cannot use the setInputFromMemory method of the Execution object you are using because the OperandType object and/or");
        strcat(message, "the Memory object you provided are set to null");
        showMessageError(env, message);
        return;
    }
    jclass Execution = env->GetObjectClass(execution);
    jclass OperandType = env->GetObjectClass(operandType);
    jclass Memory = env->GetObjectClass(memory);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreatedExecution = env->GetFieldID(Execution, "isCreated", "Z");
    jfieldID currentOperandType = env->GetFieldID(OperandType, "currentOperandType", "I");
    jfieldID isCreatedOperandType = env->GetFieldID(OperandType, "isCreated", "Z");
    jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
    jfieldID isCreatedMemory = env->GetFieldID(Memory, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jint _currentOperandType = env->GetIntField(operandType, currentOperandType);
    jint _currentMemory = env->GetIntField(memory, currentMemory);
    jboolean _isCreatedExecution = env->GetBooleanField(execution, isCreatedExecution);
    jboolean _isCreatedOperandType = env->GetBooleanField(operandType, isCreatedOperandType);
    jboolean _isCreatedMemory = env->GetBooleanField(memory, isCreatedMemory);
    if (_currentExecution == -1 || _currentOperandType == -1 || _currentMemory == -1) {
        char *message = (char*)malloc(sizeof(char) * 183);
        strcpy(message, "You cannot use the setInputFromMemory method of the Execution object you are using because it is invalid and/or the ");
        strcat(message, "OperandType objectand/or the Memory object you provided are invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedExecution || !_isCreatedOperandType || !_isCreatedMemory) {
        char *message = (char*)malloc(sizeof(char) * 331);
        strcpy(message, "You cannot use the setInputFromMemory method of the Execution object you are using because the OperandType object was ");
        strcat(message, "not created with the newOperandType method and/or the Execution object was not created with the newExecution method ");
        strcat(message, "and/or the Memory object was not created with the newMemory or newMemoryFromFileDescriptor method");
        showMessageError(env, message);
        return;
    }
    if (indexInput < 0 || offset < 0 || size <= 0) {
        char *message = (char*)malloc(sizeof(char) * 262);
        strcpy(message, "You cannot use the setInputFromMemory method of the Execution object you are using because the value of the indexInput ");
        strcat(message, "variable is less than 0 and/or the value of the offset variable is less than 0 and/or the value of the size variable is ");
        strcat(message, "less than or equal to 0");
        showMessageError(env, message);
        return;
    }
    void **Result = setInputFromMemory(_currentExecution, _currentOperandType, _currentMemory, indexInput, offset, size);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_setLoopTimeout(JNIEnv *env, jobject execution, jlong duration) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jint _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 94);
        strcpy(message, "You cannot use the setLoopTimeout method because the Execution object you are using is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 128);
        strcpy(message, "You cannot use the setLoopTimeout method because the Execution object you are using was not created with the ");
        strcat(message, "newExecution method");
        showMessageError(env, message);
        return;
    }
    if (duration < 0) {
        char *message = (char*)malloc(sizeof(char) * 136);
        strcpy(message, "You cannot use the setLoopTimeout method of the Execution object you are using because the value of the duration ");
        strcat(message, "variable is less than 0");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 30
    void **Result = setLoopTimeout(_currentExecution, duration);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 203);
    strcpy(message, "You cannot use the setLoopTimeout method of the Execution object you are using because the version of android that ");
    strcat(message, "your android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_setMeasureTiming(JNIEnv *env,jobject execution, jboolean isMeasure) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 102);
        strcpy(message, "You cannot use the setMeasureTiming method of the Execution object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 136);
        strcpy(message, "You cannot use the setMeasureTiming method of the Execution object you are using because it was not created with ");
        strcat(message, "the newExecution method");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 29
    void **Result = setMeasureTiming(_currentExecution, isMeasure);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 205);
    strcpy(message, "You cannot use the setMeasureTiming method of the Execution object you are using because the version of android that ");
    strcat(message, "your android device has installed is lower than the version Q better known as android 10");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_setOutput(JNIEnv *env, jobject execution, jobject operandType, jobject data, jint indexOutput) {
    if (operandType == NULL || data == NULL) {
        char *message = (char*)malloc(sizeof(char) * 156);
        strcpy(message, "You cannot use the setOutput method of the Execution object you are using because the OperandType object and/or the ");
        strcat(message, "data object you provided are set to null");
        showMessageError(env, message);
        return;
    }
    jclass Execution = env->GetObjectClass(execution);
    jclass OperandType = env->GetObjectClass(operandType);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreatedExecution = env->GetFieldID(Execution, "isCreated", "Z");
    jfieldID currentOperandType = env->GetFieldID(OperandType, "currentOperandType", "I");
    jfieldID isCreatedOperandType = env->GetFieldID(OperandType, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jint _currentOperandType = env->GetIntField(operandType, currentOperandType);
    jboolean _isCreatedExecution = env->GetBooleanField(execution, isCreatedExecution);
    jboolean _isCreatedOperandType = env->GetBooleanField(operandType, isCreatedOperandType);
    if (_currentExecution == -1 || _currentOperandType == -1) {
        char *message = (char*)malloc(sizeof(char) * 135);
        strcpy(message, "You cannot use the setOutput method of the Execution object you are using because this object and/or the OperandType ");
        strcat(message, "object are invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedExecution || !_isCreatedOperandType) {
        char *message = (char*)malloc(sizeof(char) * 206);
        strcpy(message, "You cannot use the setOutput method of the Execution object you are using because it was not created with the ");
        strcat(message, "newExecution method and/or the OperandType object was not created with the newOperandType method");
        showMessageError(env, message);
        return;
    }
    if (indexOutput < 0) {
        char *message = (char*)malloc(sizeof(char) * 134);
        strcpy(message, "You cannot use the setOutput method of the Execution object you are using because the value of the indexOutput ");
        strcat(message, "variable is less than 0");
        showMessageError(env, message);
        return;
    }
    jclass Object = env->FindClass("java/lang/Object");
    jclass Class = env->FindClass("java/lang/Class");
    jmethodID getClass = env->GetMethodID(Object, "getClass", "()Ljava/lang/Class;");
    jmethodID getName = env->GetMethodID(Class, "getName", "()Ljava/lang/String;");
    jmethodID isArray = env->GetMethodID(Class, "isArray", "()Z");
    jobject classObject = env->CallObjectMethod(data, getClass);
    jstring nameClass = (jstring)env->CallObjectMethod(classObject, getName);
    jboolean _isArray = env->CallBooleanMethod(classObject, isArray);
    const char *_nameClass = env->GetStringUTFChars(nameClass, NULL);
    void **Result;
    if (_isArray) {
        jobjectArray _data = (jobjectArray)data;
        jsize sizeData = env->GetArrayLength(_data);
        if (sizeData == 0) {
            char *message = (char*)malloc(sizeof(char) * 155);
            strcpy(message, "You cannot use the setOutput method of the Execution object you are using because the data you provided in the ");
            strcat(message, "data object of type Object is an empty array");
            showMessageError(env, message);
            return;
        }
        if (strcmp(_nameClass, "[I") == 0) {
            jint *numbers = env->GetIntArrayElements((jintArray)_data, NULL);
            Result = setOutput(_currentExecution, _currentOperandType, indexOutput, numbers, sizeof(jint) * sizeData);
        } else if (strcmp(_nameClass, "[S") == 0) {
            jshort *numbers = env->GetShortArrayElements((jshortArray)_data, NULL);
            Result = setOutput(_currentExecution, _currentOperandType, indexOutput, numbers, sizeof(jshort) * sizeData);
        } else if (strcmp(_nameClass, "[J") == 0) {
            jlong *numbers = env->GetLongArrayElements((jlongArray)_data, NULL);
            Result = setOutput(_currentExecution, _currentOperandType, indexOutput, numbers, sizeof(jlong) * sizeData);
        } else if (strcmp(_nameClass, "[F") == 0) {
            jfloat *numbers = env->GetFloatArrayElements((jfloatArray)_data, NULL);
            Result = setOutput(_currentExecution, _currentOperandType, indexOutput, numbers, sizeof(jfloat) * sizeData);
        } else if (strcmp(_nameClass, "[B") == 0) {
            jbyte *numbers = env->GetByteArrayElements((jbyteArray)_data, NULL);
            Result = setOutput(_currentExecution, _currentOperandType, indexOutput, numbers, sizeof(jbyte) * sizeData);
        } else if (strcmp(_nameClass, "[C") == 0) {
            char *numbers = (char*)env->GetCharArrayElements((jcharArray)_data, NULL);
            Result = setOutput(_currentExecution, _currentOperandType, indexOutput, numbers, sizeof(jbyte) * sizeData);
        } else {
            char *message = (char*)malloc(sizeof(char) * 196);
            strcpy(message, "You cannot use the setOutput method of the Execution object you are using because the array you provided in the ");
            strcat(message, "data object of type Object can only be of type int, short, long, float, byte or char");
            showMessageError(env, message);
            return;
        }
    } else {
        char *message = (char*)malloc(sizeof(char) * 153);
        strcpy(message, "You cannot use the setOutput method of the Execution object you are using because the data you provided in the ");
        strcat(message, "data object of type Object is not an array");
        showMessageError(env, message);
        return;
    }
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_setOutputFromMemory(JNIEnv *env, jobject execution, jobject operandType, jobject memory,
                                           jint indexOutput, jint offset, jint size) {
    if (operandType == NULL || memory == NULL) {
        char *message = (char*)malloc(sizeof(char) * 168);
        strcpy(message, "You cannot use the setOutputFromMemory method of the Execution object you are using because the OperandType object ");
        strcat(message, "and/or the Memory object you provided are set to null");
        showMessageError(env, message);
        return;
    }
    jclass Execution = env->GetObjectClass(execution);
    jclass Memory = env->GetObjectClass(memory);
    jclass OperandType = env->GetObjectClass(operandType);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreatedExecution = env->GetFieldID(Execution, "isCreated", "Z");
    jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
    jfieldID isCreatedMemory = env->GetFieldID(Memory, "isCreated", "Z");
    jfieldID currentOperandType = env->GetFieldID(OperandType, "currentOperandType", "I");
    jfieldID isCreatedOperandType = env->GetFieldID(OperandType, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jint _currentMemory = env->GetIntField(memory, currentMemory);
    jint _currentOperandType = env->GetIntField(operandType, currentOperandType);
    jboolean _isCreatedExecution = env->GetBooleanField(execution, isCreatedExecution);
    jboolean _isCreatedMemory = env->GetBooleanField(memory, isCreatedMemory);
    jboolean _isCreatedOperandType = env->GetBooleanField(operandType, isCreatedOperandType);
    if (_currentExecution == -1 || _currentMemory == -1 || _currentOperandType == -1) {
        char *message = (char*)malloc(sizeof(char) * 164);
        strcpy(message, "You cannot use the setOutputFromMemory method of the Execution object you are using because the OperandType object ");
        strcat(message, "and/or the Memory object you provided are invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedExecution || !_isCreatedMemory || !_isCreatedOperandType) {
        char *message = (char*)malloc(sizeof(char) * 314);
        strcpy(message, "You cannot use the setOutputFromMemory method of the Execution object you are using because it was not created with ");
        strcat(message, "the newExecution method and/or the Memory object was not created with the newMemory or newMemoryFromFileDescriptor ");
        strcat(message, "method and/or the OperandType object was not created with the newOperandType method");
        showMessageError(env, message);
        return;
    }
    if (indexOutput < 0 || offset < 0 || size <= 0) {
        char *message = (char*)malloc(sizeof(char) * 264);
        strcpy(message, "You cannot use the setOutputFromMemory method of the Execution object you are using because the value of the indexOutput ");
        strcat(message, "variable is less than 0 and/or the value of the offset variable is less than 0 and/or the value of the size variable is less ");
        strcat(message, "than or equal to 0");
        showMessageError(env, message);
        return;
    }
    void **Result = setOutputFromMemory(_currentExecution, _currentOperandType, _currentMemory, indexOutput, offset, size);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_setTimeout(JNIEnv *env, jobject execution, jlong duration) {
    jclass Execution = env->GetObjectClass(execution);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 90);
        strcpy(message, "You cannot use the setTimeout method because the Execution object you are using is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 124);
        strcpy(message, "You cannot use the setTimeout method because the Execution object you are using was not created with the newExecution method");
        showMessageError(env, message);
        return;
    }
    if (duration < 0) {
        char *message = (char*)malloc(sizeof(char) * 132);
        strcpy(message, "You cannot use the setTimeout method of the Execution object you are using because the value of the duration variable ");
        strcat(message, "is less than 0");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 30
    void **Result = setTimeout(_currentExecution, duration);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 199);
    strcpy(message, "You cannot use the setTimeout method of the Execution object you are using because the version of android that your ");
    strcat(message, "android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Execution_startCompute(JNIEnv *env, jobject execution, jobject event) {
    if (event == NULL) {
        char *message = (char*)malloc(sizeof(char) * 129);
        strcpy(message, "You cannot use the startCompute method of the Execution object you are using because the Event object you provided is ");
        strcat(message, "set to null");
        showMessageError(env, message);
        return;
    }
    jclass Execution = env->GetObjectClass(execution);
    jclass Event = env->GetObjectClass(event);
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreated = env->GetFieldID(Execution, "isCreated", "Z");
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jfieldID isCreatedEvent = env->GetFieldID(Event, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jint _currentEvent = env->GetIntField(event, currentEvent);
    jboolean _isCreated = env->GetBooleanField(execution, isCreated);
    jboolean _isCreatedEvent = env->GetBooleanField(event, isCreatedEvent);
    if (_currentExecution == -1 || _currentEvent == -1) {
        char *message = (char*)malloc(sizeof(char) * 146);
        strcpy(message, "You cannot use the startCompute method of the Execution object you are using because it is invalid and/or the Event ");
        strcat(message, "object you provided is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated || !_isCreatedEvent) {
        char *message = (char*)malloc(sizeof(char) * 210);
        strcpy(message, "You cannot use the startCompute method of the Execution object you are using because it was not created with the ");
        strcat(message, "newExecution method and/or the Event object you provided was not created with the newEvent method");
        showMessageError(env, message);
        return;
    }
    void **Result = startCompute(_currentExecution, _currentEvent);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Execution_startComputeWithDependencies(JNIEnv *env, jobject execution, jobjectArray events,
                                                             jlong duration) {
    if (events == NULL) {
        char *message = (char*)malloc(sizeof(char) * 148);
        strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the array of events ");
        strcat(message, "you provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass Execution = env->GetObjectClass(execution);
    jclass Event = env->FindClass("com/draico/asvappra/neuralnetworks/Event");
    jfieldID currentExecution = env->GetFieldID(Execution, "currentExecution", "I");
    jfieldID isCreatedExecution = env->GetFieldID(Execution, "isCreated", "Z");
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jfieldID isCreatedEvent = env->GetFieldID(Event, "isCreated", "Z");
    jint _currentExecution = env->GetIntField(execution, currentExecution);
    jboolean _isCreatedExectuion = env->GetBooleanField(execution, isCreatedExecution);
    if (_currentExecution == -1) {
        char *message = (char*)malloc(sizeof(char) * 114);
        strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreatedExectuion) {
        char *message = (char*)malloc(sizeof(char) * 148);
        strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because it was not created ");
        strcat(message, "with the newExecution method");
        showMessageError(env, message);
        return NULL;
    }
    jsize sizeArrayEvents = env->GetArrayLength(events);
    if (sizeArrayEvents == 0) {
        char *message = (char*)malloc(sizeof(char) * 139);
        strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the array events ");
        strcat(message, "you provided is empty");
        showMessageError(env, message);
        return NULL;
    }
    if (sizeArrayEvents > 1) {
        for (jint position1 = 0; position1 < (sizeArrayEvents - 1); position1++) {
            jobject event1 = env->GetObjectArrayElement(events, position1);
            for (jint position2 = position1 + 1; position2 < sizeArrayEvents; position2++) {
                jobject event2 = env->GetObjectArrayElement(events, position2);
                jint currentEvent1 = env->GetIntField(event1, currentEvent);
                jint currentEvent2 = env->GetIntField(event2, currentEvent);
                jboolean isCreatedEvent1 = env->GetBooleanField(event1, isCreatedEvent);
                jboolean isCreatedEvent2 = env->GetBooleanField(event2, isCreatedEvent);
                if (currentEvent1 == -1 || currentEvent2 == -1) {
                    char *message = (char*)malloc(sizeof(char) * 180);
                    strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the events ");
                    strcat(message, "array you provided contains at least 1 Event object which is invalid");
                    showMessageError(env, message);
                    return NULL;
                }
                if (!isCreatedEvent1 || !isCreatedEvent2) {
                    char *message = (char*)malloc(sizeof(char) * 100);
                    strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the events ");
                    strcat(message, "array you provided contains at least 1 Event object that was not created with the newEvent method");
                    showMessageError(env, message);
                    return NULL;
                }
                if (env->IsSameObject(event1, event2)) {
                    char *message = (char*)malloc(sizeof(char) * 196);
                    strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the events ");
                    strcat(message, "array you provided contains at least 1 Event object that repeats in the events array");
                    showMessageError(env, message);
                    return NULL;
                }
            }
        }
    } else {
        jobject event = env->GetObjectArrayElement(events, 0);
        jint _currentEvent = env->GetIntField(event, currentEvent);
        jboolean _isCreatedEvent = env->GetBooleanField(event, isCreatedEvent);
        if (_currentEvent == -1) {
            char *message = (char*)malloc(sizeof(char) * 180);
            strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the events ");
            strcat(message, "array you provided contains at least 1 Event object which is invalid");
            showMessageError(env, message);
            return NULL;
        }
        if (!_isCreatedEvent) {
            char *message = (char*)malloc(sizeof(char) * 100);
            strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the events ");
            strcat(message, "array you provided contains at least 1 Event object that was not created with the newEvent method");
            showMessageError(env, message);
            return NULL;
        }
    }
    jint *eventsList = (jint*)malloc(sizeof(jint) * sizeArrayEvents);
    for (jint position = 0; position < sizeArrayEvents; position++) {
        jobject event = env->GetObjectArrayElement(events, position);
        eventsList[position] = env->GetIntField(event, currentEvent);
    }
    jobject event = NULL;
#if __ANDROID_API__ >= 30
    void **Result = startComputeWithDependencies(_currentExecution, eventsList, sizeArrayEvents, duration);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        event = env->AllocObject(Event);
        env->SetIntField(event, currentEvent, positionCurrentEvent);
        env->SetBooleanField(event, isCreatedEvent, JNI_TRUE);
        currentPositionEvent++;
        if (currentPositionEvent == 10000) currentPositionEvent = 0;
    } else showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 100);
    strcpy(message, "You cannot use the startComputeWithDependencies method of the Execution object you are using because the version of ");
    strcat(message, "android that your android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
    return event;
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Burst_newBurst(JNIEnv *env, jclass Burst, jobject compilation) {
    if (compilation == NULL) {
        char *message = (char*)malloc(sizeof(char) * 118);
        strcpy(message, "You cannot use the newBurst method to create a Burst object because the Compilation object you provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass Compilation = env->GetObjectClass(compilation);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreatedCompilation = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreatedCompilation = env->GetBooleanField(compilation, isCreatedCompilation);
    if (_currentCompilation == -1) {
        char *message = (char*)malloc(sizeof(char) * 114);
        strcpy(message, "You cannot use the newBurst method to create a Burst object because the Compilation object you provided is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreatedCompilation) {
        char *message = (char*)malloc(sizeof(char) * 150);
        strcpy(message, "You cannot use the newBurst method to create a Burst object because the Compilation object you provided was not created ");
        strcat(message, "with the newCompilation method");
        showMessageError(env, message);
        return NULL;
    }
    jobject burst = NULL;
#if __ANDROID_API__ >= 29
    void **Result = createBurst(_currentCompilation);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID currentBurst = env->GetFieldID(Burst, "currentBurst", "I");
        jfieldID isCreatedBurst = env->GetFieldID(Burst, "isCreated", "Z");
        burst = env->AllocObject(Burst);
        env->SetIntField(burst, currentBurst, currentPositionBurst);
        env->SetBooleanField(burst, isCreatedBurst, JNI_TRUE);
        currentPositionBurst++;
        if (currentPositionBurst == 100) currentPositionBurst = 0;
    } else showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 181);
    strcpy(message, "You cannot use the newBurst method to create a Burst object because the android version that your android device has ");
    strcat(message, "installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
    return burst;
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Burst_delete(JNIEnv *env, jobject burst) {
    jclass Burst = env->GetObjectClass(burst);
    jfieldID currentBurst = env->GetFieldID(Burst, "currentBurst", "I");
    jfieldID isCreated = env->GetFieldID(Burst, "isCreated", "Z");
    jint _currentBurst = env->GetIntField(burst, currentBurst);
    jboolean _isCreated = env->GetBooleanField(burst, isCreated);
    if (_currentBurst == -1) {
        char *message = (char*)malloc(sizeof(char) * 88);
        strcpy(message, "You cannot use the delete method of the Burst object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 118);
        strcpy(message, "You cannot use the delete method of the Burst object you are using because it was not created with the newBurst method");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 29
    deleteBurst(_currentBurst);
#else
    char *message = (char*)malloc(sizeof(char) * 188);
    strcpy(message, "You cannot use the delete method of the Burst object you are using because the android version that your android device ");
    strcat(message, "has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Memory_newMemory(JNIEnv *env, jclass Memory, jobject memoryDescriptor) {
    if (memoryDescriptor == NULL) {
        char *message = (char*)malloc(sizeof(char) * 125);
        strcpy(message, "You cannot use the newMemory method to create a Memory object because the MemoryDescriptor object you provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass MemoryDescriptor = env->GetObjectClass(memoryDescriptor);
    jfieldID currentMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "currentMemoryDescriptor", "I");
    jfieldID isCreatedMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "isCreated", "Z");
    jint _currentMemoryDescriptor = env->GetIntField(memoryDescriptor, currentMemoryDescriptor);
    jboolean _isCreatedMemoryDescriptor = env->GetBooleanField(memoryDescriptor, isCreatedMemoryDescriptor);
    if (_currentMemoryDescriptor == -1) {
        char *message = (char*)malloc(sizeof(char) * 121);
        strcpy(message, "You cannot use the newMemory method to create a Memory object because the MemoryDescriptor object you provided is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreatedMemoryDescriptor) {
        char *message = (char*)malloc(sizeof(char) * 162);
        strcpy(message, "You cannot use the newMemory method to create a Memory object because the MemoryDescriptor object you provided was not ");
        strcat(message, "created with the newMemoryDescriptor method");
        showMessageError(env, message);
        return NULL;
    }
    jobject memory = NULL;
#if __ANDROID_API__ >= 30
    void **Result = newMemory(_currentMemoryDescriptor);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
        jfieldID isCreatedMemory = env->GetFieldID(Memory, "isCreated", "Z");
        memory = env->AllocObject(Memory);
        env->SetIntField(memory, currentMemory, currentPositionMemory);
        env->SetBooleanField(memory, isCreatedMemory, JNI_TRUE);
        currentPositionMemory++;
        if (currentPositionMemory == 100) currentPositionMemory = 0;
    }
#else
    char *message = (char*)malloc(sizeof(char) * 183);
    strcpy(message, "You cannot use the newMemory method to create a Memory object because the android version that your android device has ");
    strcat(message, "installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
    return memory;
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Memory_newMemoryFronFileDescriptor(JNIEnv *env, jclass Memory, jint fileDescriptor, jint size, jint offset,
                                                          jint mode) {
    if (size <= 0 || offset < 0) {
        char *message = (char*)malloc(sizeof(char) * 200);
        strcpy(message, "You cannot use the newMemoryFromFileDescriptor method to create a Memory object because the value of the size variable is ");
        strcat(message, "less than or equal to 0 and/or the value of the offset variable is less than 0");
        showMessageError(env, message);
        return NULL;
    }
    jclass ParcelFileDescriptor = env->FindClass("android/os/ParcelFileDescriptor");
    jfieldID MODE_READ_ONLY = env->GetStaticFieldID(ParcelFileDescriptor, "MODE_READ_ONLY", "I");
    jfieldID MODE_WRITE_ONLY = env->GetStaticFieldID(ParcelFileDescriptor, "MODE_WRITE_ONLY", "I");
    jfieldID MODE_READ_WRITE = env->GetStaticFieldID(ParcelFileDescriptor, "MODE_READ_WRITE", "I");
    jmethodID getStatSize = env->GetMethodID(ParcelFileDescriptor, "getStatSize", "()J");
    jmethodID fromFileDescriptor = env->GetStaticMethodID(ParcelFileDescriptor, "fromFd", "(I)Landroid/os/ParcelFileDescriptor;");
    jint _MODE_READ_ONLY = env->GetStaticIntField(ParcelFileDescriptor, MODE_READ_ONLY);
    jint _MODE_WRITE_ONLY = env->GetStaticIntField(ParcelFileDescriptor, MODE_WRITE_ONLY);
    jint _MODE_READ_WRITE = env->GetStaticIntField(ParcelFileDescriptor, MODE_READ_WRITE);
    jint _mode = mode;
    jint count = 0;
    if ((_mode & _MODE_READ_ONLY) == _MODE_READ_ONLY) { _mode -= _MODE_READ_ONLY; count++; }
    if ((_mode & _MODE_WRITE_ONLY) == _MODE_WRITE_ONLY) { _mode -= _MODE_WRITE_ONLY; count++; }
    if ((_mode & _MODE_READ_WRITE) == _MODE_READ_WRITE) { _mode -= _MODE_READ_WRITE; count++; }
    if (_mode != 0 || count == 0 || count > 1) {
        char *message = (char*)malloc(sizeof(char) * 266);
        strcpy(message, "You cannot use the newMemoryFromFileDescriptor method to create a Memory object because the mode flag you provided can only ");
        strcat(message, "use one of the following 3 data:\nParcelFileDescriptor.MODE_READ_ONLY\nParcelFileDescriptor.MODE_WRITE_ONLY\n");
        strcat(message, "ParcelFileDescriptor.MODE_READ_WRITE");
        showMessageError(env, message);
        return NULL;
    }
    jobject parcelFileDescriptor = env->CallStaticObjectMethod(ParcelFileDescriptor, fromFileDescriptor, fileDescriptor);
    jlong sizeFile = env->CallLongMethod(parcelFileDescriptor, getStatSize);
    if (sizeFile == -1) {
        char *message = (char*)malloc(sizeof(char) * 338);
        strcpy(message, "You cannot use the newMemoryFromFileDescriptor method to create a Memory object because the value of the fileDescriptor");
        strcat(message, "variable is not the descriptor of any file, to get the correct file descriptor use the open method of the ParcelFileDescriptor ");
        strcat(message, "class and create an object with which you can get the file descriptor with the getFd method");
        showMessageError(env, message);
        return NULL;
    }
    if (size > sizeFile) {
        char *message = (char*)malloc(sizeof(char) * 205);
        strcpy(message, "You cannot use the newMemoryFromFileDescriptor method to create a Memory object because the value of the size variable is ");
        strcat(message, "greater than the size of the file that is referenced by the fileDescriptor variable");
        showMessageError(env, message);
        return NULL;
    }
    void **Result = newMemoryFromFileDescriptor(size, mode, fileDescriptor, offset);
    jobject memory = NULL;
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
        jfieldID isCreated = env->GetFieldID(Memory, "isCreated", "Z");
        memory = env->AllocObject(Memory);
        env->SetIntField(memory, currentMemory, currentPositionMemory);
        env->SetBooleanField(memory, isCreated, JNI_TRUE);
        currentPositionMemory++;
        if (currentPositionMemory == 100) currentPositionMemory = 0;
    } else showMessageError(env, (char*)Result[0]);
    return memory;
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Memory_newMemoryFromBuffer(JNIEnv *env, jclass Memory, jobject buffer) {
    if (buffer == NULL) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the newMemoryFromBuffer method because the buffer object you provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreatedBuffer = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreatedBuffer = env->GetBooleanField(buffer, isCreatedBuffer);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 95);
        strcpy(message, "You cannot use the newMemoryFromBuffer method because the buffer object you provided is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreatedBuffer) {
        char *message = (char*)malloc(sizeof(char) * 126);
        strcpy(message, "You cannot use the newMemoryFromBuffer method because the buffer object you provided was not created with the ");
        strcat(message, "getBuffer method");
        showMessageError(env, message);
        return NULL;
    }
    jobject memory = NULL;
#if __ANDROID_API__ >= 29
    void **Result = newMemoryFromBuffer(_currentBuffer);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
        jfieldID isCreatedMemory = env->GetFieldID(Memory, "isCreated", "Z");
        memory = env->AllocObject(Memory);
        env->SetIntField(memory, currentMemory, currentPositionMemory);
        env->SetBooleanField(memory, isCreatedMemory, JNI_TRUE);
        currentPositionMemory++;
        if (currentPositionMemory == 100) currentPositionMemory = 0;
    }
#else
    char *message = (char*)malloc(sizeof(char) * 167);
    strcpy(message, "You cannot use the newMemoryFromBuffer method because the android version that your android device has installed is ");
    strcat(message, "lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
    return memory;
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Memory_copy(JNIEnv *env, jobject memory, jobject memorySrc) {
    if (memorySrc == NULL) {
        char *message = (char*)malloc(sizeof(char) * 122);
        strcpy(message, "You cannot use the copy method of the Memory object you are using because the memorySrc object you provided is set to null");
        showMessageError(env, message);
        return;
    }
    jclass Memory = env->GetObjectClass(memory);
    jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
    jfieldID isCreated = env->GetFieldID(Memory, "isCreated", "Z");
    jint _currentMemory = env->GetIntField(memory, currentMemory);
    jint currentMemorySrc = env->GetIntField(memorySrc, currentMemory);
    jboolean _isCreatedMemory = env->GetBooleanField(memory, isCreated);
    jboolean isCreatedMemorySrc = env->GetBooleanField(memorySrc, isCreated);
    if (_currentMemory == -1 || currentMemorySrc == -1) {
        char *message = (char*)malloc(sizeof(char) * 130);
        strcpy(message, "You cannot use the copy method of the Memory object you are using is invalid and / or the memorySrc object you provided ");
        strcat(message, "is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedMemory || !isCreatedMemorySrc) {
        char *message = (char*)malloc(sizeof(char) * 225);
        strcpy(message, "You cannot use the copy method of the Memory object you are using because this Memory object and / or the memorySrc object ");
        strcat(message, "were not created with any of the newMemory, newMemoryFromFileDescriptor or newMemoryFromBuffer methods");
        showMessageError(env, message);
        return;
    }
#if __ANDROID_API__ >= 30
    void **Result = copyMemory(_currentMemory, currentMemorySrc);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 187);
    strcpy(message, "You cannot use the copy method of the Memory object you are using because the android version that your android device ");
    strcat(message, "has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Memory_delete(JNIEnv *env, jobject memory) {
    jclass Memory = env->GetObjectClass(memory);
    jfieldID currentMemory = env->GetFieldID(Memory, "currentMemory", "I");
    jfieldID isCreated = env->GetFieldID(Memory, "isCreated", "Z");
    jint _currentMemory = env->GetIntField(memory, currentMemory);
    jboolean _isCreated = env->GetBooleanField(memory, isCreated);
    if (_currentMemory == -1) {
        char *message = (char*)malloc(sizeof(char) * 89);
        strcpy(message, "You cannot use the delete method of the Memory object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 191);
        strcpy(message, "You cannot use the delete method of the Memory object you are using because it was not created with any of the following ");
        strcat(message, "methods: newMemory, newMemoryFromFileDescriptor or newMemoryFromBuffer");
        showMessageError(env, message);
        return;
    }
    deleteMemory(_currentMemory);
    env->SetIntField(memory, currentMemory, -1);
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_newMemoryDescriptor(JNIEnv *env, jclass MemoryDescriptor) {
#if __ANDROID_API__ >= 30
    void **Result = createMemoryDescriptor();
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID currentMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "currentMemoryDescriptor", "I");
        jfieldID isCreated = env->GetFieldID(MemoryDescriptor, "isCreated", "Z");
        jobject memoryDescriptor = env->AllocObject(MemoryDescriptor);
        env->SetIntField(memoryDescriptor, currentMemoryDescriptor, currentPositionMemoryDescriptor);
        env->SetBooleanField(memoryDescriptor, isCreated, JNI_TRUE);
        currentPositionMemoryDescriptor++;
        if (currentPositionMemoryDescriptor == 100) currentPositionMemoryDescriptor = 0;
        return memoryDescriptor;
    } else {
        showMessageError(env, (char*)Result[0]);
        return NULL;
    }
#else
    char *message = (char*)malloc(sizeof(char) * 203);
    strcpy(message, "You cannot use the newMemoryDescriptor method to create a MemoryDescriptor object because the android version that your ");
    strcat(message, "android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
    return NULL;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_finish(JNIEnv *env, jobject memoryDescriptor) {
#if  __ANDROID_API__ >= 30
    jclass MemoryDescriptor = env->GetObjectClass(memoryDescriptor);
    jfieldID currentMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "currentMemoryDescriptor", "I");
    jfieldID isCreated = env->GetFieldID(MemoryDescriptor, "isCreated", "Z");
    jint _currentMemoryDescriptor = env->GetIntField(memoryDescriptor, currentMemoryDescriptor);
    jboolean _isCreated = env->GetBooleanField(memoryDescriptor, isCreated);
    if (_currentMemoryDescriptor == -1) {
        char *message = (char*)malloc(sizeof(char) * 99);
        strcpy(message, "You cannot use the finish method of the MemoryDescriptor object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 140);
        strcpy(message, "You cannot use the finish method of the MemoryDescriptor object you are using because it was not created with the ");
        strcat(message, "newMemoryDescriptor method");
        showMessageError(env, message);
        return;
    }
    void **finishMemoryDescriptor(_currentMemoryDescriptor);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 199);
    strcpy(message, "You cannot use the finish method of the MemoryDescriptor object you are using because the android version that your ");
    strcat(message, "android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_delete(JNIEnv *env, jobject memoryDescriptor) {
#if __ANDROID_API__ >= 30
    jclass MemoryDescriptor = env->GetObjectClass(memoryDescritptor);
    jfieldID currentMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "currentMemoryDescriptor", "I");
    jfieldID isCreated = env->GetFieldID(MemoryDescriptor, "isCreated", "Z");
    jint _currentMemoryDescriptor = env->GetIntField(memoryDescriptor, currentMemoryDescriptor);
    jboolean _isCreated = env->GetBooleanField(memoryDescriptor, isCreated);
    if (_currentMemoryDescriptor == -1) {
        char *message = (char*)malloc(sizeof(char) * 99);
        strcpy(message, "You cannot use the delete method of the MemoryDescriptor object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 140);
        strcpy(message, "You cannot use the delete method of the MemoryDescriptor object you are using because it was not created with the ");
        strcat(message, "newMemoryDescriptor method");
        showMessageError(env, message);
        return;
    }
    deleteMemoryDescriptor(_currentMemoryDescriptor);
    env->SetIntField(memoryDescriptor, currentMemoryDescriptor, -1);
#else
    char *message = (char*)malloc(sizeof(char) * 199);
    strcpy(message, "You cannot use the delete method of the MemoryDescriptor object you are using because the android version that your ");
    strcat(message, "android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_addInputRole(JNIEnv *env, jobject memoryDescriptor, jobject compilation,
                                           jint indexInput, jfloat frequency) {
#if __ANDROID_API__ >= 30
    if (compilation == NULL) {
        char *message = (char*)malloc(sizeof(char) * 142);
        strcpy(message, "You cannot use the addInputRole method of the MemoryDescriptor object you are using because the Compilation object you ");
        strcat(message, "provided is set to null");
        showMessageError(env, message);
        return;
    }
    if (indexInput < 0) {
        char *message = (char*)malloc(sizeof(char) * 143);
        strcpy(message, "You cannot use the addInputRole method of the MemoryDescriptor object you are using because the value of the indexInput ");
        strcat(message, "variable is less than 0");
        showMessageError(env, message);
        return;
    }
    if (frequency < 0 || frequency > 1) {
        char *message = (char*)malloc(sizeof(char) * 207);
        strcpy(message, "You cannot use the addInputRole method of the MemoryDescriptor object you are using because the value of the frequency ");
        strcat(message, "variable is less than 0 or is greater than 1, the allowed values must be between 0 and 1");
        showMessageError(env, message);
        return;
    }
    jclass Compilation = env->GetObjectClass(compilation);
    jclass MemoryDescriptor = env->GetObjectClass(memoryDescriptor);
    jfieldID currentMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "currentMemoryDescriptor", "I");
    jfieldID isCreatedMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "isCreated", "Z");
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreatedCompilation = env->GetFieldID(Compilation, "isCreated", "Z");
    jint _currentMemoryDescriptor = env->GetIntField(memoryDescriptor, currentMemoryDescriptor);
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jboolean _isCreatedMemoryDescriptor = env->GetBooleanField(memoryDescriptor, isCreatedMemoryDescriptor);
    jboolean _isCreatedCompilation = env->GetBooleanField(compilation, isCreatedCompilation);
    if (_currentCompilation == -1 || _currentMemoryDescriptor == -1) {
        char *message = (char*)malloc(sizeof(char) * 159);
        strcpy(message, "You cannot use the addInputRole method of the MemoryDescriptor object you are using because it is invalid and/or the ");
        strcat(message, "Compilation object you provided is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedCompilation || !_isCreatedMemoryDescriptor) {
        char *message = (char*)malloc(sizeof(char) * 266);
        strcpy(message, "You cannot use the addInputRole method of the MemoryDescriptor object you are using because it was not created with the ");
        strcat(message, "newMemoryDescriptor method and / or the Compilation object you provided was not created with the newCompilation or ");
        strcat(message, "newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    void **Result = addInputRoleMemoryDescriptor(_currentMemoryDescriptor, _currentCompilation, indexInput, frequency);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 208);
    strcpy(message, "You cannot use the addInputRole method of the MemoryDescriptor object you are using because the version of android that ");
    strcat(message, "your android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_addOutputRole(JNIEnv *env, jobject memoryDescriptor, jobject compilation,
                                            jint indexOutput, jfloat frequency) {
#if __ANDROID_API__ >= 30
    if (compilation == NULL) {
        char *message = (char*)malloc(sizeof(char) * 143);
        strcpy(message, "You cannot use the addOutputRole method of the MemoryDescriptor object you are using because the compilation object you ");
        strcat(message, "provided is set to null");
        showMessageError(env, message);
        return;
    }
    if (indexOutput < 0) {
        char *message = (char*)malloc(sizeof(char) * 145);
        strcpy(message, "You cannot use the addOutputRole method of the MemoryDescriptor object you are using because the value of the indexOutput ");
        strcat(message, "variable is less than 0");
        showMessageError(env, message);
        return;
    }
    if (frequency < 0 || frequency > 1) {
        char *message = (char*)malloc(sizeof(char) * 210);
        strcpy(message, "You cannot use the addOutputRole method of the MemoryDescriptor object you are using because the value of the frequency ");
        strcat(message, "variable is less than 0 or greater than 1 you can only use values that are between 0 and 1");
        showMessageError(env, message);
        return;
    }
    jclass Compilation = env->GetObjectClass(compilation);
    jclass MemoryDescriptor = env->GetObjectClass(memoryDescriptor);
    jfieldID currentCompilation = env->GetFieldID(Compilation, "currentCompilation", "I");
    jfieldID isCreatedCompilation = env->GetFieldID(Compilation, "isCreated", "Z");
    jfieldID currentMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "currentMemoryDescriptor", "I");
    jfieldID isCreatedMemoryDescriptor = env->GetFieldID(MemoryDescriptor, "isCreated", "Z");
    jint _currentCompilation = env->GetIntField(compilation, currentCompilation);
    jint _currentMemoryDescriptor = env->GetIntField(memoryDescriptor, currentMemoryDescriptor);
    jboolean _isCreatedCompilation = env-GetBooleanField(compilation, isCreatedCompilation);
    jboolean _isCreatedMemoryDescriptor = env->GetBooleanField(memoryDescriptor, isCreatedMemoryDescriptor);
    if (_currentCompilation == -1 || _currentMemoryDescriptor == -1) {
        char *message = (char*)malloc(sizeof(char) * 160);
        strcpy(message, "You cannot use the addOutputRole method of the MemoryDescriptor object you are using because it is invalid and/or the ");
        strcat(message, "Compilation object you provided is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreatedCompilation || !_isCreatedMemoryDescriptor) {
        char *message = (char*)malloc(sizeof(char) * 265);
        strcpy(message, "You cannot use the addOutputRole method of the MemoryDescriptor object you are using because it was not created with the ");
        strcat(message, "newMemoryDescriptor method and/or the Compilation object you provided was not created with the newCompilation or newCompilationForDevices method");
        showMessageError(env, message);
        return;
    }
    void **Result = addOutputRoleMemoryDescriptor(_currentMemoryDescriptor, _currentCompilation, indexOutput, frequency);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 209);
    strcpy(message, "You cannot use the addOutputRole method of the MemoryDescriptor object you are using because the version of android that ");
    strcat(message, "your android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_copy(JNIEnv *env, jobject memoryDescriptor, jobject memoryDescriptorSrc) {
#if __ANDROID_API__ >= 30
    if (memoryDescriptorSrc == NULL) {
        char *message = (char*)malloc(sizeof(char) * 142);
        strcpy(message, "You cannot use the copy method of the MemoryDescriptor object you are using because the memoryDescriptorSrc object you ");
        strcat(message, "provided is set to null");
        showMessageError(env, message);
        return;
    }
    jclass MemoryDescriptor = env->GetObjectClass(memoryDescriptor);
    jfieldID currentMemoryDescriptor = env-GetFieldID(MemoryDescriptor, "currentMemoryDescriptor", "I");
    jfieldID isCreated = env->GetFielID(MemoryDescriptor, "isCreated", "Z");
    jint _currentMemoryDescriptor = env->GetIntField(memoryDescriptor, currentMemoryDescriptor);
    jint _currentMemoryDescriptorSrc = env->GetIntField(memoryDescriptorSrc, currentMemoryDescriptor);
    jboolean isCreatedMemoryDescriptor = env->GetBooleanField(memoryDescriptor, isCreated);
    jboolean isCreatedMemoryDescriptorSrc = env->GetBooleanField(memoryDescriptorSrc, isCreated);
    if (_currentMemoryDescriptor == -1 || _currentMemoryDescriptorSrc == -1) {
        char *message = (char*)malloc(sizeof(char) * 159);
        strcpy(message, "You cannot use the copy method of the MemoryDescriptor object you are using because it is invalid and/or the ");
        strcat(message, "memoryDescriptorSrc object you provided is invalid");
        showMessageError(env, message);
        return;
    }
    if (!isCreatedMemoryDescriptor || !isCreatedMemoryDescriptorSrc) {
        char *message = (char*)malloc(sizeof(char) * 210);
        strcpy(message, "You cannot use the copy method of the MemoryDescriptor object you are using because both that object and the ");
        strcat(message, "memoryDescriptorSrc object you provided may not have been created with the newMemoryDescriptor method");
        showMessageError(env, message);
        return;
    }
    void **Result = copyMemoryDescriptor(_currentMemoryDescriptor, _currentMemoryDescriptorSrc);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
#else
    char *message = (char*)malloc(sizeof(char) * 197);
    strcpy(message, "You cannot use the copy method of the MemoryDescriptor object you are using because the android version that your android ");
    strcat(message, "device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_MemoryDescriptor_setDimensions(JNIEnv *env, jobject memoryDescriptor, jintArray dimensions) {
#if __ANDROID_API__ >= 30
    if (dimensions == NULL) {
        char *message = (char*)malloc(sizeof(char) * 141);
        strcpy(message, "You cannot use the setDimensions method of the MemoryDescriptor object you are using because the array dimensions you ");
        strcat(message, "provided is set to null");
        showMessageError(env, message);
        return;
    }
    jsize sizeDimensions = env->GetArrayLength(dimensions);
    if (sizeDimensions == 0) {
        char *message = (char*)malloc(sizeof(char) * 135);
        strcpy(message, "You cannot use the setDimensions method of the MemoryDescriptor object you are using because the array dimensions you ");
        strcat(message, "provided is empty");
        showMessageError(env, message);
        return;
    }
    jint *dataDimensions = env->GetIntArrayElements(dimensions, NULL);
    for (jint position = 0; position < sizeDimensions; position++) {
        if (dataDimensions[position] <= 0) {
            char *message = (char*)malloc(sizeof(char) * 244);
            strcpy(message, "You cannot use the setDimensions method of the MemoryDescriptor object you are using because the dimensions array ");
            strcat(message, "you provided contains data less than or equal to 0, since it's specify the number of inputs and outputs of the ");
            strcat(message, "model you are using");
            showMessageError(env, message);
            return;
        }
    }
    void **Result = setDimensionsMemoryDescriptor(_currentMemoryDescriptor, dataDimensions, sizeDimensions);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, message);
#else
    char *message = (char*)malloc(sizeof(char) * 206);
    strcpy(message, "You cannot use the setDimensions method of the MemoryDescriptor object you are using because the android version that ");
    strcat(message, "your android device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
#endif
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_Event_newEvent(JNIEnv *env, jclass Event, jint sync_fence_fd) {
#if __ANDROID_API__ >= 30
    jclass ParcelFileDescriptor = env->FindClass("android/os/ParcelFileDescriptor");
    jmethodID fromFileDescriptor = env->GetStaticMethodID(ParcelFileDescriptor, "fromFd", "(I)Landroid/os/ParcelFileDescriptor;");
    jmethodID getStatSize = env->GetMethodID(ParcelFileDescriptor, "getStatSize", "()J");
    jobject parcelFileDescriptor = env->CallStaticObjectMethod(ParcelFileDescriptor, fromFileDescriptor, sync_fence_fd);
    jlong sizeFile = env->CallLongMethod(parcelFileDescriptor, getStatSize);
    if (sizeFile == -1) {
        char *message = (char*)malloc(sizeof(char) * 156);
        strcpy(message, "You cannot use the newEvent method to create a new Event object because the value of the sync_fence_fd variable you ");
        strcat(message, "provided is not the descriptor of a file");
        showMessageError(env, message);
        return NULL;
    }
    void **Result = createEventFromSyncFenceFd(sync_fence_fd);
    jobject event = NULL;
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
        jfieldID isCreated = env->GetFieldID(Event, "isCreated", "Z");
        jobject event = env->AllocObject(Event);
        env->SetIntField(event, currentEvent, currentPositionEvent);
        env->SetBooleanField(event, isCreated, JNI_TRUE);
        currentPositionEvent++;
        if (currentPositionEvent == 10000) currentPositionEvent = 0;
    } else showMessageError(env, (char*)Result[0]);
    return event;
#else
    char *message = (char*)malloc(sizeof(char) * 156);
    strcpy(message, "You cannot use the newEvent method because the android version that your android device has installed is lower than the ");
    strcat(message, "R version better known as android 11");
    showMessageError(env, message);
    return NULL;
#endif
}
JNICALL jint Java_com_draico_asvappra_neuralnetworks_Event_getSyncFenceFD(JNIEnv *env, jobject event) {
#if __ANDROID_API__ >= 30
    jclass Event = env->GetObjectClass(event);
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jfieldID isCreated = env->GetFieldID(Event, "isCreated", "Z");
    jint _currentEvent = env->>GetIntField(event, currentEvent);
    jboolean _isCreated = env->GetBooleanField(event, isCreated);
    if (_currentEvent == -1) {
        char *message = (char*)malloc(sizeof(char) * 96);
        strcpy(message, "You cannot use the getSyncFenceFD method of the Event object you are using because it is invalid");
        showMessageError(env, message);
        return -1;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 126);
        strcpy(message, "You cannot use the getSyncFenceFD method of the Event object you are using because it was not created with the ");
        strcat(message, "newEvent method");
        showMessageError(env, message);
        return -1;
    }
    jint sync_fence_fd = 0;
    void **Result = getSyncFenceFd(_currentEvent, &sync_fence_fd);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
    return sync_fence_fd;
#else
    char *message = (char*)malloc(sizeof(char) * 196);
    strcpy(message, "You cannot use the getSyncFenceFD method of the Event object you are using because the android version that your android ");
    strcat(message, "device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
    return -1;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Event_waitExecutionComplete(JNIEnv *env, jobject event) {
    jclass Event = env->GetObjectClass(event);
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jfieldID isCreated = env->GetFieldID(Event, "isCreated", "Z");
    jint _currentEvent = env->GetIntField(event, currentEvent);
    jboolean _isCreated = env->GetBooleanField(event, isCreated);
    if (_currentEvent == -1) {
        char *message = (char*)malloc(sizeof(char) * 103);
        strcpy(message, "You cannot use the waitExecutionComplete method of the Event object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 204);
        strcpy(message, "You cannot use the waitExecutionComplete method of the Event object you are using because it was not created with the ");
        strcat(message, "newEvent method or with the startComputeWithDependencies method of an Execution object");
        showMessageError(env, message);
        return;
    }
    void **Result = waitExecutionComplete(_currentEvent);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) showMessageError(env, (char*)Result[0]);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Event_delete(JNIEnv *env, jobject event) {
    jclass Event = env->GetObjectClass(event);
    jfieldID currentEvent = env->GetFieldID(Event, "currentEvent", "I");
    jfieldID isCreated = env->GetFieldID(Event, "isCreated", "Z");
    jint _currentEvent = env->GetIntField(event, currentEvent);
    jboolean _isCreated = env->GetBooleanField(event, isCreated);
    if (_currentEvent == -1) {
        char *message = (char*)malloc(sizeof(char) * 88);
        strcpy(message, "You cannot use the delete method of the Event object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 189);
        strcpy(message, "You cannot use the delete method of the Event object you are using because it was not created with the newEvent method ");
        strcat(message, "or with the startComputeWithDependencies method of an Execution object");
        showMessageError(env, message);
        return;
    }
    deleteEvent(_currentEvent);
    env->SetIntField(event, currentEvent, -1);
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_OperandType_newOperandType(JNIEnv *env, jclass OperandType, jintArray valueDimensions, jfloat scale,
                                                      jint operandType, jint zeroPoint) {
    if (valueDimensions == NULL) {
        char *message = (char*)malloc(sizeof(char) * 114);
        strcpy(message, "You cannot create an object of type OperandType because the array valueDimensions you have provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jsize sizeDimensions = env->GetArrayLength(valueDimensions);
    if (sizeDimensions == 0) {
        char *message = (char*)malloc(sizeof(char) * 108);
        strcpy(message, "You cannot create an object of type OperandType because the array valueDimensions that you provided is empty");
        showMessageError(env, message);
        return NULL;
    }
    jint *data = env->GetIntArrayElements(valueDimensions, NULL);
    for (jint position = 0; position < sizeDimensions; position++) {
        if (data[position] < 0) {
            char *message = (char*)malloc(sizeof(char) * 129);
            strcpy(message, "You cannot create an object of type OperandType because the array valueDimensions you provided contains number less ");
            strcat(message, "than to zero");
            showMessageError(env, message);
            return NULL;
        }
    }
    if (zeroPoint < 0) {
        char *message = (char*)malloc(sizeof(char) * 96);
        strcpy(message, "You cannot create an object of type OperandType because the variable zeroPoint is less than zero");
        showMessageError(env, message);
        return NULL;
    }
    jfieldID OPERAND_TYPE_FLOAT32 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_FLOAT32", "I");
    jfieldID OPERAND_TYPE_INT32 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_INT32", "I");
    jfieldID OPERAND_TYPE_UINT32 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_UINT32", "I");
    jfieldID OPERAND_TYPE_TENSOR_FLOAT32 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_FLOAT32", "I");
    jfieldID OPERAND_TYPE_TENSOR_INT32 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_INT32", "I");
    jfieldID OPERAND_TYPE_TENSOR_QUANT8_ASYMM = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_QUANT8_ASYMM", "I");
    jfieldID OPERAND_TYPE_BOOL = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_BOOL", "I");
    jfieldID OPERAND_TYPE_TENSOR_QUANT16_SYMM = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_QUANT16_SYMM", "I");
    jfieldID OPERAND_TYPE_TENSOR_FLOAT16 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_FLOAT16", "I");
    jfieldID OPERAND_TYPE_TENSOR_BOOL8 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_BOOL8", "I");
    jfieldID OPERAND_TYPE_FLOAT16 = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_FLOAT16", "I");
    jfieldID OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL", "I");
    jfieldID OPERAND_TYPE_TENSOR_QUANT16_ASYMM = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_QUANT16_ASYMM", "I");
    jfieldID OPERAND_TYPE_TENSOR_QUANT8_SYMM = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_QUANT8_SYMM", "I");
    jfieldID OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED", "I");
    jfieldID OPERAND_TYPE_MODEL = env->GetStaticFieldID(OperandType, "OPERAND_TYPE_MODEL", "I");
    jint _OPERAND_TYPE_FLOAT32 = env->GetStaticIntField(OperandType, OPERAND_TYPE_FLOAT32);
    jint _OPERAND_TYPE_INT32 = env->GetStaticIntField(OperandType, OPERAND_TYPE_INT32);
    jint _OPERAND_TYPE_UINT32 = env->GetStaticIntField(OperandType, OPERAND_TYPE_UINT32);
    jint _OPERAND_TYPE_TENSOR_FLOAT32 = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_FLOAT32);
    jint _OPERAND_TYPE_TENSOR_INT32 = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_INT32);
    jint _OPERAND_TYPE_TENSOR_QUANT8_ASYMM = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_QUANT8_ASYMM);
    jint _OPERAND_TYPE_BOOL = env->GetStaticIntField(OperandType, OPERAND_TYPE_BOOL);
    jint _OPERAND_TYPE_TENSOR_QUANT16_SYMM = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_QUANT16_SYMM);
    jint _OPERAND_TYPE_TENSOR_FLOAT16 = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_FLOAT16);
    jint _OPERAND_TYPE_TENSOR_BOOL8 = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_BOOL8);
    jint _OPERAND_TYPE_FLOAT16 = env->GetStaticIntField(OperandType, OPERAND_TYPE_FLOAT16);
    jint _OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL);
    jint _OPERAND_TYPE_TENSOR_QUANT16_ASYMM = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_QUANT16_ASYMM);
    jint _OPERAND_TYPE_TENSOR_QUANT8_SYMM = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_QUANT8_SYMM);
    jint _OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED = env->GetStaticIntField(OperandType, OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED);
    jint _OPERAND_TYPE_MODEL = env->GetStaticIntField(OperandType, OPERAND_TYPE_MODEL);
    jint _operandType = operandType;
    if ((_operandType & _OPERAND_TYPE_FLOAT32) == _OPERAND_TYPE_FLOAT32 ) _operandType -= _OPERAND_TYPE_FLOAT32;
    if ((_operandType & _OPERAND_TYPE_INT32) == _OPERAND_TYPE_INT32) _operandType -= _OPERAND_TYPE_INT32;
    if ((_operandType & _OPERAND_TYPE_UINT32) == _OPERAND_TYPE_UINT32) _operandType -= _OPERAND_TYPE_UINT32;
    if ((_operandType & _OPERAND_TYPE_TENSOR_FLOAT32) == _OPERAND_TYPE_TENSOR_FLOAT32) _operandType -= _OPERAND_TYPE_TENSOR_FLOAT32;
    if ((_operandType & _OPERAND_TYPE_TENSOR_INT32) == _OPERAND_TYPE_TENSOR_INT32) _operandType -= _OPERAND_TYPE_TENSOR_INT32;
    if ((_operandType & _OPERAND_TYPE_TENSOR_QUANT8_ASYMM) == _OPERAND_TYPE_TENSOR_QUANT8_ASYMM) _operandType -= _OPERAND_TYPE_TENSOR_QUANT8_ASYMM;
    if ((_operandType & _OPERAND_TYPE_BOOL) == _OPERAND_TYPE_BOOL) _operandType -= _OPERAND_TYPE_BOOL;
    if ((_operandType & _OPERAND_TYPE_TENSOR_QUANT16_SYMM) == _OPERAND_TYPE_TENSOR_QUANT16_SYMM) _operandType -= _OPERAND_TYPE_TENSOR_QUANT16_SYMM;
    if ((_operandType & _OPERAND_TYPE_TENSOR_FLOAT16) == _OPERAND_TYPE_TENSOR_FLOAT16) _operandType -= _OPERAND_TYPE_TENSOR_FLOAT16;
    if ((_operandType & _OPERAND_TYPE_TENSOR_BOOL8) == _OPERAND_TYPE_TENSOR_BOOL8) _operandType -= _OPERAND_TYPE_TENSOR_BOOL8;
    if ((_operandType & _OPERAND_TYPE_FLOAT16) == _OPERAND_TYPE_FLOAT16) _operandType -= _OPERAND_TYPE_FLOAT16;
    if ((_operandType & _OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL) == _OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL) _operandType -= _OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL;
    if ((_operandType & _OPERAND_TYPE_TENSOR_QUANT16_ASYMM) == _OPERAND_TYPE_TENSOR_QUANT16_ASYMM) _operandType -= _OPERAND_TYPE_TENSOR_QUANT16_ASYMM;
    if ((_operandType & _OPERAND_TYPE_TENSOR_QUANT8_SYMM) == _OPERAND_TYPE_TENSOR_QUANT8_SYMM) _operandType -= _OPERAND_TYPE_TENSOR_QUANT8_SYMM;
    if ((_operandType & _OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED) == _OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED) _operandType -= _OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED;
    if ((_operandType & _OPERAND_TYPE_MODEL) == _OPERAND_TYPE_MODEL) _operandType -= _OPERAND_TYPE_MODEL;
    if (_operandType != 0) {
        char *message = (char*)malloc(sizeof(char) * 804);
        strcpy(message, "You cannot create an object of type OperandType because the operandType variable you provided contains data other than the ");
        strcat(message, "following list (you can only use data from that list):\nOperandType.OPERAND_TYPE_FLOAT32\n");
        strcat(message, "OperandType.OPERAND_TYPE_INT32\nOperandType.OPERAND_TYPE_UINT32\n");
        strcat(message, "OperandType.OPERAND_TYPE_TENSOR_FLOAT32\nOperandType.OPERAND_TYPE_TENSOR_INT32\n");
        strcat(message, "OperandType.OPERAND_TYPE_TENSOR_QUANT8_ASYMM\nOperandType.OPERAND_TYPE_BOOL\n");
        strcat(message, "OperandType.OPERAND_TYPE_TENSOR_QUANT16_SYMM\nOperandType.OPERAND_TYPE_TENSOR_FLOAT16\n");
        strcat(message, "OperandType.OPERAND_TYPE_TENSOR_BOOL8\nOperandType.OPERAND_TYPE_FLOAT16\n");
        strcat(message, "OperandType.OPERAND_TYPE_QUANT8_SYMM_PER_CHANNEL\nOperandType.OPERAND_TYPE_TENSOR_QUANT16_ASYMM\n");
        strcat(message, "OperandType.OPERAND_TYPE_TENSOR_QUANT8_SYMM\nOperandType.OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED\n");
        strcat(message, "OperandType.OPERAND_TYPE_MODEL");
        showMessageError(env, message);
        return NULL;
    }
    uint32_t *dataDimensions = (uint32_t*)data;
    ANeuralNetworksOperandType *structOperandType = (ANeuralNetworksOperandType*)malloc(sizeof(ANeuralNetworksOperandType));
    structOperandType->dimensions = dataDimensions;
    structOperandType->dimensionCount = sizeDimensions;
    structOperandType->scale = scale;
    structOperandType->type = operandType;
    structOperandType->zeroPoint = zeroPoint;
    operandTypeList[currentPositionOperandType] = *structOperandType;
    jfieldID currentNeuralNetworkOperandType = env->GetFieldID(OperandType, "currentOperandType", "I");
    jfieldID isCreated = env->GetFieldID(OperandType, "isCreated", "Z");
    jobject _neuralNetworkOperantType = env->AllocObject(OperandType);
    env->SetIntField(_neuralNetworkOperantType, currentNeuralNetworkOperandType, currentPositionOperandType);
    env->SetBooleanField(_neuralNetworkOperantType, isCreated, JNI_TRUE);
    currentPositionOperandType++;
    if (currentPositionOperandType == 100) currentPositionOperandType = 0;
    return _neuralNetworkOperantType;
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_OperandType_delete(JNIEnv *env, jobject operandType) {
    jclass OperandType = env->GetObjectClass(operandType);
    jfieldID isCreated = env->GetFieldID(OperandType, "isCreated", "Z");
    jboolean _isCreated = env->GetBooleanField(operandType, isCreated);
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 151);
        strcpy(message, "You cannot use the delete method to delete the object of type OperandType that you are using because it was not created ");
        strcat(message, "using the newOperandType method");
        showMessageError(env, message);
        return;
    }
    jfieldID currentOperandType = env->GetFieldID(OperandType, "currentOperandType", "I");
    jint _currentOperandType = env->GetIntField(operandType, currentOperandType);
    if (_currentOperandType == -1) {
        char *message = (char*)malloc(sizeof(char) * 140);
        strcpy(message, "You cannot use the delete method to delete the object of type OperandType that you used previously and you cannot ");
        strcat(message, "continue using this object");
        showMessageError(env, message);
        return;
    }
    env->SetIntField(operandType, currentOperandType, -1);
}
JNICALL jobjectArray Java_com_draico_asvappra_neuralnetworks_Device_getDevices(JNIEnv *env, jclass Device) {
#if __ANDROID_API__ >= 29
    jfieldID isCreatedDeviceList = env->GetStaticFieldID(Device, "isCreatedDeviceList", "Z");
    jboolean _isCreatedDeviceList = env->GetStaticBooleanField(Device, isCreatedDeviceList);
    if (isCreatedDeviceList) {
        char *message = (char*)malloc(sizeof(char) * 171);
        strcpy(message, "You can't use the getDevices method because you already got the list of devices previously, to use this method release ");
        strcat(message, "the whole list of devices with delete method");
        showMessageError(env, message);
        return NULL;
    }
    jmethodID devicesAvailable = env->GetStaticMethodID(Device, "devicesAvailable", "()I");
    jint _devicesAvailable = env->CallStaticIntMethod(Device, devicesAvailable);
    jobjectArray devices = env->NewObjectArray(_devicesAvailable, Device, NULL);
    for (jint position = 0; position < _devicesAvailable; position++) {
        void **Result = getDevice(position);
        if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
            jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
            jfieldID isCreated = env->GetFieldID(Device, "isCreated", "Z");
            jobject device = env->AllocObject(Device);
            env->SetIntField(device, currentDevice, currentPositionDevice);
            env->SetBooleanField(device, isCreated, JNI_TRUE);
            env->SetObjectArrayElement(devices, position, device);
            currentPositionDevice++;
            if (currentPositionDevice == 10000) currentPositionDevice = 0;
        } else {
            currentPositionDevice -= position + 1;
            devices = NULL;
            return devices;
        }
    }
    env->SetStaticBooleanField(Device, isCreatedDeviceList, JNI_TRUE);
    return devices;
#else
    char *message = (char*)malloc(sizeof(char) * 158);
    strcpy(message, "You cannot use the getDevices method because the android version that your android device has installed is lower than ");
    strcat(message, "the Q version better known as android 10");
    showMessageError(env, message);
    return NULL;
#endif
}
JNICALL jint Java_com_draico_asvappra_neuralnetworks_Device_getType(JNIEnv *env, jobject device) {
#if __ANDROID_API__ >= 29
    jclass Device = env->GetObjectClass(device);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID isCreated = env->GetFieldID(Device, "isCreated", "Z");
    jint _currentDevice = env->GetIntField(device, currentDevice);
    jboolean _isCreated = env->GetBooleanField(device, isCreated);
    if (_currentDevice == -1) {
        char *message = (char*)malloc(sizeof(char) * 90);
        strcpy(message, "You cannot use the getType method of the Device object you are using because it is invalid");
        showMessageError(env, message);
        return -1;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 122);
        strcpy(message, "You cannot use the getType method of the Device object you are using because it was not created with the getDevices method");
        showMessageError(env, message);
        return -1;
    }
    jint typeDevice = 0;
    void **Result = getDeviceType(_currentDevice, &typeDevice);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) {
        showMessageError(env, (char*)Result[0]);
        typeDevice = -1;
    }
    return typeDevice;
#else
    char *message = (char*)malloc(sizeof(char) * 190);
    strcpy(message, "You cannot use the getType method of the Device object you are using because the android version that your android ");
    strcat(message, "device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
    return -1;
#endif
}
JNICALL jstring Java_com_draico_asvappra_neuralnetworks_Device_getName(JNIEnv *env, jobject device) {
#if __ANDROID_API__ >= 29
    jclass Device = env->GetObjectClass(device);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID isCreated = env->GetFieldID(Device, "isCreated", "Z");
    jint _currentDevice = env->GetIntField(device, currentDevice);
    jboolean _isCreated = env->GetBooleanField(device, isCreated);
    if (_currentDevice == -1) {
        char *message = (char*)malloc(sizeof(char) * 90);
        strcpy(message, "You cannot use the getName method of the Device object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 122);
        strcpy(message, "You cannot use the getName method of the Device object you are using because it was not created with the getDevices method");
        showMessageError(env, message);
        return NULL;
    }
    jstring _name = NULL;
    char *name = (char*)malloc(sizeof(char) * 200);
    void **Result = getDeviceName(_currentDevice, &name);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) _name = env->NewStringUTF(name);
    else showMessageError(env, (char*)Result[0]);
    return _name;
#else
    char *message = (char*)malloc(sizeof(char) * 190);
    strcpy(message, "You cannot use the getName method of the Device object you are using because the android version that your android ");
    strcat(message, "device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
    return NULL;
#endif
}
JNICALL jstring Java_com_draico_asvappra_neuralnetworks_Device_getVersion(JNIEnv *env, jobject device) {
#if __ANDROID_API__ >= 29
    jclass Device = env->GetObjectClass(device);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID isCreated = env->GetFieldID(Device, "isCreated", "Z");
    jint _currentDevice = env->GetIntField(device, currentDevice);
    jboolean _isCreated = env->GetBooleanField(device, isCreated);
    if (_currentDevice == -1) {
        char *message = (char*)malloc(sizeof(char) * 93);
        strcpy(message, "You cannot use the getVersion method of the Device object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 125);
        strcpy(message, "You cannot use the getVersion method of the Device object you are using because it was not created with the getDevices method");
        showMessageError(env, message);
        return NULL;
    }
    jstring _version = NULL;
    char *version = (char*)malloc(sizeof(char) * 200);
    void **Result = getDeviceVersion(_currentDevice, &version);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) _version = env->NewStringUTF(version);
    else showMessageError(env, (char*)Result[0]);
    return _version;
#else
    char *message = (char*)malloc(sizeof(char) * 193);
    strcpy(message, "You cannot use the getVersion method of the Device object you are using because the android version that your android ");
    strcat(message, "device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
    return NULL;
#endif
}
JNICALL jlong Java_com_draico_asvappra_neuralnetworks_Device_getNeuralNetworksAPIVersion(JNIEnv *env, jobject device) {
#if __ANDROID_API__ >= 29
    jclass Device = env->GetObjectClass(device);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID isCreated = env->GetFieldID(Device, "isCreated", "Z");
    jint _currentDevice = env->GetIntField(device, currentDevice);
    jboolean _isCreated = env->GetBooleanField(device, isCreated);
    if (_currentDevice == -1) {
        char *message = (char*)malloc(sizeof(char) * 110);
        strcpy(message, "You cannot use the getNeuralNetworksAPIVersion method of the Device object you are using because it is invalid");
        showMessageError(env, message);
        return 0;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 142);
        strcpy(message, "You cannot use the getNeuralNetworksAPIVersion method of the Device object you are using because it was not created ");
        strcat(message, "with the getDevices method");
        showMessageError(env, message);
        return 0;
    }
    jlong versionApi = 0;
    void **Result = getNeuralNetworksAPIVersion(_currentDevice, &versionApi);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) {
        showMessageError(env, (char*)Result[0]);
        return 0;
    } else return versionApi;
#else
    char *message = (char*)malloc(sizeof(char) * 213);
    strcpy(message, "You cannot use the getNeuralNetworksAPIVersion method of the Device object you are using because the version of android ");
    strcat(message, "that your android device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
    return 0;
#endif
}
JNICALL jboolean Java_com_draico_asvappra_neuralnetworks_Device_isLiveState(JNIEnv *env, jobject device) {
#if __ANDROID_API__ >= 30
    jclass Device = env->GetObjectClass(device);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID isCreated = env->GetFieldID(Device, "isCreated", "Z");
    jint _currentDevice = env->GetIntField(device, currentDevice);
    jboolean _isCreated = env->GetBooleanField(device, isCreated);
    if (_currentDevice == -1) {
        char *message = (char*)malloc(sizeof(char) * 94);
        strcpy(message, "You cannot use the isLiveState method of the Device object you are using because it is invalid");
        showMessageError(env, message);
        return false;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 126);
        strcpy(message, "You cannot use the isLiveState method of the Device object you are using because it was not created with the ");
        strcat(message, "getDevices method");
        showMessageError(env, message);
        return false;
    }
    void **Result = isLiveStateDevice(_currentDevice);
    if (*(jint*)Result[1] != ANEURALNETWORKS_NO_ERROR) {
        showMessageError(env, (char*)Result[0]);
        return false;
    } else return true;
#else
    char *message = (char*)malloc(sizeof(char) * 194);
    strcpy(message, "You cannot use the isLiveState method of the Device object you are using because the android version that your android ");
    strcat(message, "device has installed is lower than the R version better known as android 11");
    showMessageError(env, message);
    return false;
#endif
}
JNICALL jint Java_com_draico_asvappra_neuralnetworks_Device_devicesAvailable(JNIEnv *env, jclass Device) {
#if __ANDROID_API__ >= 29
    jint devices;
    void **Result = devicesAvailable(&devices);
    if (*(jint*)Result[1] == ANEURALNETWORKS_NO_ERROR) {
        jfieldID numberDevicesAvailable = env->GetStaticFieldID(Device, "numberDevicesAvailable", "I");
        env->SetStaticIntField(Device, numberDevicesAvailable, devices);
        return devices;
    } else {
        showMessageError(env, (char*)Result[0]);
        return 0;
    }
#else
    char *message = (char*)malloc(sizeof(char) * 162);
    strcpy(message, "You cannot use the devicesAvaible method because the android version that your android device has installed is lower ");
    strcat(message, "than the Q version better known as android 10");
    showMessageError(env, message);
    return 0;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_Device_delete(JNIEnv *env, jobject device) {
#if __ANDROID_API__ >= 29
    jclass Device = env->GetObjectClass(device);
    jfieldID currentDevice = env->GetFieldID(Device, "currentDevice", "I");
    jfieldID isCreated = env->GetFieldID(Device, "isCreated", "Z");
    jfieldID numberDevicesAvailable = env->GetStaticFieldID(Device, "numberDevicesAvailable", "I");
    jfieldID isCreatedDeviceList = env->GetStaticFieldID(Device, "isCreatedDeviceList", "Z");
    jint _currentDevice = env->GetIntField(device, currentDevice);
    jboolean _isCreated = env->GetBooleanField(device, isCreated);
    if (_currentDevice == -1) {
        char *message = (char*)malloc(sizeof(char) * 89);
        strcpy(message, "You cannot use the delete method of the Device object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 121);
        strcpy(message, "You cannot use the delete method of the Device object you are using because it was not created with the getDevices method");
        showMessageError(env, message);
        return;
    }
    devicesList[_currentDevice] = NULL;
    jint _numberDevicesAvailable = env->GetStaticIntField(Device, numberDevicesAvailable);
    _numberDevicesAvailable--;
    if (_numberDevicesAvailable == 0) {
        currentPositionDevice = 0;
        env->SetStaticIntField(Device, numberDevicesAvailable, 0);
        env->SetStaticBooleanField(Device, isCreatedDeviceList, JNI_FALSE);
    }
    env->SetIntField(device, currentDevice, -1);
#else
    char *message = (char*)malloc(sizeof(char) * 189);
    strcpy(message, "You cannot use the delete method of the Device object you are using because the android version that your android ");
    strcat(message, "device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
#endif
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_getBuffer(JNIEnv *env, jclass Buffer) {
    jobject buffer = env->AllocObject(Buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    AHardwareBuffer_acquire(bufferList[currentPositionBuffer]);
    env->SetIntField(buffer, currentBuffer, currentPositionBuffer);
    env->SetBooleanField(buffer, isCreated, JNI_TRUE);
    currentPositionBuffer++;
    if (currentPositionBuffer == 100) currentPositionBuffer = 0;
    return buffer;
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_allocate(JNIEnv *env, jclass Buffer, jobject bufferDescription) {
    if (bufferDescription == NULL) {
        char *message = (char*)malloc(sizeof(char) * 125);
        strcpy(message, "You cannot use the allocate method to create a Buffer object because the bufferDescription object you provided is set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass BufferDescription = env->GetObjectClass(bufferDescription);
    jfieldID USAGE_CPU_READ_NEVER = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_NEVER", "J");
    jfieldID USAGE_CPU_READ_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_RARELY", "J");
    jfieldID USAGE_CPU_READ_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_OFTEN", "J");
    jfieldID USAGE_CPU_READ_MASK = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_MASK", "J");
    jfieldID USAGE_CPU_WRITE_NEVER = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_NEVER", "J");
    jfieldID USAGE_CPU_WRITE_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_RARELY", "J");
    jfieldID USAGE_CPU_WRITE_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_OFTEN", "J");
    jfieldID USAGE_CPU_WRITE_MASK = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_MASK", "J");
    jfieldID USAGE_GPU_SAMPLED_IMAGE = env->GetStaticFieldID(Buffer, "USAGE_GPU_SAMPLED_IMAGE", "J");
    jfieldID USAGE_GPU_FRAMEBUFFER = env->GetStaticFieldID(Buffer, "USAGE_GPU_FRAMEBUFFER", "J");
    jfieldID USAGE_GPU_COLOR_OUTPUT = env->GetStaticFieldID(Buffer, "USAGE_GPU_COLOR_OUTPUT", "J");
    jfieldID USAGE_COMPOSER_OVERLAY = env->GetStaticFieldID(Buffer, "USAGE_COMPOSER_OVERLAY", "J");
    jfieldID USAGE_PROTECTED_CONTENT = env->GetStaticFieldID(Buffer, "USAGE_PROTECTED_CONTENT", "J");
    jfieldID USAGE_VIDEO_ENCODE = env->GetStaticFieldID(Buffer, "USAGE_VIDEO_ENCODE", "J");
    jfieldID USAGE_SENSOR_DIRECT_DATA = env->GetStaticFieldID(Buffer, "USAGE_SENSOR_DIRECT_DATA", "J");
    jfieldID USAGE_GPU_DATA_BUFFER = env->GetStaticFieldID(Buffer, "USAGE_GPU_DATA_BUFFER", "J");
    jfieldID USAGE_GPU_CUBE_MAP = env->GetStaticFieldID(Buffer, "USAGE_GPU_CUBE_MAP", "J");
    jfieldID USAGE_GPU_MIPMAP_COMPLETE = env->GetStaticFieldID(Buffer, "USAGE_GPU_MIPMAP_COMPLETE", "J");
    jfieldID USAGE_VENDOR_0 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_0", "J");
    jfieldID USAGE_VENDOR_1 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_1", "J");
    jfieldID USAGE_VENDOR_2 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_2", "J");
    jfieldID USAGE_VENDOR_3 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_3", "J");
    jfieldID USAGE_VENDOR_4 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_4", "J");
    jfieldID USAGE_VENDOR_5 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_5", "J");
    jfieldID USAGE_VENDOR_6 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_6", "J");
    jfieldID USAGE_VENDOR_7 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_7", "J");
    jfieldID USAGE_VENDOR_8 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_8", "J");
    jfieldID USAGE_VENDOR_9 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_9", "J");
    jfieldID USAGE_VENDOR_10 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_10", "J");
    jfieldID USAGE_VENDOR_11 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_11", "J");
    jfieldID USAGE_VENDOR_12 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_12", "J");
    jfieldID USAGE_VENDOR_13 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_13", "J");
    jfieldID USAGE_VENDOR_14 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_14", "J");
    jfieldID USAGE_VENDOR_15 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_15", "J");
    jfieldID USAGE_VENDOR_16 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_16", "J");
    jfieldID USAGE_VENDOR_17 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_17", "J");
    jfieldID USAGE_VENDOR_18 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_18", "J");
    jfieldID USAGE_VENDOR_19 = env->GetStaticFieldID(Buffer, "USAGE_VENDOR_19", "J");
    jfieldID FORMAT_R8G8B8A8_UNORM = env->GetStaticFieldID(BufferDescription, "FORMAT_R8G8B8A8_UNORM", "I");
    jfieldID FORMAT_R8G8B8X8_UNORM = env->GetStaticFieldID(BufferDescription, "FORMAT_R8G8B8X8_UNORM", "I");
    jfieldID FORMAT_R8G8B8_UNORM = env->GetStaticFieldID(BufferDescription, "FORMAT_R8G8B8_UNORM", "I");
    jfieldID FORMAT_R5G6B5_UNORM = env->GetStaticFieldID(BufferDescription, "FORMAT_R5G6B5_UNORM", "I");
    jfieldID FORMAT_R16G16B16A16_FLOAT = env->GetStaticFieldID(BufferDescription, "FORMAT_R16G16B16A16_FLOAT", "I");
    jfieldID FORMAT_R10G10B10A2_UNORM = env->GetStaticFieldID(BufferDescription, "FORMAT_R10G10B10A2_UNORM", "I");
    jfieldID FORMAT_BLOB = env->GetStaticFieldID(BufferDescription, "FORMAT_BLOB", "I");
    jfieldID FORMAT_D16_UNORM = env->GetStaticFieldID(BufferDescription, "FORMAT_D16_UNORM", "I");
    jfieldID FORMAT_D24_UNORM = env->GetStaticFieldID(BufferDescription, "FORMAT_D24_UNORM", "I");
    jfieldID FORMAT_D24_UNORM_S8_UINT = env->GetStaticFieldID(BufferDescription, "FORMAT_D24_UNORM_S8_UINT", "I");
    jfieldID FORMAT_D32_FLOAT = env->GetStaticFieldID(BufferDescription, "FORMAT_D32_FLOAT", "I");
    jfieldID FORMAT_D32_FLOAT_S8_UINT = env->GetStaticFieldID(BufferDescription, "FORMAT_D32_FLOAT_S8_UINT", "I");
    jfieldID FORMAT_S8_UINT = env->GetStaticFieldID(BufferDescription, "FORMAT_S8_UINT", "I");
    jfieldID FORMAT_Y8Cb8Cr8_420 = env->GetStaticFieldID(BufferDescription, "FORMAT_Y8Cb8Cr8_420", "I");
    jfieldID format = env->GetFieldID(BufferDescription, "format", "I");
    jfieldID height = env->GetFieldID(BufferDescription, "height", "I");
    jfieldID width = env->GetFieldID(BufferDescription, "width", "I");
    jfieldID layers = env->GetFieldID(BufferDescription, "layers", "I");
    jfieldID rfu0 = env->GetFieldID(BufferDescription, "rfu0", "I");
    jfieldID rfu1 = env->GetFieldID(BufferDescription, "rfu1", "I");
    jfieldID stride = env->GetFieldID(BufferDescription, "stride", "I");
    jfieldID usage = env->GetFieldID(BufferDescription, "usage", "J");
    jlong _USAGE_CPU_READ_NEVER = env->GetStaticLongField(Buffer, USAGE_CPU_READ_NEVER);
    jlong _USAGE_CPU_READ_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_READ_RARELY);
    jlong _USAGE_CPU_READ_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_READ_OFTEN);
    jlong _USAGE_CPU_READ_MASK = env->GetStaticLongField(Buffer, USAGE_CPU_READ_MASK);
    jlong _USAGE_CPU_WRITE_NEVER = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_NEVER);
    jlong _USAGE_CPU_WRITE_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_RARELY);
    jlong _USAGE_CPU_WRITE_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_OFTEN);
    jlong _USAGE_CPU_WRITE_MASK = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_MASK);
    jlong _USAGE_GPU_SAMPLED_IMAGE = env->GetStaticLongField(Buffer, USAGE_GPU_SAMPLED_IMAGE);
    jlong _USAGE_GPU_FRAMEBUFFER = env->GetStaticLongField(Buffer, USAGE_GPU_FRAMEBUFFER);
    jlong _USAGE_GPU_COLOR_OUTPUT = env->GetStaticLongField(Buffer, USAGE_GPU_COLOR_OUTPUT);
    jlong _USAGE_COMPOSER_OVERLAY = env->GetStaticLongField(Buffer, USAGE_COMPOSER_OVERLAY);
    jlong _USAGE_PROTECTED_CONTENT = env->GetStaticLongField(Buffer, USAGE_PROTECTED_CONTENT);
    jlong _USAGE_VIDEO_ENCODE = env->GetStaticLongField(Buffer, USAGE_VIDEO_ENCODE);
    jlong _USAGE_SENSOR_DIRECT_DATA = env->GetStaticLongField(Buffer, USAGE_SENSOR_DIRECT_DATA);
    jlong _USAGE_GPU_DATA_BUFFER = env->GetStaticLongField(Buffer, USAGE_GPU_DATA_BUFFER);
    jlong _USAGE_GPU_CUBE_MAP = env->GetStaticLongField(Buffer, USAGE_GPU_CUBE_MAP);
    jlong _USAGE_GPU_MIPMAP_COMPLETE = env->GetStaticLongField(Buffer, USAGE_GPU_MIPMAP_COMPLETE);
    jlong _USAGE_VENDOR_0 = env->GetStaticLongField(Buffer, USAGE_VENDOR_0);
    jlong _USAGE_VENDOR_1 = env->GetStaticLongField(Buffer, USAGE_VENDOR_1);
    jlong _USAGE_VENDOR_2 = env->GetStaticLongField(Buffer, USAGE_VENDOR_2);
    jlong _USAGE_VENDOR_3 = env->GetStaticLongField(Buffer, USAGE_VENDOR_3);
    jlong _USAGE_VENDOR_4 = env->GetStaticLongField(Buffer, USAGE_VENDOR_4);
    jlong _USAGE_VENDOR_5 = env->GetStaticLongField(Buffer, USAGE_VENDOR_5);
    jlong _USAGE_VENDOR_6 = env->GetStaticLongField(Buffer, USAGE_VENDOR_6);
    jlong _USAGE_VENDOR_7 = env->GetStaticLongField(Buffer, USAGE_VENDOR_7);
    jlong _USAGE_VENDOR_8 = env->GetStaticLongField(Buffer, USAGE_VENDOR_8);
    jlong _USAGE_VENDOR_9 = env->GetStaticLongField(Buffer, USAGE_VENDOR_9);
    jlong _USAGE_VENDOR_10 = env->GetStaticLongField(Buffer, USAGE_VENDOR_10);
    jlong _USAGE_VENDOR_11 = env->GetStaticLongField(Buffer, USAGE_VENDOR_11);
    jlong _USAGE_VENDOR_12 = env->GetStaticLongField(Buffer, USAGE_VENDOR_12);
    jlong _USAGE_VENDOR_13 = env->GetStaticLongField(Buffer, USAGE_VENDOR_13);
    jlong _USAGE_VENDOR_14 = env->GetStaticLongField(Buffer, USAGE_VENDOR_14);
    jlong _USAGE_VENDOR_15 = env->GetStaticLongField(Buffer, USAGE_VENDOR_15);
    jlong _USAGE_VENDOR_16 = env->GetStaticLongField(Buffer, USAGE_VENDOR_16);
    jlong _USAGE_VENDOR_17 = env->GetStaticLongField(Buffer, USAGE_VENDOR_17);
    jlong _USAGE_VENDOR_18 = env->GetStaticLongField(Buffer, USAGE_VENDOR_18);
    jlong _USAGE_VENDOR_19 = env->GetStaticLongField(Buffer, USAGE_VENDOR_19);
    jint _FORMAT_R8G8B8A8_UNORM = env->GetStaticIntField(BufferDescription, FORMAT_R8G8B8A8_UNORM);
    jint _FORMAT_R8G8B8X8_UNORM = env->GetStaticIntField(BufferDescription, FORMAT_R8G8B8X8_UNORM);
    jint _FORMAT_R8G8B8_UNORM = env->GetStaticIntField(BufferDescription, FORMAT_R8G8B8_UNORM);
    jint _FORMAT_R5G6B5_UNORM = env->GetStaticIntField(BufferDescription, FORMAT_R5G6B5_UNORM);
    jint _FORMAT_R16G16B16A16_FLOAT = env->GetStaticIntField(BufferDescription, FORMAT_R16G16B16A16_FLOAT);
    jint _FORMAT_R10G10B10A2_UNORM = env->GetStaticIntField(BufferDescription, FORMAT_R10G10B10A2_UNORM);
    jint _FORMAT_BLOB = env->GetStaticIntField(BufferDescription, FORMAT_BLOB);
    jint _FORMAT_D16_UNORM = env->GetStaticIntField(BufferDescription, FORMAT_D16_UNORM);
    jint _FORMAT_D24_UNORM = env->GetStaticIntField(BufferDescription, FORMAT_D24_UNORM);
    jint _FORMAT_D24_UNORM_S8_UINT = env->GetStaticIntField(BufferDescription, FORMAT_D24_UNORM_S8_UINT);
    jint _FORMAT_D32_FLOAT = env->GetStaticIntField(BufferDescription, FORMAT_D32_FLOAT);
    jint _FORMAT_D32_FLOAT_S8_UINT = env->GetStaticIntField(BufferDescription, FORMAT_D32_FLOAT_S8_UINT);
    jint _FORMAT_S8_UINT = env->GetStaticIntField(BufferDescription, FORMAT_S8_UINT);
    jint _FORMAT_Y8Cb8Cr8_420 = env->GetStaticIntField(BufferDescription, FORMAT_Y8Cb8Cr8_420);
    jint _format = env->GetIntField(bufferDescription, format);
    jint _height = env->GetIntField(bufferDescription, height);
    jint _width = env->GetIntField(bufferDescription, width);
    jint _layers = env->GetIntField(bufferDescription, layers);
    jint _rfu0 = env->GetIntField(bufferDescription, rfu0);
    jint _rfu1 = env->GetIntField(bufferDescription, rfu1);
    jint _stride = env->GetIntField(bufferDescription, stride);
    jlong _usage = env->GetLongField(bufferDescription, usage);
    jint __format = _format;
    jint counterFormat = 0;
    jlong __usage = _usage;
    if ((__format & _FORMAT_R8G8B8A8_UNORM) == _FORMAT_R8G8B8A8_UNORM) { __format -= _FORMAT_R8G8B8A8_UNORM; counterFormat++; }
    if ((__format & _FORMAT_R8G8B8X8_UNORM) == _FORMAT_R8G8B8X8_UNORM) { __format -= _FORMAT_R8G8B8X8_UNORM; counterFormat++; }
    if ((__format & _FORMAT_R8G8B8_UNORM) == _FORMAT_R8G8B8_UNORM) { __format -= _FORMAT_R8G8B8_UNORM; counterFormat++; }
    if ((__format & _FORMAT_R5G6B5_UNORM) == _FORMAT_R5G6B5_UNORM) { __format -= _FORMAT_R5G6B5_UNORM; counterFormat++; }
    if ((__format & _FORMAT_R16G16B16A16_FLOAT) == _FORMAT_R16G16B16A16_FLOAT) { __format -= _FORMAT_R16G16B16A16_FLOAT; counterFormat++; }
    if ((__format & _FORMAT_R10G10B10A2_UNORM) == _FORMAT_R10G10B10A2_UNORM) { __format -= _FORMAT_R10G10B10A2_UNORM; counterFormat++; }
    if ((__format & _FORMAT_BLOB) == _FORMAT_BLOB) { __format -= _FORMAT_BLOB; counterFormat++; }
    if ((__format & _FORMAT_D16_UNORM) == _FORMAT_D16_UNORM) { __format -= _FORMAT_D16_UNORM; counterFormat++; }
    if ((__format & _FORMAT_D24_UNORM) == _FORMAT_D24_UNORM) { __format -= _FORMAT_D24_UNORM; counterFormat++; }
    if ((__format & _FORMAT_D24_UNORM_S8_UINT) == _FORMAT_D24_UNORM_S8_UINT) { __format -= _FORMAT_D24_UNORM_S8_UINT; counterFormat++; }
    if ((__format & _FORMAT_D32_FLOAT) == _FORMAT_D32_FLOAT) { __format -= _FORMAT_D32_FLOAT; counterFormat++; }
    if ((__format & _FORMAT_D32_FLOAT_S8_UINT) == _FORMAT_D32_FLOAT_S8_UINT) { __format -= _FORMAT_D32_FLOAT_S8_UINT; counterFormat++; }
    if ((__format & _FORMAT_S8_UINT) == _FORMAT_S8_UINT) { __format -= _FORMAT_S8_UINT; counterFormat++; }
    if ((__format & _FORMAT_Y8Cb8Cr8_420) == _FORMAT_Y8Cb8Cr8_420) { __format -= _FORMAT_Y8Cb8Cr8_420; counterFormat++; }
    if (__format != 0 || counterFormat > 1) {
        char *message = (char*)malloc(sizeof(char) * 763);
        strcpy(message, "You cannot use the allocate method to create a Buffer object because the bufferDescription object you provided, you set ");
        strcat(message, "a different value in the format variable of the constructor of the bufferDescription object and make sure you only use one ");
        strcat(message, "of the following values:\nBuffeDescription.FORMAT_R8G8B8A8_UNORM\nBufferDescription.FORMAT_R8G8B8X8_UNORM\n");
        strcat(message, "BufferDescription.FORMAT_R8G8B8_UNORM\nBufferDescription.FORMAT_R5G6B5_UNORM\nBufferDescription.FORMAT_R16G16B16_FLOAT\n");
        strcat(message, "BufferDescription.FORMAT_R10G10B10A2_UNORM\nBufferDescription.FORMAT_BLOB\nBufferDescription.FORMAT_D16_UNORM\n");
        strcat(message, "BufferDescription.FORMAT_D24_UNORM\nBufferDescription.FORMAT_D24_UNORM_S8_UINT\nBufferDescription.FORMAT_D32_FLOAT\n");
        strcat(message, "BufferDescription.FORMAT_D32_FLOAT_S8_UINT\nBufferDescription.FORMAT_Y8Cb8Cr8_420");
        showMessageError(env, message);
        return NULL;
    }
    if ((__usage & _USAGE_CPU_READ_NEVER) == _USAGE_CPU_READ_NEVER) __usage -= _USAGE_CPU_READ_NEVER;
    if ((__usage & _USAGE_CPU_READ_RARELY) == _USAGE_CPU_READ_RARELY) __usage -= _USAGE_CPU_READ_RARELY;
    if ((__usage & _USAGE_CPU_READ_OFTEN) == _USAGE_CPU_READ_OFTEN) __usage -= _USAGE_CPU_READ_OFTEN;
    if ((__usage & _USAGE_CPU_READ_MASK) == _USAGE_CPU_READ_MASK) __usage -= _USAGE_CPU_READ_MASK;
    if ((__usage & _USAGE_CPU_WRITE_NEVER) == _USAGE_CPU_WRITE_NEVER) __usage -= _USAGE_CPU_WRITE_NEVER;
    if ((__usage & _USAGE_CPU_WRITE_RARELY) == _USAGE_CPU_WRITE_RARELY) __usage -= _USAGE_CPU_WRITE_RARELY;
    if ((__usage & _USAGE_CPU_WRITE_OFTEN) == _USAGE_CPU_WRITE_OFTEN) __usage -= _USAGE_CPU_WRITE_OFTEN;
    if ((__usage & _USAGE_CPU_WRITE_MASK) == _USAGE_CPU_WRITE_MASK) __usage -= _USAGE_CPU_WRITE_MASK;
    if ((__usage & _USAGE_GPU_SAMPLED_IMAGE) == _USAGE_GPU_SAMPLED_IMAGE) __usage -= _USAGE_GPU_SAMPLED_IMAGE;
    if ((__usage & _USAGE_GPU_FRAMEBUFFER) == _USAGE_GPU_FRAMEBUFFER) __usage -= _USAGE_GPU_FRAMEBUFFER;
    if ((__usage & _USAGE_GPU_COLOR_OUTPUT) == _USAGE_GPU_COLOR_OUTPUT) __usage -= _USAGE_GPU_COLOR_OUTPUT;
    if ((__usage & _USAGE_COMPOSER_OVERLAY) == _USAGE_COMPOSER_OVERLAY) __usage -= _USAGE_COMPOSER_OVERLAY;
    if ((__usage & _USAGE_PROTECTED_CONTENT) == _USAGE_PROTECTED_CONTENT) __usage -= _USAGE_PROTECTED_CONTENT;
    if ((__usage & _USAGE_VIDEO_ENCODE) == _USAGE_VIDEO_ENCODE) __usage -= _USAGE_VIDEO_ENCODE;
    if ((__usage & _USAGE_SENSOR_DIRECT_DATA) == _USAGE_SENSOR_DIRECT_DATA) __usage -= _USAGE_SENSOR_DIRECT_DATA;
    if ((__usage & _USAGE_GPU_DATA_BUFFER) == _USAGE_GPU_DATA_BUFFER) __usage -= _USAGE_GPU_DATA_BUFFER;
    if ((__usage & _USAGE_GPU_CUBE_MAP) == _USAGE_GPU_CUBE_MAP) __usage -= _USAGE_GPU_CUBE_MAP;
    if ((__usage & _USAGE_GPU_MIPMAP_COMPLETE) == _USAGE_GPU_MIPMAP_COMPLETE) __usage -= _USAGE_GPU_MIPMAP_COMPLETE;
    if ((__usage & _USAGE_VENDOR_0) == _USAGE_VENDOR_0) __usage -= _USAGE_VENDOR_0;
    if ((__usage & _USAGE_VENDOR_1) == _USAGE_VENDOR_1) __usage -= _USAGE_VENDOR_1;
    if ((__usage & _USAGE_VENDOR_2) == _USAGE_VENDOR_2) __usage -= _USAGE_VENDOR_2;
    if ((__usage & _USAGE_VENDOR_3) == _USAGE_VENDOR_3) __usage -= _USAGE_VENDOR_3;
    if ((__usage & _USAGE_VENDOR_4) == _USAGE_VENDOR_4) __usage -= _USAGE_VENDOR_4;
    if ((__usage & _USAGE_VENDOR_5) == _USAGE_VENDOR_5) __usage -= _USAGE_VENDOR_5;
    if ((__usage & _USAGE_VENDOR_6) == _USAGE_VENDOR_6) __usage -= _USAGE_VENDOR_6;
    if ((__usage & _USAGE_VENDOR_7) == _USAGE_VENDOR_7) __usage -= _USAGE_VENDOR_7;
    if ((__usage & _USAGE_VENDOR_8) == _USAGE_VENDOR_8) __usage -= _USAGE_VENDOR_8;
    if ((__usage & _USAGE_VENDOR_9) == _USAGE_VENDOR_9) __usage -= _USAGE_VENDOR_9;
    if ((__usage & _USAGE_VENDOR_10) == _USAGE_VENDOR_10) __usage -= _USAGE_VENDOR_10;
    if ((__usage & _USAGE_VENDOR_11) == _USAGE_VENDOR_11) __usage -= _USAGE_VENDOR_11;
    if ((__usage & _USAGE_VENDOR_12) == _USAGE_VENDOR_12) __usage -= _USAGE_VENDOR_12;
    if ((__usage & _USAGE_VENDOR_13) == _USAGE_VENDOR_13) __usage -= _USAGE_VENDOR_13;
    if ((__usage & _USAGE_VENDOR_14) == _USAGE_VENDOR_14) __usage -= _USAGE_VENDOR_14;
    if ((__usage & _USAGE_VENDOR_15) == _USAGE_VENDOR_15) __usage -= _USAGE_VENDOR_15;
    if ((__usage & _USAGE_VENDOR_16) == _USAGE_VENDOR_16) __usage -= _USAGE_VENDOR_16;
    if ((__usage & _USAGE_VENDOR_17) == _USAGE_VENDOR_17) __usage -= _USAGE_VENDOR_17;
    if ((__usage & _USAGE_VENDOR_18) == _USAGE_VENDOR_18) __usage -= _USAGE_VENDOR_18;
    if ((__usage & _USAGE_VENDOR_19) == _USAGE_VENDOR_19) __usage -= _USAGE_VENDOR_19;
    if (__usage != 0) {
        char *message = (char*)malloc(sizeof(char) * 1200);
        strcpy(message, "You cannot use the allocate method to create a Buffer object because the value of the usage variable that you provided ");
        strcat(message, "in the constructor of the bufferDescription object you can make combinations with the following values:\n");
        strcat(message, "Buffer.USAGE_CPU_READ_NEVER\nBuffer.USAGE_CPU_READ_RARELY\nBuffer.USAGE_CPU_READ_OFTEN\nBuffer.USAGE_CPU_READ_MASK\n");
        strcat(message, "Buffer.USAGE_CPU_WRITE_NEVER\nBuffer.USAGE_CPU_WRITE_RARELY\nBuffer.USAGE_CPU_WRITE_OFTEN\nBuffer.USAGE_CPU_WRITE_MASK\n");
        strcat(message, "Buffer.USAGE_GPU_SAMPLED_IMAGE\nBuffer.USAGE_GPU_FRAMEBUFFER\nBuffer.USAGE_GPU_COLOR_OUTPUT\nBuffer.USAGE_COMPOSER_OVERLAY\n");
        strcat(message, "Buffer.USAGE_PROTECTED_CONTENT\nBuffer.USAGE_VIDEO_ENCODE\nBuffer.USAGE_SENSOR_DIRECT_DATA\nBuffer.USAGE_GPU_DATA_BUFFER\n");
        strcat(message, "Buffer.USAGE_GPU_CUBE_MAP\nBuffer.USAGE_GPU_MIPMAP_COMPLETE\nBuffer.USAGE_VENDOR_0\nBuffer.USAGE_VENDOR_1\n");
        strcat(message, "Buffer.USAGE_VENDOR_2\nBuffer.USAGE_VENDOR_3\nBuffer.USAGE_VENDOR_4\nBuffer.USAGE_VENDOR_5\nBuffer.USAGE_VENDOR_6\n");
        strcat(message, "Buffer.USAGE_VENDOR_7\nBuffer.USAGE_VENDOR_8\nBuffer.USAGE_VENDOR_9\nBuffer.USAGE_VENDOR_10\nBuffer.USAGE_VENDOR_11\n");
        strcat(message, "Buffer.USAGE_VENDOR_12\nBuffer.USAGE_VENDOR_13\nBuffer.USAGE_VENDOR_14\nBuffer.USAGE_VENDOR_15\nBuffer.USAGE_VENDOR_16\n");
        strcat(message, "Buffer.USAGE_VENDOR_17\nBuffer.USAGE_VENDOR_18\nBuffer.USAGE_VENDOR_19");
        showMessageError(env, message);
        return NULL;
    }
    if (_height <= 0 || _width <= 0) {
        char *message = (char*)malloc(sizeof(char) * 210);
        strcpy(message, "You cannot use the allocate method to create a Buffer object because the values of the width and height variables that ");
        strcat(message, "you provided in the constructor of the bufferDescription object are less than or equal to 0");
        showMessageError(env, message);
        return NULL;
    }
    if (_layers <= 0) {
        char *message = (char*)malloc(sizeof(char) * 198);
        strcpy(message, "You cannot use the allocate method to create a Buffer object because the value of the layers variable that you provided in ");
        strcat(message, "the constructor of the bufferDescription object are less than or equal to 0");
        showMessageError(env, message);
        return NULL;
    }
    if (_stride <= 0) {
        char *message = (char*)malloc(sizeof(char) * 198);
        strcpy(message, "You cannot use the allocate method to create a Buffer object because the value of the stride variable that you provided in ");
        strcat(message, "the constructor of the bufferDescription object are less than or equal to 0");
        showMessageError(env, message);
        return NULL;
    }
    _rfu0 = _rfu1 = 0;
    AHardwareBuffer_Desc *bufferDesc = (AHardwareBuffer_Desc*)malloc(sizeof(AHardwareBuffer_Desc));
    bufferDesc->height = _height;
    bufferDesc->width = _width;
    bufferDesc->format = _format;
    bufferDesc->layers = _layers;
    bufferDesc->stride = _stride;
    bufferDesc->usage = _usage;
    bufferDesc->rfu0 = _rfu0;
    bufferDesc->rfu1 = _rfu1;
    int result = AHardwareBuffer_allocate((const AHardwareBuffer_Desc*)bufferDesc, &bufferList[currentPositionBuffer]);
    if (result == 0) {
        jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
        jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
        jobject buffer = env->AllocObject(Buffer);
        env->SetIntField(buffer, currentBuffer, currentPositionBuffer);
        env->SetBooleanField(buffer, isCreated, JNI_TRUE);
        currentPositionBuffer++;
        if (currentPositionBuffer == 100) currentPositionBuffer = 0;
        return buffer;
    } else {
        char *message = (char*)malloc(sizeof(char) * 327);
        strcpy(message, "You cannot use the allocate method to create a Buffer object because there was a problem so you will have to check that ");
        strcat(message, "the data you provided to the bufferDescription object is correct such as the width and height of the screen, the frames ");
        strcat(message, "per second that the camera captures, the number number of bytes of pixels per row, etc.");
        showMessageError(env, message);
        return NULL;
    }
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_fromHardwareBuffer(JNIEnv *env, jclass Buffer, jobject hardwareBuffer) {
    if (hardwareBuffer == NULL) {
        char *message = (char*)malloc(sizeof(char) * 132);
        strcpy(message, "You cannot use the fromHardwareBuffer method to create a Buffer object because the hardwareBuffer object you provided is ");
        strcat(message, "set to null");
        showMessageError(env, message);
        return NULL;
    }
    bufferList[currentPositionBuffer] = AHardwareBuffer_fromHardwareBuffer(env, hardwareBuffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jobject buffer = env->AllocObject(Buffer);
    env->SetIntField(buffer, currentBuffer, currentPositionBuffer);
    env->SetBooleanField(buffer, isCreated, JNI_TRUE);
    currentPositionBuffer++;
    if (currentPositionBuffer == 100) currentPositionBuffer = 0;
    return buffer;
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_getDescription(JNIEnv *env, jobject buffer) {
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 97);
        strcpy(message, "You cannot use the getDescription method of the Buffer object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 175);
        strcpy(message, "You cannot use the getDescription method of the Buffer object you are using because it was not created with any of ");
        strcat(message, "the following methods:\ngetBuffer\nallocate\nfromHardwareBuffer");
        showMessageError(env, message);
        return NULL;
    }
    AHardwareBuffer_Desc *bufferDescription = (AHardwareBuffer_Desc*)malloc(sizeof(AHardwareBuffer_Desc));
    AHardwareBuffer_describe((const AHardwareBuffer*)bufferList[_currentBuffer], bufferDescription);
    jclass BufferDescription = env->FindClass("com/draico/asvappra/neuralnetworks/hardware/BufferDescription");
    jfieldID format = env->GetFieldID(BufferDescription, "format", "I");
    jfieldID height = env->GetFieldID(BufferDescription, "height", "I");
    jfieldID width = env->GetFieldID(BufferDescription, "width", "I");
    jfieldID layers = env->GetFieldID(BufferDescription, "layers", "I");
    jfieldID rfu0 = env->GetFieldID(BufferDescription, "rfu0", "I");
    jfieldID rfu1 = env->GetFieldID(BufferDescription, "rfu1", "I");
    jfieldID stride = env->GetFieldID(BufferDescription, "stride", "I");
    jfieldID usage = env->GetFieldID(BufferDescription, "usage", "J");
    jobject _bufferDescription = env->AllocObject(BufferDescription);
    env->SetIntField(_bufferDescription, format, (jint)bufferDescription->format);
    env->SetIntField(_bufferDescription, height, (jint)bufferDescription->height);
    env->SetIntField(_bufferDescription, width, (jint)bufferDescription->width);
    env->SetIntField(_bufferDescription, layers, (jint)bufferDescription->layers);
    env->SetIntField(_bufferDescription, rfu0, (jint)bufferDescription->rfu0);
    env->SetIntField(_bufferDescription, rfu1, (jint)bufferDescription->rfu1);
    env->SetIntField(_bufferDescription, stride, (jint)bufferDescription->stride);
    env->SetLongField(_bufferDescription, usage, (jlong)bufferDescription->usage);
    return _bufferDescription;
}
JNICALL jboolean Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_isSupported(JNIEnv *env, jclass Buffer, jobject bufferDescription) {
#if __ANDROID_API__ >= 29
    if (bufferDescription == NULL) {
        char *message = (char*)malloc(sizeof(char) * 140);
        strcpy(message, "You cannot use the isSupported method to check if the bufferDescription object is supported by the device because this ");
        strcat(message, "object is set to null");
        showMessageError(env, message);
        return false;
    }
    AHardwareBuffer_Desc *_bufferDescription = (AHardwareBuffer_Desc*)malloc(sizeof(AHardwareBuffer_Desc));
    jclass BufferDescription = env->GetObjectClass(bufferDescription);
    jfieldID format = env->GetFieldID(BufferDescription, "format", "I");
    jfieldID height = env->GetFieldID(BufferDescription, "height", "I");
    jfieldID width = env->GetFieldID(BufferDescription, "width", "I");
    jfieldID layers = env->GetFieldID(BufferDescription, "layers", "I");
    jfieldID rfu0 = env->GetFieldID(BufferDescription, "rfu0", "I");
    jfieldID rfu1 = env->GetFieldID(BufferDescription, "rfu1", "I");
    jfieldID stride = env->GetFieldID(BufferDescription, "stride", "I");
    jfieldID usage = env->GetFieldID(BufferDescription, "usage", "J");
    _bufferDescription->format = (uint32_t)env->GetIntField(bufferDescription, format);
    _bufferDescription->width = (uint32_t)env->GetIntField(bufferDescription, width);
    _bufferDescription->height = (uint32_t)env->GetIntField(bufferDescription, height);
    _bufferDescription->layers = (uint32_t)env->GetIntField(bufferDescription, layers);
    _bufferDescription->stride = (uint32_t)env->GetIntField(bufferDescription, stride);
    _bufferDescription->usage = (uint64_t)env->GetLongField(bufferDescription, usage);
    _bufferDescription->rfu0 = (uint32_t)0;
    _bufferDescription->rfu1 = (uint32_t)0;
    jboolean isSupported = (jboolean)AHardwareBuffer_isSupported((const AHardwareBuffer_Desc*)_bufferDescription);
    return isSupported;
#else
    char *message = (char*)malloc(sizeof(char) * 227);
    strcpy(message, "You cannot use the isSupported method to check if the bufferDescription object is supported by the device because the ");
    strcat(message, "android version that your android device has installed is lower than the Q version better known as android 10");
    showMessageError(env, message);
    return false;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_delete(JNIEnv *env, jobject buffer) {
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 89);
        strcpy(message, "You cannot use the delete method of the Buffer object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 167);
        strcpy(message, "You cannot use the delete method of the Buffer object you are using because it was not created with any of the following ");
        strcat(message, "methods:\ngetBuffer\nallocate\nfromHardwareBuffer");
        showMessageError(env, message);
        return;
    }
    AHardwareBuffer_release(bufferList[_currentBuffer]);
    env->SetIntField(buffer, currentBuffer, -1);
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_lock(JNIEnv *env, jobject buffer, jobject memoryVirtual, jobject bufferArea, jlong usageType,
                         jint fence) {
    if (memoryVirtual == NULL) {
        char *message = (char*)malloc(sizeof(char) * 125);
        strcpy(message, "You cannot use the lock method of the Buffer object you are using because the memoryVirtual object you provided is set to null");
        showMessageError(env, message);
        return;
    }
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 87);
        strcpy(message, "You cannot use the lock method of the Buffer object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 165);
        strcpy(message, "You cannot use the lock method of the Buffer object you are using because it was not created with any of the following ");
        strcat(message, "methods:\ngetBuffer\nallocate\nfromHardwareBuffer");
        showMessageError(env, message);
        return;
    }
    ARect *area = NULL;
    if (bufferArea != NULL) {
        jclass BufferArea = env->GetObjectClass(bufferArea);
        jfieldID top = env->GetFieldID(BufferArea, "top", "I");
        jfieldID bottom = env->GetFieldID(BufferArea, "bottom", "I");
        jfieldID right = env->GetFieldID(BufferArea, "right", "I");
        jfieldID left = env->GetFieldID(BufferArea, "left", "I");
        jint _top = env->GetIntField(bufferArea, top);
        jint _bottom = env->GetIntField(bufferArea, bottom);
        jint _right = env->GetIntField(bufferArea, right);
        jint _left = env->GetIntField(bufferArea, left);
        if (_top < 0 || _bottom < 0 || _left < 0 || _right < 0) {
            char *message = (char*)malloc(sizeof(char) * 214);
            strcpy(message, "You cannot use the lock method of the Buffer object you are using because at least some of the top, bottom, left, or right ");
            strcat(message, "values you provided in the constructor of the bufferArea object you provided is less than 0");
            showMessageError(env, message);
            return;
        }
        area = (ARect*)malloc(sizeof(ARect));
        area->bottom = (int32_t)_bottom;
        area->top = (int32_t)_top;
        area->right = (int32_t)_right;
        area->left = (int32_t)_left;
    }
    jfieldID USAGE_CPU_READ_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_RARELY", "J");
    jfieldID USAGE_CPU_READ_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_OFTEN", "J");
    jfieldID USAGE_CPU_WRITE_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_RARELY", "J");
    jfieldID USAGE_CPU_WRITE_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_OFTEN", "J");
    jlong _USAGE_CPU_READ_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_READ_RARELY);
    jlong _USAGE_CPU_READ_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_READ_OFTEN);
    jlong _USAGE_CPU_WRITE_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_RARELY);
    jlong _USAGE_CPU_WRITE_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_OFTEN);
    jint _usageType = usageType;
    if ((_usageType & _USAGE_CPU_READ_RARELY) == _USAGE_CPU_READ_RARELY) _usageType -= _USAGE_CPU_READ_RARELY;
    if ((_usageType & _USAGE_CPU_READ_OFTEN) == _USAGE_CPU_READ_OFTEN) _usageType -= _USAGE_CPU_READ_OFTEN;
    if ((_usageType & _USAGE_CPU_WRITE_RARELY) == _USAGE_CPU_WRITE_RARELY) _usageType -= _USAGE_CPU_WRITE_RARELY;
    if ((_usageType & _USAGE_CPU_WRITE_OFTEN) == _USAGE_CPU_WRITE_OFTEN) _usageType -= _USAGE_CPU_WRITE_OFTEN;
    if (_usageType != 0) {
        char *message = (char*)malloc(sizeof(char) * 363);
        strcpy(message, "You cannot use the lock method of the Buffer object you are using because the usageType variable you provided has an ");
        strcat(message, "illegal value, you can only use one or more combined values from the following list of allowed values:\n");
        strcat(message, "Buffer.USAGE_CPU_READ_RARELY\nBuffer.USAGE_CPU_READ_OFTEN\nBuffer.USAGE_CPU_READ_MASK\n");
        strcat(message, "Buffer.USAGE_CPU_WRITE_RARELY\nBuffer.USAGE_CPU_WRITE_OFTEN\n");
        showMessageError(env, message);
        return;
    }
    void *dataMemoryVirtual[1];
    if (fence >= 0) {
        jclass File = env->FindClass("java/io/File");
        jmethodID isFile = env->GetMethodID(File, "isFile", "()Z");
        jmethodID exists = env->GetMethodID(File, "exists", "()Z");
        jmethodID getAbsolutePath = env->GetMethodID(File, "getAbsolutePath", "()Ljava/lang/String;");
        jboolean _isFile = env->CallBooleanMethod((jobject)memoryVirtual, isFile);
        jboolean _exists = env->CallBooleanMethod((jobject)memoryVirtual, exists);
        jstring path = (jstring)env->CallObjectMethod((jobject)memoryVirtual, getAbsolutePath);
        if (!_isFile) {
            char *message = (char*)malloc(sizeof(char) * 125);
            strcpy(message, "You cannot use the lock method of the Buffer object you are using because the memoryVirtual object you provided is not a file");
            showMessageError(env, message);
            return;
        }
        if (!_exists) {
            char *message = (char*)malloc(sizeof(char) * 126);
            strcpy(message, "You cannot use the lock method of the Buffer object you are using because the memoryVirtual object you provided does not exist");
            showMessageError(env, message);
            return;
        }
        const char *_path = env->GetStringUTFChars(path, NULL);
        jint *fileDescription = (jint*)malloc(sizeof(jint));
        *fileDescription = open(_path, O_RDWR);
        if (*fileDescription == -1) {
            char *message = (char*)malloc(sizeof(char) * 158);
            strcpy(message, "You cannot use the lock method of the Buffer object you are using because the memoryVirtual object you provided cannot be ");
            strcat(message, "opened to read or modify its content");
            showMessageError(env, message);
            return;
        }
        dataMemoryVirtual[0] = fileDescription;
        fence = *fileDescription;
    } else {
        jclass Object = env->FindClass("java/lang/Object");
        jclass Class = env->FindClass("java/lang/Class");
        jmethodID getClass = env->GetMethodID(Object, "getClass", "()Ljava/lang/Class;");
        jmethodID getName = env->GetMethodID(Class, "getName", "()Ljava/lang/String;");
        jmethodID isArray = env->GetMethodID(Class, "isArray", "()Z");
        jobject objectClass = env->CallObjectMethod((jobject)memoryVirtual, getClass);
        jstring nameClass = (jstring)env->CallObjectMethod(objectClass, getName);
        const char *_nameClass = env->GetStringUTFChars(nameClass, NULL);
        if (strcmp(_nameClass, "[I") == 0) dataMemoryVirtual[0] = env->GetIntArrayElements((jintArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[S") == 0) dataMemoryVirtual[0] = env->GetShortArrayElements((jshortArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[J") == 0) dataMemoryVirtual[0] = env->GetLongArrayElements((jlongArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[F") == 0) dataMemoryVirtual[0] = env->GetFloatArrayElements((jfloatArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[D") == 0) dataMemoryVirtual[0] = env->GetDoubleArrayElements((jdoubleArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[C") == 0) dataMemoryVirtual[0] = (char*)env->GetCharArrayElements((jcharArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[B") == 0) dataMemoryVirtual[0] = env->GetByteArrayElements((jbyteArray)memoryVirtual, NULL);
        else {
            char *message = (char*)malloc(sizeof(char) * 207);
            strcpy(message, "You cannot use the lock method of the buffer object you are using because the memoryVirtual variable can only contain ");
            strcat(message, "a one-dimensional array of int, short, long, float, double, char and byte primitive types");
            showMessageError(env, message);
            return;
        }
    }
    AHardwareBuffer *_buffer = bufferList[_currentBuffer];
    int result = AHardwareBuffer_lock(_buffer, (uint64_t)usageType, (int32_t)fence, area, dataMemoryVirtual);
    if (result != 0) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the lock method of the Buffer object you are using because the buffer has more than 1 layer or your ");
        strcat(message, "device is out of memory to use virtual memory");
        showMessageError(env, message);
    }
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_lockAndGetInfo(JNIEnv *env, jobject buffer, jobject memoryVirtual, jobject bufferArea,
                                   jint usageType, jint fence, jint bytesPerPixel, jint bytesPerStride) {
#if __ANDROID_API__ >= 29
    if (memoryVirtual == NULL) {
        char *message = (char*)malloc(sizeof(char) * 136);
        strcpy(message, "You cannot use the lockAndGetInfo method of the buffer object you are using because the memoryVirtual object you provided is ");
        strcat(message, "set to null");
        showMessageError(env, message);
        return;
    }
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 97);
        strcpy(message, "You cannot use the lockAndGetInfo method of the buffer object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 175);
        strcpy(message, "You cannot use the lockAndGetInfo method of the buffer object you are using because it was not created ");
        strcat(message, "with any of the following methods:\ngetBuffer\nallocate\nfromHardwareBuffer");
        showMessageError(env, message);
        return;
    }
    ARect *area = NULL;
    if (bufferArea != NULL) {
        jclass BufferArea = env->GetObjectClass(bufferArea);
        jfieldID top = env->GetFieldID(BufferArea, "top", "I");
        jfieldID bottom = env->GetFieldID(BufferArea, "bottom", "I");
        jfieldID right = env->GetFieldID(BufferArea, "right", "I");
        jfieldID left = env->GetFieldID(BufferArea, "left", "I");
        jint _top = env->GetIntField(bufferArea, top);
        jint _bottom = env->GetIntField(bufferArea, bottom);
        jint _right = env->GetIntField(bufferArea, right);
        jint _left = env->GetIntField(bufferArea, left);
        if (_top < 0 || _bottom < 0 || _left < 0 || _right < 0) {
            char *message = (char*)malloc(sizeof(char) * 224);
            strcpy(message, "You cannot use the lockAndGetInfo method of the Buffer object you are using because at least some of the top, ");
            strcat(message, "bottom, left, or right values you provided in the constructor of the bufferArea object you provided is less than 0");
            showMessageError(env, message);
            return;
        }
        area = (ARect*)malloc(sizeof(ARect));
        area->bottom = (int32_t)_bottom;
        area->top = (int32_t)_top;
        area->right = (int32_t)_right;
        area->left = (int32_t)_left;
    }
    jfieldID USAGE_CPU_READ_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_RARELY", "J");
    jfieldID USAGE_CPU_READ_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_OFTEN", "J");
    jfieldID USAGE_CPU_WRITE_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_RARELY", "J");
    jfieldID USAGE_CPU_WRITE_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_OFTEN", "J");
    jlong _USAGE_CPU_READ_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_READ_RARELY);
    jlong _USAGE_CPU_READ_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_READ_OFTEN);
    jlong _USAGE_CPU_WRITE_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_RARELY);
    jlong _USAGE_CPU_WRITE_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_OFTEN);
    jint _usageType = usageType;
    if ((_usageType & _USAGE_CPU_READ_RARELY) == _USAGE_CPU_READ_RARELY) _usageType -= _USAGE_CPU_READ_RARELY;
    if ((_usageType & _USAGE_CPU_READ_OFTEN) == _USAGE_CPU_READ_OFTEN) _usageType -= _USAGE_CPU_READ_OFTEN;
    if ((_usageType & _USAGE_CPU_WRITE_RARELY) == _USAGE_CPU_WRITE_RARELY) _usageType -= _USAGE_CPU_WRITE_RARELY;
    if ((_usageType & _USAGE_CPU_WRITE_OFTEN) == _USAGE_CPU_WRITE_OFTEN) _usageType -= _USAGE_CPU_WRITE_OFTEN;
    if (_usageType != 0) {
        char *message = (char*)malloc(sizeof(char) * 373);
        strcpy(message, "You cannot use the lockAndGetInfo method of the Buffer object you are using because the usageType variable you provided has an ");
        strcat(message, "illegal value, you can only use one or more combined values from the following list of allowed values:\n");
        strcat(message, "Buffer.USAGE_CPU_READ_RARELY\nBuffer.USAGE_CPU_READ_OFTEN\nBuffer.USAGE_CPU_READ_MASK\n");
        strcat(message, "Buffer.USAGE_CPU_WRITE_RARELY\nBuffer.USAGE_CPU_WRITE_OFTEN\n");
        showMessageError(env, message);
        return;
    }
    void *dataMemoryVirtual[1];
    if (fence >= 0) {
        jclass File = env->FindClass("java/io/File");
        jmethodID isFile = env->GetMethodID(File, "isFile", "()Z");
        jmethodID exists = env->GetMethodID(File, "exists", "()Z");
        jmethodID getAbsolutePath = env->GetMethodID(File, "getAbsolutePath", "()Ljava/lang/String;");
        jboolean _isFile = env->CallBooleanMethod((jobject)memoryVirtual, isFile);
        jboolean _exists = env->CallBooleanMethod((jobject)memoryVirtual, exists);
        jstring path = (jstring)env->CallObjectMethod((jobject)memoryVirtual, getAbsolutePath);
        if (!_isFile) {
            char *message = (char*)malloc(sizeof(char) * 135);
            strcpy(message, "You cannot use the lockAndGetInfo method of the Buffer object you are using because the memoryVirtual object you ");
            strcat(message, "provided is not a file");
            showMessageError(env, message);
            return;
        }
        if (!_exists) {
            char *message = (char*)malloc(sizeof(char) * 136);
            strcpy(message, "You cannot use the lockAndGetInfo method of the Buffer object you are using because the memoryVirtual object you ");
            strcat(message, "provided does not exist");
            showMessageError(env, message);
            return;
        }
        const char *_path = env->GetStringUTFChars(path, NULL);
        jint *fileDescription = (jint*)malloc(sizeof(jint));
        *fileDescription = open(_path, O_RDWR);
        if (*fileDescription == -1) {
            char *message = (char*)malloc(sizeof(char) * 158);
            strcpy(message, "You cannot use the lockAndGetInfo method of the Buffer object you are using because the memoryVirtual object you ");
            strcat(message, "provided cannot be opened to read or modify its content");
            showMessageError(env, message);
            return;
        }
        dataMemoryVirtual[0] = fileDescription;
        fence = *fileDescription;
    } else {
        jclass Object = env->FindClass("java/lang/Object");
        jclass Class = env->FindClass("java/lang/Class");
        jmethodID getClass = env->GetMethodID(Object, "getClass", "()Ljava/lang/Class;");
        jmethodID getName = env->GetMethodID(Class, "getName", "()Ljava/lang/String;");
        jmethodID isArray = env->GetMethodID(Class, "isArray", "()Z");
        jobject objectClass = env->CallObjectMethod((jobject)memoryVirtual, getClass);
        jstring nameClass = (jstring)env->CallObjectMethod(objectClass, getName);
        const char *_nameClass = env->GetStringUTFChars(nameClass, NULL);
        if (strcmp(_nameClass, "[I") == 0) dataMemoryVirtual[0] = env->GetIntArrayElements((jintArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[S") == 0) dataMemoryVirtual[0] = env->GetShortArrayElements((jshortArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[J") == 0) dataMemoryVirtual[0] = env->GetLongArrayElements((jlongArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[F") == 0) dataMemoryVirtual[0] = env->GetFloatArrayElements((jfloatArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[D") == 0) dataMemoryVirtual[0] = env->GetDoubleArrayElements((jdoubleArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[C") == 0) dataMemoryVirtual[0] = (char*)env->GetCharArrayElements((jcharArray)memoryVirtual, NULL);
        else if (strcmp(_nameClass, "[B") == 0) dataMemoryVirtual[0] = env->GetByteArrayElements((jbyteArray)memoryVirtual, NULL);
        else {
            char *message = (char*)malloc(sizeof(char) * 217);
            strcpy(message, "You cannot use the lockAndGetInfo method of the buffer object you are using because the memoryVirtual variable can ");
            strcat(message, "only contain a one-dimensional array of int, short, long, float, double, char and byte primitive types");
            showMessageError(env, message);
            return;
        }
    }
    if (bytesPerPixel <= 0 || bytesPerStride <= 0) {
        char *message = (char*)malloc(sizeof(char) * 171);
        strcpy(message, "You cannot use the lock method GetInfo of the buffer object you are using because the bytesPerPixel variable and/or the ");
        strcat(message, "bytesPerStride variable are less than or equal to 0");
        showMessageError(env, message);
        return;
    }
    AHardwareBuffer *_buffer = bufferList[_currentBuffer];
    int result = AHardwareBuffer_lock(_buffer, (uint64_t)usageType, (int32_t)fence, area, dataMemoryVirtual);
    if (result != 0) {
        char *message = (char*)malloc(sizeof(char) * 523);
        strcpy(message, "You cannot use the lockAndGetInfo method of the buffer object you are using because the buffer you are using has more ");
        strcat(message, "than 1 layer or your device ran out of memory, or the value of the bytesPerPixel variable does not correspond to the ");
        strcat(message, "format you chose when you created the buffer object with the allocate method or with the fromHardwareBuffer method, ");
        strcat(message, "another possible problem is the value of the variable bytesPerStride that does not correspond to the number of pixels ");
        strcat(message, "per row and the number of bytes per pixels of each row");
        showMessageError(env, message);
    }
#else
    char *message = (char*)malloc(sizeof(char) * 200);
    strcpy(message, "You cannot use the lockAndGetInfo method of the buffer object you are using because the version of android that your ");
    strcat(message, "android device has installed is lower than the version Q better known as android 10");
    showMessageError(env, message);
#endif
}
JNICALL jobject Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_getHardwareBuffer(JNIEnv *env, jobject buffer) {
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the getHardwareBuffer method of the Buffer object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 178);
        strcpy(message, "You cannot use the getHardwareBuffer method of the Buffer object you are using because it was not created with any of ");
        strcat(message, "the following methods:\ngetBuffer\nallocate\nfromHardwareBuffer");
        showMessageError(env, message);
        return NULL;
    }
    return AHardwareBuffer_toHardwareBuffer(env, bufferList[_currentBuffer]);
}
JNICALL jobjectArray Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_lockPlanes(JNIEnv *env, jobject buffer, jobject bufferArea, jlong usageType, jint fence) {
#if __ANDROID_API__ >= 29
    if (bufferArea == NULL) {
        char *message = (char*)malloc(sizeof(char) * 129);
        strcpy(message, "You cannot use the lockPlanes method of the Buffer object you are using because the bufferArea object you provided is ");
        strcat(message, "set to null");
        showMessageError(env, message);
        return NULL;
    }
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 93);
        strcpy(message, "You cannot use the lockPlanes method of the Buffer object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 171);
        strcpy(message, "You cannot use the lockPlanes method of the Buffer object you are using because it was not created with any of the following methods:\\ngetBuffer\\nallocate\\nfromHardwareBuffer");
        showMessageError(env, message);
        return NULL;
    }
    jclass BufferArea = env->GetObjectClass(bufferArea);
    jfieldID top = env->GetFieldID(BufferArea, "top", "I");
    jfieldID bottom = env->GetFieldID(BufferArea, "bottom", "I");
    jfieldID right = env->GetFieldID(BufferArea, "right", "I");
    jfieldID left = env->GetFieldID(BufferArea, "left", "I");
    jint _top = env->GetIntField(bufferArea, top);
    jint _bottom = env->GetIntField(bufferArea, bottom);
    jint _right = env->GetIntField(bufferArea, right);
    jint _left = env->GetIntField(bufferArea, left);
    if (_top < 0 || _bottom <= 0 || _right <= 0 || _left < 0) {
        char *message = (char*)malloc(sizeof(char) * 222);
        strcpy(message, "You cannot use the lockPlanes method of the Buffer object you are using because some of the data in the bufferArea ");
        strcat(message, "object that provided top and/or left are less than 0 and/or bottom and/or right are equal to or less than 0");
        showMessageError(env, message);
        return NULL;
    }
    ARect *area = (ARect*)malloc(sizeof(ARect));
    area->top = _top;
    area->bottom = _bottom;
    area->right = _right;
    area->left = _left;
    jfieldID USAGE_CPU_READ_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_RARELY", "J");
    jfieldID USAGE_CPU_READ_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_READ_OFTEN", "J");
    jfieldID USAGE_CPU_WRITE_RARELY = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_RARELY", "J");
    jfieldID USAGE_CPU_WRITE_OFTEN = env->GetStaticFieldID(Buffer, "USAGE_CPU_WRITE_OFTEN", "J");
    jlong _USAGE_CPU_READ_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_READ_RARELY);
    jlong _USAGE_CPU_READ_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_READ_OFTEN);
    jlong _USAGE_CPU_WRITE_RARELY = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_RARELY);
    jlong _USAGE_CPU_WRITE_OFTEN = env->GetStaticLongField(Buffer, USAGE_CPU_WRITE_OFTEN);
    jint _usageType = usageType;
    if ((_usageType & _USAGE_CPU_READ_RARELY) == _USAGE_CPU_READ_RARELY) _usageType -= _USAGE_CPU_READ_RARELY;
    if ((_usageType & _USAGE_CPU_READ_OFTEN) == _USAGE_CPU_READ_OFTEN) _usageType -= _USAGE_CPU_READ_OFTEN;
    if ((_usageType & _USAGE_CPU_WRITE_RARELY) == _USAGE_CPU_WRITE_RARELY) _usageType -= _USAGE_CPU_WRITE_RARELY;
    if ((_usageType & _USAGE_CPU_WRITE_OFTEN) == _USAGE_CPU_WRITE_OFTEN) _usageType -= _USAGE_CPU_WRITE_OFTEN;
    if (_usageType != 0) {
        char *message = (char*)malloc(sizeof(char) * 369);
        strcpy(message, "You cannot use the lockPlanes method of the Buffer object you are using because the usageType variable you provided has an ");
        strcat(message, "illegal value, you can only use one or more combined values from the following list of allowed values:\n");
        strcat(message, "Buffer.USAGE_CPU_READ_RARELY\nBuffer.USAGE_CPU_READ_OFTEN\nBuffer.USAGE_CPU_READ_MASK\n");
        strcat(message, "Buffer.USAGE_CPU_WRITE_RARELY\nBuffer.USAGE_CPU_WRITE_OFTEN\n");
        showMessageError(env, message);
        return NULL;
    }
    AHardwareBuffer_Planes *planes = (AHardwareBuffer_Planes*)malloc(sizeof(AHardwareBuffer_Planes));
    jint result = AHardwareBuffer_lockPlanes(bufferList[_currentBuffer], (uint64_t)usageType, (int32_t)fence, area, planes);
    if (result == 0) {
        jclass Plane = env->FindClass("com/draico/ia/neuralnetworks/hardware/Plane");
        jfieldID currentPlanes = env->GetFieldID(Plane, "currentPlanes", "I");
        jfieldID currentPlane = env->GetFieldID(Plane, "currentPlane", "I");
        jfieldID isCreated = env->GetFieldID(Plane, "isCreated", "Z");
        jsize sizeArray = (jint)planes->planeCount;
        jobjectArray planes = env->NewObjectArray(sizeArray, Plane, NULL);
        for (jint position = 0; position < sizeArray; position++) {
            jobject plane = env->AllocObject(Plane);
            env->SetIntField(plane, currentPlanes, currentPositionPlanes);
            env->SetIntField(plane, currentPlane, position);
            env->SetBooleanField(plane, isCreated, JNI_TRUE);
            env->SetObjectArrayElement(planes, position, plane);
        }
        currentPositionPlanes++;
        if (currentPositionPlanes == 12) currentPositionPlanes = 0;
        return planes;
    } else {
        char *message = (char*)malloc(sizeof(char) * 634);
        strcpy(message, "You cannot use the lockPlanes method of the Buffer object you are using because the buffer has more than 1 layer, your ");
        strcat(message, "device has run out of memory, or the value of the fence variable is not adequate since if you used the lock or ");
        strcat(message, "lockAndGetInfo method it must use the same fence value type, if you used a file descriptor use that file descriptor as ");
        strcat(message, "the value for the fence variable, if you used one-dimensional arrays of the primitive data type int, short, long, ");
        strcat(message, "float, double, char or byte the fence value must be negative, another possible problem is that the values that I ");
        strcat(message, "specify in the BufferArea object constructor are not valid");
        showMessageError(env, message);
        return NULL;
    }
#else
    char *message = (char*)malloc(sizeof(char) * 196);
    strcpy(message, "You cannot use the lockPlanes method of the Buffer object you are using because the version of android that your ");
    strcat(message, "android device has installed is lower than the version Q better known as android 10");
    showMessageError(env, message);
    return NULL;
#endif
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_sendHandleToUnixSocket(JNIEnv *env, jobject buffer, jint sockedFd) {
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 104);
        strcpy(message, "You cannot use the setHandleToUnixSocket method of the Buffer object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 182);
        strcpy(message, "You cannot use the setHandleToUnixSocket method of the Buffer object you are using because it was not created with any ");
        strcat(message, "of the following methods:\ngetBuffer\nallocate\nfromHardwareBuffer");
        showMessageError(env, message);
        return;
    }
    if (sockedFd < 0) {
        char *message = (char*)malloc(sizeof(char) * 100);
        strcpy(message, "You cannot use the setHandleToUnixSocket method of the Buffer object you are using because the value of the variable ");
        strcat(message, "sockedFd is less than 0");
        showMessageError(env, message);
        return;
    }
    jint result = AHardwareBuffer_sendHandleToUnixSocket(bufferList[_currentBuffer], sockedFd);
    if (result != 0) {
        char *message = (char*)malloc(sizeof(char) * 261);
        strcpy(message, "You cannot use the setHandleToUnixSocket method of the Buffer object you are using because there is possibly a problem ");
        strcat(message, "with the file you are handling with the file descriptor socket, or another kind of error associated with the file being ");
        strcat(message, "used as virtual memory");
        showMessageError(env, message);
    }
}
JNICALL void Java_com_draico_asvappra_neuralnetworks_hardware_Buffer_unlock(JNIEnv *env, jobject buffer, jint fence) {
    jclass Buffer = env->GetObjectClass(buffer);
    jfieldID currentBuffer = env->GetFieldID(Buffer, "currentBuffer", "I");
    jfieldID isCreated = env->GetFieldID(Buffer, "isCreated", "Z");
    jint _currentBuffer = env->GetIntField(buffer, currentBuffer);
    jboolean _isCreated = env->GetBooleanField(buffer, isCreated);
    if (_currentBuffer == -1) {
        char *message = (char*)malloc(sizeof(char) * 89);
        strcpy(message, "You cannot use the unlock method of the Buffer object you are using because it is invalid");
        showMessageError(env, message);
        return;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 167);
        strcpy(message, "You cannot use the unlock method of the Buffer object you are using because it was not created with any of the following ");
        strcat(message, "methods:\ngetBuffer\nallocate\nfromHardwareBuffer");
        showMessageError(env, message);
        return;
    }
    jint result = AHardwareBuffer_unlock(bufferList[_currentBuffer], (int32_t*)&fence);
    if (result != 0) {
        char *message = (char*)malloc(sizeof(char) * 311);
        strcpy(message, "You cannot use the unlock method of the Buffer object you are using because your device may have run out of memory, ");
        strcat(message, "there was a problem with the file you are using as virtual memory or with the one-dimensional array you have used instead of ");
        strcat(message, "the file, or there is another kind of problem related to file handling");
        showMessageError(env, message);
    }
}
JNICALL jint Java_com_draico_asvappra_neuralnetworks_hardware_Plane_getNumberBytesPerPixel(JNIEnv *env, jobject plane) {
    jclass Plane = env->GetObjectClass(plane);
    jfieldID currentPlanes = env->GetFieldID(Plane, "currentPlanes", "I");
    jfieldID currentPlane = env->GetFieldID(Plane, "currentPlane", "I");
    jfieldID isCreated = env->GetFieldID(Plane, "isCreated", "Z");
    jint _currentPlanes = env->GetIntField(plane, currentPlanes);
    jint _currentPlane = env->GetIntField(plane, currentPlane);
    jboolean _isCreated = env->GetBooleanField(plane, isCreated);
    if (_currentPlanes == -1) {
        char *message = (char*)malloc(sizeof(char) * 104);
        strcpy(message, "You cannot use the getNumberBytesPerPixel method of the Plane object you are using because it is invalid");
        showMessageError(env, message);
        return 0;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 155);
        strcpy(message, "You cannot use the getNumberBytesPerPixel method of the Plane object you are using because it was not created with the ");
        strcat(message, "lockPlanes method of a Buffer object");
        showMessageError(env, message);
        return 0;
    }
    AHardwareBuffer_Planes *planes = planesList[_currentPlanes];
    return (jint)planes->planes[_currentPlane].pixelStride;
}
JNICALL jint Java_com_draico_asvappra_neuralnetworks_hardware_Plane_getNumberBytesPerRow(JNIEnv *env, jobject plane) {
    jclass Plane = env->GetObjectClass(plane);
    jfieldID currentPlanes = env->GetFieldID(Plane, "currentPlanes", "I");
    jfieldID currentPlane = env->GetFieldID(Plane, "currentPlane", "I");
    jfieldID isCreated = env->GetFieldID(Plane, "isCreated", "Z");
    jint _currentPlanes = env->GetIntField(plane, currentPlanes);
    jint _currentPlane = env->GetIntField(plane, currentPlane);
    jboolean _isCreated = env->GetBooleanField(plane, isCreated);
    if (_currentPlanes == -1) {
        char *message = (char*)malloc(sizeof(char) * 102);
        strcpy(message, "You cannot use the getNumberBytesPerRow method of the Plane object you are using because it is invalid");
        showMessageError(env, message);
        return 0;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 153);
        strcpy(message, "You cannot use the getNumberBytesPerRow method of the Plane object you are using because it was not created with the ");
        strcat(message, "lockPlanes method of a Buffer object");
        showMessageError(env, message);
        return 0;
    }
    AHardwareBuffer_Planes *planes = planesList[_currentPlanes];
    return planes->planes[_currentPlane].rowStride;
}
JNICALL jbyteArray Java_com_draico_asvappra_neuralnetworks_hardware_Plane_getPixels(JNIEnv *env, jobject plane) {
    jclass Plane = env->GetObjectClass(plane);
    jfieldID currentPlanes = env->GetFieldID(Plane, "currentPlanes", "I");
    jfieldID currentPlane = env->GetFieldID(Plane, "currentPlane", "I");
    jfieldID isCreated = env->GetFieldID(Plane, "isCreated", "Z");
    jint _currentPlanes = env->GetIntField(plane, currentPlanes);
    jint _currentPlane = env->GetIntField(plane, currentPlane);
    jboolean _isCreated = env->GetBooleanField(plane, isCreated);
    if (_currentPlanes == -1) {
        char *message = (char*)malloc(sizeof(char) * 91);
        strcpy(message, "You cannot use the getPixels method of the Plane object you are using because it is invalid");
        showMessageError(env, message);
        return NULL;
    }
    if (!_isCreated) {
        char *message = (char*)malloc(sizeof(char) * 142);
        strcpy(message, "You cannot use the getPixels method of the Plane object you are using because it was not created with the lockPlanes ");
        strcat(message, "method of a Buffer object");
        showMessageError(env, message);
        return NULL;
    }
    jbyte *data = (jbyte*)planesList[_currentPlanes]->planes[_currentPlane].data;
    jsize sizeData = sizeof(data);
    if (sizeData == 0) return NULL;
    jbyteArray _data = env->NewByteArray(sizeData);
    env->SetByteArrayRegion(_data, 0, sizeData, data);
    return _data;
}
};