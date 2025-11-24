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


inline session::config::ConfigSig* ptrToConfigSig(JNIEnv* env, jobject obj) {
    auto sigClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/ConfigSig"));
    jfieldID pointerField = env->GetFieldID(sigClass.get(), "pointer", "J");
    return (session::config::ConfigSig*) env->GetLongField(obj, pointerField);
}

#endif