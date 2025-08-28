#include <jni.h>
#include <session/session_protocol.hpp>

#include "jni_utils.h"

using namespace jni_utils;

template <size_t N>
static std::optional<std::array<unsigned char, N>> java_to_cpp_array(JNIEnv *env, jbyteArray array) {
    if (!array) {
        return std::nullopt;
    }

    JavaByteArrayRef bytes(env, array);
    auto span = bytes.get();
    if (span.size() != N) {
        throw std::runtime_error("Invalid byte array length from java, expecting " + std::to_string(N) + " got " + std::to_string(span.size()));
    }

    std::array<unsigned char, N> out;
    std::copy(span.begin(), span.end(), out.begin());
    return out;
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_protocol_Destination_00024Contact_toNativeDestination(
        JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto &dest = *reinterpret_cast<session::Destination *>(native_ptr);

    JavaLocalRef clazz(env, env->GetObjectClass(thiz));

    JavaLocalRef pub_key(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getRecipientPubKey", "()[B"))));

    JavaLocalRef sig(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getProSignature", "()[B"))));
    jlong timestamp = env->CallLongMethod(thiz, env->GetMethodID(clazz.get(), "getSentTimestampMs",
                                                                 "()J"));

    run_catching_cxx_exception_or_throws<void>(env, [&] {
        dest.type = session::DestinationType::Contact;
        dest.pro_sig = java_to_cpp_array<64>(env, sig.get());
        dest.recipient_pubkey = java_to_cpp_array<33>(env, pub_key.get()).value();
        dest.sent_timestamp_ms = std::chrono::milliseconds{timestamp};
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_protocol_Destination_00024Sync_toNativeDestination(
        JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto &dest = *reinterpret_cast<session::Destination *>(native_ptr);

    JavaLocalRef clazz(env, env->GetObjectClass(thiz));

    JavaLocalRef pub_key(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getMyPubKey", "()[B"))));

    JavaLocalRef sig(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getProSignature", "()[B"))));
    jlong timestamp = env->CallLongMethod(thiz, env->GetMethodID(clazz.get(), "getSentTimestampMs",
                                                                 "()J"));

    run_catching_cxx_exception_or_throws<void>(env, [&] {
        dest.type = session::DestinationType::SyncMessage;
        dest.pro_sig = java_to_cpp_array<64>(env, sig.get());
        dest.recipient_pubkey = java_to_cpp_array<33>(env, pub_key.get()).value();
        dest.sent_timestamp_ms = std::chrono::milliseconds{timestamp};
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_protocol_Destination_00024Group_toNativeDestination(
        JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto &dest = *reinterpret_cast<session::Destination *>(native_ptr);

    JavaLocalRef clazz(env, env->GetObjectClass(thiz));

    JavaLocalRef pub_key(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getEd25519PubKey", "()[B"))));

    JavaLocalRef priv_key(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getEd25519PrivKey", "()[B"))));

    JavaLocalRef sig(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getProSignature", "()[B"))));

    jlong timestamp = env->CallLongMethod(thiz, env->GetMethodID(clazz.get(), "getSentTimestampMs",
                                                                 "()J"));

    run_catching_cxx_exception_or_throws<void>(env, [&] {
        dest.type = session::DestinationType::Group;
        dest.pro_sig = java_to_cpp_array<64>(env, sig.get());
        dest.group_ed25519_privkey = java_to_cpp_array<32>(env, priv_key.get()).value();
        dest.group_ed25519_pubkey = java_to_cpp_array<33>(env, pub_key.get()).value();
        dest.sent_timestamp_ms = std::chrono::milliseconds{timestamp};
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_protocol_Destination_00024Community_toNativeDestination(
        JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto &dest = *reinterpret_cast<session::Destination *>(native_ptr);

    JavaLocalRef clazz(env, env->GetObjectClass(thiz));
    JavaLocalRef sig(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getProSignature", "()[B"))));

    jlong timestamp = env->CallLongMethod(thiz, env->GetMethodID(clazz.get(), "getSentTimestampMs",
                                                                 "()J"));

    run_catching_cxx_exception_or_throws<void>(env, [&] {
        dest.type = session::DestinationType::Community;
        dest.pro_sig = java_to_cpp_array<64>(env, sig.get());
        dest.sent_timestamp_ms = std::chrono::milliseconds{timestamp};
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_protocol_Destination_00024CommunityInbox_toNativeDestination(
        JNIEnv *env, jobject thiz, jlong native_ptr) {
    auto &dest = *reinterpret_cast<session::Destination *>(native_ptr);

    JavaLocalRef clazz(env, env->GetObjectClass(thiz));

    JavaLocalRef recipient_pub_key(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getRecipientPubKey", "()[B"))));


    JavaLocalRef pub_key(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getCommunityPubKey", "()[B"))));

    JavaLocalRef sig(
            env,
            reinterpret_cast<jbyteArray>(env->CallObjectMethod(
                    thiz,
                    env->GetMethodID(clazz.get(), "getProSignature", "()[B"))));

    jlong timestamp = env->CallLongMethod(thiz, env->GetMethodID(clazz.get(), "getSentTimestampMs",
                                                                 "()J"));

    run_catching_cxx_exception_or_throws<void>(env, [&] {
        dest.type = session::DestinationType::CommunityInbox;
        dest.pro_sig = java_to_cpp_array<64>(env, sig.get());
        dest.community_inbox_server_pubkey = java_to_cpp_array<32>(env, pub_key.get()).value();
        dest.recipient_pubkey = java_to_cpp_array<33>(env, recipient_pub_key.get()).value();
        dest.sent_timestamp_ms = std::chrono::milliseconds{timestamp};
    });
}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_encryptForDestination(
        JNIEnv *env,
        jobject thiz,
        jbyteArray java_message,
        jbyteArray java_my_ed25519_privkey,
        jobject java_destination,
        jint java_namespace) {
    session::Destination dest;
    auto to_native_method = env->GetMethodID(env->GetObjectClass(java_destination), "toNativeDestination", "(J)V");
    env->CallVoidMethod(java_destination, to_native_method, reinterpret_cast<jlong>(&dest));

    // Make sure nothing went wrong in toNativeDestination
    if (env->ExceptionCheck()) {
        return nullptr;
    }

    return run_catching_cxx_exception_or_throws<jbyteArray>(env, [&] {
        auto result = session::encrypt_for_destination(
                JavaByteArrayRef(env, java_message).get(),
                JavaByteArrayRef(env, java_my_ed25519_privkey).get(),
                dest,
                static_cast<session::config::Namespace>(java_namespace));
        if (result.encrypted) {
            return util::bytes_from_vector(env, result.ciphertext);
        } else {
            return (jbyteArray) nullptr;
        }
    });
}

static JavaLocalRef<jobject> serializeProStatus(JNIEnv *env, const session::DecryptedEnvelope & envelope) {
    if (!envelope.pro.has_value()) {
        JavaLocalRef noneClass(env, env->FindClass("network/loki/messenger/libsession_util/protocol/ProStatus$None"));
        auto fieldId = env->GetStaticFieldID(
                noneClass.get(),
                "INSTANCE", "Lnetwork/loki/messenger/libsession_util/protocol/ProStatus$None;");
        return JavaLocalRef(env, env->GetStaticObjectField(noneClass.get(), fieldId));
    }

    if (envelope.pro->status == session::config::ProStatus::Valid) {
        JavaLocalRef validClass(env, env->FindClass("network/loki/messenger/libsession_util/protocol/ProStatus$Valid"));
        auto init = env->GetMethodID(validClass.get(), "<init>", "(JJ)V");
        return JavaLocalRef(env, env->NewObject(validClass.get(), init,
                       static_cast<jlong>(envelope.pro->proof.expiry_unix_ts.time_since_epoch().count()),
                       static_cast<jlong>(envelope.pro->features)));
    }

    JavaLocalRef invalidClass(env, env->FindClass("network/loki/messenger/libsession_util/protocol/ProStatus$Invalid"));
    auto fieldId = env->GetStaticFieldID(
            invalidClass.get(),
            "INSTANCE", "Lnetwork/loki/messenger/libsession_util/protocol/ProStatus$Invalid;");
    return JavaLocalRef(env, env->GetStaticObjectField(invalidClass.get(), fieldId));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_protocol_SessionProtocol_decryptEnvelope(JNIEnv *env,
                                                                                      jobject thiz,
                                                                                      jobject java_key,
                                                                                      jbyteArray java_payload,
                                                                                      jlong now_epoch_seconds,
                                                                                      jbyteArray java_pro_backend_pub_key) {

    session::DecryptEnvelopeKey key;

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
        auto envelop = session::decrypt_envelope(key, JavaByteArrayRef(env, java_payload).get(),
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

