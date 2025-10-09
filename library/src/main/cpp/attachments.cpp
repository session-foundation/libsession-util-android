#include <jni.h>
#include <session/attachments.hpp>
#include "jni_utils.h"

using namespace session::attachment;
using namespace jni_utils;

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_Attachments_encryptedSize(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jlong plaintext_size) {
    return encrypted_size(plaintext_size);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_Attachments_decryptedMaxSize(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jlong ciphertext_size) {
    auto s = decrypted_max_size(ciphertext_size);
    if (s.has_value()) {
        return *s;
    } else {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"),
                      "ciphertext_size too small to be a valid encrypted attachment");
        return 0;
    }
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_Attachments_encryptBytes(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jbyteArray seed,
                                                                              jbyteArray plaintext_in,
                                                                              jbyteArray cipher_out,
                                                                              jint domain) {
    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto key = encrypt(
                JavaByteArrayRef(env, seed).get_as_bytes(),
                JavaByteArrayRef(env, plaintext_in).get_as_bytes(),
                static_cast<Domain>(domain),
                JavaByteArrayRef(env, cipher_out).get_as_bytes());


        return util::bytes_from_span(env, std::span(reinterpret_cast<unsigned char *>(key.data()), key.size()));
    });
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_encrypt_Attachments_decryptBytes(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jbyteArray key,
                                                                              jbyteArray cipher_in,
                                                                              jbyteArray plain_out) {
    return run_catching_cxx_exception_or_throws<jlong>(env, [=] {
        JavaByteArrayRef key_ref(env, key);

        return decrypt(
                JavaByteArrayRef(env, cipher_in).get_as_bytes(),
                std::span<std::byte, ENCRYPT_KEY_SIZE>(
                        reinterpret_cast<std::byte *>(key_ref.get().data()),
                        ENCRYPT_KEY_SIZE
                ),
                JavaByteArrayRef(env, plain_out).get_as_bytes()
        );
    });
}