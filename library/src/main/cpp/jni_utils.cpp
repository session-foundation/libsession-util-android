#include "jni_utils.h"

namespace jni_utils {
    jobject new_kotlin_pair(JNIEnv *env, jobject first, jobject second) {
        auto pair_class = JavaLocalRef(env, env->FindClass("kotlin/Pair"));
        jmethodID constructor = env->GetMethodID(pair_class.get(), "<init>", "(Ljava/lang/Object;Ljava/lang/Object;)V");
        return env->NewObject(pair_class.get(), constructor, first, second);
    }
}