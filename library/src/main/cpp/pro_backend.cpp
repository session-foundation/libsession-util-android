#include <jni.h>
#include "jni_utils.h"
#include "util.h"

#include <session/pro_backend.hpp>

using namespace jni_utils;

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_AddProPaymentRequests_buildJson(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jint version,
                                                                                 jbyteArray master_private_key,
                                                                                 jbyteArray rotating_private_key,
                                                                                 jint payment_provider,
                                                                                 jstring payment_transaction_id,
                                                                                 jstring payment_order_id) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=]() {
        return util::jstringFromOptional(env, session::pro_backend::AddProPaymentRequest::build_to_json(
                version,
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                static_cast<SESSION_PRO_BACKEND_PAYMENT_PROVIDER>(payment_provider),
                JavaStringRef(env, payment_transaction_id).get_raw(),
                JavaStringRef(env, payment_order_id).get_raw()
                )
        );
    });
}


extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProofResponse_00024Companion_nativeParseRaw(
        JNIEnv *env, jobject thiz, jstring json) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=]() {
        auto response = session::pro_backend::AddProPaymentOrGetProProofResponse::parse(
                JavaStringRef(env, json).view()
        );

        if (response.errors.empty()) {
            JavaLocalRef<jclass> pro_proof_class(env, env->FindClass("network/loki/messenger/libsession_util/pro/ProProof"));
            JavaLocalRef<jclass> success_class(env, env->FindClass("network/loki/messenger/libsession_util/pro/ProProofResponse$Success"));

            return env->NewObject(
                    success_class.get(),
                    env->GetMethodID(success_class.get(), "<init>", "(Lnetwork/loki/messenger/libsession_util/pro/ProProof;)V"),
                    env->NewObject(
                            pro_proof_class.get(),
                            env->GetMethodID(pro_proof_class.get(), "<init>", "(J)V"),
                            reinterpret_cast<jlong>(new session::ProProof(response.proof))
                    )
            );
        }

        JavaLocalRef<jclass> error_class(env, env->FindClass("network/loki/messenger/libsession_util/pro/ProProofResponse$Failure"));

        return env->NewObject(
                error_class.get(),
                env->GetMethodID(error_class.get(), "<init>", "(ILjava/util/List;)V"),
                static_cast<jint>(response.status),
                jstring_list_from_collection(env, response.errors)
        );
    });
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_GetProProofRequests_buildJson(JNIEnv *env,
                                                                               jint version,
                                                                               jobject thiz,
                                                                               jbyteArray master_private_key,
                                                                               jbyteArray rotating_private_key,
                                                                               jlong now_unix_ts) {
    return run_catching_cxx_exception_or_throws<jstring>(env, [=] () {
        return util::jstringFromOptional(env, session::pro_backend::GetProProofRequest::build_to_json(
                version,
                JavaByteArrayRef(env, master_private_key).get(),
                JavaByteArrayRef(env, rotating_private_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds> { std::chrono::milliseconds{static_cast<int64_t>(now_unix_ts)} }
        ));
    });
}