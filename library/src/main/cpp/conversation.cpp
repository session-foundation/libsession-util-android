#include <jni.h>
#include "conversation.h"
#include "jni_utils.h"
#include "user_groups.h"

using namespace jni_utils;


JavaLocalRef<jobject> serialize_pro_proof_info(JNIEnv *env,
                                 std::optional<std::span<const unsigned char>> gen_index_hash,
                                 const std::chrono::sys_time<std::chrono::milliseconds> & expiry) {
    if (!gen_index_hash) {
        return {env, nullptr};
    }

    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/Conversation$ProProofInfo",
            "([BJ)V"
    );

    return {env, env->NewObject(
            class_info.java_class,
            class_info.constructor,
            util::bytes_from_span(env, *gen_index_hash).get(),
            static_cast<jlong>(expiry.time_since_epoch().count())
    )};
}

JavaLocalRef<jobject> serialize_one_to_one(JNIEnv *env, const session::config::convo::one_to_one &one_to_one) {
    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/Conversation$OneToOne",
            "(Ljava/lang/String;JZLnetwork/loki/messenger/libsession_util/util/Conversation$ProProofInfo;)V"
    );

    return {env, env->NewObject(class_info.java_class,
                          class_info.constructor,
                          JavaLocalRef(env, env->NewStringUTF(one_to_one.session_id.data())).get(),
                          (jlong) one_to_one.last_read,
                          (jboolean) one_to_one.unread,
                          serialize_pro_proof_info(env, one_to_one.pro_gen_index_hash,
                                                   one_to_one.pro_expiry_unix_ts).get())};
}

session::config::convo::one_to_one deserialize_one_to_one(JNIEnv *env, jobject info) {
    struct ClassInfo {
        jclass java_class;
        jmethodID id_getter;
        jmethodID lastRead_getter;
        jmethodID unread_getter;

        ClassInfo(JNIEnv *env, jclass clazz)
            :java_class((jclass) env->NewGlobalRef(clazz)),
             id_getter(env->GetMethodID(clazz, "getAccountId", "()Ljava/lang/String;")),
             lastRead_getter(env->GetMethodID(clazz, "getLastRead", "()J")),
             unread_getter(env->GetMethodID(clazz, "getUnread", "()Z")) {}
    };

    static ClassInfo class_info(env, JavaLocalRef(env, env->GetObjectClass(info)).get());

    session::config::convo::one_to_one r(
            JavaStringRef(
                    env,
                    JavaLocalRef(env, (jstring)(env->CallObjectMethod(info, class_info.id_getter))).get()
            ).view()
        );

    r.last_read = env->CallLongMethod(info, class_info.lastRead_getter);
    r.unread = env->CallBooleanMethod(info, class_info.unread_getter);
    return r;
}

JavaLocalRef<jobject> serialize_community(JNIEnv *env, const session::config::convo::community& community) {
    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/Conversation$Community",
            "(Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;JZ)V"
    );

    return {env, env->NewObject(class_info.java_class,
                          class_info.constructor,
                          serialize_base_community(env, community).get(),
                          (jlong) community.last_read,
                          (jboolean) community.unread)};
}

session::config::convo::community deserialize_community(JNIEnv *env, jobject info) {
    struct ClassInfo {
        jclass java_class;
        jmethodID base_community_getter;
        jmethodID last_read_getter;
        jmethodID unread_getter;

        ClassInfo(JNIEnv *env, jclass clazz)
            :java_class((jclass) env->NewGlobalRef(clazz)),
             base_community_getter(env->GetMethodID(clazz, "getBaseCommunityInfo", "()Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;")),
             last_read_getter(env->GetMethodID(clazz, "getLastRead", "()J")),
             unread_getter(env->GetMethodID(clazz, "getUnread", "()Z")) {}
    };

    static ClassInfo class_info(env, JavaLocalRef(env, env->GetObjectClass(info)).get());

    auto base_community = deserialize_base_community(
            env,
            JavaLocalRef(env, env->CallObjectMethod(info, class_info.base_community_getter)).get());

    session::config::convo::community community(
            base_community.base_url(),
            base_community.room(),
            base_community.pubkey()
            );

    community.last_read = env->CallLongMethod(info, class_info.last_read_getter);
    community.unread = env->CallBooleanMethod(info, class_info.unread_getter);

    return community;
}


JavaLocalRef<jobject> serialize_legacy_group(JNIEnv *env, const session::config::convo::legacy_group& group) {
    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/Conversation$LegacyGroup",
            "(Ljava/lang/String;JZ)V"
    );

    return {env, env->NewObject(class_info.java_class,
                          class_info.constructor,
                          JavaLocalRef(env, env->NewStringUTF(group.id.data())).get(),
                          (jlong) group.last_read,
                          (jboolean) group.unread)};
}

session::config::convo::legacy_group deserialize_legacy_closed_group(JNIEnv *env, jobject info) {
    struct ClassInfo {
        jclass java_class;
        jmethodID groupId_getter;
        jmethodID lastRead_getter;
        jmethodID unread_getter;

        ClassInfo(JNIEnv *env, jclass clazz)
            :java_class((jclass) env->NewGlobalRef(clazz)),
             groupId_getter(env->GetMethodID(clazz, "getGroupId", "()Ljava/lang/String;")),
             lastRead_getter(env->GetMethodID(clazz, "getLastRead", "()J")),
             unread_getter(env->GetMethodID(clazz, "getUnread", "()Z")) {}
    };

    static ClassInfo class_info(env, JavaLocalRef(env, env->GetObjectClass(info)).get());

    session::config::convo::legacy_group lg(
            JavaStringRef(env, JavaLocalRef(env, static_cast<jstring>(env->CallObjectMethod(info, class_info.groupId_getter))).get()).view()
    );

    lg.last_read = env->CallLongMethod(info, class_info.lastRead_getter);
    lg.unread = env->CallBooleanMethod(info, class_info.unread_getter);
    return lg;
}

JavaLocalRef<jobject> serialize_closed_group(JNIEnv* env, const session::config::convo::group &group) {
    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/Conversation$ClosedGroup",
            "(Ljava/lang/String;JZ)V");

    return {env, env->NewObject(class_info.java_class,
                          class_info.constructor,
                          JavaLocalRef(env, env->NewStringUTF(group.id.data())).get(),
                          (jlong) group.last_read,
                          (jboolean) group.unread)};
}

session::config::convo::group deserialize_closed_group(JNIEnv* env, jobject info) {
    struct ClassInfo {
        jclass java_class;
        jmethodID id_getter;
        jmethodID last_read_getter;
        jmethodID unread_getter;

        ClassInfo(JNIEnv *env, jclass clazz)
        :java_class((jclass) env->NewGlobalRef(clazz)),
         id_getter(env->GetMethodID(clazz, "getAccountId", "()Ljava/lang/String;")),
         last_read_getter(env->GetMethodID(clazz, "getLastRead", "()J")),
         unread_getter(env->GetMethodID(clazz, "getUnread", "()Z")) {}
    };

    static ClassInfo class_info(env, JavaLocalRef(env, env->GetObjectClass(info)).get());

    session::config::convo::group g(
            JavaStringRef(env, JavaLocalRef(env, (jstring) env->CallObjectMethod(info, class_info.id_getter)).get()).view());

    g.last_read = env->CallLongMethod(info, class_info.last_read_getter);
    g.unread = env->CallBooleanMethod(info, class_info.unread_getter);

    return g;
}

JavaLocalRef<jobject> serialize_blinded_one_to_one(JNIEnv *env, const session::config::convo::blinded_one_to_one &blinded_one_to_one) {
    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/Conversation$BlindedOneToOne",
            "(Ljava/lang/String;JZLnetwork/loki/messenger/libsession_util/util/Conversation$ProProofInfo;)V");

    return {env, env->NewObject(
            class_info.java_class,
            class_info.constructor,
            JavaLocalRef(env, env->NewStringUTF(blinded_one_to_one.blinded_session_id.data())).get(),
            (jlong) blinded_one_to_one.last_read,
            (jboolean) blinded_one_to_one.unread
    )};
}

session::config::convo::blinded_one_to_one deserialize_blinded_one_to_one(JNIEnv *env, jobject info) {
    struct ClassInfo {
        jclass java_class;
        jmethodID id_getter;
        jmethodID last_read_getter;
        jmethodID unread_getter;

        ClassInfo(JNIEnv *env, jclass clazz)
            :java_class((jclass) env->NewGlobalRef(clazz)),
             id_getter(env->GetMethodID(clazz, "getBlindedAccountId", "()Ljava/lang/String;")),
             last_read_getter(env->GetMethodID(clazz, "getLastRead", "()J")),
             unread_getter(env->GetMethodID(clazz, "getUnread", "()Z")) {}
    };

    static ClassInfo class_info(env, JavaLocalRef(env, env->GetObjectClass(info)).get());

    session::config::convo::blinded_one_to_one r(
            JavaStringRef(env, JavaLocalRef(env, (jstring) env->CallObjectMethod(info, class_info.id_getter)).get()).view());

    r.last_read = env->CallLongMethod(info, class_info.last_read_getter);
    r.unread = env->CallBooleanMethod(info, class_info.unread_getter);

    return r;
}

JavaLocalRef<jobject> serialize_any(JNIEnv *env, session::config::convo::any any) {
    if (auto* dm = std::get_if<session::config::convo::one_to_one>(&any)) {
        return serialize_one_to_one(env, *dm);
    } else if (auto* og = std::get_if<session::config::convo::community>(&any)) {
        return serialize_community(env, *og);
    } else if (auto* lgc = std::get_if<session::config::convo::legacy_group>(&any)) {
        return serialize_legacy_group(env, *lgc);
    } else if (auto* gc = std::get_if<session::config::convo::group>(&any)) {
        return serialize_closed_group(env, *gc);
    } else if (auto *bc = std::get_if<session::config::convo::blinded_one_to_one>(&any)) {
        return serialize_blinded_one_to_one(env, *bc);
    }
    return {env, nullptr};
}


extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_sizeOneToOnes(JNIEnv *env,
                                                                                      jobject thiz) {
    auto conversations = ptrToConvoInfo(env, thiz);
    return conversations->size_1to1();
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseAll(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jobject predicate) {
    auto conversations = ptrToConvoInfo(env, thiz);

    JavaLocalRef<jclass> predicate_class(env, env->GetObjectClass(predicate));
    jmethodID predicate_call = env->GetMethodID(predicate_class.get(), "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");

    JavaLocalRef<jclass> bool_class(env, env->FindClass("java/lang/Boolean"));
    jmethodID bool_get = env->GetMethodID(bool_class.get(), "booleanValue", "()Z");

    int removed = 0;
    auto to_erase = std::vector<session::config::convo::any>();

    for (auto it = conversations->begin(); it != conversations->end(); ++it) {
        JavaLocalRef result(env, env->CallObjectMethod(predicate, predicate_call, serialize_any(env, *it).get()));
        bool bool_result = env->CallBooleanMethod(result.get(), bool_get);
        if (bool_result) {
            to_erase.push_back(*it);
        }
    }

    for (auto & entry : to_erase) {
        if (conversations->erase(entry)) {
            removed++;
        }
    }

    return removed;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_size(JNIEnv *env,
                                                                             jobject thiz) {
    auto config = ptrToConvoInfo(env, thiz);
    return (jint)config->size();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_empty(JNIEnv *env,
                                                                              jobject thiz) {
    auto config = ptrToConvoInfo(env, thiz);
    return config->empty();
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOneToOne(JNIEnv *env,
                                                                                    jobject thiz,
                                                                                    jstring pub_key_hex) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto internal = convos->get_1to1(JavaStringRef(env, pub_key_hex).view());
    if (internal) {
        return serialize_one_to_one(env, *internal).leak();
    }
    return nullptr;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructOneToOne(
        JNIEnv *env, jobject thiz, jstring pub_key_hex) {
    auto convos = ptrToConvoInfo(env, thiz);
    return serialize_one_to_one(env, convos->get_or_construct_1to1(JavaStringRef(env, pub_key_hex).view())).leak();
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseOneToOne(JNIEnv *env,
                                                                                      jobject thiz,
                                                                                      jstring pub_key_hex) {
    auto convos = ptrToConvoInfo(env, thiz);
    return convos->erase_1to1(JavaStringRef(env, pub_key_hex).view());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getCommunity__Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto open = convos->get_community(JavaStringRef(env, base_url).view(), JavaStringRef(env, room).view());
    if (open) {
        return serialize_community(env, *open).leak();
    }
    return nullptr;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructCommunity__Ljava_lang_String_2Ljava_lang_String_2_3B(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room, jbyteArray pub_key) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto community = convos->get_or_construct_community(
            JavaStringRef(env, base_url).view(),
            JavaStringRef(env, room).view(),
            JavaByteArrayRef(env, pub_key).get());
    return serialize_community(env, community).leak();
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructCommunity__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room, jstring pub_key_hex) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto community = convos->get_or_construct_community(
            JavaStringRef(env, base_url).view(),
            JavaStringRef(env, room).view(),
            JavaStringRef(env, pub_key_hex).view());
    return serialize_community(env, community).leak();
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseCommunity__Lnetwork_loki_messenger_libsession_1util_util_Conversation_Community_2(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jobject open_group) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto deserialized = deserialize_community(env, open_group);
    return convos->erase(deserialized);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseCommunity__Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room) {
    auto convos = ptrToConvoInfo(env, thiz);
    return convos->erase_community(
            JavaStringRef(env, base_url).view(),
            JavaStringRef(env, room).view());
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getLegacyClosedGroup(
        JNIEnv *env, jobject thiz, jstring group_id) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto lgc = convos->get_legacy_group(JavaStringRef(env, group_id).view());
    if (lgc) {
        return serialize_legacy_group(env, *lgc).leak();
    }

    return nullptr;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructLegacyGroup(
        JNIEnv *env, jobject thiz, jstring group_id) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto lgc = convos->get_or_construct_legacy_group(JavaStringRef(env, group_id).view());
    return serialize_legacy_group(env, lgc).leak();
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseLegacyClosedGroup(
        JNIEnv *env, jobject thiz, jstring group_id) {
    auto convos = ptrToConvoInfo(env, thiz);
    return convos->erase_legacy_group(JavaStringRef(env, group_id).view());
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_setLegacyGroup(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jobject o) {
    ptrToConvoInfo(env, thiz)->set(deserialize_legacy_closed_group(env, o));
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_sizeCommunities(JNIEnv *env,
                                                                                       jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return convos->size_communities();
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_sizeLegacyClosedGroups(
        JNIEnv *env, jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return convos->size_legacy_groups();
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_all(JNIEnv *env,
                                                                            jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);

    return jlist_from_collection(env, *convos, serialize_any);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allOneToOnes(JNIEnv *env,
                                                                                     jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jlist_from_iterator(env, convos->begin_1to1(), convos->end(),
                                          serialize_one_to_one);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allCommunities(JNIEnv *env,
                                                                                      jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jlist_from_iterator(env, convos->begin_communities(), convos->end(),
                                          serialize_community);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allLegacyClosedGroups(
        JNIEnv *env, jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jlist_from_iterator(env, convos->begin_legacy_groups(), convos->end(),
                                          serialize_legacy_group);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allClosedGroups(JNIEnv *env,
                                                                                        jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jlist_from_iterator(env, convos->begin_groups(), convos->end(),
                                          serialize_closed_group);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getClosedGroup(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jstring session_id) {
    auto config = ptrToConvoInfo(env, thiz);
    auto group = config->get_group(JavaStringRef(env, session_id).view());
    if (group) {
        return serialize_closed_group(env, *group).leak();
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructClosedGroup(
        JNIEnv *env, jobject thiz, jstring session_id) {
    auto config = ptrToConvoInfo(env, thiz);
    auto group = config->get_or_construct_group(JavaStringRef(env, session_id).view());
    return serialize_closed_group(env, group).leak();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseClosedGroup(
        JNIEnv *env, jobject thiz, jstring session_id) {
    auto config = ptrToConvoInfo(env, thiz);
    return config->erase_group(JavaStringRef(env, session_id).view());
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_setOneToOne(
        JNIEnv *env, jobject thiz, jobject o) {
    ptrToConvoInfo(env, thiz)->set(deserialize_one_to_one(env, o));
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_setCommunity(
        JNIEnv *env, jobject thiz, jobject o) {
    ptrToConvoInfo(env, thiz)->set(deserialize_community(env, o));
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_setLegacyGruop(
        JNIEnv *env, jobject thiz, jobject o) {
    ptrToConvoInfo(env, thiz)->set(deserialize_legacy_closed_group(env, o));
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_setClosedGroup(
        JNIEnv *env, jobject thiz, jobject o) {
    ptrToConvoInfo(env, thiz)->set(deserialize_closed_group(env, o));
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_setBlindedOneToOne(
        JNIEnv *env, jobject thiz, jobject o) {
    ptrToConvoInfo(env, thiz)->set(deserialize_blinded_one_to_one(env, o));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructedBlindedOneToOne(
        JNIEnv *env, jobject thiz, jstring blinded_id) {
    return serialize_blinded_one_to_one(
            env,
            ptrToConvoInfo(env, thiz)->get_or_construct_blinded_1to1(JavaStringRef(env, blinded_id).view())).get();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseBlindedOneToOne(
        JNIEnv *env, jobject thiz, jstring blinded_id) {
    return ptrToConvoInfo(env, thiz)->erase_blinded_1to1(JavaStringRef(env, blinded_id).view());
}

