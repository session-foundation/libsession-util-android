#include <jni.h>
#include <session/blinding.hpp>

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
        const auto [pk, sk] = session::blind_version_key_pair(util::ustring_from_bytes(env, ed25519_secret_key));

        jclass kp_class = env->FindClass("network/loki/messenger/libsession_util/util/KeyPair");
        jmethodID kp_constructor = env->GetMethodID(kp_class, "<init>", "([B[B)V");
        return env->NewObject(kp_class, kp_constructor, util::bytes_from_ustring(env, {pk.data(), pk.size()}), util::bytes_from_ustring(env, {sk.data(), sk.size()}));
    });
}
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_util_BlindKeyAPI_blindVersionSign(JNIEnv *env,
                                                                               jobject thiz,
                                                                               jbyteArray ed25519_secret_key,
                                                                               jlong timestamp) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto bytes = session::blind_version_sign(util::ustring_from_bytes(env, ed25519_secret_key), session::Platform::android, timestamp);
        return util::bytes_from_ustring(env, bytes);
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
                util::ustring_from_bytes(env, ed25519_secret_key),
                timestamp,
                methodC,
                pathC,
                body ? std::optional(util::ustring_from_bytes(env, body)) : std::nullopt
        );
        return util::bytes_from_ustring(env, bytes);
    });
}