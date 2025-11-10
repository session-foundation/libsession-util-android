#include <session/session_protocol.hpp>

#include <jni.h>
#include "util.h"


extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeGetVersion(
        JNIEnv *env, jobject thiz, jlong native_value) {
    return static_cast<jint>(
            reinterpret_cast<session::ProProof*>(native_value)->version);
}


extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeGetExpiry(
        JNIEnv *env, jobject thiz, jlong native_value) {
    return static_cast<jlong>(
            reinterpret_cast<session::ProProof*>(native_value)->expiry_unix_ts.time_since_epoch().count());
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeGetRotatingPubKey(
        JNIEnv *env, jobject thiz, jlong native_value) {
    const auto& rotating_pubkey =
            reinterpret_cast<session::ProProof*>(native_value)->rotating_pubkey;

    return util::bytes_from_span(env, rotating_pubkey);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeSerialize(
        JNIEnv *env, jobject thiz, jlong native_value) {
    // TODO: implement nativeSerialize()
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeDeserialize(
        JNIEnv *env, jobject thiz, jstring data) {
    // TODO: implement nativeDeserialize()
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeDestroy(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jlong native_value) {
    delete reinterpret_cast<session::ProProof*>(native_value);
}