#ifndef SESSION_ANDROID_GROUP_KEYS_H
#define SESSION_ANDROID_GROUP_KEYS_H

#include "util.h"
#include "jni_utils.h"

inline session::config::groups::Keys* ptrToKeys(JNIEnv* env, jobject obj) {
    auto configClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/GroupKeysConfig"));
    jfieldID pointerField = env->GetFieldID(configClass.get(), "pointer", "J");
    return (session::config::groups::Keys*) env->GetLongField(obj, pointerField);
}

#endif //SESSION_ANDROID_GROUP_KEYS_H
