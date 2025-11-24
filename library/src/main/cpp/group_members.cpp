#include "group_members.h"

#include "jni_utils.h"

using namespace jni_utils;

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_00024Companion_newInstance(
        JNIEnv *env, jobject thiz, jbyteArray pub_key, jbyteArray secret_key,
        jbyteArray initial_dump) {
    auto pub_key_bytes = util::vector_from_bytes(env, pub_key);
    std::optional<std::vector<unsigned char>> secret_key_optional{std::nullopt};
    std::optional<std::vector<unsigned char>> initial_dump_optional{std::nullopt};
    if (secret_key && env->GetArrayLength(secret_key) > 0) {
        auto secret_key_bytes = util::vector_from_bytes(env, secret_key);
        secret_key_optional = secret_key_bytes;
    }
    if (initial_dump && env->GetArrayLength(initial_dump) > 0) {
        auto initial_dump_bytes = util::vector_from_bytes(env, initial_dump);
        initial_dump_optional = initial_dump_bytes;
    }

    auto* group_members = new session::config::groups::Members(pub_key_bytes, secret_key_optional, initial_dump_optional);
    return reinterpret_cast<jlong>(group_members);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_all(JNIEnv *env, jobject thiz) {
    auto config = ptrToMembers(env, thiz);
    return jlist_from_collection(env, *config, serialize_group_member);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_erase(JNIEnv *env, jobject thiz, jstring pub_key_hex) {
    auto config = ptrToMembers(env, thiz);
    auto erased = config->erase(JavaStringRef(env, pub_key_hex).view());
    return erased;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_get(JNIEnv *env, jobject thiz,
                                                                    jstring pub_key_hex) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() -> jobject {
        auto config = ptrToMembers(env, thiz);
        auto member = config->get(JavaStringRef(env, pub_key_hex).view());
        if (!member) {
            return nullptr;
        }
        return serialize_group_member(env, *member).leak();
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_getOrConstruct(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jstring pub_key_hex) {
    auto config = ptrToMembers(env, thiz);
    auto member = config->get_or_construct(JavaStringRef(env, pub_key_hex).view());
    return serialize_group_member(env, member).leak();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_set(JNIEnv *env, jobject thiz,
                                                                    jobject group_member) {
    ptrToMembers(env, thiz)->set(*ptrToMember(env, group_member));
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setInvited(JNIEnv *env,
                                                                         jobject thiz) {
    ptrToMember(env, thiz)->invite_status = session::config::groups::STATUS_NOT_SENT;
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setInviteSent(JNIEnv *env,
                                                                            jobject thiz) {
    ptrToMember(env, thiz)->set_invite_sent();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setInviteFailed(JNIEnv *env,
                                                                              jobject thiz) {
    ptrToMember(env, thiz)->set_invite_failed();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setInviteAccepted(JNIEnv *env,
                                                                          jobject thiz) {
    ptrToMember(env, thiz)->set_invite_accepted();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setPromoted(JNIEnv *env,
                                                                          jobject thiz) {
    ptrToMember(env, thiz)->set_promoted();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setPromotionSent(JNIEnv *env,
                                                                               jobject thiz) {
    ptrToMember(env, thiz)->set_promotion_sent();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setPromotionFailed(JNIEnv *env,
                                                                                 jobject thiz) {
    ptrToMember(env, thiz)->set_promotion_failed();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setPromotionAccepted(JNIEnv *env,
                                                                                   jobject thiz) {
    ptrToMember(env, thiz)->set_promotion_accepted();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setRemoved(JNIEnv *env, jobject thiz,
                                                                         jboolean also_remove_messages) {
    ptrToMember(env, thiz)->set_removed(also_remove_messages);
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setName(JNIEnv *env, jobject thiz,
                                                                      jstring name) {
    ptrToMember(env, thiz)->set_name(std::string(JavaStringRef(env, name).view()));
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_nameString(JNIEnv *env,
                                                                         jobject thiz) {
    return jni_utils::jstring_from_optional(env, ptrToMember(env, thiz)->name).leak();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_isAdmin(JNIEnv *env, jobject thiz) {
    return ptrToMember(env, thiz)->admin;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_isSupplement(JNIEnv *env,
                                                                           jobject thiz) {
    return ptrToMember(env, thiz)->supplement;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_accountId(JNIEnv *env,
                                                                              jobject thiz) {
    return jni_utils::jstring_from_optional(env, ptrToMember(env, thiz)->session_id).leak();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_destroy(JNIEnv *env, jobject thiz) {
    delete ptrToMember(env, thiz);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_profilePic(JNIEnv *env,
                                                                         jobject thiz) {
    return util::serialize_user_pic(env, ptrToMember(env, thiz)->profile_picture).leak();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setProfilePic(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jobject pic) {
    ptrToMember(env, thiz)->profile_picture = util::deserialize_user_pic(env, pic);
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupMember_setSupplement(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jboolean supplement) {
    ptrToMember(env, thiz)->supplement = supplement;
}


extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_statusInt(JNIEnv *env, jobject thiz,
                                                                       jobject group_member) {
    return static_cast<jint>(ptrToMembers(env, thiz)->get_status(*ptrToMember(env, group_member)));
}
extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupMembersConfig_setPendingSend(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jstring pub_key_hex,
                                                                               jboolean pending) {
    ptrToMembers(env, thiz)->set_pending_send(JavaStringRef(env, pub_key_hex).copy(), pending);
}

JavaLocalRef<jobject> serialize_group_member(JNIEnv* env, const session::config::groups::member& member) {
    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/GroupMember",
            "(J)V");

    return {env, env->NewObject(
                          class_info.java_class,
                          class_info.constructor,
                          reinterpret_cast<jlong>(new session::config::groups::member(member))
    )};
}