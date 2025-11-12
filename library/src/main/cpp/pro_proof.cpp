#include <session/session_protocol.hpp>
#include <oxenc/base64.h>
#include <nlohmann/json.hpp>
#include <jni.h>

#include "util.h"
#include "jni_utils.h"

using namespace jni_utils;

extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeStatus(JNIEnv *env,
                                                                                      jobject thiz,
                                                                                      jint version,
                                                                                      jbyteArray gen_index_hash,
                                                                                      jbyteArray rotating_pub_key,
                                                                                      jlong expiry_ms,
                                                                                      jbyteArray signature,
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

        session::ProProof pro_proof {
                .version = static_cast<std::uint8_t>(version),
                .gen_index_hash = *java_to_cpp_array<32>(env, gen_index_hash),
                .rotating_pubkey = *java_to_cpp_array<32>(env, rotating_pub_key),
                .expiry_unix_ts = std::chrono::sys_time<std::chrono::milliseconds>(
                        std::chrono::milliseconds(expiry_ms)),
                .sig = *java_to_cpp_array<64>(env, signature),
        };

        return static_cast<jint>(pro_proof.status(
                JavaByteArrayRef(env, verify_pub_key).get(),
                std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds(now_unix_ts)},
                signed_msg
        ));
    });
}