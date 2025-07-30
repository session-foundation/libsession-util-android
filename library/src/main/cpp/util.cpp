#include "util.h"
#include "sodium/randombytes.h"
#include <sodium/crypto_sign.h>
#include <session/multi_encrypt.hpp>
#include <session/util.hpp>
#include <string>
#include "jni_utils.h"

#include <android/log.h>

#define  LOG_TAG    "libsession_util"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

namespace util {

    jbyteArray bytes_from_vector(JNIEnv* env, const std::vector<unsigned char> &from_str) {
        size_t length = from_str.size();
        auto jlength = (jsize)length;
        jbyteArray new_array = env->NewByteArray(jlength);
        env->SetByteArrayRegion(new_array, 0, jlength, (jbyte*)from_str.data());
        return new_array;
    }

    std::vector<unsigned char> vector_from_bytes(JNIEnv* env, jbyteArray byteArray) {
        if (byteArray == nullptr) {
            return {};
        }

        return jni_utils::JavaByteArrayRef(env, byteArray).copy();
    }

    jbyteArray bytes_from_span(JNIEnv* env, std::span<const unsigned char> from_str) {
        size_t length = from_str.size();
        auto jlength = (jsize)length;
        jbyteArray new_array = env->NewByteArray(jlength);
        env->SetByteArrayRegion(new_array, 0, jlength, (jbyte*)from_str.data());
        return new_array;
    }

    jobject serialize_user_pic(JNIEnv *env, session::config::profile_pic pic) {
        jni_utils::JavaLocalRef returnObjectClass(env, env->FindClass("network/loki/messenger/libsession_util/util/UserPic"));
        jmethodID constructor = env->GetMethodID(returnObjectClass.get(), "<init>", "(Ljava/lang/String;Lnetwork/loki/messenger/libsession_util/util/Bytes;)V");
        return env->NewObject(returnObjectClass.get(), constructor,
                              jni_utils::JavaLocalRef(env, env->NewStringUTF(pic.url.data())).get(),
                              jni_utils::session_bytes_from_range(env, pic.key)
                              );
    }


    session::config::profile_pic deserialize_user_pic(JNIEnv *env, jobject user_pic) {
        jni_utils::JavaLocalRef clazz(env, env->GetObjectClass(user_pic));
        return {
            jni_utils::JavaStringRef(env, (jstring) (env->CallObjectMethod(user_pic, env->GetMethodID(clazz.get(), "getUrl", "Ljava/lang/String;")))).view(),
            util::vector_from_bytes(env, static_cast<jbyteArray>(env->CallObjectMethod(user_pic, env->GetMethodID(clazz.get(), "getKeyAsByteArray", "()[B"))))
        };
    }

    jobject serialize_base_community(JNIEnv *env, const session::config::community& community) {
        auto base_community_clazz = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/BaseCommunityInfo"));
        jmethodID base_community_constructor = env->GetMethodID(base_community_clazz.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
        auto base_url = jni_utils::JavaLocalRef(env, env->NewStringUTF(community.base_url().data()));
        auto room = jni_utils::JavaLocalRef(env, env->NewStringUTF(community.room().data()));
        auto pubkey_jstring = jni_utils::JavaLocalRef(env, env->NewStringUTF(community.pubkey_hex().data()));
        return env->NewObject(base_community_clazz.get(), base_community_constructor, base_url.get(), room.get(), pubkey_jstring.get());
    }

    session::config::community deserialize_base_community(JNIEnv *env, jobject base_community) {
        jclass base_community_clazz = env->FindClass("network/loki/messenger/libsession_util/util/BaseCommunityInfo");
        jfieldID base_url_field = env->GetFieldID(base_community_clazz, "baseUrl", "Ljava/lang/String;");
        jfieldID room_field = env->GetFieldID(base_community_clazz, "room", "Ljava/lang/String;");
        jfieldID pubkey_hex_field = env->GetFieldID(base_community_clazz, "pubKeyHex", "Ljava/lang/String;");
        jni_utils::JavaLocalRef base_url(env, (jstring)env->GetObjectField(base_community,base_url_field));
        jni_utils::JavaLocalRef room(env, (jstring)env->GetObjectField(base_community, room_field));
        jni_utils::JavaLocalRef pub_key_hex(env, (jstring)env->GetObjectField(base_community, pubkey_hex_field));

        return session::config::community(
                jni_utils::JavaStringRef(env, base_url.get()).view(),
                jni_utils::JavaStringRef(env, room.get()).view(),
                jni_utils::JavaStringRef(env, pub_key_hex.get()).view()
        );
    }

    jobject serialize_expiry(JNIEnv *env, const session::config::expiration_mode& mode, const std::chrono::seconds& time_seconds) {
        auto none = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/ExpiryMode$NONE"));
        jfieldID none_instance = env->GetStaticFieldID(none.get(), "INSTANCE", "Lnetwork/loki/messenger/libsession_util/util/ExpiryMode$NONE;");
        auto after_send = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/ExpiryMode$AfterSend"));
        jmethodID send_init = env->GetMethodID(after_send.get(), "<init>", "(J)V");
        auto after_read = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/ExpiryMode$AfterRead"));
        jmethodID read_init = env->GetMethodID(after_read.get(), "<init>", "(J)V");

        if (mode == session::config::expiration_mode::none) {
            return env->GetStaticObjectField(none.get(), none_instance);
        } else if (mode == session::config::expiration_mode::after_send) {
            return env->NewObject(after_send.get(), send_init, time_seconds.count());
        } else if (mode == session::config::expiration_mode::after_read) {
            return env->NewObject(after_read.get(), read_init, time_seconds.count());
        }
        return nullptr;
    }

    std::pair<session::config::expiration_mode, long> deserialize_expiry(JNIEnv *env, jobject expiry_mode) {
        auto parent = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/ExpiryMode"));
        auto after_read = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/ExpiryMode$AfterRead"));
        auto after_send = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/ExpiryMode$AfterSend"));
        jfieldID duration_seconds = env->GetFieldID(parent.get(), "expirySeconds", "J");

        auto object_class = jni_utils::JavaLocalRef(env, env->GetObjectClass(expiry_mode));

        if (env->IsSameObject(object_class.get(), after_read.get())) {
            return std::pair(session::config::expiration_mode::after_read, env->GetLongField(expiry_mode, duration_seconds));
        } else if (env->IsSameObject(object_class.get(), after_send.get())) {
            return std::pair(session::config::expiration_mode::after_send, env->GetLongField(expiry_mode, duration_seconds));
        }
        return std::pair(session::config::expiration_mode::none, 0);
    }

    jobject serialize_group_member(JNIEnv* env, const session::config::groups::member& member) {
        auto group_member_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/GroupMember"));
        jmethodID constructor = env->GetMethodID(group_member_class.get(), "<init>", "(J)V");
        return env->NewObject(group_member_class.get(),
                              constructor,
                              reinterpret_cast<jlong>(new session::config::groups::member(member))
                          );
    }

    jobject deserialize_swarm_auth(JNIEnv *env, session::config::groups::Keys::swarm_auth auth) {
        auto swarm_auth_class = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/GroupKeysConfig$SwarmAuth"));
        jmethodID constructor = env->GetMethodID(swarm_auth_class.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
        auto sub_account = jni_utils::JavaLocalRef(env, env->NewStringUTF(auth.subaccount.data()));
        auto sub_account_sig = jni_utils::JavaLocalRef(env, env->NewStringUTF(auth.subaccount_sig.data()));
        auto signature = jni_utils::JavaLocalRef(env, env->NewStringUTF(auth.signature.data()));

        return env->NewObject(swarm_auth_class.get(), constructor, sub_account.get(), sub_account_sig.get(), signature.get());
    }

    jobject jlongFromOptional(JNIEnv* env, std::optional<long long> optional) {
        if (!optional) {
            return nullptr;
        }
        auto longClass = jni_utils::JavaLocalRef(env, env->FindClass("java/lang/Long"));
        jmethodID constructor = env->GetMethodID(longClass.get(), "<init>", "(J)V");
        return env->NewObject(longClass.get(), constructor, (jlong)*optional);
    }

    jstring jstringFromOptional(JNIEnv* env, std::optional<std::string_view> optional) {
        if (!optional) {
            return nullptr;
        }
        return env->NewStringUTF(optional->data());
    }
}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_util_MultiEncrypt_encryptForMultipleSimple(
        JNIEnv *env, jobject thiz, jobjectArray messages, jobjectArray recipients,
        jbyteArray ed25519_secret_key, jstring domain) {
    // messages and recipients have to be the same size
    uint size = env->GetArrayLength(messages);
    if (env->GetArrayLength(recipients) != size) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "Messages and recipients must be the same size");
        return nullptr;
    }
    std::vector<std::vector<unsigned char>> message_vec{};
    std::vector<std::vector<unsigned char>> recipient_vec{};
    for (int i = 0; i < size; i++) {
        jni_utils::JavaLocalRef message_j(env, static_cast<jbyteArray>(env->GetObjectArrayElement(messages, i)));
        jni_utils::JavaLocalRef recipient_j(env, static_cast<jbyteArray>(env->GetObjectArrayElement(recipients, i)));

        message_vec.emplace_back(jni_utils::JavaByteArrayRef(env, message_j.get()).copy());
        recipient_vec.emplace_back(jni_utils::JavaByteArrayRef(env, recipient_j.get()).copy());
    }

    std::vector<std::span<const unsigned char>> message_sv_vec{};
    std::vector<std::span<const unsigned char>> recipient_sv_vec{};
    for (int i = 0; i < size; i++) {
        message_sv_vec.emplace_back(session::to_span(message_vec[i]));
        recipient_sv_vec.emplace_back(session::to_span(recipient_vec[i]));
    }

    std::array<unsigned char, 24> random_nonce;
    randombytes_buf(random_nonce.data(), random_nonce.size());

    auto result = session::encrypt_for_multiple_simple(
            message_sv_vec,
            recipient_sv_vec,
            jni_utils::JavaByteArrayRef(env, ed25519_secret_key).get(),
            jni_utils::JavaStringRef(env, domain).view(),
            std::span<const unsigned char> {random_nonce.data(), 24}
    );

    auto encoded = util::bytes_from_vector(env, result);
    return encoded;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_util_MultiEncrypt_decryptForMultipleSimple(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jbyteArray encoded,
                                                                                        jbyteArray secret_key,
                                                                                        jbyteArray sender_pub_key,
                                                                                        jstring domain) {
    auto result = session::decrypt_for_multiple_simple(
            jni_utils::JavaByteArrayRef(env, encoded).get(),
            jni_utils::JavaByteArrayRef(env, secret_key).get(),
            jni_utils::JavaByteArrayRef(env, sender_pub_key).get(),
            jni_utils::JavaStringRef(env, domain).view()
            );

    if (result) {
        return util::bytes_from_vector(env, *result);
    } else {
        LOGD("no result from decrypt");
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_util_BaseCommunityInfo_00024Companion_parseFullUrl(
        JNIEnv *env, jobject thiz, jstring full_url) {
    auto [base, room, pk] = session::config::community::parse_full_url(jni_utils::JavaStringRef(env, full_url).view());

    jclass clazz = env->FindClass("kotlin/Triple");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)V");

    auto base_j = env->NewStringUTF(base.data());
    auto room_j = env->NewStringUTF(room.data());
    auto pk_jbytes = util::bytes_from_vector(env, pk);

    jobject triple = env->NewObject(clazz, constructor, base_j, room_j, pk_jbytes);
    return triple;
}
extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_util_BaseCommunityInfo_fullUrl(JNIEnv *env,
                                                                            jobject thiz) {
    auto deserialized = util::deserialize_base_community(env, thiz);
    auto full_url = deserialized.full_url();
    return env->NewStringUTF(full_url.data());
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_DEFAULT(JNIEnv *env, jobject thiz) {
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_USER_1PROFILE(JNIEnv *env, jobject thiz) {
    return (int) session::config::Namespace::UserProfile;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_CONTACTS(JNIEnv *env, jobject thiz) {
    return (int) session::config::Namespace::Contacts;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_CONVO_1INFO_1VOLATILE(JNIEnv *env, jobject thiz) {
    return (int) session::config::Namespace::ConvoInfoVolatile;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_USER_1GROUPS(JNIEnv *env, jobject thiz) {
    return (int) session::config::Namespace::UserGroups;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_GROUP_1INFO(JNIEnv *env, jobject thiz) {
    return (int) session::config::Namespace::GroupInfo;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_GROUP_1MEMBERS(JNIEnv *env, jobject thiz) {
    return (int) session::config::Namespace::GroupMembers;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_GROUP_1KEYS(JNIEnv *env, jobject thiz) {
    return (int) session::config::Namespace::GroupKeys;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_GROUP_1MESSAGES(JNIEnv *env, jobject thiz) {
    return  (int) session::config::Namespace::GroupMessages;
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_Namespace_REVOKED_1GROUP_1MESSAGES(JNIEnv *env, jobject thiz) {
    return -11; // we don't have revoked namespace in user configs
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_Config_free(JNIEnv *env, jobject thiz) {
    jclass baseClass = env->FindClass("network/loki/messenger/libsession_util/Config");
    jfieldID pointerField = env->GetFieldID(baseClass, "pointer", "J");
    jclass sig = env->FindClass("network/loki/messenger/libsession_util/ConfigSig");
    jclass base = env->FindClass("network/loki/messenger/libsession_util/ConfigBase");
    jclass ours = env->GetObjectClass(thiz);
    if (env->IsSameObject(sig, ours)) {
        // config sig object
        auto config = (session::config::ConfigSig*) env->GetLongField(thiz, pointerField);
        delete config;
    } else if (env->IsSameObject(base, ours)) {
        auto config = (session::config::ConfigBase*) env->GetLongField(thiz, pointerField);
        delete config;
    }
}
