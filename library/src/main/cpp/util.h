#ifndef SESSION_ANDROID_UTIL_H
#define SESSION_ANDROID_UTIL_H

#include <jni.h>
#include <array>
#include <optional>
#include <span>
#include <vector>

#include "session/types.hpp"
#include "session/config/groups/info.hpp"
#include "session/config/groups/keys.hpp"
#include "session/config/groups/members.hpp"
#include "session/config/profile_pic.hpp"
#include "session/config/user_groups.hpp"
#include "session/config/expiring.hpp"

#include "jni_utils.h"

namespace util {
    jni_utils::JavaLocalRef<jbyteArray> bytes_from_vector(JNIEnv* env, const std::vector<unsigned char> &from_str);
    std::vector<unsigned char> vector_from_bytes(JNIEnv* env, jbyteArray byteArray);
    jni_utils::JavaLocalRef<jbyteArray> bytes_from_span(JNIEnv* env, std::span<const unsigned char> from_str);
    jni_utils::JavaLocalRef<jobject> serialize_user_pic(JNIEnv *env, session::config::profile_pic pic);
    session::config::profile_pic deserialize_user_pic(JNIEnv *env, jobject user_pic);

    jni_utils::JavaLocalRef<jobject> serialize_expiry(JNIEnv *env, const session::config::expiration_mode& mode, const std::chrono::seconds& time_seconds);
    std::pair<session::config::expiration_mode, long> deserialize_expiry(JNIEnv *env, jobject expiry_mode);

    jni_utils::JavaLocalRef<jobject> jlongFromOptional(JNIEnv* env, std::optional<long long> optional);
}

#endif