#include "user_groups.h"
#include "oxenc/hex.h"

#include "session/ed25519.hpp"

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

    auto community = conf->get_community(jni_utils::JavaStringRef(env, base_url).view(), jni_utils::JavaStringRef(env, room).view());

    if (community) {
        return serialize_community_info(env, *community);
    }

    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getLegacyGroupInfo(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jstring account_id) {
    auto conf = ptrToUserGroups(env, thiz);
    auto legacy_group = conf->get_legacy_group(jni_utils::JavaStringRef(env, account_id).view());
    jobject return_group = nullptr;
    if (legacy_group) {
        return_group = serialize_legacy_group_info(env, *legacy_group);
    }
    return return_group;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getOrConstructCommunityInfo(
        JNIEnv *env, jobject thiz, jstring base_url, jstring room, jstring pub_key_hex) {
    auto conf = ptrToUserGroups(env, thiz);

    auto group = conf->get_or_construct_community(
            jni_utils::JavaStringRef(env, base_url).view(),
            jni_utils::JavaStringRef(env, room).view(),
            jni_utils::JavaStringRef(env, pub_key_hex).view());

    return serialize_community_info(env, group);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getOrConstructLegacyGroupInfo(
        JNIEnv *env, jobject thiz, jstring account_id) {
    auto conf = ptrToUserGroups(env, thiz);
    auto group = conf->get_or_construct_legacy_group(jni_utils::JavaStringRef(env, account_id).view());
    return serialize_legacy_group_info(env, group);
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
    auto conf = ptrToConvoInfo(env, thiz);
    return conf->size();
}

inline jobject iterator_as_java_list(JNIEnv *env, session::config::UserGroups::iterator begin, session::config::UserGroups::iterator end) {
    return jni_utils::jlist_from_iterator(env, begin, end, [](JNIEnv *env, const session::config::UserGroups::value_type &item) {
        std::optional<jobject> serialized = std::nullopt;
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
    auto base_community = util::deserialize_base_community(env, base_community_info);
    return conf->erase_community(base_community.base_url(),base_community.room());
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_eraseCommunity__Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jobject thiz, jstring server, jstring room) {
    auto conf = ptrToUserGroups(env, thiz);
    auto community = conf->get_community(
            jni_utils::JavaStringRef(env, server).view(),
            jni_utils::JavaStringRef(env, room).view());
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
    bool return_bool = conf->erase_legacy_group(jni_utils::JavaStringRef(env, account_id).view());
    return return_bool;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getClosedGroup(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jstring session_id) {
    auto config = ptrToUserGroups(env, thiz);
    auto group = config->get_group(jni_utils::JavaStringRef(env, session_id).view());

    if (group) {
        return serialize_closed_group_info(env, *group);
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserGroupsConfig_getOrConstructClosedGroup(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jstring session_id) {
    auto config = ptrToUserGroups(env, thiz);
    auto group = config->get_or_construct_group(jni_utils::JavaStringRef(env, session_id).view());
    return serialize_closed_group_info(env, group);
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
    return serialize_closed_group_info(env, group);
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
    bool return_value = config->erase_group(jni_utils::JavaStringRef(env, session_id).view());
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
            jni_utils::JavaByteArrayRef(env, seed).get()).second;

    return util::bytes_from_span(env, std::span<const unsigned char>(admin_key.data(), admin_key.size()));
}