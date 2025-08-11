#include <jni.h>

#include <session/session_encrypt.hpp>

#include "jni_utils.h"

#include <sodium.h>

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

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_decryptPushNotification(JNIEnv *env,
                                                                                    jobject thiz,
                                                                                    jbyteArray message,
                                                                                    jbyteArray secret_key) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto data = session::decrypt_push_notification(
                jni_utils::JavaByteArrayRef(env, message).get(),
                jni_utils::JavaByteArrayRef(env, secret_key).get()
        );

        return jni_utils::session_bytes_from_range(env, data);
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_decryptOnsResponse(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jstring lowercase_name,
                                                                               jbyteArray ciphertext,
                                                                               jbyteArray nonce) {
    return jni_utils::run_catching_cxx_exception_or_throws<jstring>(env, [=] {
        auto data = session::decrypt_ons_response(
                jni_utils::JavaStringRef(env, lowercase_name).view(),
                jni_utils::JavaByteArrayRef(env, ciphertext).get(),
                nonce ? std::make_optional(jni_utils::JavaByteArrayRef(env, nonce).get()) : std::nullopt
        );

        return util::jstringFromOptional(env, data);
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_calculateECHDAgreement(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jbyteArray x25519_pub_key,
                                                                                   jbyteArray x25519_priv_key) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        JavaByteArrayRef sk(env, x25519_priv_key);
        JavaByteArrayRef pk(env, x25519_pub_key);

        if (sk.get().size() != crypto_scalarmult_SCALARBYTES) {
            throw std::invalid_argument{"Invalid x25519_priv_key: expected 32 bytes"};
        }

        if (pk.get().size() != crypto_scalarmult_BYTES) {
            throw std::invalid_argument{"Invalid x25519_pub_key: expected 32 bytes"};
        }

        std::array<unsigned char, crypto_scalarmult_BYTES> shared_secret {0};
        if (crypto_scalarmult(shared_secret.data(),sk.get().data(), pk.get().data()) != 0) {
            throw std::runtime_error{"An error occurred while attempting to calculate the shared "
                                      "secret; is the key valid?"};
        }

        return util::bytes_from_span(env, shared_secret);
    });

}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_EncryptionStream_00024Companion_createEncryptionStreamState(
        JNIEnv *env, jobject thiz, jbyteArray javaKey, jbyteArray javaHeaderOut) {
    JavaByteArrayRef key(env, javaKey);
    JavaByteArrayRef headerOut(env, javaHeaderOut);

    if (headerOut.get().size() < crypto_secretstream_xchacha20poly1305_HEADERBYTES) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"),
                      "Invalid headerOut: not enough space");
        return 0;
    }

    if (key.get().size() != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"),
                      "Invalid key: unexpected size");
        return 0;
    }

    auto state = std::make_unique<crypto_secretstream_xchacha20poly1305_state>();
    crypto_secretstream_xchacha20poly1305_init_push(state.get(),
                                                    headerOut.get().data(),
                                                    key.get().data());

    return reinterpret_cast<jlong>(state.release());
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_EncryptionStream_00024Companion_encryptionStreamHeaderSize(
        JNIEnv *env, jobject thiz) {
    return static_cast<jint>(crypto_secretstream_xchacha20poly1305_HEADERBYTES);
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_EncryptionStream_00024Companion_encryptionStreamChunkOverhead(
        JNIEnv *env, jobject thiz) {
    return static_cast<jint>(crypto_secretstream_xchacha20poly1305_ABYTES);
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_EncryptionStream_00024Companion_encryptStreamPush(
        JNIEnv *env, jobject thiz, jlong state_ptr, jbyteArray java_in_buf, jint in_buf_size, jbyteArray java_out_buf) {
    auto state = reinterpret_cast<crypto_secretstream_xchacha20poly1305_state*>(state_ptr);

    JavaByteArrayRef in_buf(env, java_in_buf);
    JavaByteArrayRef out_buf(env, java_out_buf);

    unsigned long long cipher_len = out_buf.get().size();

    if (crypto_secretstream_xchacha20poly1305_push(
            state,
            out_buf.get().data(), &cipher_len, // Cipher data out
            in_buf.get().data(), in_buf_size, // Plaintext data in
            nullptr, 0, // Additional data (not used here)
            0 // Tag (not used here, can be 0 for message)
    )) {
        env->ThrowNew(env->FindClass("java/lang/IllegalStateException"),
                      "Failed to push data into encryption stream");
        return 0;
    }

    // Return the size of the ciphertext written to the output buffer
    return cipher_len;
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_EncryptionStream_00024Companion_destroyEncryptionStreamState(
        JNIEnv *env, jobject thiz, jlong state_ptr) {
    delete reinterpret_cast<crypto_secretstream_xchacha20poly1305_state*>(state_ptr);
}


extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_DecryptionStream_00024Companion_createDecryptionStreamState(
        JNIEnv *env, jobject thiz, jbyteArray javaKey, jbyteArray javaHeader) {
    JavaByteArrayRef key(env, javaKey);
    JavaByteArrayRef header(env, javaHeader);

    if (header.get().size() < crypto_secretstream_xchacha20poly1305_HEADERBYTES) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"),
                      "Invalid header: unexpected size");
        return 0;
    }

    if (key.get().size() != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"),
                      "Invalid key: unexpected size");
        return 0;
    }

    auto state = std::make_unique<crypto_secretstream_xchacha20poly1305_state>();

    if (crypto_secretstream_xchacha20poly1305_init_pull(state.get(), header.get().data(), key.get().data()) != 0) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"),
                      "Failed to initialize decryption stream state");
        return 0;
    }

    return reinterpret_cast<jlong>(state.release());
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_DecryptionStream_00024Companion_decryptionStreamPull(
        JNIEnv *env, jobject thiz, jlong native_state_ptr, jbyteArray java_in_buf, jint in_buf_len, jbyteArray java_out_buf) {
    JavaByteArrayRef out_buf(env, java_out_buf);
    JavaByteArrayRef in_buf(env, java_in_buf);

    unsigned long long mlen = out_buf.get().size();

    if (crypto_secretstream_xchacha20poly1305_pull(
            reinterpret_cast<crypto_secretstream_xchacha20poly1305_state*>(native_state_ptr),
            out_buf.get().data(), &mlen, // Plaintext data out
            nullptr, // Tag (not used here)
            in_buf.get().data(), in_buf_len, // Ciphertext data in
            nullptr, 0 // Additional data (not used here)
            )) {
        env->ThrowNew(env->FindClass("java/lang/IllegalStateException"),
                      "Failed to pull data from decryption stream");
        return 0;
    }

    return mlen;
}