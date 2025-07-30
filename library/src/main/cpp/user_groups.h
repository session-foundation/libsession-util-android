
#ifndef SESSION_ANDROID_USER_GROUPS_H
#define SESSION_ANDROID_USER_GROUPS_H

#include "jni.h"
#include "util.h"
#include "jni_utils.h"
#include "conversation.h"
#include "session/config/user_groups.hpp"

inline session::config::UserGroups* ptrToUserGroups(JNIEnv *env, jobject obj) {
    auto configClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/UserGroupsConfig"));
    jfieldID pointerField = env->GetFieldID(configClass.get(), "pointer", "J");
    return (session::config::UserGroups*) env->GetLongField(obj, pointerField);
}

inline void deserialize_members_into(JNIEnv *env, jobject members_map, session::config::legacy_group_info& to_append_group) {
    auto map_class = jni_utils::JavaLocalRef(env, env->FindClass("java/util/Map"));
    auto map_entry_class = jni_utils::JavaLocalRef(env, env->FindClass("java/util/Map$Entry"));
    auto set_class = jni_utils::JavaLocalRef(env, env->FindClass("java/util/Set"));
    auto iterator_class = jni_utils::JavaLocalRef(env, env->FindClass("java/util/Iterator"));
    auto boxed_bool = jni_utils::JavaLocalRef(env, env->FindClass("java/lang/Boolean"));

    jmethodID get_entry_set = env->GetMethodID(map_class.get(), "entrySet", "()Ljava/util/Set;");
    jmethodID get_at = env->GetMethodID(set_class.get(), "iterator", "()Ljava/util/Iterator;");
    jmethodID has_next = env->GetMethodID(iterator_class.get(), "hasNext", "()Z");
    jmethodID next = env->GetMethodID(iterator_class.get(), "next", "()Ljava/lang/Object;");
    jmethodID get_key = env->GetMethodID(map_entry_class.get(), "getKey", "()Ljava/lang/Object;");
    jmethodID get_value = env->GetMethodID(map_entry_class.get(), "getValue", "()Ljava/lang/Object;");
    jmethodID get_bool_value = env->GetMethodID(boxed_bool.get(), "booleanValue", "()Z");

    auto entry_set = jni_utils::JavaLocalRef(env, env->CallObjectMethod(members_map, get_entry_set));
    auto iterator = jni_utils::JavaLocalRef(env, env->CallObjectMethod(entry_set.get(), get_at));

    while (env->CallBooleanMethod(iterator.get(), has_next)) {
        jni_utils::JavaLocalRef entry(env, env->CallObjectMethod(iterator.get(), next));
        jni_utils::JavaLocalRef key(env, static_cast<jstring>(env->CallObjectMethod(entry.get(), get_key)));
        jni_utils::JavaLocalRef boxed(env, env->CallObjectMethod(entry.get(), get_value));
        bool is_admin = env->CallBooleanMethod(boxed.get(), get_bool_value);
        to_append_group.insert(std::string(jni_utils::JavaStringRef(env, key.get()).view()), is_admin);
    }
}

inline session::config::legacy_group_info deserialize_legacy_group_info(JNIEnv *env, jobject info, session::config::UserGroups* conf) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$LegacyGroupInfo"));
    auto id_field = env->GetFieldID(clazz.get(), "accountId", "Ljava/lang/String;");
    auto name_field = env->GetFieldID(clazz.get(), "name", "Ljava/lang/String;");
    auto members_field = env->GetFieldID(clazz.get(), "members", "Ljava/util/Map;");
    auto enc_pub_key_method = env->GetMethodID(clazz.get(), "getEncPubKeyAsByteArray", "()[B");
    auto enc_sec_key_method = env->GetMethodID(clazz.get(), "getEncSecKeyAsByteArray", "()[B");
    auto priority_field = env->GetFieldID(clazz.get(), "priority", "J");
    auto disappearing_timer_field = env->GetFieldID(clazz.get(), "disappearingTimer", "J");
    auto joined_at_field = env->GetFieldID(clazz.get(), "joinedAtSecs", "J");
    auto id = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, id_field)));
    auto name = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, name_field)));
    auto members_map = jni_utils::JavaLocalRef(env, env->GetObjectField(info, members_field));
    auto enc_pub_key = jni_utils::JavaLocalRef(env, static_cast<jbyteArray>(env->CallObjectMethod(info, enc_pub_key_method)));
    auto enc_sec_key = jni_utils::JavaLocalRef(env, static_cast<jbyteArray>(env->CallObjectMethod(info, enc_sec_key_method)));
    int priority = env->GetLongField(info, priority_field);
    long joined_at = env->GetLongField(info, joined_at_field);

    auto info_deserialized = conf->get_or_construct_legacy_group(jni_utils::JavaStringRef(env, id.get()).view());

    auto current_members = info_deserialized.members();
    for (auto member = current_members.begin(); member != current_members.end(); ++member) {
        info_deserialized.erase(member->first);
    }
    deserialize_members_into(env, members_map.get(), info_deserialized);
    info_deserialized.name = jni_utils::JavaStringRef(env, name.get()).view();
    info_deserialized.enc_pubkey = jni_utils::JavaByteArrayRef(env, enc_pub_key.get()).copy();
    info_deserialized.enc_seckey = jni_utils::JavaByteArrayRef(env, enc_sec_key.get()).copy();
    info_deserialized.priority = priority;
    info_deserialized.disappearing_timer = std::chrono::seconds(env->GetLongField(info, disappearing_timer_field));
    info_deserialized.joined_at = joined_at;
    return info_deserialized;
}

inline session::config::community_info deserialize_community_info(JNIEnv *env, jobject info, session::config::UserGroups* conf) {
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$CommunityGroupInfo"));
    auto base_info = env->GetFieldID(clazz.get(), "community", "Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;");
    auto priority = env->GetFieldID(clazz.get(), "priority", "J");
    auto base_community_info = jni_utils::JavaLocalRef(env, env->GetObjectField(info, base_info));
    auto deserialized_base_info = util::deserialize_base_community(env, base_community_info.get());
    int deserialized_priority = env->GetLongField(info, priority);
    auto community_info = conf->get_or_construct_community(deserialized_base_info.base_url(), deserialized_base_info.room(), deserialized_base_info.pubkey_hex());
    community_info.priority = deserialized_priority;
    return community_info;
}

inline jobject serialize_members(JNIEnv *env, std::map<std::string, bool> members_map) {
    auto map_class = jni_utils::JavaLocalRef(env, env->FindClass("java/util/HashMap"));
    auto boxed_bool = jni_utils::JavaLocalRef(env, env->FindClass("java/lang/Boolean"));
    jmethodID map_constructor = env->GetMethodID(map_class.get(), "<init>", "()V");
    jmethodID insert = env->GetMethodID(map_class.get(), "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jmethodID new_bool = env->GetMethodID(boxed_bool.get(), "<init>", "(Z)V");

    auto new_map = env->NewObject(map_class.get(), map_constructor);
    for (auto it = members_map.begin(); it != members_map.end(); it++) {
        auto account_id = jni_utils::JavaLocalRef(env, env->NewStringUTF(it->first.data()));
        bool is_admin = it->second;
        auto jbool = jni_utils::JavaLocalRef(env, env->NewObject(boxed_bool.get(), new_bool, is_admin));
        env->CallObjectMethod(new_map, insert, account_id.get(), jbool.get());
    }
    return new_map;
}

inline jobject serialize_legacy_group_info(JNIEnv *env, session::config::legacy_group_info info) {
    auto account_id = jni_utils::JavaLocalRef(env, env->NewStringUTF(info.session_id.data()));
    auto name = jni_utils::JavaLocalRef(env, env->NewStringUTF(info.name.data()));
    auto members = jni_utils::JavaLocalRef(env, serialize_members(env, info.members()));
    auto enc_pubkey = jni_utils::session_bytes_from_range(env, info.enc_pubkey);
    auto enc_seckey = jni_utils::session_bytes_from_range(env, info.enc_seckey);
    long long priority = info.priority;
    long long joined_at = info.joined_at;

    auto legacy_group_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$LegacyGroupInfo"));
    jmethodID constructor = env->GetMethodID(legacy_group_class.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;Lnetwork/loki/messenger/libsession_util/util/Bytes;Lnetwork/loki/messenger/libsession_util/util/Bytes;JJJ)V");
    return env->NewObject(legacy_group_class.get(), constructor, account_id.get(), name.get(), members.get(), enc_pubkey, enc_seckey, priority, (jlong) info.disappearing_timer.count(), joined_at);
}

inline jobject serialize_closed_group_info(JNIEnv* env, session::config::group_info info) {
    auto session_id = util::jstringFromOptional(env, info.id);
    auto admin_bytes = jni_utils::JavaLocalRef(env, info.secretkey.empty() ? nullptr : jni_utils::session_bytes_from_range(env, info.secretkey));
    auto auth_bytes = jni_utils::JavaLocalRef(env, info.auth_data.empty() ? nullptr : jni_utils::session_bytes_from_range(env, info.auth_data));
    auto name = jni_utils::JavaLocalRef(env, util::jstringFromOptional(env, info.name));

    auto group_info_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$ClosedGroupInfo"));
    jmethodID constructor = env->GetMethodID(group_info_class.get(), "<init>","(Ljava/lang/String;Lnetwork/loki/messenger/libsession_util/util/Bytes;Lnetwork/loki/messenger/libsession_util/util/Bytes;JZLjava/lang/String;ZZJ)V");
    return env->NewObject(group_info_class.get(), constructor,
                          session_id, admin_bytes.get(), auth_bytes.get(), (jlong)info.priority, info.invited, name.get(),
                          info.kicked(), info.is_destroyed(), info.joined_at);
}

inline session::config::group_info deserialize_closed_group_info(JNIEnv* env, jobject info_serialized) {
    auto closed_group_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$ClosedGroupInfo"));
    jfieldID id_field = env->GetFieldID(closed_group_class.get(), "groupAccountId", "Ljava/lang/String;");
    auto secret_method = env->GetMethodID(closed_group_class.get(), "getAdminKeyAsByteArray", "()[B");
    auto auth_method = env->GetMethodID(closed_group_class.get(), "getAuthDataAsByteArray", "()[B");
    jfieldID priority_field = env->GetFieldID(closed_group_class.get(), "priority", "J");
    jfieldID invited_field = env->GetFieldID(closed_group_class.get(), "invited", "Z");
    jfieldID name_field = env->GetFieldID(closed_group_class.get(), "name", "Ljava/lang/String;");
    jfieldID destroy_field = env->GetFieldID(closed_group_class.get(), "destroyed", "Z");
    jfieldID kicked_field = env->GetFieldID(closed_group_class.get(), "kicked", "Z");
    jfieldID joined_at_field = env->GetFieldID(closed_group_class.get(), "joinedAtSecs", "J");


    auto id_jobject = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info_serialized, id_field)));
    auto secret_jBytes = jni_utils::JavaLocalRef(env, (jbyteArray)env->CallObjectMethod(info_serialized, secret_method));
    auto auth_jBytes = jni_utils::JavaLocalRef(env, (jbyteArray)env->CallObjectMethod(info_serialized, auth_method));
    auto name_jstring = jni_utils::JavaLocalRef(env, (jstring)env->GetObjectField(info_serialized, name_field));

    auto secret_bytes = util::vector_from_bytes(env, secret_jBytes.get());
    auto auth_bytes = util::vector_from_bytes(env, auth_jBytes.get());

    session::config::group_info group_info(jni_utils::JavaStringRef(env, id_jobject.get()).copy());
    group_info.auth_data = auth_bytes;
    group_info.secretkey = secret_bytes;
    group_info.priority = env->GetLongField(info_serialized, priority_field);
    group_info.invited = env->GetBooleanField(info_serialized, invited_field);
    group_info.name = jni_utils::JavaStringRef(env, name_jstring.get()).view();
    group_info.joined_at = env->GetLongField(info_serialized, joined_at_field);

    if (env->GetBooleanField(info_serialized, kicked_field)) {
        group_info.mark_kicked();
    }

    if (env->GetBooleanField(info_serialized, destroy_field)) {
        group_info.mark_destroyed();
    }

    return group_info;
}

inline jobject serialize_community_info(JNIEnv *env, session::config::community_info info) {
    auto priority = (long long)info.priority;
    auto serialized_info = util::serialize_base_community(env, info);
    auto clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$CommunityGroupInfo"));
    jmethodID constructor = env->GetMethodID(clazz.get(), "<init>", "(Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;J)V");
    jobject serialized = env->NewObject(clazz.get(), constructor, serialized_info, priority);
    return serialized;
}

#endif //SESSION_ANDROID_USER_GROUPS_H
