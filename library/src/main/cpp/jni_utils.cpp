#include "jni_utils.h"

namespace jni_utils {
    jobject new_kotlin_pair(JNIEnv *env, jobject first, jobject second) {
        auto pair_class = JavaLocalRef(env, env->FindClass("kotlin/Pair"));
        jmethodID constructor = env->GetMethodID(pair_class.get(), "<init>", "(Ljava/lang/Object;Ljava/lang/Object;)V");
        return env->NewObject(pair_class.get(), constructor, first, second);
    }

    jobject new_key_pair(JNIEnv *env, jbyteArray pubKey, jbyteArray secKey) {
        auto kp_class = JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/KeyPair"));
        jmethodID kp_constructor = env->GetMethodID(kp_class.get(), "<init>", "([B[B)V");
        return env->NewObject(kp_class.get(), kp_constructor, pubKey, secKey);
    }
}