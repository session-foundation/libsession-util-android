#ifndef SESSION_ANDROID_CONFIG_BASE_H
#define SESSION_ANDROID_CONFIG_BASE_H

#include "session/config/base.hpp"
#include "util.h"
#include "jni_utils.h"
#include <jni.h>
#include <string>
#include <vector>

inline session::config::ConfigBase* ptrToConfigBase(JNIEnv *env, jobject obj) {
    auto baseClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/ConfigBase"));
    jfieldID pointerField = env->GetFieldID(baseClass.get(), "pointer", "J");
    return (session::config::ConfigBase*) env->GetLongField(obj, pointerField);
}

inline std::pair<std::string, std::vector<unsigned char>> extractHashAndData(JNIEnv *env, jobject kotlin_pair) {
    auto pair = jni_utils::JavaLocalRef(env, env->FindClass("kotlin/Pair"));
    jfieldID first = env->GetFieldID(pair.get(), "first", "Ljava/lang/Object;");
    jfieldID second = env->GetFieldID(pair.get(), "second", "Ljava/lang/Object;");
    auto hash_as_jstring = jni_utils::JavaLocalRef(env, reinterpret_cast<jstring>(env->GetObjectField(kotlin_pair, first)));
    auto data_as_jbytes = jni_utils::JavaLocalRef(env, reinterpret_cast<jbyteArray>(env->GetObjectField(kotlin_pair, second)));

    return std::make_pair(
            std::string(jni_utils::JavaStringRef(env, hash_as_jstring.get()).view()),
            jni_utils::JavaByteArrayRef(env, data_as_jbytes.get()).copy()
        );
}

inline session::config::ConfigSig* ptrToConfigSig(JNIEnv* env, jobject obj) {
    auto sigClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/ConfigSig"));
    jfieldID pointerField = env->GetFieldID(sigClass.get(), "pointer", "J");
    return (session::config::ConfigSig*) env->GetLongField(obj, pointerField);
}

#endif