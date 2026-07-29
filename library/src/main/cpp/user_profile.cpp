#include <session/config/user_profile.hpp>

#include "util.h"
#include "pro_proof_util.h"
#include "config_base.h"

inline auto ptrToProfile(JNIEnv* env, jobject obj) {
    return dynamic_cast<session::config::UserProfile *>(ptrToConfigBase(env, obj));
}

extern "C" {
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setName(
        JNIEnv *env,
        jobject thiz,
        jstring newName) {
    auto profile = ptrToProfile(env, thiz);
    profile->set_name(jni_utils::JavaStringRef(env, newName).view());
}

JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getName(JNIEnv *env, jobject thiz) {
    auto profile = ptrToProfile(env, thiz);
    auto name = profile->get_name();
    if (name == std::nullopt) return nullptr;
    return env->NewStringUTF(name->data());
}

JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getPic(JNIEnv *env, jobject thiz) {
    auto profile = ptrToProfile(env, thiz);
    auto pic = profile->get_profile_pic();
    return util::serialize_user_pic(env, pic).release();
}

JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setPic(JNIEnv *env, jobject thiz,
                                                                jobject user_pic) {
    ptrToProfile(env, thiz)->set_profile_pic(util::deserialize_user_pic(env, user_pic));
}

}
extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setNtsPriority(JNIEnv *env, jobject thiz,
                                                                        jlong priority) {
    auto profile = ptrToProfile(env, thiz);
    profile->set_nts_priority(priority);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getNtsPriority(JNIEnv *env, jobject thiz) {
    auto profile = ptrToProfile(env, thiz);
    return profile->get_nts_priority();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setNtsExpiry(JNIEnv *env, jobject thiz,
                                                                      jobject expiry_mode) {
    auto profile = ptrToProfile(env, thiz);
    auto expiry = util::deserialize_expiry(env, expiry_mode);
    profile->set_nts_expiry(std::chrono::seconds(expiry.second));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getNtsExpiry(JNIEnv *env, jobject thiz) {
    auto profile = ptrToProfile(env, thiz);
    auto nts_expiry = profile->get_nts_expiry();
    if (nts_expiry == std::nullopt) {
        auto expiry = util::serialize_expiry(env, session::config::expiration_mode::none,
                                             std::chrono::seconds(0));
        return expiry.release();
    }
    auto expiry = util::serialize_expiry(env, session::config::expiration_mode::after_send,
                                         std::chrono::seconds(*nts_expiry));
    return expiry.release();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getCommunityMessageRequests(
        JNIEnv *env, jobject thiz) {
    auto profile = ptrToProfile(env, thiz);
    auto blinded_msg_requests = profile->get_blinded_msgreqs();
    if (blinded_msg_requests.has_value()) {
        return *blinded_msg_requests;
    }
    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setCommunityMessageRequests(
        JNIEnv *env, jobject thiz, jboolean blocks) {
    auto profile = ptrToProfile(env, thiz);
    profile->set_blinded_msgreqs(std::optional{(bool) blocks});
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_isBlockCommunityMessageRequestsSet(
        JNIEnv *env, jobject thiz) {
    auto profile = ptrToProfile(env, thiz);
    return profile->get_blinded_msgreqs().has_value();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getProfileUpdatedSeconds(JNIEnv *env,
                                                                                  jobject thiz) {
    return ptrToProfile(env, thiz)->get_profile_updated().time_since_epoch().count();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setReuploadedPic(JNIEnv *env, jobject thiz,
                                                                          jobject user_pic) {
    ptrToProfile(env, thiz)->set_reupload_profile_pic(util::deserialize_user_pic(env, user_pic));
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_removeProConfig(JNIEnv *env,
                                                                         jobject thiz) {
    ptrToProfile(env, thiz)->remove_pro_config();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setProConfig(JNIEnv *env, jobject thiz,
                                                                      jobject proof,
                                                                      jbyteArray rotating_private_key) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=]() {

        jni_utils::JavaByteArrayRef key_ref(env, rotating_private_key);
        auto r = key_ref.get();
        session::cleared_uc64 rotating_privkey;
        std::copy(r.begin(), r.end(), rotating_privkey.begin());

        ptrToProfile(env, thiz)->set_pro_config(
                {
                        .rotating_privkey = rotating_privkey,
                        .proof = java_to_cpp_proof(env, proof),
                }
        );
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setProBadge(JNIEnv *env, jobject thiz,
                                                                     jboolean pro_badge) {
    ptrToProfile(env, thiz)->set_pro_badge(pro_badge);
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setAnimatedAvatar(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jboolean enabled) {
    ptrToProfile(env, thiz)->set_animated_avatar(enabled);
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_setProAccessExpiry(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jlong epoch_seconds) {
    ptrToProfile(env, thiz)->set_pro_access_expiry(std::chrono::sys_seconds{
            std::chrono::seconds{epoch_seconds}
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_removeProAccessExpiry(JNIEnv *env,
                                                                               jobject thiz) {
    ptrToProfile(env, thiz)->set_pro_access_expiry(std::nullopt);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getProFeaturesRaw(JNIEnv *env,
                                                                           jobject thiz) {
    return static_cast<jlong>(ptrToProfile(env, thiz)->get_profile_bitset().data);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getProConfig(JNIEnv *env, jobject thiz) {
    auto profile = ptrToProfile(env, thiz)->get_pro_config();
    if (!profile) {
        return nullptr;
    }

    static jni_utils::BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/pro/ProConfig",
            "(Lnetwork/loki/messenger/libsession_util/pro/ProProof;[B)V"
    );

    return env->NewObject(class_info.java_class,
                          class_info.constructor,
                          cpp_to_java_proof(env, profile->proof).get(),
                          util::bytes_from_span(env, profile->rotating_privkey).get()
    );
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getProAccessExpiryOrZero(JNIEnv *env,
                                                                                   jobject thiz) {
    auto expiry = ptrToProfile(env, thiz)->get_pro_access_expiry();
    return expiry ? expiry->time_since_epoch().count() : 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getRefundRequestedOrZero(JNIEnv *env,
                                                                                  jobject thiz) {
    auto when = ptrToProfile(env, thiz)->get_refund_requested();
    return when ? when->time_since_epoch().count() : 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_nativeSetRefundRequested(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jlong epoch_seconds) {
    if (epoch_seconds <= 0)
        ptrToProfile(env, thiz)->set_refund_requested(std::nullopt);
    else
        ptrToProfile(env, thiz)->set_refund_requested(
                std::chrono::sys_seconds{std::chrono::seconds{epoch_seconds}});
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getProPrepaidOrZero(JNIEnv *env,
                                                                             jobject thiz) {
    auto when = ptrToProfile(env, thiz)->get_pro_prepaid();
    return when ? when->time_since_epoch().count() : 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_nativeSetProPrepaid(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jlong epoch_seconds) {
    if (epoch_seconds <= 0)
        ptrToProfile(env, thiz)->set_pro_prepaid(std::nullopt);
    else
        ptrToProfile(env, thiz)->set_pro_prepaid(
                std::chrono::sys_seconds{std::chrono::seconds{epoch_seconds}});
}

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_UserProfile_getProRenewalTargetOrZero(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jlong now_seconds) {
    auto target = ptrToProfile(env, thiz)->pro_renewal_target(
            std::chrono::sys_seconds{std::chrono::seconds{now_seconds}});
    return target ? target->time_since_epoch().count() : 0;
}