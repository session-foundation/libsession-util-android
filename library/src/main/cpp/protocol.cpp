#include <jni.h>
#include <session/session_protocol.hpp>
#include <session/sodium_array.hpp>

#include "jni_utils.h"

using namespace jni_utils;



static JavaLocalRef<jobject> serializeProStatus(JNIEnv *env, const session::DecodedEnvelope & envelope) {
    if (!envelope.pro.has_value()) {
        JavaLocalRef noneClass(env, env->FindClass("network/loki/messenger/libsession_util/protocol/ProStatus$None"));
        auto fieldId = env->GetStaticFieldID(
                noneClass.get(),
                "INSTANCE", "Lnetwork/loki/messenger/libsession_util/protocol/ProStatus$None;");
        return {env, env->GetStaticObjectField(noneClass.get(), fieldId)};
    }

    if (envelope.pro->status == session::ProStatus::Valid) {
        JavaLocalRef validClass(env, env->FindClass("network/loki/messenger/libsession_util/protocol/ProStatus$Valid"));
        auto init = env->GetMethodID(validClass.get(), "<init>", "(JJ)V");
        return {env, env->NewObject(validClass.get(), init,
                       static_cast<jlong>(envelope.pro->proof.expiry_unix_ts.time_since_epoch().count()),
                       static_cast<jlong>(envelope.pro->features))};
    }

    JavaLocalRef invalidClass(env, env->FindClass("network/loki/messenger/libsession_util/protocol/ProStatus$Invalid"));
    auto fieldId = env->GetStaticFieldID(
            invalidClass.get(),
            "INSTANCE", "Lnetwork/loki/messenger/libsession_util/protocol/ProStatus$Invalid;");
    return {env, env->GetStaticObjectField(invalidClass.get(), fieldId)};
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_decryptEnvelope(JNIEnv *env,
                                                                                      jobject thiz,
                                                                                      jobject java_key,
                                                                                      jbyteArray java_payload,
                                                                                      jlong now_epoch_seconds,
                                                                                      jbyteArray java_pro_backend_pub_key) {

    session::DecodeEnvelopeKey key;

    std::vector<std::span<const unsigned char>> privateKeysStorage;

    struct RegularStorage {
        JavaLocalRef<jbyteArray> ed25519PrivKeyRef;
        JavaByteArrayRef ed25519PrivKeyBytesRef;
    };

    struct GroupData {
        JavaLocalRef<jbyteArray> groupPubKeyRef;
        JavaByteArrayRef groupPubKeyBytesRef;

        std::vector<std::pair<JavaLocalRef<jbyteArray>, JavaByteArrayRef>> groupKeysRef;
    };

    std::optional<RegularStorage> regularStorage;
    std::optional<GroupData> groupStorage;

    JavaLocalRef regularClazz(env, env->FindClass("network/loki/messenger/libsession_util/protocol/DecryptEnvelopeKey$Regular"));
    if (env->IsInstanceOf(java_key, regularClazz.get())) {
        auto bytes = reinterpret_cast<jbyteArray>(env->CallObjectMethod(java_key, env->GetMethodID(regularClazz.get(), "getEd25519PrivKey", "()[B")));
        regularStorage.emplace(RegularStorage {
                .ed25519PrivKeyRef = JavaLocalRef(env, bytes),
                .ed25519PrivKeyBytesRef = JavaByteArrayRef(env, bytes)
        });

        privateKeysStorage.push_back(regularStorage->ed25519PrivKeyBytesRef.get());
    }

    JavaLocalRef groupClazz(env, env->FindClass("network/loki/messenger/libsession_util/protocol/DecryptEnvelopeKey$Group"));
    if (env->IsInstanceOf(java_key, groupClazz.get())) {
        auto pubKeyBytes = reinterpret_cast<jbyteArray>(env->CallObjectMethod(java_key, env->GetMethodID(groupClazz.get(), "getGroupEd25519PubKey", "()[B")));
        groupStorage.emplace(GroupData {
                .groupPubKeyRef = JavaLocalRef(env, pubKeyBytes),
                .groupPubKeyBytesRef = JavaByteArrayRef(env, pubKeyBytes)
        });

        key.group_ed25519_pubkey.emplace(groupStorage->groupPubKeyBytesRef.get());

        JavaLocalRef privKeyArrays(env, reinterpret_cast<jobjectArray>(env->CallObjectMethod(java_key, env->GetMethodID(groupClazz.get(), "getGroupKeys", "()[[B"))));
        for (int i = 0, size = env->GetArrayLength(privKeyArrays.get()); i < size; i++) {
            auto bytes = reinterpret_cast<jbyteArray>(env->GetObjectArrayElement(privKeyArrays.get(), i));
            const auto &last = groupStorage->groupKeysRef.emplace_back(JavaLocalRef(env, bytes), JavaByteArrayRef(env, bytes));
            privateKeysStorage.emplace_back(last.second.get());
        }
    }

    key.ed25519_privkeys = { privateKeysStorage.data(), privateKeysStorage.size() };

    return run_catching_cxx_exception_or_throws<jobject>(env, [&] {
        auto envelop = session::decode_envelope(key, JavaByteArrayRef(env, java_payload).get(),
                                                 std::chrono::sys_seconds { std::chrono::seconds { now_epoch_seconds } },
                                                 *java_to_cpp_array<32>(env, java_pro_backend_pub_key));

        JavaLocalRef sender_ed25519(env, util::bytes_from_span(env, envelop.sender_ed25519_pubkey));
        JavaLocalRef sender_x25519(env, util::bytes_from_span(env, envelop.sender_x25519_pubkey));
        JavaLocalRef content(env, util::bytes_from_vector(env, envelop.content_plaintext));

        JavaLocalRef envelopClass(env, env->FindClass("network/loki/messenger/libsession_util/protocol/DecryptedEnvelope"));
        jmethodID init = env->GetMethodID(
                envelopClass.get(),
                "<init>",
                "(Lnetwork/loki/messenger/libsession_util/protocol/ProStatus;[B[B[BJ)V"
        );

        return env->NewObject(envelopClass.get(), init,
                              serializeProStatus(env, envelop).get(),
                              content.get(),
                              sender_ed25519.get(),
                              sender_x25519.get(),
                              static_cast<jlong>(envelop.envelope.timestamp.count()));
    });
}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encryptFor1o1(JNIEnv *env,
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
                        std::chrono::milliseconds { timestamp_ms },
                        *java_to_cpp_array<33>(env, recipient_pub_key),
                        rotating_key ? JavaByteArrayRef(env, rotating_key).get() : std::span<uint8_t>()
        ));
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encryptForCommunityInbox(
        JNIEnv *env, jobject thiz, jbyteArray plaintext, jbyteArray my_ed25519_priv_key,
        jlong timestamp_ms, jbyteArray recipient_pub_key, jbyteArray community_server_pub_key,
        jbyteArray rotating_key) {
    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [=] {
        return util::bytes_from_vector(
                env,
                session::encode_for_community_inbox(
                        JavaByteArrayRef(env, plaintext).get(),
                        JavaByteArrayRef(env, my_ed25519_priv_key).get(),
                        std::chrono::milliseconds { timestamp_ms },
                        *java_to_cpp_array<33>(env, recipient_pub_key),
                        *java_to_cpp_array<32>(env, community_server_pub_key),
                        rotating_key ? JavaByteArrayRef(env, rotating_key).get() : std::span<uint8_t>()
                ));
    });
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encryptForGroup(JNIEnv *env,
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
                        std::chrono::milliseconds { timestamp_ms },
                        *java_to_cpp_array<33>(env, group_ed25519_public_key),
                        group_private_key,
                        rotating_key ? JavaByteArrayRef(env, rotating_key).get() : std::span<uint8_t>()
                ));
    });
}