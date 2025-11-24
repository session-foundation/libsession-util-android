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
        jmethodID get_gen_index_hash;
        jmethodID get_rotating_pub_key;
        jmethodID get_expiry_ms;
        jmethodID get_signature;

        ProProofMethods(JNIEnv *env, jobject obj)
            : JavaClassInfo(env, obj)
            , get_version(env->GetMethodID(java_class, "getVersion", "()I"))
            , get_gen_index_hash(env->GetMethodID(java_class, "getGenIndexHashHex", "()Ljava/lang/String;"))
            , get_rotating_pub_key(env->GetMethodID(java_class, "getRotatingPubKeyHex", "()Ljava/lang/String;"))
            , get_expiry_ms(env->GetMethodID(java_class, "getExpiryMs", "()J"))
            , get_signature(env->GetMethodID(java_class, "getSignatureHex", "()Ljava/lang/String;")) {}
    };

    // Cache method IDs
    static ProProofMethods methods(env, proof);

    jni_utils::JavaLocalRef<jstring> gen_index_hash(env, (jstring) env->CallObjectMethod(proof, methods.get_gen_index_hash));
    jni_utils::JavaLocalRef<jstring> rotating_pub_key(env, (jstring) env->CallObjectMethod(proof, methods.get_rotating_pub_key));
    jni_utils::JavaLocalRef<jstring> signature(env, (jstring) env->CallObjectMethod(proof, methods.get_signature));

    return {
            .version = static_cast<std::uint8_t>(env->CallIntMethod(proof, methods.get_version)),
            .gen_index_hash = from_hex<32>(jni_utils::JavaStringRef(env, gen_index_hash.get()).get()),
            .rotating_pubkey = from_hex<32>(jni_utils::JavaStringRef(env, rotating_pub_key.get()).get()),
            .expiry_unix_ts = std::chrono::sys_time<std::chrono::milliseconds>(
                    std::chrono::milliseconds(env->CallLongMethod(proof, methods.get_expiry_ms))),
            .sig = from_hex<64>(jni_utils::JavaStringRef(env, signature.get()).get()),
    };
}

jobject cpp_to_java_proof(JNIEnv *env, const session::ProProof &proof) {
    static BasicJavaClassInfo class_info(env,
            "network/loki/messenger/libsession_util/pro/ProProof",
            "(I[B[BJ[B)V");

    return env->NewObject(
            class_info.java_class,
            class_info.constructor,
            static_cast<jint>(proof.version),
            util::bytes_from_span(env, proof.gen_index_hash).get(),
            util::bytes_from_span(env, proof.rotating_pubkey).get(),
            static_cast<jlong>(proof.expiry_unix_ts.time_since_epoch().count()),
            util::bytes_from_span(env, proof.sig).get()
    );
}

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_nativeStatus(JNIEnv *env, jobject thiz,
                                                                       jlong now_unix_ts,
                                                                       jbyteArray verify_pub_key,
                                                                       jbyteArray signed_message_data,
                                                                       jbyteArray signed_message_signature) {
    return run_catching_cxx_exception_or_throws<jint>(env, [=]() {
        std::optional<session::ProSignedMessage> signed_msg;
        JavaByteArrayRef signed_message_data_ref(env, signed_message_data);
        JavaByteArrayRef signed_message_signature_ref(env, signed_message_signature);

        if (signed_message_data && signed_message_signature) {
            signed_msg.emplace(session::ProSignedMessage {
                    .sig = signed_message_signature_ref.get(),
                    .msg = signed_message_data_ref.get(),
            });
        }

        return static_cast<jint>(java_to_cpp_proof(env, thiz).status(
                JavaByteArrayRef(env, verify_pub_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds(now_unix_ts)},
                signed_msg
        ));
    });
}