#include "jni_utils.h"
#include "util.h"

#include <session/ed25519.hpp>

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_ED25519_sign(JNIEnv *env, jobject thiz,
                                                          jbyteArray ed25519_private_key,
                                                          jbyteArray message) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto sigs = session::ed25519::sign(
                jni_utils::JavaByteArrayRef(env, ed25519_private_key).get(),
                jni_utils::JavaByteArrayRef(env, message).get());

        return util::bytes_from_vector(env, sigs);
    });
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ED25519_verify(JNIEnv *env, jobject thiz,
                                                            jbyteArray ed25519_public_key,
                                                            jbyteArray message,
                                                            jbyteArray signature) {
    return jni_utils::run_catching_cxx_exception_or_throws<jboolean>(env, [=] {
        return session::ed25519::verify(
                jni_utils::JavaByteArrayRef(env, signature).get(),
                jni_utils::JavaByteArrayRef(env, ed25519_public_key).get(),
                jni_utils::JavaByteArrayRef(env, message).get()
                );
    });
}