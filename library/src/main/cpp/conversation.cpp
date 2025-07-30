#include <jni.h>
#include "conversation.h"
#include "jni_utils.h"


 jobject serialize_one_to_one(JNIEnv *env, const session::config::convo::one_to_one &one_to_one) {
    jni_utils::JavaLocalRef clazz(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$OneToOne"));
    return env->NewObject(clazz.get(),
                          env->GetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;JZ)V"),
                          jni_utils::JavaLocalRef(env, env->NewStringUTF(one_to_one.session_id.data())).get(),
                          (jlong) one_to_one.last_read,
                          (jboolean) one_to_one.unread);
}

session::config::convo::one_to_one deserialize_one_to_one(JNIEnv *env, jobject info) {
    jni_utils::JavaLocalRef clazz(env, env->GetObjectClass(info));

    auto id_getter = env->GetFieldID(clazz.get(), "accountId", "Ljava/lang/String;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");

    session::config::convo::one_to_one r(
            jni_utils::JavaStringRef(env, jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, id_getter))).get()).view()
        );

    r.last_read = env->GetLongField(info, last_read_getter);
    r.unread = env->GetBooleanField(info, unread_getter);
    return r;
}

jobject serialize_community(JNIEnv *env, const session::config::convo::community& community) {
    jni_utils::JavaLocalRef clazz(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$Community"));
    return env->NewObject(clazz.get(),
                          env->GetMethodID(clazz.get(), "<init>",
                                           "(Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;JZ)V"),
                          jni_utils::JavaLocalRef(env, util::serialize_base_community(env, community)).get(),
                          (jlong) community.last_read,
                          (jboolean) community.unread);
}

session::config::convo::community deserialize_community(JNIEnv *env, jobject info) {
    jni_utils::JavaLocalRef clazz(env, env->GetObjectClass(info));
    auto base_community_getter = env->GetFieldID(clazz.get(), "baseCommunityInfo", "Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");

    auto base_community = util::deserialize_base_community(env, jni_utils::JavaLocalRef(env, env->GetObjectField(info, base_community_getter)).get());

    session::config::convo::community community(
            base_community.base_url(),
            base_community.room(),
            base_community.pubkey()
            );

    community.last_read = env->GetLongField(info, last_read_getter);
    community.unread = env->GetBooleanField(info, unread_getter);

    return community;
}


jobject serialize_legacy_group(JNIEnv *env, const session::config::convo::legacy_group& group) {
    jni_utils::JavaLocalRef clazz(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$LegacyGroup"));
    return env->NewObject(clazz.get(),
                          env->GetMethodID(clazz.get(), "<init>","(Ljava/lang/String;JZ)V"),
                          jni_utils::JavaLocalRef(env, env->NewStringUTF(group.id.data())).get(),
                          (jlong) group.last_read,
                          (jboolean) group.unread);
}

session::config::convo::legacy_group deserialize_legacy_closed_group(JNIEnv *env, jobject info) {
    jni_utils::JavaLocalRef clazz(env, env->GetObjectClass(info));
    auto group_id_getter = env->GetFieldID(clazz.get(), "groupId", "Ljava/lang/String;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");

    session::config::convo::legacy_group lg(
            jni_utils::JavaStringRef(env, jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, group_id_getter))).get()).view()
    );

    lg.last_read = env->GetLongField(info, last_read_getter);
    lg.unread = env->GetBooleanField(info, unread_getter);
    return lg;
}

jobject serialize_closed_group(JNIEnv* env, const session::config::convo::group &group) {
    jni_utils::JavaLocalRef clazz(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$ClosedGroup"));
    return env->NewObject(clazz.get(),
                          env->GetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;JZ)V"),
                          jni_utils::JavaLocalRef(env, env->NewStringUTF(group.id.data())).get(),
                          (jlong) group.last_read,
                          (jboolean) group.unread);
}

session::config::convo::group deserialize_closed_group(JNIEnv* env, jobject info) {
    jni_utils::JavaLocalRef clazz(env, env->GetObjectClass(info));
    auto id_getter = env->GetFieldID(clazz.get(), "accountId", "Ljava/lang/String;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");

    session::config::convo::group g(
            jni_utils::JavaStringRef(env, jni_utils::JavaLocalRef(env, (jstring) env->GetObjectField(info, id_getter)).get()).view());

    g.last_read = env->GetLongField(info, last_read_getter);
    g.unread = env->GetBooleanField(info, unread_getter);

    return g;
}

jobject serialize_blinded_one_to_one(JNIEnv *env, const session::config::convo::blinded_one_to_one &blinded_one_to_one) {
    jni_utils::JavaLocalRef clazz(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$BlindedOneToOne"));
    return env->NewObject(
            clazz.get(),
            env->GetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;JZ)V"),
            jni_utils::JavaLocalRef(env, env->NewStringUTF(blinded_one_to_one.blinded_session_id.data())).get(),
            (jlong) blinded_one_to_one.last_read,
            (jboolean) blinded_one_to_one.unread
    );
}

session::config::convo::blinded_one_to_one deserialize_blinded_one_to_one(JNIEnv *env, jobject info) {
    jni_utils::JavaLocalRef clazz(env, env->GetObjectClass(info));
    auto id_field_id = env->GetFieldID(clazz.get(), "blindedAccountId", "Ljava/lang/String;");
    auto last_read_field_id = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_field_id = env->GetFieldID(clazz.get(), "unread", "Z");

    session::config::convo::blinded_one_to_one r(
            jni_utils::JavaStringRef(env, jni_utils::JavaLocalRef(env, (jstring) env->GetObjectField(info, id_field_id)).get()).view(), true);

    r.last_read = env->GetLongField(info, last_read_field_id);
    r.unread = env->GetBooleanField(info, unread_field_id);

    return r;
}

jobject serialize_any(JNIEnv *env, session::config::convo::any any) {
    if (auto* dm = std::get_if<session::config::convo::one_to_one>(&any)) {
        return serialize_one_to_one(env, *dm);
    } else if (auto* og = std::get_if<session::config::convo::community>(&any)) {
        return serialize_community(env, *og);
    } else if (auto* lgc = std::get_if<session::config::convo::legacy_group>(&any)) {
        return serialize_legacy_group(env, *lgc);
    } else if (auto* gc = std::get_if<session::config::convo::group>(&any)) {
        return serialize_closed_group(env, *gc);
    }
    return nullptr;
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

    jni_utils::JavaLocalRef<jclass> predicate_class(env, env->GetObjectClass(predicate));
    jmethodID predicate_call = env->GetMethodID(predicate_class.get(), "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");

    jni_utils::JavaLocalRef<jclass> bool_class(env, env->FindClass("java/lang/Boolean"));
    jmethodID bool_get = env->GetMethodID(bool_class.get(), "booleanValue", "()Z");

    int removed = 0;
    auto to_erase = std::vector<session::config::convo::any>();

    for (auto it = conversations->begin(); it != conversations->end(); ++it) {
        jni_utils::JavaLocalRef result(env, env->CallObjectMethod(predicate, predicate_call, jni_utils::JavaLocalRef(env, serialize_any(env, *it)).get()));
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
    auto internal = convos->get_1to1(jni_utils::JavaStringRef(env, pub_key_hex).view());
    if (internal) {
        return serialize_one_to_one(env, *internal);
    }
    return nullptr;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructOneToOne(
        JNIEnv *env, jobject thiz, jstring pub_key_hex) {
    auto convos = ptrToConvoInfo(env, thiz);
    return serialize_one_to_one(env, convos->get_or_construct_1to1(jni_utils::JavaStringRef(env, pub_key_hex).view()));
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseOneToOne(JNIEnv *env,
                                                                                      jobject thiz,
                                                                                      jstring pub_key_hex) {
    auto convos = ptrToConvoInfo(env, thiz);
    return convos->erase_1to1(jni_utils::JavaStringRef(env, pub_key_hex).view());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getCommunity__Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto open = convos->get_community(jni_utils::JavaStringRef(env, base_url).view(), jni_utils::JavaStringRef(env, room).view());
    if (open) {
        return serialize_community(env, *open);
    }
    return nullptr;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructCommunity__Ljava_lang_String_2Ljava_lang_String_2_3B(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room, jbyteArray pub_key) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto community = convos->get_or_construct_community(
            jni_utils::JavaStringRef(env, base_url).view(),
            jni_utils::JavaStringRef(env, room).view(),
            jni_utils::JavaByteArrayRef(env, pub_key).get());
    return serialize_community(env, community);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructCommunity__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room, jstring pub_key_hex) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto community = convos->get_or_construct_community(
            jni_utils::JavaStringRef(env, base_url).view(),
            jni_utils::JavaStringRef(env, room).view(),
            jni_utils::JavaStringRef(env, pub_key_hex).view());
    return serialize_community(env, community);
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
            jni_utils::JavaStringRef(env, base_url).view(),
            jni_utils::JavaStringRef(env, room).view());
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getLegacyClosedGroup(
        JNIEnv *env, jobject thiz, jstring group_id) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto lgc = convos->get_legacy_group(jni_utils::JavaStringRef(env, group_id).view());
    if (lgc) {
        return serialize_legacy_group(env, *lgc);
    }

    return nullptr;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructLegacyGroup(
        JNIEnv *env, jobject thiz, jstring group_id) {
    auto convos = ptrToConvoInfo(env, thiz);
    auto lgc = convos->get_or_construct_legacy_group(jni_utils::JavaStringRef(env, group_id).view());
    return serialize_legacy_group(env, lgc);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseLegacyClosedGroup(
        JNIEnv *env, jobject thiz, jstring group_id) {
    auto convos = ptrToConvoInfo(env, thiz);
    return convos->erase_legacy_group(jni_utils::JavaStringRef(env, group_id).view());
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

    return jni_utils::jlist_from_collection(env, *convos, serialize_any);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allOneToOnes(JNIEnv *env,
                                                                                     jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jni_utils::jlist_from_iterator(env, convos->begin_1to1(), convos->end(),
                                          serialize_one_to_one);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allCommunities(JNIEnv *env,
                                                                                      jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jni_utils::jlist_from_iterator(env, convos->begin_communities(), convos->end(),
                                          serialize_community);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allLegacyClosedGroups(
        JNIEnv *env, jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jni_utils::jlist_from_iterator(env, convos->begin_legacy_groups(), convos->end(),
                                          serialize_legacy_group);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_allClosedGroups(JNIEnv *env,
                                                                                        jobject thiz) {
    auto convos = ptrToConvoInfo(env, thiz);
    return jni_utils::jlist_from_iterator(env, convos->begin_groups(), convos->end(),
                                          serialize_closed_group);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getClosedGroup(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jstring session_id) {
    auto config = ptrToConvoInfo(env, thiz);
    auto group = config->get_group(jni_utils::JavaStringRef(env, session_id).view());
    if (group) {
        return serialize_closed_group(env, *group);
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_getOrConstructClosedGroup(
        JNIEnv *env, jobject thiz, jstring session_id) {
    auto config = ptrToConvoInfo(env, thiz);
    auto group = config->get_or_construct_group(jni_utils::JavaStringRef(env, session_id).view());
    return serialize_closed_group(env, group);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseClosedGroup(
        JNIEnv *env, jobject thiz, jstring session_id) {
    auto config = ptrToConvoInfo(env, thiz);
    return config->erase_group(jni_utils::JavaStringRef(env, session_id).view());
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
    return serialize_blinded_one_to_one(env, ptrToConvoInfo(env, thiz)->get_or_construct_blinded_1to1(jni_utils::JavaStringRef(env, blinded_id).view(), true));
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConversationVolatileConfig_eraseBlindedOneToOne(
        JNIEnv *env, jobject thiz, jstring blinded_id) {
    return ptrToConvoInfo(env, thiz)->erase_blinded_1to1(jni_utils::JavaStringRef(env, blinded_id).view(), true);
}
