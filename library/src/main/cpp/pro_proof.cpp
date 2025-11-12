#include <session/session_protocol.hpp>
#include <oxenc/base64.h>
#include <nlohmann/json.hpp>
#include <jni.h>

#include "util.h"
#include "jni_utils.h"

using namespace jni_utils;

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_status(JNIEnv *env, jobject thiz,
                                                                 jbyteArray verify_pub_key,
                                                                 jlong now_unix_ts,
                                                                 jbyteArray signed_message_data,
                                                                 jbyteArray signed_message_signature) {
    return run_catching_cxx_exception_or_throws<jint>(env, [=]() {
        JavaLocalRef<jclass> clazz(env, env->GetObjectClass(thiz));
        auto get_version_method = env->GetMethodID(clazz.get(), "getVersion", "()I");
        auto get_gen_index_hash_method = env->GetMethodID(clazz.get(), "getGenIndexHash", "()[B");
        auto get_rotating_pub_key_method = env->GetMethodID(clazz.get(), "getRotatingPubKey",
                                                            "()[B");
        auto get_expiry_method = env->GetMethodID(clazz.get(), "getExpiryMs", "()J");
        auto get_signature_method = env->GetMethodID(clazz.get(), "getSignature", "()[B");

        auto version = env->CallIntMethod(thiz, get_version_method);
        JavaLocalRef<jbyteArray> gen_index_hash_bytes_obj(env,
                                                          static_cast<jbyteArray>(env->CallObjectMethod(
                                                                  thiz,
                                                                  get_gen_index_hash_method)));
        JavaLocalRef<jbyteArray> rotating_pub_key_bytes_obj(env,
                                                            static_cast<jbyteArray>(env->CallObjectMethod(
                                                                    thiz,
                                                                    get_rotating_pub_key_method)));
        auto expiry_ms = env->CallLongMethod(thiz, get_expiry_method);
        JavaLocalRef<jbyteArray> signature_bytes_obj(env,
                                                     static_cast<jbyteArray>(env->CallObjectMethod(
                                                             thiz, get_signature_method)));

        std::optional<session::ProSignedMessage> signed_msg;
        JavaByteArrayRef signed_message_data_ref(env, signed_message_data);
        JavaByteArrayRef signed_message_signature_ref(env, signed_message_signature);

        if (signed_message_data && signed_message_signature) {
            signed_msg.emplace(session::ProSignedMessage {
                .sig = signed_message_signature_ref.get(),
                .msg = signed_message_data_ref.get(),
            });
        }

        session::ProProof pro_proof {
                .version = static_cast<std::uint8_t>(version),
                .gen_index_hash = *java_to_cpp_array<32>(env, gen_index_hash_bytes_obj.get()),
                .rotating_pubkey = *java_to_cpp_array<32>(env, rotating_pub_key_bytes_obj.get()),
                .expiry_unix_ts = std::chrono::sys_time<std::chrono::milliseconds>(
                        std::chrono::milliseconds(expiry_ms)),
                .sig = *java_to_cpp_array<64>(env, signature_bytes_obj.get()),
        };

        return static_cast<jint>(pro_proof.status(
                JavaByteArrayRef(env, verify_pub_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds(now_unix_ts)},
                signed_msg
        ));
    });
}