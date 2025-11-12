#include <jni.h>
#include "jni_utils.h"
#include "util.h"

#include <session/pro_backend.hpp>

using namespace jni_utils;

static jobject serializeMasterRotatingSignatures(
        JNIEnv *env, const session::pro_backend::MasterRotatingSignatures &sigs) {
    JavaLocalRef<jclass> result_class(
            env,
            env->FindClass(
                    "network/loki/messenger/libsession_util/pro/BackendRequests$MasterRotatingSignatures"));

    return env->NewObject(
            result_class.get(),
            env->GetMethodID(result_class.get(), "<init>", "([B[B)V"),
            util::bytes_from_span(env, sigs.master_sig),
            util::bytes_from_span(env, sigs.rotating_sig)
    );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_signAddProPaymentRequest(
        JNIEnv *env, jobject thiz, jint version, jbyteArray master_private_key,
        jbyteArray rotating_private_key, jint payment_provider, jstring payment_id,
        jstring order_id) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto sigs = session::pro_backend::AddProPaymentRequest::build_sigs(
                static_cast<std::uint8_t>(version),
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                static_cast<SESSION_PRO_BACKEND_PAYMENT_PROVIDER>(
                        payment_provider),
                JavaStringRef(env, payment_id).get_raw(),
                JavaStringRef(env, order_id).get_raw());

        return serializeMasterRotatingSignatures(env, sigs);
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_signGetProProofRequest(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jint version,
                                                                                        jbyteArray master_private_key,
                                                                                        jbyteArray rotating_private_key,
                                                                                        jlong unix_ts_ms) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto sigs = session::pro_backend::GetProProofRequest::build_sigs(
                version,
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds>{
                        std::chrono::milliseconds{unix_ts_ms}}
        );

        return serializeMasterRotatingSignatures(env, sigs);
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_pro_BackendRequests_signGetProStatusRequest(
        JNIEnv *env, jobject thiz, jint version, jbyteArray master_private_key, jlong now_ms,
        jint count) {
    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        auto sig = session::pro_backend::GetProStatusRequest::build_sig(
                static_cast<std::uint8_t>(version),
                JavaByteArrayRef(env, master_private_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds>{
                        std::chrono::milliseconds{now_ms}},
                static_cast<std::uint32_t>(count)
        );

        return util::bytes_from_span(env, sig);
    });
}