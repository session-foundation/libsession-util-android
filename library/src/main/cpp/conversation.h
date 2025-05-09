#ifndef SESSION_ANDROID_CONVERSATION_H
#define SESSION_ANDROID_CONVERSATION_H

#include <jni.h>
#include <android/log.h>
#include "util.h"
#include "session/config/convo_info_volatile.hpp"
#include "jni_utils.h"

inline session::config::ConvoInfoVolatile *ptrToConvoInfo(JNIEnv *env, jobject obj) {
    auto contactsClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/ConversationVolatileConfig"));
    jfieldID pointerField = env->GetFieldID(contactsClass.get(), "pointer", "J");
    return (session::config::ConvoInfoVolatile *) env->GetLongField(obj, pointerField);
}

inline jobject serialize_one_to_one(JNIEnv *env, session::config::convo::one_to_one one_to_one) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$OneToOne"));
    jmethodID constructor = env->GetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;JZ)V");
    auto account_id = jni_utils::JavaLocalRef(env, env->NewStringUTF(one_to_one.session_id.data()));
    auto last_read = one_to_one.last_read;
    auto unread = one_to_one.unread;
    jobject serialized = env->NewObject(clazz.get(), constructor, account_id.get(), last_read, unread);
    return serialized;
}

inline jobject serialize_open_group(JNIEnv *env, session::config::convo::community community) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$Community"));
    auto base_community = util::serialize_base_community(env, community);
    jmethodID constructor = env->GetMethodID(clazz.get(), "<init>",
                                             "(Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;JZ)V");
    auto last_read = community.last_read;
    auto unread = community.unread;
    jobject serialized = env->NewObject(clazz.get(), constructor, base_community, last_read, unread);
    return serialized;
}

inline jobject serialize_legacy_group(JNIEnv *env, session::config::convo::legacy_group group) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$LegacyGroup"));
    jmethodID constructor = env->GetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;JZ)V");
    auto group_id = jni_utils::JavaLocalRef(env, env->NewStringUTF(group.id.data()));
    auto last_read = group.last_read;
    auto unread = group.unread;
    jobject serialized = env->NewObject(clazz.get(), constructor, group_id.get(), last_read, unread);
    return serialized;
}

inline jobject serialize_closed_group(JNIEnv* env, session::config::convo::group group) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$ClosedGroup"));
    jmethodID constructor = env->GetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;JZ)V");
    auto session_id = jni_utils::JavaLocalRef(env, env->NewStringUTF(group.id.data()));
    auto last_read = group.last_read;
    auto unread = group.unread;
    return env->NewObject(clazz.get(), constructor, session_id.get(), last_read, unread);
}

inline jobject serialize_any(JNIEnv *env, session::config::convo::any any) {
    if (auto* dm = std::get_if<session::config::convo::one_to_one>(&any)) {
        return serialize_one_to_one(env, *dm);
    } else if (auto* og = std::get_if<session::config::convo::community>(&any)) {
        return serialize_open_group(env, *og);
    } else if (auto* lgc = std::get_if<session::config::convo::legacy_group>(&any)) {
        return serialize_legacy_group(env, *lgc);
    } else if (auto* gc = std::get_if<session::config::convo::group>(&any)) {
        return serialize_closed_group(env, *gc);
    }
    return nullptr;
}

inline session::config::convo::one_to_one deserialize_one_to_one(JNIEnv *env, jobject info, session::config::ConvoInfoVolatile *conf) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$OneToOne"));
    auto id_getter = env->GetFieldID(clazz.get(), "accountId", "Ljava/lang/String;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");
    auto id = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, id_getter)));
    auto id_chars = env->GetStringUTFChars(id.get(), nullptr);
    std::string id_string = std::string{id_chars};
    auto deserialized = conf->get_or_construct_1to1(id_string);
    deserialized.last_read = env->GetLongField(info, last_read_getter);
    deserialized.unread = env->GetBooleanField(info, unread_getter);
    env->ReleaseStringUTFChars(id.get(), id_chars);
    return deserialized;
}

inline session::config::convo::community deserialize_community(JNIEnv *env, jobject info, session::config::ConvoInfoVolatile *conf) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$Community"));
    auto base_community_getter = env->GetFieldID(clazz.get(), "baseCommunityInfo", "Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");

    auto base_community_info = jni_utils::JavaLocalRef(env, env->GetObjectField(info, base_community_getter));

    auto base_community_deserialized = util::deserialize_base_community(env, base_community_info.get());
    auto deserialized = conf->get_or_construct_community(
        base_community_deserialized.base_url(),
        base_community_deserialized.room(),
        base_community_deserialized.pubkey()
    );

    deserialized.last_read = env->GetLongField(info, last_read_getter);
    deserialized.unread = env->GetBooleanField(info, unread_getter);

    return deserialized;
}

inline session::config::convo::legacy_group deserialize_legacy_closed_group(JNIEnv *env, jobject info, session::config::ConvoInfoVolatile *conf) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$LegacyGroup"));
    auto group_id_getter = env->GetFieldID(clazz.get(), "groupId", "Ljava/lang/String;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");
    auto group_id = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, group_id_getter)));
    auto group_id_bytes = env->GetStringUTFChars(group_id.get(), nullptr);
    auto group_id_string = std::string{group_id_bytes};
    auto deserialized = conf->get_or_construct_legacy_group(group_id_string);
    deserialized.last_read = env->GetLongField(info, last_read_getter);
    deserialized.unread = env->GetBooleanField(info, unread_getter);
    env->ReleaseStringUTFChars(group_id.get(), group_id_bytes);
    return deserialized;
}

inline session::config::convo::group deserialize_closed_group(JNIEnv* env, jobject info, session::config::ConvoInfoVolatile* conf) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$ClosedGroup"));
    auto id_getter = env->GetFieldID(clazz.get(), "accountId", "Ljava/lang/String;");
    auto last_read_getter = env->GetFieldID(clazz.get(), "lastRead", "J");
    auto unread_getter = env->GetFieldID(clazz.get(), "unread", "Z");
    auto session_id = (jstring)env->GetObjectField(info, id_getter);
    auto session_id_bytes = env->GetStringUTFChars(session_id, nullptr);
    auto last_read = env->GetLongField(info, last_read_getter);
    auto unread = env->GetBooleanField(info, unread_getter);

    auto group = conf->get_or_construct_group(session_id_bytes);
    group.last_read = last_read;
    group.unread = unread;

    env->ReleaseStringUTFChars(session_id, session_id_bytes);
    return group;
}

inline std::optional<session::config::convo::any> deserialize_any(JNIEnv *env, jobject convo, session::config::ConvoInfoVolatile *conf) {
    auto oto_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$OneToOne"));
    auto og_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$Community"));
    auto lgc_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$LegacyGroup"));
    auto gc_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Conversation$ClosedGroup"));

    auto object_class = jni_utils::JavaLocalRef(env, env->GetObjectClass(convo));
    if (env->IsSameObject(object_class.get(), oto_class.get())) {
        return session::config::convo::any{deserialize_one_to_one(env, convo, conf)};
    } else if (env->IsSameObject(object_class.get(), og_class.get())) {
        return session::config::convo::any{deserialize_community(env, convo, conf)};
    } else if (env->IsSameObject(object_class.get(), lgc_class.get())) {
        return session::config::convo::any{deserialize_legacy_closed_group(env, convo, conf)};
    } else if (env->IsSameObject(object_class.get(), gc_class.get())) {
        return session::config::convo::any{deserialize_closed_group(env, convo, conf)};
    }
    return std::nullopt;
}

#endif //SESSION_ANDROID_CONVERSATION_H