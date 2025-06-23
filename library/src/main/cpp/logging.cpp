#include <jni.h>
#include <string_view>
#include <functional>

#include "session/logging.hpp"
#include "session/log_level.h"

#include "jni_utils.h"

using namespace jni_utils;

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_Logger_00024Companion_addLogger(JNIEnv *env, jclass clazz,
                                                                   jobject logger) {
    JavaVM *vm = nullptr;
    env->GetJavaVM(&vm);
    if (!vm) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), "Failed to get JavaVM");
        return;
    }

    jmethodID logMethod = env->GetMethodID(env->GetObjectClass(logger), "log",
                                  "(Ljava/lang/String;Ljava/lang/String;I)V");

    auto loggerRef = env->NewGlobalRef(logger);
    session::add_logger([vm, logMethod, loggerRef](std::string_view msg, std::string_view category, session::LogLevel level) {
        JNIEnv *env = nullptr;
        vm->AttachCurrentThread(&env, nullptr);
        if (!env) {
            return; // Failed to attach thread, cannot log
        }

        env->CallVoidMethod(loggerRef, logMethod,
                            JavaLocalRef(env, env->NewStringUTF(msg.data())).get(),
                            JavaLocalRef(env, env->NewStringUTF(category.data())).get(),
                            static_cast<jint>(level.level)
        );
    });
}
