#include <jni.h>

#include <session/session_encrypt.hpp>

#include "jni_utils.h"

using jni_utils::JavaByteArrayRef;

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_decryptForBlindedRecipient(
        JNIEnv *env,
        jobject _thiz,
        jbyteArray ciphertext,
        jbyteArray my_ed25519_privte_key,
        jbyteArray open_group_public_key,
        jbyteArray sender_blinded_id,
        jbyteArray recipient_blind_id) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto [plain_text, session_id] = session::decrypt_from_blinded_recipient(
                JavaByteArrayRef(env, my_ed25519_privte_key).get(),
                JavaByteArrayRef(env, open_group_public_key).get(),
                JavaByteArrayRef(env, sender_blinded_id).get(),
                JavaByteArrayRef(env, recipient_blind_id).get(),
                JavaByteArrayRef(env, ciphertext).get()
        );

        return jni_utils::new_kotlin_pair(
                env,
                util::jstringFromOptional(env, session_id),
                jni_utils::session_bytes_from_range(env, plain_text)
        );
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_encryptForRecipient(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jbyteArray ed25519_private_key,
                                                                                jbyteArray recipient_x25519_public_key,
                                                                                jbyteArray message) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto data = session::encrypt_for_recipient(
                JavaByteArrayRef(env, ed25519_private_key).get(),
                JavaByteArrayRef(env, recipient_x25519_public_key).get(),
                JavaByteArrayRef(env, message).get()
        );

        return jni_utils::session_bytes_from_range(env, data);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_decryptIncoming(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jbyteArray x25519_pub_key,
                                                                            jbyteArray x25519_priv_key,
                                                                            jbyteArray ciphertext) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto [plain_text, session_id] = session::decrypt_incoming_session_id(
                JavaByteArrayRef(env, x25519_pub_key).get(),
                JavaByteArrayRef(env, x25519_priv_key).get(),
                JavaByteArrayRef(env, ciphertext).get()
        );

        return jni_utils::new_kotlin_pair(
                env,
                util::jstringFromOptional(env, session_id),
                jni_utils::session_bytes_from_range(env, plain_text)
        );
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_encryptForBlindedRecipient(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jbyteArray message,
                                                                                       jbyteArray my_ed25519_privkey,
                                                                                       jbyteArray server_pub_key,
                                                                                       jbyteArray recipient_blind_id) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto data = session::encrypt_for_blinded_recipient(
                JavaByteArrayRef(env, my_ed25519_privkey).get(),
                JavaByteArrayRef(env, server_pub_key).get(),
                JavaByteArrayRef(env, recipient_blind_id).get(),
                JavaByteArrayRef(env, message).get()
        );

        return jni_utils::session_bytes_from_range(env, data);
    });
}