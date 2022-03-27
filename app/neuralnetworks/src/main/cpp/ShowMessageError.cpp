#include "ShowMessageError.h"

void showMessageError(JNIEnv *env, char *message) {
    jclass MessageError = env->FindClass("com/draico/asvappra/opencl/tools/MessageError");
    jmethodID messageError = env->GetStaticMethodID(MessageError, "showMessage", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(MessageError, messageError, env->NewStringUTF(message));
}