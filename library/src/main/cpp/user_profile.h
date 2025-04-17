#ifndef SESSION_ANDROID_USER_PROFILE_H
#define SESSION_ANDROID_USER_PROFILE_H

#include "session/config/user_profile.hpp"
#include <jni.h>
#include <string>

#include "jni_utils.h"

inline session::config::UserProfile* ptrToProfile(JNIEnv* env, jobject obj) {
    auto configClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/UserProfile"));
    jfieldID pointerField = env->GetFieldID(configClass.get(), "pointer", "J");
    return (session::config::UserProfile*) env->GetLongField(obj, pointerField);
}

#endif