#include <jni.h>
#include <session/blinding.hpp>
#include <session/util.hpp>

#include "util.h"
#include "jni_utils.h"

//
// Created by Thomas Ruffie on 29/7/2024.
//


extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_util_BlindKeyAPI_blindVersionKeyPair(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jbyteArray ed25519_secret_key) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        const auto [pk, sk] = session::blind_version_key_pair(util::vector_from_bytes(env, ed25519_secret_key));
        return jni_utils::new_key_pair(env, util::bytes_from_span(env, pk), util::bytes_from_span(env, sk));
    });
}
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_util_BlindKeyAPI_blindVersionSign(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jbyteArray ed25519_secret_key,
                                                                               jlong timestamp) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto bytes = session::blind_version_sign(
                jni_utils::JavaByteArrayRef(env, ed25519_secret_key).get(),
                session::Platform::android,
                timestamp
        );
        return util::bytes_from_vector(env, bytes);
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_util_BlindKeyAPI_blindVersionSignRequest(JNIEnv *env,
                                                                                      jobject thiz,
                                                                                      jbyteArray ed25519_secret_key,
                                                                                      jlong timestamp,
                                                                                      jstring method,
                                                                                      jstring path,
                                                                                      jbyteArray body) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto methodC = util::string_from_jstring(env, method);
        auto pathC = util::string_from_jstring(env, path);

        auto bytes = session::blind_version_sign_request(
                jni_utils::JavaByteArrayRef(env, ed25519_secret_key).get(),
                timestamp,
                methodC,
                pathC,
                body ? std::make_optional(jni_utils::JavaByteArrayRef(env, body).get()) : std::nullopt
        );
        return util::bytes_from_vector(env, bytes);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_util_BlindKeyAPI_blind15KeyPair(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jbyteArray ed25519_secret_key,
                                                                             jbyteArray server_pub_key) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto [pk, sk] = session::blind15_key_pair(
                jni_utils::JavaByteArrayRef(env, ed25519_secret_key).get(),
                jni_utils::JavaByteArrayRef(env, server_pub_key).get()
                );
        return jni_utils::new_key_pair(env, util::bytes_from_span(env, pk), util::bytes_from_span(env, sk));
    });
}