#include <jni.h>
#include "jni_utils.h"
#include "util.h"

#include <session/pro_backend.hpp>

using namespace jni_utils;

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildAddProPaymentRequestJson(
        JNIEnv *env, jobject thiz, jint version, jbyteArray master_private_key,
        jbyteArray rotating_private_key, jint payment_provider, jstring payment_id,
        jstring order_id) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        auto json = session::pro_backend::AddProPaymentRequest::build_to_json(
                version,
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                static_cast<SESSION_PRO_BACKEND_PAYMENT_PROVIDER>(payment_provider),
                JavaStringRef(env, payment_id).get_raw(),
                JavaStringRef(env, order_id).get_raw());

        return util::jstringFromOptional(env, json);
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildGenerateProProofRequestJson(
        JNIEnv *env, jobject thiz, jint version, jbyteArray master_private_key,
        jbyteArray rotating_private_key, jlong now_ms) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        auto json = session::pro_backend::GenerateProProofRequest::build_to_json(
                version,
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds> {
                    std::chrono::milliseconds(now_ms)
                }
        );

        return util::jstringFromOptional(env, json);
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildGetProDetailsRequestJson(
        JNIEnv *env, jobject thiz, jint version, jbyteArray pro_master_private_key, jlong now_ms,
        jint count) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        auto json = session::pro_backend::GetProDetailsRequest::build_to_json(
                version,
                JavaByteArrayRef(env, pro_master_private_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds> {
                    std::chrono::milliseconds(now_ms)
                },
                static_cast<uint32_t>(count)
        );

        return util::jstringFromOptional(env, json);
    });
}