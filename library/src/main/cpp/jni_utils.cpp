#include "jni_utils.h"

namespace jni_utils {
    jobject new_kotlin_pair(JNIEnv *env, jobject first, jobject second) {
        static BasicJavaClassInfo info(env, "kotlin/Pair", "(Ljava/lang/Object;Ljava/lang/Object;)V");
        return env->NewObject(info.java_class, info.constructor, first, second);
    }

    jobject new_key_pair(JNIEnv *env, jbyteArray pubKey, jbyteArray secKey) {
        static BasicJavaClassInfo class_info(
                env,
                "network/loki/messenger/libsession_util/util/KeyPair",
                "([B[B)V"
        );

        return env->NewObject(class_info.java_class, class_info.constructor, pubKey, secKey);
    }

    const ArrayListClassInfo & ArrayListClassInfo::get(JNIEnv *env) {
        static ArrayListClassInfo instance(env);
        return instance;
    }

    ArrayListClassInfo::ArrayListClassInfo(JNIEnv *env)
            :BasicJavaClassInfo(env, "java/util/ArrayList", "()V"),
             add_method(env->GetMethodID(java_class, "add", "(Ljava/lang/Object;)Z")) {}
}
