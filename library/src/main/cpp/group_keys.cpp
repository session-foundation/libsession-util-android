#include "group_keys.h"
#include "group_info.h"
#include "group_members.h"

#include "jni_utils.h"

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
    auto pending_bytes = util::bytes_from_span(env, *pending);
    return pending_bytes;
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
    auto pending_bytes = util::bytes_from_span(env, *pending);
    return pending_bytes;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_rekey(JNIEnv *env, jobject thiz,
                                                                   jlong info_ptr, jlong members_ptr) {
    auto keys = ptrToKeys(env, thiz);
    auto info = reinterpret_cast<session::config::groups::Info*>(info_ptr);
    auto members = reinterpret_cast<session::config::groups::Members*>(members_ptr);
    auto rekey = keys->rekey(*info, *members);
    auto rekey_bytes = util::bytes_from_span(env, rekey);
    return rekey_bytes;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_dump(JNIEnv *env, jobject thiz) {
    auto keys = ptrToKeys(env, thiz);
    auto dump = keys->dump();
    auto byte_array = util::bytes_from_vector(env, dump);
    return byte_array;
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
        return util::bytes_from_vector(env, enc);
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
        auto sender_session_id = util::jstringFromOptional(env, sender.data());
        auto pair_class = env->FindClass("kotlin/Pair");
        auto pair_constructor = env->GetMethodID(pair_class, "<init>", "(Ljava/lang/Object;Ljava/lang/Object;)V");
        auto pair_obj = env->NewObject(pair_class, pair_constructor, plaintext_bytes, sender_session_id);
        return pair_obj;
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_keys(JNIEnv *env, jobject thiz) {
    auto ptr = ptrToKeys(env, thiz);
    return jni_utils::jlist_from_collection(env, ptr->group_keys(), util::bytes_from_span);
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
    auto ptr = ptrToKeys(env, thiz);
    auto new_subaccount_key = ptr->swarm_make_subaccount(jni_utils::JavaStringRef(env, session_id).view(), can_write, can_delete);
    auto jbytes = util::bytes_from_vector(env, new_subaccount_key);
    return jbytes;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_getSubAccountToken(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jstring session_id,
                                                                                jboolean can_write,
                                                                                jboolean can_delete) {
    auto ptr = ptrToKeys(env, thiz);
    auto token = ptr->swarm_subaccount_token(jni_utils::JavaStringRef(env, session_id).view(), can_write, can_delete);
    auto jbytes = util::bytes_from_vector(env, token);
    return jbytes;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_subAccountSign(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jbyteArray message,
                                                                            jbyteArray signing_value) {
    auto ptr = ptrToKeys(env, thiz);
    auto message_vector = util::vector_from_bytes(env, message);
    auto signing_value_vector = util::vector_from_bytes(env, signing_value);
    auto swarm_auth = ptr->swarm_subaccount_sign(message_vector, signing_value_vector, false);
    return util::deserialize_swarm_auth(env, swarm_auth);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_GroupKeysConfig_supplementFor(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jobjectArray j_user_session_ids) {
    auto ptr = ptrToKeys(env, thiz);
    std::vector<std::string> user_session_ids;
    for (int i = 0, size = env->GetArrayLength(j_user_session_ids); i < size; i++) {
        user_session_ids.push_back(jni_utils::JavaStringRef(env, jni_utils::JavaLocalRef(env, (jstring)(env->GetObjectArrayElement(j_user_session_ids, i))).get()).copy());
    }
    auto supplement = ptr->key_supplement(user_session_ids);
    return util::bytes_from_vector(env, supplement);
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
