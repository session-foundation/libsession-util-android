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

JavaLocalRef<jobject> serialize_response_header(JNIEnv* env, const pb::ResponseBase& r) {
    // Delta #12: status is now an enum (Ok/Fail/Error), plus an optional machine slug (error_code)
    // and an optional English diagnostic (error); no more errors[] array. We marshal status as its
    // ordinal (int) — the Kotlin @Keep ctor maps it to the ProResponseStatus enum.
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProResponseHeader",
                                  "(ILjava/lang/String;Ljava/lang/String;)V");
    auto sv = [](const std::optional<std::string>& o) -> std::optional<std::string_view> {
        if (o)
            return std::string_view{*o};
        return std::nullopt;
    };
    return {env, env->NewObject(cls.java_class, cls.constructor,
                                static_cast<jint>(r.status),
                                jstring_from_optional(env, sv(r.error_code)).get(),
                                jstring_from_optional(env, sv(r.error)).get())};
}

jobject serialize_pro_request(JNIEnv* env, const pb::ProRequest& r) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProRequest",
                                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    return env->NewObject(cls.java_class, cls.constructor,
                          jstring_from_optional(env, r.endpoint).get(),
                          jstring_from_optional(env, r.content_type).get(),
                          jstring_from_optional(env, std::string_view(r.data)).get());
}

JavaLocalRef<jobject> serialize_provider_urls(JNIEnv* env, const pb::ProviderURLs& u) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProviderUrls",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    return {env, env->NewObject(cls.java_class, cls.constructor,
                                jstring_from_optional(env, u.refund_platform_url).get(),
                                jstring_from_optional(env, u.refund_support_url).get(),
                                jstring_from_optional(env, u.refund_status_url).get(),
                                jstring_from_optional(env, u.update_subscription_url).get(),
                                jstring_from_optional(env, u.cancel_subscription_url).get())};
}

// Delta #14/#15: `plan` is now a parsed ProPlanPeriod (count + unit) rather than a raw slug. Render it
// back to the canonical "<N><unit>" slug (or "lifetime") the Kotlin/app layer still consumes as a
// String, so the app-facing shape is unchanged.
std::string plan_to_string(const pb::ProPlanPeriod& plan) {
    switch (plan.unit) {
        case pb::ProPlanUnit::second: return std::to_string(plan.count) + "s";
        case pb::ProPlanUnit::day: return std::to_string(plan.count) + "d";
        case pb::ProPlanUnit::week: return std::to_string(plan.count) + "w";
        case pb::ProPlanUnit::month: return std::to_string(plan.count) + "m";
        case pb::ProPlanUnit::year: return std::to_string(plan.count) + "y";
        case pb::ProPlanUnit::lifetime: return "lifetime";
    }
    return "";
}

JavaLocalRef<jobject> serialize_payment_item(JNIEnv* env, const pb::ProPaymentItem& it) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProPaymentItem",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZJJJJJJJLjava/lang/String;)V");
    auto plan = plan_to_string(it.plan);
    return {env, env->NewObject(cls.java_class, cls.constructor,
            jstring_from_optional(env, std::string_view(it.status)).get(),   // opaque status code string
            jstring_from_optional(env, std::string_view(plan)).get(),
            jstring_from_optional(env, std::string_view(it.payment_provider)).get(),
            static_cast<jboolean>(it.auto_renewing),
            static_cast<jlong>(it.purchased_at.time_since_epoch().count()),           // ms
            static_cast<jlong>(it.redeemed_at.time_since_epoch().count()),            // s
            static_cast<jlong>(it.expiry_at.time_since_epoch().count()),              // s
            static_cast<jlong>(it.grace_period_duration.count()),                          // s
            static_cast<jlong>(it.platform_refund_expiry_at.time_since_epoch().count()), // s
            static_cast<jlong>(it.revoked_at.time_since_epoch().count()),             // ms
            static_cast<jlong>(it.refund_requested_at.time_since_epoch().count()),    // s
            jstring_from_optional(env, std::string_view(it.payment_id)).get())};
}

JavaLocalRef<jobject> serialize_revocation_item(JNIEnv* env, const pb::ProRevocationItem& it) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProRevocationItem",
                                  "(Ljava/lang/String;J)V");
    auto hex = oxenc::to_hex(it.revocation_tag.begin(), it.revocation_tag.end());
    return {env, env->NewObject(cls.java_class, cls.constructor,
            jstring_from_optional(env, std::string_view(hex)).get(),
            static_cast<jlong>(it.effective_at.time_since_epoch().count()))};
}

jobject serialize_pro_proof_response(JNIEnv* env, const pb::ProProofResponse& resp) {
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/ProProofResponse",
            "(Lnetwork/loki/messenger/libsession_util/pro/ProResponseHeader;"
            "Lnetwork/loki/messenger/libsession_util/pro/ProProof;)V");
    auto header = serialize_response_header(env, resp);
    JavaLocalRef<jobject> proof(env, nullptr);
    if (resp)  // ResponseBase::operator bool: true iff status == Ok (proof populated on success)
        proof = cpp_to_java_proof(env, resp.proof);
    return env->NewObject(cls.java_class, cls.constructor, header.get(), proof.get());
}

jobject serialize_pro_status_response(JNIEnv* env, const pb::ProStatusResponse& resp) {
    // Delta #15: get_pro_details split into get_pro_status; the response now carries a single optional
    // latest_payment (has-flag + nullable item) instead of an items[] list + payments_total.
    static BasicJavaClassInfo cls(env, "network/loki/messenger/libsession_util/pro/GetProStatusResponse",
            "(Lnetwork/loki/messenger/libsession_util/pro/ProResponseHeader;Ljava/lang/String;Z"
            "Lnetwork/loki/messenger/libsession_util/pro/ProPaymentItem;IZJJJ)V");
    auto header = serialize_response_header(env, resp);
    JavaLocalRef<jobject> latest_payment(env, nullptr);
    if (resp.latest_payment)
        latest_payment = serialize_payment_item(env, *resp.latest_payment);
    return env->NewObject(cls.java_class, cls.constructor,
            header.get(),
            jstring_from_optional(env, std::string_view(resp.user_status)).get(),   // opaque status code string
            static_cast<jboolean>(resp.latest_payment.has_value()),
            latest_payment.get(),
            static_cast<jint>(resp.error_report),
            static_cast<jboolean>(resp.auto_renewing),
            static_cast<jlong>(resp.expiry_at.time_since_epoch().count()),
            static_cast<jlong>(resp.grace_period_duration.count()),
            static_cast<jlong>(resp.refund_requested_at.time_since_epoch().count()));
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
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_buildGetProStatusRequest(
        JNIEnv* env, jobject, jbyteArray master_private_key, jlong now_seconds) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        auto req = pb::pro_status_request(
                JavaByteArrayRef(env, master_private_key).get(),
                std::chrono::sys_seconds{std::chrono::seconds(now_seconds)});
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
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_parseProStatusResponse(
        JNIEnv* env, jobject, jstring json) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        JavaStringRef json_ref(env, json);
        return serialize_pro_status_response(env, pb::parse_pro_status(json_ref.view()));
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

// --- Visible (purchasable) payment-provider slugs (single source of truth in libsession) ---

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_visiblePlatforms(
        JNIEnv* env, jobject) {
    return run_catching_cxx_exception_or_throws<jobjectArray>(env, [=]() -> jobjectArray {
        auto platforms = pb::visible_platforms();  // std::span<const std::string_view>
        jclass string_cls = env->FindClass("java/lang/String");
        jobjectArray arr = env->NewObjectArray(
                static_cast<jsize>(platforms.size()), string_cls, nullptr);
        jsize i = 0;
        for (auto slug : platforms) {
            std::string s(slug);
            jstring js = env->NewStringUTF(s.c_str());
            env->SetObjectArrayElement(arr, i++, js);
            env->DeleteLocalRef(js);
        }
        return arr;
    });
}
