#include <session/session_protocol.hpp>
#include <oxenc/base64.h>
#include <oxenc/hex.h>
#include <nlohmann/json.hpp>
#include <jni.h>

#include "util.h"
#include "jni_utils.h"
#include "pro_proof_util.h"

using namespace jni_utils;

template <size_t N>
static std::array<unsigned char, N> from_hex(std::span<char> input) {
    std::array<unsigned char, N> output = {0};
    oxenc::from_hex(input.begin(), input.end(), output.begin());
    return output;
}

session::ProProof java_to_cpp_proof(JNIEnv *env, jobject proof) {
    struct ProProofMethods : public JavaClassInfo {
        jmethodID get_version;
        jmethodID get_revocation_tag;
        jmethodID get_rotating_pub_key;
        jmethodID get_expiry_seconds;
        jmethodID get_signature;

        ProProofMethods(JNIEnv *env, jobject obj)
            : JavaClassInfo(env, obj)
            , get_version(env->GetMethodID(java_class, "getVersion", "()I"))
            , get_revocation_tag(env->GetMethodID(java_class, "getRevocationTagHex", "()Ljava/lang/String;"))
            , get_rotating_pub_key(env->GetMethodID(java_class, "getRotatingPubKeyHex", "()Ljava/lang/String;"))
            , get_expiry_seconds(env->GetMethodID(java_class, "getExpirySeconds", "()J"))
            , get_signature(env->GetMethodID(java_class, "getSignatureHex", "()Ljava/lang/String;")) {}
    };

    // Cache method IDs
    static ProProofMethods methods(env, proof);

    jni_utils::JavaLocalRef<jstring> revocation_tag(env, (jstring) env->CallObjectMethod(proof, methods.get_revocation_tag));
    jni_utils::JavaLocalRef<jstring> rotating_pub_key(env, (jstring) env->CallObjectMethod(proof, methods.get_rotating_pub_key));
    jni_utils::JavaLocalRef<jstring> signature(env, (jstring) env->CallObjectMethod(proof, methods.get_signature));

    return {
            .version = static_cast<std::uint8_t>(env->CallIntMethod(proof, methods.get_version)),
            .revocation_tag = from_hex<32>(jni_utils::JavaStringRef(env, revocation_tag.get()).get()),
            .rotating_pubkey = from_hex<32>(jni_utils::JavaStringRef(env, rotating_pub_key.get()).get()),
            .expiry_at = std::chrono::sys_seconds(
                    std::chrono::seconds(env->CallLongMethod(proof, methods.get_expiry_seconds))),
            .sig = from_hex<64>(jni_utils::JavaStringRef(env, signature.get()).get()),
    };
}

JavaLocalRef<jobject> cpp_to_java_proof(JNIEnv *env, const session::ProProof &proof) {
    static BasicJavaClassInfo class_info(env,
            "network/loki/messenger/libsession_util/pro/ProProof",
            "(I[B[BJ[B)V");

    return {env, env->NewObject(
            class_info.java_class,
            class_info.constructor,
            static_cast<jint>(proof.version),
            util::bytes_from_span(env, proof.revocation_tag).get(),
            util::bytes_from_span(env, proof.rotating_pubkey).get(),
            static_cast<jlong>(proof.expiry_at.time_since_epoch().count()),
            util::bytes_from_span(env, proof.sig).get()
    )};
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_nativeStatus(JNIEnv *env, jobject thiz,
                                                                       jlong now_unix_ts,
                                                                       jbyteArray verify_pub_key,
                                                                       jbyteArray signed_message_data,
                                                                       jbyteArray signed_message_signature) {
    return run_catching_cxx_exception_or_throws<jint>(env, [=]() {
        JavaByteArrayRef signed_message_data_ref(env, signed_message_data);
        JavaByteArrayRef signed_message_signature_ref(env, signed_message_signature);

        // §4: ProProof::status now takes an optional 64-byte user signature span + the message span
        // directly (ProSignedMessage was removed).
        std::optional<std::span<const std::uint8_t>> user_sig;
        std::span<const std::uint8_t> signed_msg;
        if (signed_message_data && signed_message_signature) {
            user_sig = signed_message_signature_ref.get();
            signed_msg = signed_message_data_ref.get();
        }

        return static_cast<jint>(java_to_cpp_proof(env, thiz).status(
                JavaByteArrayRef(env, verify_pub_key).get(),
                std::chrono::sys_seconds{std::chrono::seconds(now_unix_ts)},
                user_sig,
                signed_msg
        ));
    });
}