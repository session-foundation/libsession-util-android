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

        jclass kp_class = env->FindClass("network/loki/messenger/libsession_util/util/KeyPair");
        jmethodID kp_constructor = env->GetMethodID(kp_class, "<init>", "([B[B)V");
        return env->NewObject(kp_class, kp_constructor, util::bytes_from_vector(env, {pk.data(), pk.data() + pk.size()}), util::bytes_from_vector(env, {sk.data(), sk.data() + sk.size()}));
    });
}
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_util_BlindKeyAPI_blindVersionSign(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jbyteArray ed25519_secret_key,
                                                                               jlong timestamp) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto bytes = session::blind_version_sign(util::vector_from_bytes(env, ed25519_secret_key), session::Platform::android, timestamp);
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
        auto keyBytes = util::vector_from_bytes(env, ed25519_secret_key);
        auto bodyBytes = body ? std::optional(util::vector_from_bytes(env, body)) : std::nullopt;

        auto bytes = session::blind_version_sign_request(
                session::to_span(keyBytes),
                timestamp,
                methodC,
                pathC,
                body ? std::optional(session::to_span(*bodyBytes)) : std::nullopt
        );
        return util::bytes_from_vector(env, bytes);
    });
}