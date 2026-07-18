#include <jni.h>
#include "jni_utils.h"
#include "util.h"

#include <session/pro_backend.hpp>

using namespace jni_utils;

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildAddProPaymentRequestJson(
        JNIEnv *env, jobject thiz, jint version, jbyteArray master_private_key,
        jbyteArray rotating_private_key, jstring provider_code, jstring payment_id) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        JavaStringRef provider_code_ref(env, provider_code);
        JavaStringRef payment_id_ref(env, payment_id);
        auto payment_id_view = payment_id_ref.view();

        auto json = session::pro_backend::AddProPaymentRequest::build_to_json(
                version,
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                provider_code_ref.view(),
                std::span<const uint8_t>(
                        reinterpret_cast<const uint8_t*>(payment_id_view.data()),
                        payment_id_view.size()));

        return jni_utils::jstring_from_optional(env, json).release();
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildGenerateProProofRequestJson(
        JNIEnv *env, jobject thiz, jint version, jbyteArray master_private_key,
        jbyteArray rotating_private_key, jlong now_seconds) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        auto json = session::pro_backend::GenerateProProofRequest::build_to_json(
                version,
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                std::chrono::sys_seconds {
                    std::chrono::seconds(now_seconds)
                }
        );

        return jni_utils::jstring_from_optional(env, json).release();
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildGetProDetailsRequestJson(
        JNIEnv *env, jobject thiz, jint version, jbyteArray pro_master_private_key, jlong now_seconds,
        jint count) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        auto json = session::pro_backend::GetProDetailsRequest::build_to_json(
                version,
                JavaByteArrayRef(env, pro_master_private_key).get(),
                std::chrono::sys_seconds {
                    std::chrono::seconds(now_seconds)
                },
                static_cast<uint32_t>(count)
        );

        return jni_utils::jstring_from_optional(env, json).release();
    });
}
