#include <jni.h>
#include <session/session_protocol.hpp>
#include <session/sodium_array.hpp>

#include "jni_utils.h"

using namespace jni_utils;


static JavaLocalRef<jobject> serializeProProof(JNIEnv *env, const session::ProProof &proof) {
    JavaLocalRef<jclass> pro_proof_clazz(env, env->FindClass(
            "network/loki/messenger/libsession_util/pro/ProProof"));
    jmethodID init = env->GetMethodID(pro_proof_clazz.get(), "<init>", "(I[B[BJ[B)V");
    return {env, env->NewObject(
            pro_proof_clazz.get(),
            init,
            static_cast<jint>(proof.version),
            util::bytes_from_span(env, proof.gen_index_hash),
            util::bytes_from_span(env, proof.rotating_pubkey),
            static_cast<jlong>(proof.expiry_unix_ts.time_since_epoch().count()),
            util::bytes_from_span(env, proof.sig)
    )};
}

static JavaLocalRef<jobject> serializeEnvelop(JNIEnv *env, const session::Envelope &envelope) {
    JavaLocalRef envelopClass(env, env->FindClass(
            "network/loki/messenger/libsession_util/protocol/Envelope"));
    jmethodID init = env->GetMethodID(
            envelopClass.get(),
            "<init>",
            "(J[BJ[B)V"
    );

    return {env, env->NewObject(envelopClass.get(),
                                init,
                                static_cast<jlong>(envelope.timestamp.count()),
                                (envelope.flags & SESSION_PROTOCOL_ENVELOPE_FLAGS_SOURCE)
                                ? util::bytes_from_span(env, envelope.source)
                                : nullptr,
                                (envelope.flags & SESSION_PROTOCOL_ENVELOPE_FLAGS_SERVER_TIMESTAMP)
                                ? static_cast<jlong>(envelope.server_timestamp)
                                : 0,
                                util::bytes_from_span(env, envelope.pro_sig))};
}

static jobject serializeDecodedEnvelope(JNIEnv *env, const session::DecodedEnvelope &envelop) {
    JavaLocalRef sender_ed25519(env, util::bytes_from_span(env, envelop.sender_ed25519_pubkey));
    JavaLocalRef sender_x25519(env, util::bytes_from_span(env, envelop.sender_x25519_pubkey));
    JavaLocalRef content(env, util::bytes_from_vector(env, envelop.content_plaintext));

    JavaLocalRef envelopClass(env, env->FindClass(
            "network/loki/messenger/libsession_util/protocol/DecodedEnvelope"));
    jmethodID init = env->GetMethodID(
            envelopClass.get(),
            "<init>",
            "(Lnetwork/loki/messenger/libsession_util/protocol/Envelope;Lnetwork/loki/messenger/libsession_util/pro/ProProof$Status;Lnetwork/loki/messenger/libsession_util/pro/ProProof;[B[B[BJ)V"
    );

    return env->NewObject(envelopClass.get(), init,
                          serializeEnvelop(env, envelop.envelope).get(),
                          envelop.pro ? static_cast<jint>(envelop.pro->status)
                                       : static_cast<jint>(-1),
                          envelop.pro ? serializeProProof(env, envelop.pro->proof).get() : nullptr,
                          content.get(),
                          sender_ed25519.get(),
                          sender_x25519.get(),
                          static_cast<jlong>(envelop.envelope.timestamp.count()));

}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encodeFor1o1(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jbyteArray plaintext,
                                                                                   jbyteArray my_ed25519_priv_key,
                                                                                   jlong timestamp_ms,
                                                                                   jbyteArray recipient_pub_key,
                                                                                   jbyteArray rotating_key) {
    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        return util::bytes_from_vector(
                env,
                session::encode_for_1o1(
                        JavaByteArrayRef(env, plaintext).get(),
                        JavaByteArrayRef(env, my_ed25519_priv_key).get(),
                        std::chrono::milliseconds{timestamp_ms},
                        *java_to_cpp_array<33>(env, recipient_pub_key),
                        rotating_key ? std::optional(JavaByteArrayRef(env, rotating_key).get())
                                     : std::nullopt
                )
        );
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encodeForCommunityInbox(
        JNIEnv *env, jobject thiz, jbyteArray plaintext, jbyteArray my_ed25519_priv_key,
        jlong timestamp_ms, jbyteArray recipient_pub_key, jbyteArray community_server_pub_key,
        jbyteArray rotating_key) {
    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        return util::bytes_from_vector(
                env,
                session::encode_for_community_inbox(
                        JavaByteArrayRef(env, plaintext).get(),
                        JavaByteArrayRef(env, my_ed25519_priv_key).get(),
                        std::chrono::milliseconds{timestamp_ms},
                        *java_to_cpp_array<33>(env, recipient_pub_key),
                        *java_to_cpp_array<32>(env, community_server_pub_key),
                        rotating_key ? std::optional(JavaByteArrayRef(env, rotating_key).get())
                                     : std::nullopt
                )
        );
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encodeForGroup(JNIEnv *env,
                                                                                     jobject thiz,
                                                                                     jbyteArray plaintext,
                                                                                     jbyteArray my_ed25519_priv_key,
                                                                                     jlong timestamp_ms,
                                                                                     jbyteArray group_ed25519_public_key,
                                                                                     jbyteArray group_ed25519_private_key,
                                                                                     jbyteArray rotating_key) {
    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        session::cleared_uc32 group_private_key;

        auto array = *java_to_cpp_array<32>(env, group_ed25519_private_key);
        std::copy(array.begin(), array.end(), group_private_key.begin());

        return util::bytes_from_vector(
                env,
                session::encode_for_group(
                        JavaByteArrayRef(env, plaintext).get(),
                        JavaByteArrayRef(env, my_ed25519_priv_key).get(),
                        std::chrono::milliseconds{timestamp_ms},
                        *java_to_cpp_array<33>(env, group_ed25519_public_key),
                        group_private_key,
                        rotating_key ? std::optional(JavaByteArrayRef(env, rotating_key).get())
                                     : std::nullopt
                )
        );
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_decodeForCommunity(
        JNIEnv *env, jobject thiz, jbyteArray payload, jlong now_epoch_ms,
        jbyteArray pro_backend_pub_key) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        jni_utils::JavaByteArrayRef payload_ref(env, payload);

        auto decoded = session::decode_for_community(
                payload_ref.get(),
                std::chrono::sys_time<std::chrono::milliseconds>{
                        std::chrono::milliseconds{now_epoch_ms}},
                *java_to_cpp_array<32>(env, pro_backend_pub_key)
        );

        JavaLocalRef envelopClass(env, env->FindClass(
                "network/loki/messenger/libsession_util/protocol/DecodedCommunityMessage"));
        jmethodID init = env->GetMethodID(
                envelopClass.get(),
                "<init>",
                "(Lnetwork/loki/messenger/libsession_util/protocol/pro/ProProof$Status;Lnetwork/loki/messenger/libsession_util/protocol/pro/ProProof;[B)V"
        );

        return env->NewObject(
                envelopClass.get(),
                init,
                decoded.pro ? static_cast<jint>(decoded.pro->status)
                             : static_cast<jint>(-1),
                decoded.pro ? serializeProProof(env, decoded.pro->proof).get() : nullptr,
                util::bytes_from_vector(env, decoded.content_plaintext)
        );
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encodeForCommunity(
        JNIEnv *env,
        jobject thiz,
        jbyteArray plaintext,
        jbyteArray rotating_key) {
    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        return util::bytes_from_vector(
                env,
                session::encode_for_community(
                        JavaByteArrayRef(env, plaintext).get(),
                        rotating_key ? std::optional(JavaByteArrayRef(env, rotating_key).get())
                                     : std::nullopt
                )
        );
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_decodeFor1o1(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jbyteArray key,
                                                                                   jbyteArray payload,
                                                                                   jlong now_epoch_ms,
                                                                                   jbyteArray pro_backend_pub_key) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        JavaByteArrayRef key_ref(env, key);

        std::array<std::span<const uint8_t>, 1> keys = {key_ref.get()};

        session::DecodeEnvelopeKey decode_key{
                .decrypt_keys = std::span(keys.data(), keys.size()),
        };

        return serializeDecodedEnvelope(env, session::decode_envelope(
                decode_key,
                JavaByteArrayRef(env, payload).get(),
                std::chrono::sys_time<std::chrono::milliseconds>{
                        std::chrono::milliseconds{now_epoch_ms}},
                *java_to_cpp_array<32>(env, pro_backend_pub_key)
        ));
    });
}


extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_decodeForGroup(JNIEnv *env,
                                                                                     jobject thiz,
                                                                                     jbyteArray payload,
                                                                                     jbyteArray my_ed25519_priv_key,
                                                                                     jlong now_epoch_ms,
                                                                                     jbyteArray group_ed25519_public_key,
                                                                                     jobjectArray group_ed25519_private_keys,
                                                                                     jbyteArray pro_backend_pub_key) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        std::vector<JavaByteArrayRef> private_keys_refs;
        std::vector<std::span<const uint8_t>> private_keys_spans;
        for (int i = 0, size = env->GetArrayLength(group_ed25519_private_keys); i < size; i++) {
            auto bytes = reinterpret_cast<jbyteArray>(env->GetObjectArrayElement(
                    group_ed25519_private_keys, i));
            private_keys_spans.emplace_back(private_keys_refs.emplace_back(env, bytes).get());
        }

        JavaByteArrayRef group_pub_key_ref(env, group_ed25519_public_key);

        session::DecodeEnvelopeKey decode_key{
                .group_ed25519_pubkey = std::make_optional(group_pub_key_ref.get()),
                .decrypt_keys = std::span(private_keys_spans.data(), private_keys_spans.size()),
        };

        return serializeDecodedEnvelope(env, session::decode_envelope(
                decode_key,
                JavaByteArrayRef(env, payload).get(),
                std::chrono::sys_time<std::chrono::milliseconds>{
                        std::chrono::milliseconds{now_epoch_ms}},
                *java_to_cpp_array<32>(env, pro_backend_pub_key)
        ));
    });
}