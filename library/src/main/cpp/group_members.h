#ifndef SESSION_ANDROID_GROUP_MEMBERS_H
#define SESSION_ANDROID_GROUP_MEMBERS_H

#include "jni_utils.h"

#include <session/config/groups/members.hpp>


jni_utils::JavaLocalRef<jobject> serialize_group_member(JNIEnv* env, const session::config::groups::member& member);

#endif //SESSION_ANDROID_GROUP_MEMBERS_H
