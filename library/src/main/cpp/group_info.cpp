#include <session/config/groups/info.hpp>
#include <jni.h>

#include "util.h"
#include "jni_utils.h"
#include "config_base.h"

inline auto ptrToInfo(JNIEnv* env, jobject obj) {
    return dynamic_cast<session::config::groups::Info*>(ptrToConfigBase(env, obj));
}


extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_00024Companion_newInstance(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jbyteArray pub_key,
                                                                                        jbyteArray secret_key,
                                                                                        jbyteArray initial_dump) {
    std::optional<std::vector<unsigned char>> secret_key_optional{std::nullopt};
    std::optional<std::vector<unsigned char>> initial_dump_optional{std::nullopt};
    auto pub_key_bytes = util::vector_from_bytes(env, pub_key);
    if (secret_key && env->GetArrayLength(secret_key) > 0) {
        auto secret_key_bytes = util::vector_from_bytes(env, secret_key);
        secret_key_optional = secret_key_bytes;
    }
    if (initial_dump && env->GetArrayLength(initial_dump) > 0) {
        auto initial_dump_bytes = util::vector_from_bytes(env, initial_dump);
        initial_dump_optional = initial_dump_bytes;
    }

    auto* group_info = new session::config::groups::Info(pub_key_bytes, secret_key_optional, initial_dump_optional);
    return reinterpret_cast<jlong>(group_info);
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_destroyGroup(JNIEnv *env,
                                                                          jobject thiz) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->destroy_group();
    });
}


extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_getCreated(JNIEnv *env, jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return util::jlongFromOptional(env, group_info->get_created()).release();
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_getDeleteAttachmentsBefore(JNIEnv *env,
                                                                                        jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return util::jlongFromOptional(env, group_info->get_delete_attach_before()).release();
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_getDeleteBefore(JNIEnv *env,
                                                                             jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return util::jlongFromOptional(env, group_info->get_delete_before()).release();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_getExpiryTimer(JNIEnv *env,
                                                                            jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    auto timer = group_info->get_expiry_timer();
    if (!timer) {
        return 0;
    }
    long long in_seconds = timer->count();
    return in_seconds;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_getName(JNIEnv *env, jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return jni_utils::jstring_from_optional(env, group_info->get_name()).release();
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_getProfilePic(JNIEnv *env,
                                                                           jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return util::serialize_user_pic(env, group_info->get_profile_pic()).release();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_isDestroyed(JNIEnv *env,
                                                                         jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return group_info->is_destroyed();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_setCreated(JNIEnv *env, jobject thiz,
                                                                        jlong created_at) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->set_created(created_at);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_setDeleteAttachmentsBefore(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jlong delete_before) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->set_delete_attach_before(delete_before);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_setDeleteBefore(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jlong delete_before) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->set_delete_before(delete_before);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_setExpiryTimer(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jlong  expire_seconds) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->set_expiry_timer(std::chrono::seconds{expire_seconds});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_setName(JNIEnv *env, jobject thiz,
                                                                     jstring new_name) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->set_name(jni_utils::JavaStringRef(env, new_name).view());
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_setProfilePic(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jobject new_profile_pic) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->set_profile_pic(util::deserialize_user_pic(env, new_profile_pic));
    });
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_storageNamespace(JNIEnv *env,
                                                                              jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return static_cast<jlong>(group_info->storage_namespace());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_id(JNIEnv *env, jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    return jni_utils::jstring_from_optional(env, group_info->id).release();
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_getDescription(JNIEnv *env,
                                                                            jobject thiz) {
    auto group_info = ptrToInfo(env, thiz);
    auto description = group_info->get_description();
    if (!description) {
        return nullptr;
    }
    return env->NewStringUTF(description->data());
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupInfoConfig_setDescription(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jstring new_description) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto group_info = ptrToInfo(env, thiz);
        group_info->set_description(jni_utils::JavaStringRef(env, new_description).view());
    });
}