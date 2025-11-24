#include "user_groups.h"
#include "oxenc/hex.h"
#include "util.h"
#include "session/ed25519.hpp"

using namespace jni_utils;

inline session::config::UserGroups* ptrToUserGroups(JNIEnv *env, jobject obj) {
    auto configClass = JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/UserGroupsConfig"));
    jfieldID pointerField = env->GetFieldID(configClass.get(), "pointer", "J");
    return (session::config::UserGroups*) env->GetLongField(obj, pointerField);
}

static void deserialize_members_into(JNIEnv *env, jobject members_map, session::config::legacy_group_info& to_append_group) {
    auto map_class = JavaLocalRef(env, env->FindClass("java/util/Map"));
    auto map_entry_class = JavaLocalRef(env, env->FindClass("java/util/Map$Entry"));
    auto set_class = JavaLocalRef(env, env->FindClass("java/util/Set"));
    auto iterator_class = JavaLocalRef(env, env->FindClass("java/util/Iterator"));
    auto boxed_bool = JavaLocalRef(env, env->FindClass("java/lang/Boolean"));

    jmethodID get_entry_set = env->GetMethodID(map_class.get(), "entrySet", "()Ljava/util/Set;");
    jmethodID get_at = env->GetMethodID(set_class.get(), "iterator", "()Ljava/util/Iterator;");
    jmethodID has_next = env->GetMethodID(iterator_class.get(), "hasNext", "()Z");
    jmethodID next = env->GetMethodID(iterator_class.get(), "next", "()Ljava/lang/Object;");
    jmethodID get_key = env->GetMethodID(map_entry_class.get(), "getKey", "()Ljava/lang/Object;");
    jmethodID get_value = env->GetMethodID(map_entry_class.get(), "getValue", "()Ljava/lang/Object;");
    jmethodID get_bool_value = env->GetMethodID(boxed_bool.get(), "booleanValue", "()Z");

    auto entry_set = JavaLocalRef(env, env->CallObjectMethod(members_map, get_entry_set));
    auto iterator = JavaLocalRef(env, env->CallObjectMethod(entry_set.get(), get_at));

    while (env->CallBooleanMethod(iterator.get(), has_next)) {
        JavaLocalRef entry(env, env->CallObjectMethod(iterator.get(), next));
        JavaLocalRef key(env, static_cast<jstring>(env->CallObjectMethod(entry.get(), get_key)));
        JavaLocalRef boxed(env, env->CallObjectMethod(entry.get(), get_value));
        bool is_admin = env->CallBooleanMethod(boxed.get(), get_bool_value);
        to_append_group.insert(std::string(JavaStringRef(env, key.get()).view()), is_admin);
    }
}


static session::config::legacy_group_info deserialize_legacy_group_info(JNIEnv *env, jobject info, session::config::UserGroups* conf) {
    auto clazz = JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$LegacyGroupInfo"));
    auto id_field = env->GetFieldID(clazz.get(), "accountId", "Ljava/lang/String;");
    auto name_field = env->GetFieldID(clazz.get(), "name", "Ljava/lang/String;");
    auto members_field = env->GetFieldID(clazz.get(), "members", "Ljava/util/Map;");
    auto enc_pub_key_method = env->GetMethodID(clazz.get(), "getEncPubKeyAsByteArray", "()[B");
    auto enc_sec_key_method = env->GetMethodID(clazz.get(), "getEncSecKeyAsByteArray", "()[B");
    auto priority_field = env->GetFieldID(clazz.get(), "priority", "J");
    auto disappearing_timer_field = env->GetFieldID(clazz.get(), "disappearingTimer", "J");
    auto joined_at_field = env->GetFieldID(clazz.get(), "joinedAtSecs", "J");
    auto id = JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, id_field)));
    auto name = JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, name_field)));
    auto members_map = JavaLocalRef(env, env->GetObjectField(info, members_field));
    auto enc_pub_key = JavaLocalRef(env, static_cast<jbyteArray>(env->CallObjectMethod(info, enc_pub_key_method)));
    auto enc_sec_key = JavaLocalRef(env, static_cast<jbyteArray>(env->CallObjectMethod(info, enc_sec_key_method)));
    int priority = env->GetLongField(info, priority_field);
    long joined_at = env->GetLongField(info, joined_at_field);

    auto info_deserialized = conf->get_or_construct_legacy_group(JavaStringRef(env, id.get()).view());

    auto current_members = info_deserialized.members();
    for (auto member = current_members.begin(); member != current_members.end(); ++member) {
        info_deserialized.erase(member->first);
    }
    deserialize_members_into(env, members_map.get(), info_deserialized);
    info_deserialized.name = JavaStringRef(env, name.get()).view();
    info_deserialized.enc_pubkey = JavaByteArrayRef(env, enc_pub_key.get()).copy();
    info_deserialized.enc_seckey = JavaByteArrayRef(env, enc_sec_key.get()).copy();
    info_deserialized.priority = priority;
    info_deserialized.disappearing_timer = std::chrono::seconds(env->GetLongField(info, disappearing_timer_field));
    info_deserialized.joined_at = joined_at;
    return info_deserialized;
}

static session::config::community_info deserialize_community_info(JNIEnv *env, jobject info, session::config::UserGroups* conf) {
    struct ClassInfo : JavaClassInfo {
        jmethodID base_info_getter;
        jmethodID priority_getter;

        ClassInfo(JNIEnv *env): JavaClassInfo(env, "network/loki/messenger/libsession_util/util/GroupInfo$CommunityGroupInfo"),
            base_info_getter(env->GetMethodID(java_class, "getCommunity", "()Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;")),
            priority_getter(env->GetMethodID(java_class, "getPriority", "()J")) {}
    };

    static ClassInfo class_info(env);

    auto base_community_info = JavaLocalRef(env, env->CallObjectMethod(info, class_info.base_info_getter));
    auto deserialized_base_info = deserialize_base_community(env, base_community_info.get());
    int deserialized_priority = env->CallLongMethod(info, class_info.priority_getter);
    auto community_info = conf->get_or_construct_community(deserialized_base_info.base_url(), deserialized_base_info.room(), deserialized_base_info.pubkey_hex());
    community_info.priority = deserialized_priority;
    return community_info;
}

static jobject serialize_members(JNIEnv *env, std::map<std::string, bool> members_map) {
    auto map_class = JavaLocalRef(env, env->FindClass("java/util/HashMap"));
    auto boxed_bool = JavaLocalRef(env, env->FindClass("java/lang/Boolean"));
    jmethodID map_constructor = env->GetMethodID(map_class.get(), "<init>", "()V");
    jmethodID insert = env->GetMethodID(map_class.get(), "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jmethodID new_bool = env->GetMethodID(boxed_bool.get(), "<init>", "(Z)V");

    auto new_map = env->NewObject(map_class.get(), map_constructor);
    for (auto it = members_map.begin(); it != members_map.end(); it++) {
        auto account_id = JavaLocalRef(env, env->NewStringUTF(it->first.data()));
        bool is_admin = it->second;
        auto jbool = JavaLocalRef(env, env->NewObject(boxed_bool.get(), new_bool, is_admin));
        env->CallObjectMethod(new_map, insert, account_id.get(), jbool.get());
    }
    return new_map;
}

static JavaLocalRef<jobject> serialize_legacy_group_info(JNIEnv *env, session::config::legacy_group_info info) {
    auto account_id = JavaLocalRef(env, env->NewStringUTF(info.session_id.data()));
    auto name = JavaLocalRef(env, env->NewStringUTF(info.name.data()));
    auto members = JavaLocalRef(env, serialize_members(env, info.members()));
    auto enc_pubkey = session_bytes_from_range(env, info.enc_pubkey);
    auto enc_seckey = session_bytes_from_range(env, info.enc_seckey);
    long long priority = info.priority;
    long long joined_at = info.joined_at;

    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/GroupInfo$LegacyGroupInfo",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;Lnetwork/loki/messenger/libsession_util/util/Bytes;Lnetwork/loki/messenger/libsession_util/util/Bytes;JJJ)V"
    );
    
    return {env, env->NewObject(class_info.java_class, class_info.constructor,
                                account_id.get(), name.get(), members.get(), enc_pubkey.get(),
                                enc_seckey.get(), priority,
                                (jlong) info.disappearing_timer.count(), joined_at)};
}

static JavaLocalRef<jobject> serialize_closed_group_info(JNIEnv* env, session::config::group_info info) {
    auto session_id = jstring_from_optional(env, info.id);
    auto admin_bytes = JavaLocalRef(env, info.secretkey.empty() ? nullptr : session_bytes_from_range(env, info.secretkey).leak());
    auto auth_bytes = JavaLocalRef(env, info.auth_data.empty() ? nullptr : session_bytes_from_range(env, info.auth_data).leak());
    auto name = jstring_from_optional(env, info.name);

    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/GroupInfo$ClosedGroupInfo",
            "(Ljava/lang/String;Lnetwork/loki/messenger/libsession_util/util/Bytes;Lnetwork/loki/messenger/libsession_util/util/Bytes;JZLjava/lang/String;ZZJ)V"
    );

    return {env, env->NewObject(class_info.java_class, class_info.constructor,
                          session_id.get(), admin_bytes.get(), auth_bytes.get(), (jlong)info.priority, info.invited, name.get(),
                          info.kicked(), info.is_destroyed(), info.joined_at)};
}

static session::config::group_info deserialize_closed_group_info(JNIEnv* env, jobject info_serialized) {
    struct ClassInfo : public JavaClassInfo {
        jmethodID id_getter;
        jmethodID secret_method;
        jmethodID auth_method;
        jmethodID name_getter;
        jmethodID priority_getter;
        jmethodID invited_getter;
        jmethodID destroyed_getter;
        jmethodID kicked_getter;
        jmethodID joined_at_getter;

        ClassInfo(JNIEnv *env, jobject obj)
            : JavaClassInfo(env, obj)
            , id_getter(env->GetMethodID(java_class, "getGroupAccountId", "()Ljava/lang/String;"))
            , secret_method(env->GetMethodID(java_class, "getAdminKeyAsByteArray", "()[B"))
            , auth_method(env->GetMethodID(java_class, "getAuthDataAsByteArray", "()[B"))
            , name_getter(env->GetMethodID(java_class, "getName", "()Ljava/lang/String;"))
            , priority_getter(env->GetMethodID(java_class, "getPriority", "()J"))
            , invited_getter(env->GetMethodID(java_class, "getInvited", "()Z"))
            , destroyed_getter(env->GetMethodID(java_class, "getDestroyed", "()Z"))
            , kicked_getter(env->GetMethodID(java_class, "getKicked", "()Z"))
            , joined_at_getter(env->GetMethodID(java_class, "getJoinedAtSecs", "()J"))
            {}
    };

    static ClassInfo class_info(env, info_serialized);

    auto id_jobject = JavaLocalRef(env, static_cast<jstring>(env->CallObjectMethod(info_serialized, class_info.id_getter)));
    auto secret_jBytes = JavaLocalRef(env, (jbyteArray)env->CallObjectMethod(info_serialized, class_info.secret_method));
    auto auth_jBytes = JavaLocalRef(env, (jbyteArray)env->CallObjectMethod(info_serialized, class_info.auth_method));
    auto name_jstring = JavaLocalRef(env, (jstring)env->CallObjectMethod(info_serialized, class_info.name_getter));

    auto secret_bytes = util::vector_from_bytes(env, secret_jBytes.get());
    auto auth_bytes = util::vector_from_bytes(env, auth_jBytes.get());

    session::config::group_info group_info(JavaStringRef(env, id_jobject.get()).copy());
    group_info.auth_data = auth_bytes;
    group_info.secretkey = secret_bytes;
    group_info.priority = env->CallLongMethod(info_serialized, class_info.priority_getter);
    group_info.invited = env->CallBooleanMethod(info_serialized, class_info.invited_getter);
    group_info.name = JavaStringRef(env, name_jstring.get()).view();
    group_info.joined_at = env->CallLongMethod(info_serialized, class_info.joined_at_getter);

    if (env->CallBooleanMethod(info_serialized, class_info.kicked_getter)) {
        group_info.mark_kicked();
    }

    if (env->CallBooleanMethod(info_serialized, class_info.destroyed_getter)) {
        group_info.mark_destroyed();
    }

    return group_info;
}

static JavaLocalRef<jobject> serialize_community_info(JNIEnv *env, const session::config::community_info &info) {
    auto priority = (long long)info.priority;
    auto serialized_info = serialize_base_community(env, info);

    static BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/GroupInfo$CommunityGroupInfo",
            "(Lnetwork/loki/messenger/libsession_util/util/BaseCommunityInfo;J)V"
    );

    return {env, env->NewObject(class_info.java_class, class_info.constructor, serialized_info.get(), priority)};
}

JavaLocalRef<jobject> serialize_base_community(JNIEnv *env, const session::config::community& community) {
    static BasicJavaClassInfo class_info(env, "network/loki/messenger/libsession_util/util/BaseCommunityInfo",
                                        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

    auto base_url = jni_utils::JavaLocalRef(env, env->NewStringUTF(community.base_url().data()));
    auto room = jni_utils::JavaLocalRef(env, env->NewStringUTF(community.room().data()));
    auto pubkey_jstring = jni_utils::JavaLocalRef(env, env->NewStringUTF(community.pubkey_hex().data()));
    return {env, env->NewObject(class_info.java_class,
                                class_info.constructor,
                                base_url.get(), room.get(), pubkey_jstring.get())};
}

session::config::community deserialize_base_community(JNIEnv *env, jobject base_community) {
    struct ClassInfo : public JavaClassInfo {
        jmethodID base_url_getter;
        jmethodID room_getter;
        jmethodID pubkey_getter;

        ClassInfo(JNIEnv *env, jobject obj)
            : JavaClassInfo(env, obj)
            , base_url_getter(env->GetMethodID(java_class, "getBaseUrl", "()Ljava/lang/String;"))
            , room_getter(env->GetMethodID(java_class, "getRoom", "()Ljava/lang/String;"))
            , pubkey_getter(env->GetMethodID(java_class, "getPubKeyHex", "()Ljava/lang/String;"))
            {}
    };

    static ClassInfo class_info(env, base_community);

    jni_utils::JavaLocalRef base_url(env, (jstring)env->CallObjectMethod(base_community, class_info.base_url_getter));
    jni_utils::JavaLocalRef room(env, (jstring)env->CallObjectMethod(base_community, class_info.room_getter));
    jni_utils::JavaLocalRef pub_key_hex(env, (jstring)env->CallObjectMethod(base_community, class_info.pubkey_getter));

    return session::config::community(
            jni_utils::JavaStringRef(env, base_url.get()).view(),
            jni_utils::JavaStringRef(env, room.get()).view(),
            jni_utils::JavaStringRef(env, pub_key_hex.get()).view()
    );
}


extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupInfo_00024LegacyGroupInfo_00024Companion_NAME_1MAX_1LENGTH(
        JNIEnv *env, jobject thiz) {
    return session::config::legacy_group_info::NAME_MAX_LENGTH;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getCommunityInfo(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jstring base_url,
                                                                               jstring room) {
    auto conf = ptrToUserGroups(env, thiz);

    auto community = conf->get_community(JavaStringRef(env, base_url).view(), JavaStringRef(env, room).view());

    if (community) {
        return serialize_community_info(env, *community).leak();
    }

    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getLegacyGroupInfo(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jstring account_id) {
    auto conf = ptrToUserGroups(env, thiz);
    auto legacy_group = conf->get_legacy_group(JavaStringRef(env, account_id).view());
    jobject return_group = nullptr;
    if (legacy_group) {
        return_group = serialize_legacy_group_info(env, *legacy_group).leak();
    }
    return return_group;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getOrConstructCommunityInfo(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room, jstring pub_key_hex) {
    auto conf = ptrToUserGroups(env, thiz);

    auto group = conf->get_or_construct_community(
            JavaStringRef(env, base_url).view(),
            JavaStringRef(env, room).view(),
            JavaStringRef(env, pub_key_hex).view());

    return serialize_community_info(env, group).leak();
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getOrConstructLegacyGroupInfo(
        JNIEnv *env, jobject thiz, jstring account_id) {
    auto conf = ptrToUserGroups(env, thiz);
    auto group = conf->get_or_construct_legacy_group(JavaStringRef(env, account_id).view());
    return serialize_legacy_group_info(env, group).leak();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_set__Lnetwork_loki_messenger_libsession_1util_util_GroupInfo_2(
        JNIEnv *env, jobject thiz, jobject group_info) {
    auto conf = ptrToUserGroups(env, thiz);
    auto community_info = env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$CommunityGroupInfo");
    auto legacy_info = env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$LegacyGroupInfo");
    auto closed_group_info = env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$ClosedGroupInfo");

    auto object_class = env->GetObjectClass(group_info);
    if (env->IsSameObject(community_info, object_class)) {
        auto deserialized = deserialize_community_info(env, group_info, conf);
        conf->set(deserialized);
    } else if (env->IsSameObject(legacy_info, object_class)) {
        auto deserialized = deserialize_legacy_group_info(env, group_info, conf);
        conf->set(deserialized);
    } else if (env->IsSameObject(closed_group_info, object_class)) {
        auto deserialized = deserialize_closed_group_info(env, group_info);
        conf->set(deserialized);
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_erase__Lnetwork_loki_messenger_libsession_1util_util_GroupInfo_2(
        JNIEnv *env, jobject thiz, jobject group_info) {
    auto conf = ptrToUserGroups(env, thiz);
    auto communityInfo = env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$CommunityGroupInfo");
    auto legacyInfo = env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$LegacyGroupInfo");
    auto closedGroupInfo = env->FindClass("network/loki/messenger/libsession_util/util/GroupInfo$ClosedGroupInfo");
    auto object_class = env->GetObjectClass(group_info);
    if (env->IsSameObject(communityInfo, object_class)) {
        auto deserialized = deserialize_community_info(env, group_info, conf);
        conf->erase(deserialized);
    } else if (env->IsSameObject(legacyInfo, object_class)) {
        auto deserialized = deserialize_legacy_group_info(env, group_info, conf);
        conf->erase(deserialized);
    } else if (env->IsSameObject(closedGroupInfo, object_class)) {
        auto deserialized = deserialize_closed_group_info(env, group_info);
        conf->erase(deserialized);
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_sizeCommunityInfo(JNIEnv *env,
                                                                                jobject thiz) {
    auto conf = ptrToUserGroups(env, thiz);
    return conf->size_communities();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_sizeLegacyGroupInfo(JNIEnv *env,
                                                                                  jobject thiz) {
    auto conf = ptrToUserGroups(env, thiz);
    return conf->size_legacy_groups();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_size(JNIEnv *env, jobject thiz) {
    auto conf = ptrToUserGroups(env, thiz);
    return conf->size();
}

inline jobject iterator_as_java_list(JNIEnv *env, session::config::UserGroups::iterator begin, session::config::UserGroups::iterator end) {
    return jlist_from_iterator(env, begin, end, [](JNIEnv *env, const session::config::UserGroups::value_type &item) {
        std::optional<JavaLocalRef<jobject>> serialized = std::nullopt;
        if (auto* lgc = std::get_if<session::config::legacy_group_info>(&item)) {
            serialized = serialize_legacy_group_info(env, *lgc);
        } else if (auto* community = std::get_if<session::config::community_info>(&item)) {
            serialized = serialize_community_info(env, *community);
        } else if (auto* closed = std::get_if<session::config::group_info>(&item)) {
            serialized = serialize_closed_group_info(env, *closed);
        }

        return serialized;
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_all(JNIEnv *env, jobject thiz) {
    auto conf = ptrToUserGroups(env, thiz);
    return iterator_as_java_list(env, conf->begin(), conf->end());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_allCommunityInfo(JNIEnv *env,
                                                                               jobject thiz) {
    auto conf = ptrToUserGroups(env, thiz);
    return iterator_as_java_list(env, conf->begin_communities(), conf->end());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_allLegacyGroupInfo(JNIEnv *env,
                                                                                 jobject thiz) {
    auto conf = ptrToUserGroups(env, thiz);
    return iterator_as_java_list(env, conf->begin_legacy_groups(), conf->end());
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_eraseCommunity__Lnetwork_loki_messenger_libsession_1util_util_BaseCommunityInfo_2(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jobject base_community_info) {
    auto conf = ptrToUserGroups(env, thiz);
    auto base_community = deserialize_base_community(env, base_community_info);
    return conf->erase_community(base_community.base_url(),base_community.room());
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_eraseCommunity__Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jobject thiz, jstring server, jstring room) {
    auto conf = ptrToUserGroups(env, thiz);
    auto community = conf->get_community(
            JavaStringRef(env, server).view(),
            JavaStringRef(env, room).view());
    bool deleted = false;
    if (community) {
        deleted = conf->erase(*community);
    }
    return deleted;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_eraseLegacyGroup(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jstring account_id) {
    auto conf = ptrToUserGroups(env, thiz);
    bool return_bool = conf->erase_legacy_group(JavaStringRef(env, account_id).view());
    return return_bool;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getClosedGroup(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jstring session_id) {
    auto config = ptrToUserGroups(env, thiz);
    auto group = config->get_group(JavaStringRef(env, session_id).view());

    if (group) {
        return serialize_closed_group_info(env, *group).leak();
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getOrConstructClosedGroup(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jstring session_id) {
    auto config = ptrToUserGroups(env, thiz);
    auto group = config->get_or_construct_group(JavaStringRef(env, session_id).view());
    return serialize_closed_group_info(env, group).leak();
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_allClosedGroupInfo(JNIEnv *env,
                                                                                 jobject thiz) {
    auto conf = ptrToUserGroups(env, thiz);
    return iterator_as_java_list(env, conf->begin_groups(), conf->end());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_createGroup(JNIEnv *env,
                                                                          jobject thiz) {
    auto config = ptrToUserGroups(env, thiz);

    auto group = config->create_group();
    return serialize_closed_group_info(env, group).leak();
}

extern "C"
JNIEXPORT jlong JNICALL
  Java_network_loki_messenger_libsession_1util_UserGroupsConfig_sizeClosedGroup(JNIEnv *env,
                                                                              jobject thiz) {
    auto config = ptrToUserGroups(env, thiz);
    return config->size_groups();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_eraseClosedGroup(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jstring session_id) {
    auto config = ptrToUserGroups(env, thiz);
    bool return_value = config->erase_group(JavaStringRef(env, session_id).view());
    return return_value;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_util_GroupInfo_00024ClosedGroupInfo_adminKeyFromSeed(
        JNIEnv *env, jclass clazz, jbyteArray seed) {
    auto len = env->GetArrayLength(seed);
    if (len != 32) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "Seed must be 32 bytes");
        return nullptr;
    }

    auto admin_key = session::ed25519::ed25519_key_pair(
            JavaByteArrayRef(env, seed).get()).second;

    return util::bytes_from_span(env, std::span<const unsigned char>(admin_key.data(), admin_key.size())).leak();
}