#ifndef SESSION_ANDROID_GROUP_MEMBERS_H
#define SESSION_ANDROID_GROUP_MEMBERS_H

#include "util.h"
#include "jni_utils.h"

inline session::config::groups::Members* ptrToMembers(JNIEnv* env, jobject obj) {
    jfieldID pointerField = env->GetFieldID(jni_utils::JavaLocalRef(env, env->GetObjectClass(obj)).get(), "pointer", "J");
    return (session::config::groups::Members*) env->GetLongField(obj, pointerField);
}

inline session::config::groups::member *ptrToMember(JNIEnv *env, jobject thiz) {
    auto ptrField = env->GetFieldID(jni_utils::JavaLocalRef(env, env->GetObjectClass(thiz)).get(), "nativePtr", "J");
    return reinterpret_cast<session::config::groups::member*>(env->GetLongField(thiz, ptrField));
}


#endif //SESSION_ANDROID_GROUP_MEMBERS_H
