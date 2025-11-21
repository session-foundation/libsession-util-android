#include "jni_utils.h"
#include "util.h"

#include <session/ed25519.hpp>
#include <session/xed25519.hpp>

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

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ED25519_generate(JNIEnv *env, jobject thiz,
                                                              jbyteArray seed) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto [pk, sk] = seed
                ? session::ed25519::ed25519_key_pair(jni_utils::JavaByteArrayRef(env, seed).get())
                : session::ed25519::ed25519_key_pair();

        return jni_utils::new_key_pair(env, util::bytes_from_span(env, pk), util::bytes_from_span(env, sk));
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_ED25519_generateProMasterKey(JNIEnv *env, jobject thiz,
                                                                        jbyteArray ed25519_seed) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        return util::bytes_from_span(
                env,
                session::ed25519::ed25519_pro_privkey_for_ed25519_seed(
                        jni_utils::JavaByteArrayRef(env, ed25519_seed).get()
                )
        );

    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_ED25519_positiveEd25519PubKeyFromCurve25519(
        JNIEnv *env, jobject thiz, jbyteArray curve25519_pub_key) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        return util::bytes_from_span(
                env,
                session::xed25519::pubkey(
                        jni_utils::JavaByteArrayRef(env, curve25519_pub_key).get())
        );
    });
}