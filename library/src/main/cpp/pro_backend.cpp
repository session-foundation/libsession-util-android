#include <jni.h>
#include <optional>
#include <span>

#include <oxenc/hex.h>
#include <session/pro_backend.hpp>

#include "jni_utils.h"
#include "util.h"
#include "pro_proof_util.h"

using namespace jni_utils;

namespace {

namespace pb = session::pro_backend;

constexpr const char* PKG = "network/loki/messenger/libsession_util/pro/";

std::span<const uint8_t> string_to_span(std::string_view s) {
    return {reinterpret_cast<const uint8_t*>(s.data()), s.size()};
}

// --- struct -> Kotlin marshalling helpers ---

JavaLocalRef<jobject> serialize_response_header(JNIEnv* env, const pb::Response& r) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProResponseHeader",
                                  "(ILjava/util/List;)V");
    JavaLocalRef<jobject> errors(env, jstring_list_from_collection(env, r.errors));
    return {env, env->NewObject(cls.java_class, cls.constructor,
                                static_cast<jint>(r.status), errors.get())};
}

jobject serialize_pro_request(JNIEnv* env, const pb::ProRequest& r) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProRequest",
                                  "(Ljava/lang/String;Ljava/lang/String;)V");
    return env->NewObject(cls.java_class, cls.constructor,
                          jstring_from_optional(env, r.endpoint).get(),
                          jstring_from_optional(env, std::string_view(r.body)).get());
}

JavaLocalRef<jobject> serialize_provider_urls(JNIEnv* env, const pb::ProviderUrls& u) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProviderUrls",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    return {env, env->NewObject(cls.java_class, cls.constructor,
                                jstring_from_optional(env, u.refund_platform_url).get(),
                                jstring_from_optional(env, u.refund_support_url).get(),
                                jstring_from_optional(env, u.refund_status_url).get(),
                                jstring_from_optional(env, u.update_subscription_url).get(),
                                jstring_from_optional(env, u.cancel_subscription_url).get())};
}

JavaLocalRef<jobject> serialize_payment_item(JNIEnv* env, const pb::ProPaymentItem& it) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProPaymentItem",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZJJJJJJJLjava/lang/String;)V");
    return {env, env->NewObject(cls.java_class, cls.constructor,
            jstring_from_optional(env, std::string_view(it.status)).get(),   // opaque status code string
            jstring_from_optional(env, std::string_view(it.plan)).get(),
            jstring_from_optional(env, std::string_view(it.payment_provider)).get(),
            static_cast<jboolean>(it.auto_renewing),
            static_cast<jlong>(it.purchased_unix_ts.time_since_epoch().count()),           // ms
            static_cast<jlong>(it.redeemed_unix_ts.time_since_epoch().count()),            // s
            static_cast<jlong>(it.expiry_unix_ts.time_since_epoch().count()),              // s
            static_cast<jlong>(it.grace_period_duration.count()),                          // s
            static_cast<jlong>(it.platform_refund_expiry_unix_ts.time_since_epoch().count()), // s
            static_cast<jlong>(it.revoked_unix_ts.time_since_epoch().count()),             // ms
            static_cast<jlong>(it.refund_requested_unix_ts.time_since_epoch().count()),    // s
            jstring_from_optional(env, std::string_view(it.payment_id)).get())};
}

JavaLocalRef<jobject> serialize_revocation_item(JNIEnv* env, const pb::ProRevocationItem& it) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProRevocationItem",
                                  "(Ljava/lang/String;J)V");
    auto hex = oxenc::to_hex(it.revocation_tag.begin(), it.revocation_tag.end());
    return {env, env->NewObject(cls.java_class, cls.constructor,
            jstring_from_optional(env, std::string_view(hex)).get(),
            static_cast<jlong>(it.effective_unix_ts.time_since_epoch().count()))};
}

jobject serialize_pro_proof_response(JNIEnv* env, const pb::ProProofResponse& resp) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProProofResponse",
            "(Lnetwork/loki/messenger/libsession_util/pro/ProResponseHeader;"
            "Lnetwork/loki/messenger/libsession_util/pro/ProProof;)V");
    auto header = serialize_response_header(env, resp);
    JavaLocalRef<jobject> proof(env, nullptr);
    if (resp.errors.empty())
        proof = cpp_to_java_proof(env, resp.proof);
    return env->NewObject(cls.java_class, cls.constructor, header.get(), proof.get());
}

jobject serialize_get_details_response(JNIEnv* env, const pb::GetProDetailsResponse& resp) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/GetProDetailsResponse",
            "(Lnetwork/loki/messenger/libsession_util/pro/ProResponseHeader;Ljava/util/List;Ljava/lang/String;IZJJJI)V");
    auto header = serialize_response_header(env, resp);
    JavaLocalRef<jobject> items(env, jlist_from_collection(env, resp.items,
            [](JNIEnv* env, const pb::ProPaymentItem& it) -> std::optional<JavaLocalRef<jobject>> {
                return serialize_payment_item(env, it);
            }));
    return env->NewObject(cls.java_class, cls.constructor,
            header.get(), items.get(),
            jstring_from_optional(env, std::string_view(resp.user_status)).get(),   // opaque status code string
            static_cast<jint>(resp.error_report),
            static_cast<jboolean>(resp.auto_renewing),
            static_cast<jlong>(resp.expiry_unix_ts.time_since_epoch().count()),
            static_cast<jlong>(resp.grace_period_duration.count()),
            static_cast<jlong>(resp.refund_requested_unix_ts.time_since_epoch().count()),
            static_cast<jint>(resp.payments_total));
}

jobject serialize_revocations_response(JNIEnv* env, const pb::GetProRevocationsResponse& resp) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/GetProRevocationsResponse",
            "(Lnetwork/loki/messenger/libsession_util/pro/ProResponseHeader;JJJLjava/util/List;)V");
    auto header = serialize_response_header(env, resp);
    JavaLocalRef<jobject> items(env, jlist_from_collection(env, resp.items,
            [](JNIEnv* env, const pb::ProRevocationItem& it) -> std::optional<JavaLocalRef<jobject>> {
                return serialize_revocation_item(env, it);
            }));
    return env->NewObject(cls.java_class, cls.constructor,
            header.get(),
            static_cast<jlong>(resp.ticket),
            static_cast<jlong>(resp.retry_in.count()),
            static_cast<jlong>(resp.retain_for.count()),
            items.get());
}

jobject serialize_refund_response(JNIEnv* env, const pb::SetPaymentRefundRequestedResponse& resp) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/SetPaymentRefundRequestedResponse",
            "(Lnetwork/loki/messenger/libsession_util/pro/ProResponseHeader;Z)V");
    auto header = serialize_response_header(env, resp);
    return env->NewObject(cls.java_class, cls.constructor, header.get(),
                          static_cast<jboolean>(resp.updated));
}

}  // namespace

// --- Request builders (return ProRequest{endpoint, body}) ---

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildAddProPaymentRequest(
        JNIEnv* env, jobject, jbyteArray master_private_key, jbyteArray rotating_private_key,
        jstring provider_code, jstring payment_id) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef provider_ref(env, provider_code);
        JavaStringRef payment_ref(env, payment_id);
        auto req = pb::add_payment_request(
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                provider_ref.view(),
                string_to_span(payment_ref.view()));
        return serialize_pro_request(env, req);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildGenerateProProofRequest(
        JNIEnv* env, jobject, jbyteArray master_private_key, jbyteArray rotating_private_key,
        jlong now_seconds) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        auto req = pb::pro_proof_request(
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                std::chrono::sys_seconds{std::chrono::seconds(now_seconds)});
        return serialize_pro_request(env, req);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildGetProDetailsRequest(
        JNIEnv* env, jobject, jbyteArray master_private_key, jlong now_seconds, jint count) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        auto req = pb::payment_details_request(
                JavaByteArrayRef(env, master_private_key).get(),
                std::chrono::sys_seconds{std::chrono::seconds(now_seconds)},
                static_cast<uint32_t>(count));
        return serialize_pro_request(env, req);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildRefundRequest(
        JNIEnv* env, jobject, jbyteArray master_private_key, jlong now_seconds,
        jlong refund_requested_seconds, jstring provider_code, jstring payment_id) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef provider_ref(env, provider_code);
        JavaStringRef payment_ref(env, payment_id);
        auto req = pb::refund_request(
                JavaByteArrayRef(env, master_private_key).get(),
                std::chrono::sys_seconds{std::chrono::seconds(now_seconds)},
                std::chrono::sys_seconds{std::chrono::seconds(refund_requested_seconds)},
                provider_ref.view(),
                string_to_span(payment_ref.view()));
        return serialize_pro_request(env, req);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildRevocationsRequest(
        JNIEnv* env, jobject, jlong ticket) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        auto req = pb::revocations_request(static_cast<std::int64_t>(ticket));
        return serialize_pro_request(env, req);
    });
}

// --- Response parsers (return typed structs) ---

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_parseAddPaymentResponse(
        JNIEnv* env, jobject, jstring json) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef json_ref(env, json);
        return serialize_pro_proof_response(env, pb::parse_add_payment(json_ref.view()));
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_parseProProofResponse(
        JNIEnv* env, jobject, jstring json) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef json_ref(env, json);
        return serialize_pro_proof_response(env, pb::parse_pro_proof(json_ref.view()));
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_parsePaymentDetailsResponse(
        JNIEnv* env, jobject, jstring json) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef json_ref(env, json);
        return serialize_get_details_response(env, pb::parse_payment_details(json_ref.view()));
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_parseRefundResponse(
        JNIEnv* env, jobject, jstring json) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef json_ref(env, json);
        return serialize_refund_response(env, pb::parse_refund(json_ref.view()));
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_parseRevocationsResponse(
        JNIEnv* env, jobject, jstring json) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef json_ref(env, json);
        return serialize_revocations_response(env, pb::parse_revocations(json_ref.view()));
    });
}

// --- Provider URLs (source of truth in libsession; null if none) ---

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_providerUrls(
        JNIEnv* env, jobject, jstring provider_code) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() -> jobject {
        JavaStringRef code_ref(env, provider_code);
        auto urls = pb::provider_urls(code_ref.view());
        if (!urls)
            return nullptr;
        return serialize_provider_urls(env, *urls).release();
    });
}

// --- Backend identity constants (single source of truth in libsession) ---

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_proBackendUrl(
        JNIEnv* env, jobject) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        return env->NewStringUTF(std::string(pb::URL).c_str());
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_proBackendPubKeyHex(
        JNIEnv* env, jobject) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        auto hex = oxenc::to_hex(pb::PUBKEY.begin(), pb::PUBKEY.end());
        return env->NewStringUTF(hex.c_str());
    });
}
