
#ifndef SESSION_ANDROID_USER_GROUPS_H
#define SESSION_ANDROID_USER_GROUPS_H

#include "jni_utils.h"
#include <session/config/user_groups.hpp>

jni_utils::JavaLocalRef<jobject> serialize_base_community(JNIEnv *env, const session::config::community& base_community);
session::config::community deserialize_base_community(JNIEnv *env, jobject base_community);

#endif //SESSION_ANDROID_USER_GROUPS_H
