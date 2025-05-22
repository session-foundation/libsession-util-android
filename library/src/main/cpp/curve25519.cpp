#include "jni_utils.h"

#include <session/curve25519.hpp>

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Curve25519_fromED25519(JNIEnv *env, jobject thiz,
                                                                    jbyteArray ed25519_public_key,
                                                                    jbyteArray ed25519_private_key) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto pk = session::curve25519::to_curve25519_pubkey(jni_utils::JavaByteArrayRef(env, ed25519_public_key).get());
        auto sk = session::curve25519::to_curve25519_seckey(jni_utils::JavaByteArrayRef(env, ed25519_private_key).get());

        return jni_utils::new_key_pair(env, util::bytes_from_span(env, pk), util::bytes_from_span(env, sk));
    });
}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_Curve25519_pubKeyFromED25519(JNIEnv *env, jobject thiz,
                                                                          jbyteArray ed25519_public_key) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto pk = session::curve25519::to_curve25519_pubkey(jni_utils::JavaByteArrayRef(env, ed25519_public_key).get());
        return util::bytes_from_span(env, pk);
    });
}