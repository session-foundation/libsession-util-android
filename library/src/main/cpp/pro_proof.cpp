#include <session/session_protocol.hpp>
#include <oxenc/base64.h>
#include <nlohmann/json.hpp>
#include <jni.h>

#include "util.h"
#include "jni_utils.h"


extern "C"
JNIEXPORT jint JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeGetVersion(
        JNIEnv *env, jobject thiz, jlong native_value) {
    return static_cast<jint>(
            reinterpret_cast<session::ProProof*>(native_value)->version);
}


extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeGetExpiry(
        JNIEnv *env, jobject thiz, jlong native_value) {
    return static_cast<jlong>(
            reinterpret_cast<session::ProProof*>(native_value)->expiry_unix_ts.time_since_epoch().count());
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeGetRotatingPubKey(
        JNIEnv *env, jobject thiz, jlong native_value) {
    const auto& rotating_pubkey =
            reinterpret_cast<session::ProProof*>(native_value)->rotating_pubkey;

    return util::bytes_from_span(env, rotating_pubkey);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeSerialize(
        JNIEnv *env, jobject thiz, jlong native_value) {
    const auto& proof =
            *reinterpret_cast<session::ProProof*>(native_value);
    nlohmann::json j;
    j["version"] = proof.version;
    j["gen_index_hash"] = oxenc::to_base64(proof.gen_index_hash);
    j["rotating_pubkey"] = oxenc::to_base64(proof.rotating_pubkey);
    j["expiry_unix_ts_ms"] = proof.expiry_unix_ts.time_since_epoch().count();
    j["sig"] = oxenc::to_base64(proof.sig);

    return util::jstringFromOptional(env, j.dump());
}

template<size_t N>
void from_json(const nlohmann::json& j, std::array<uint8_t, N>& arr) {
    auto b64_str = j.get<std::string_view>();
    auto bytes = oxenc::from_base64(b64_str);
    if (bytes.size() != N) {
        throw std::invalid_argument{"Invalid array size in from_json"};
    }
    std::copy(bytes.begin(), bytes.end(), arr.begin());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeDeserialize(
        JNIEnv *env, jobject thiz, jstring data) {
    return jni_utils::run_catching_cxx_exception_or_throws<jlong>(env, [=]() {
        auto j = nlohmann::json::parse(jni_utils::JavaStringRef(env, data).view());

        return reinterpret_cast<jlong>(new session::ProProof {
            .version = j.at("version").get<uint8_t>(),
            .gen_index_hash = j.at("gen_index_hash").get<session::array_uc32>(),
            .rotating_pubkey = j.at("rotating_pubkey").get<session::array_uc32>(),
            .expiry_unix_ts = std::chrono::sys_time<std::chrono::milliseconds>{
                std::chrono::milliseconds{static_cast<int64_t>(j.at("expiry_unix_ts_ms").get<uint64_t>())}},
            .sig = j.at("sig").get<session::array_uc64>(),
        });
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_pro_ProProof_00024Companion_nativeDestroy(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jlong native_value) {
    delete reinterpret_cast<session::ProProof*>(native_value);
}