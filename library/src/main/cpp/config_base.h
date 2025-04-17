#ifndef SESSION_ANDROID_CONFIG_BASE_H
#define SESSION_ANDROID_CONFIG_BASE_H

#include "session/config/base.hpp"
#include "util.h"
#include "jni_utils.h"
#include <jni.h>
#include <string>

inline session::config::ConfigBase* ptrToConfigBase(JNIEnv *env, jobject obj) {
    auto baseClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/ConfigBase"));
    jfieldID pointerField = env->GetFieldID(baseClass.get(), "pointer", "J");
    return (session::config::ConfigBase*) env->GetLongField(obj, pointerField);
}

inline std::pair<std::string, std::vector<unsigned char>> extractHashAndData(JNIEnv *env, jobject kotlin_pair) {
    auto pair = jni_utils::JavaLocalRef(env, env->FindClass("kotlin/Pair"));
    jfieldID first = env->GetFieldID(pair.get(), "first", "Ljava/lang/Object;");
    jfieldID second = env->GetFieldID(pair.get(), "second", "Ljava/lang/Object;");
    auto hash_as_jstring = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(kotlin_pair, first)));
    auto data_as_jbytes = jni_utils::JavaLocalRef(env, static_cast<jbyteArray>(env->GetObjectField(kotlin_pair, second)));
    auto hash_as_string = env->GetStringUTFChars(hash_as_jstring.get(), nullptr);
    auto data_as_vector = util::vector_from_bytes(env, data_as_jbytes.get());
    auto ret_pair = std::pair<std::string, std::vector<unsigned char>>{hash_as_string, data_as_vector};
    env->ReleaseStringUTFChars(hash_as_jstring.get(), hash_as_string);
    return ret_pair;
}

inline session::config::ConfigSig* ptrToConfigSig(JNIEnv* env, jobject obj) {
    auto sigClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/ConfigSig"));
    jfieldID pointerField = env->GetFieldID(sigClass.get(), "pointer", "J");
    return (session::config::ConfigSig*) env->GetLongField(obj, pointerField);
}

#endif