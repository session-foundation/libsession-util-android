#include <session/config/groups/keys.hpp>

#include "jni_utils.h"
#include "util.h"
#include "config_base.h"

inline auto ptrToKeys(JNIEnv* env, jobject obj) {
    return dynamic_cast<session::config::groups::Keys*>(ptrToConfigSig(env, obj));
}

extern "C"
JNIEXPORT jint JNICALL
        Java_network_loki_messenger_libsession_1util_GroupKeysConfig_00024Companion_storageNamespace(JNIEnv* env,
                                                                                                     jobject thiz) {
    return (jint)session::config::Namespace::GroupKeys;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_00024Companion_newInstance(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jbyteArray user_secret_key,
                                                                                        jbyteArray group_public_key,
                                                                                        jbyteArray group_secret_key,
                                                                                        jbyteArray initial_dump,
                                                                                        jlong info_pointer,
                                                                                        jlong members_pointer) {
    return jni_utils::run_catching_cxx_exception_or_throws<jlong>(env, [=] {
        auto user_key_bytes = util::vector_from_bytes(env, user_secret_key);
        auto pub_key_bytes = util::vector_from_bytes(env, group_public_key);
        std::optional<std::vector<unsigned char>> secret_key_optional{std::nullopt};
        std::optional<std::vector<unsigned char>> initial_dump_optional{std::nullopt};

        if (group_secret_key && env->GetArrayLength(group_secret_key) > 0) {
            auto secret_key_bytes = util::vector_from_bytes(env, group_secret_key);
            secret_key_optional = secret_key_bytes;
        }

        if (initial_dump && env->GetArrayLength(initial_dump) > 0) {
            auto initial_dump_bytes = util::vector_from_bytes(env, initial_dump);
            initial_dump_optional = initial_dump_bytes;
        }

        auto info = reinterpret_cast<session::config::groups::Info*>(info_pointer);
        auto members = reinterpret_cast<session::config::groups::Members*>(members_pointer);

        auto* keys = new session::config::groups::Keys(user_key_bytes,
                                                       pub_key_bytes,
                                                       secret_key_optional,
                                                       initial_dump_optional,
                                                       *info,
                                                       *members);

        return reinterpret_cast<jlong>(keys);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_groupKeys(JNIEnv *env, jobject thiz) {
    auto config = ptrToKeys(env, thiz);
    return jni_utils::jlist_from_collection(env, config->group_keys(), util::bytes_from_span);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_loadKey(JNIEnv *env, jobject thiz,
                                                                     jbyteArray message,
                                                                     jstring hash,
                                                                     jlong timestamp_ms,
                                                                     jlong info_ptr,
                                                                     jlong members_ptr) {
    auto keys = ptrToKeys(env, thiz);
    auto info = reinterpret_cast<session::config::groups::Info*>(info_ptr);
    auto members = reinterpret_cast<session::config::groups::Members*>(members_ptr);

    auto processed = jni_utils::run_catching_cxx_exception_or_throws<jboolean>(env, [&] {
        return keys->load_key_message(
                jni_utils::JavaStringRef(env, hash).view(),
                jni_utils::JavaByteArrayRef(env, message).get(),
                timestamp_ms, *info, *members);
    });

    return processed;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_needsRekey(JNIEnv *env, jobject thiz) {
    auto keys = ptrToKeys(env, thiz);
    return keys->needs_rekey();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_needsDump(JNIEnv *env, jobject thiz) {
    auto keys = ptrToKeys(env, thiz);
    return keys->needs_dump();
}



extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_pendingKey(JNIEnv *env, jobject thiz) {
    auto keys = ptrToKeys(env, thiz);
    auto pending = keys->pending_key();
    if (!pending) {
        return nullptr;
    }
    return util::bytes_from_span(env, *pending).release();
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_pendingConfig(JNIEnv *env,
                                                                           jobject thiz) {
    auto keys = ptrToKeys(env, thiz);
    auto pending = keys->pending_config();
    if (!pending) {
        return nullptr;
    }
    return util::bytes_from_span(env, *pending).release();
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_rekey(JNIEnv *env, jobject thiz,
                                                                   jlong info_ptr, jlong members_ptr) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto keys = ptrToKeys(env, thiz);
        auto info = reinterpret_cast<session::config::groups::Info*>(info_ptr);
        auto members = reinterpret_cast<session::config::groups::Members*>(members_ptr);
        auto rekey = keys->rekey(*info, *members);
        return util::bytes_from_span(env, rekey).release();
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_dump(JNIEnv *env, jobject thiz) {
    auto keys = ptrToKeys(env, thiz);
    auto dump = keys->dump();
    return util::bytes_from_vector(env, dump).release();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_free(JNIEnv *env, jobject thiz) {
    auto ptr = ptrToKeys(env, thiz);
    delete ptr;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_encrypt(JNIEnv *env, jobject thiz,
                                                                     jbyteArray plaintext) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto ptr = ptrToKeys(env, thiz);
        auto plaintext_vector = util::vector_from_bytes(env, plaintext);
        auto enc = ptr->encrypt_message(plaintext_vector);
        return util::bytes_from_vector(env, enc).release();
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_decrypt(JNIEnv *env, jobject thiz,
                                                                     jbyteArray ciphertext) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto ptr = ptrToKeys(env, thiz);
        auto ciphertext_vector = util::vector_from_bytes(env, ciphertext);
        auto decrypted = ptr->decrypt_message(ciphertext_vector);
        auto sender = decrypted.first;
        auto plaintext = decrypted.second;
        auto plaintext_bytes = util::bytes_from_vector(env, plaintext);
        auto sender_session_id = jni_utils::jstring_from_optional(env, sender.data());

        return jni_utils::new_kotlin_pair(env, plaintext_bytes.get(), sender_session_id.get());
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_keys(JNIEnv *env, jobject thiz) {
    auto ptr = ptrToKeys(env, thiz);
    return jni_utils::jlist_from_collection(env, ptr->group_keys(), util::bytes_from_span);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_groupEncKey(JNIEnv *env, jobject thiz) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        return util::bytes_from_span(env, ptrToKeys(env, thiz)->group_enc_key()).release();
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_activeHashes(JNIEnv *env,
                                                                           jobject thiz) {
    auto ptr = ptrToKeys(env, thiz);
    return jni_utils::jstring_list_from_collection(env, ptr->active_hashes());
}
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_makeSubAccount(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jstring session_id,
                                                                            jboolean can_write,
                                                                            jboolean can_delete) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto new_subaccount_key = ptrToKeys(env, thiz)->swarm_make_subaccount(
                jni_utils::JavaStringRef(env, session_id).view(), can_write, can_delete);
        return util::bytes_from_vector(env, new_subaccount_key).release();
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_getSubAccountToken(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jstring session_id,
                                                                                jboolean can_write,
                                                                                jboolean can_delete) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto token = ptrToKeys(env, thiz)->swarm_subaccount_token(
                jni_utils::JavaStringRef(env, session_id).view(),
                can_write, can_delete);
        return util::bytes_from_vector(env, token).release();
    });
}

static jni_utils::JavaLocalRef<jobject> deserialize_swarm_auth(JNIEnv *env, session::config::groups::Keys::swarm_auth auth) {
    static jni_utils::BasicJavaClassInfo class_info(
            env, "network/loki/messenger/libsession_util/GroupKeysConfig$SwarmAuth", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

    jni_utils::JavaLocalRef sub_account(env, env->NewStringUTF(auth.subaccount.data()));
    jni_utils::JavaLocalRef sub_account_sig(env, env->NewStringUTF(auth.subaccount_sig.data()));
    jni_utils::JavaLocalRef signature(env, env->NewStringUTF(auth.signature.data()));

    return {env, env->NewObject(class_info.java_class, class_info.constructor, sub_account.get(), sub_account_sig.get(), signature.get())};
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_subAccountSign(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jbyteArray message,
                                                                            jbyteArray signing_value) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto ptr = ptrToKeys(env, thiz);
        auto message_vector = util::vector_from_bytes(env, message);
        auto signing_value_vector = util::vector_from_bytes(env, signing_value);
        auto swarm_auth = ptr->swarm_subaccount_sign(message_vector, signing_value_vector, false);
        return deserialize_swarm_auth(env, swarm_auth).release();
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_supplementFor(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jobjectArray j_user_session_ids) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto ptr = ptrToKeys(env, thiz);
        std::vector<std::string> user_session_ids;
        for (int i = 0, size = env->GetArrayLength(j_user_session_ids); i < size; i++) {
            jni_utils::JavaLocalRef element(
                    env, (jstring)(env->GetObjectArrayElement(j_user_session_ids, i)));

            user_session_ids.emplace_back(jni_utils::JavaStringRef(env, element.get()).view());
        }

        auto supplement = ptr->key_supplement(user_session_ids);
        return util::bytes_from_vector(env, supplement).release();
    });
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_currentGeneration(JNIEnv *env,
                                                                               jobject thiz) {
    auto ptr = ptrToKeys(env, thiz);
    return ptr->current_generation();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_admin(JNIEnv *env, jobject thiz) {
    auto ptr = ptrToKeys(env, thiz);
    return ptr->admin();
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_size(JNIEnv *env, jobject thiz) {
    auto ptr = ptrToKeys(env, thiz);
    return ptr->size();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_loadAdminKey(JNIEnv *env, jobject thiz,
                                                                          jbyteArray admin_key,
                                                                          jlong info_ptr,
                                                                          jlong members_ptr) {
    auto ptr = ptrToKeys(env, thiz);
    auto admin_key_vector = util::vector_from_bytes(env, admin_key);
    auto info = reinterpret_cast<session::config::groups::Info*>(info_ptr);
    auto members = reinterpret_cast<session::config::groups::Members*>(members_ptr);

    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [&] {
        ptr->load_admin_key(admin_key_vector, *info, *members);
    });
}
