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

        return jni_utils::jstring_from_optional(env, json).leak();
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

        return jni_utils::jstring_from_optional(env, json).leak();
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

        return jni_utils::jstring_from_optional(env, json).leak();
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_getPaymentProviderMetadata(
        JNIEnv *env, jobject thiz, jint payment_provider) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() -> jobject {
        if (payment_provider >= SESSION_PRO_BACKEND_PAYMENT_PROVIDER_COUNT || payment_provider < 0) {
            return nullptr;
        }

        const auto & metadata = SESSION_PRO_BACKEND_PAYMENT_PROVIDER_METADATA[payment_provider];

        static BasicJavaClassInfo class_info(
                env,
                "network/loki/messenger/libsession_util/protocol/PaymentProviderMetadata",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");


        return env->NewObject(
                class_info.java_class,
                class_info.constructor,
                jni_utils::jstring_from_optional(env, std::string_view(metadata.device.data, metadata.device.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.store.data, metadata.store.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.platform.data, metadata.platform.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.platform_account.data, metadata.platform_account.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.refund_platform_url.data, metadata.refund_platform_url.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.refund_support_url.data, metadata.refund_support_url.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.refund_status_url.data, metadata.refund_status_url.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.update_subscription_url.data, metadata.update_subscription_url.size)).get(),
                jni_utils::jstring_from_optional(env, std::string_view(metadata.cancel_subscription_url.data, metadata.cancel_subscription_url.size)).get()
            );
    });
}