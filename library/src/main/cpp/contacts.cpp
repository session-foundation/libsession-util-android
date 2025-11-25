#include <jni.h>
#include <session/config/contacts.hpp>

#include "util.h"
#include "jni_utils.h"

using namespace jni_utils;

static session::config::Contacts *ptrToContacts(JNIEnv *env, jobject obj) {
    auto contactsClass = JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/Contacts"));
    jfieldID pointerField = env->GetFieldID(contactsClass.get(), "pointer", "J");
    return (session::config::Contacts *) env->GetLongField(obj, pointerField);
}

static JavaLocalRef<jobject> serialize_contact(JNIEnv *env, const session::config::contact_info &info) {
    static BasicJavaClassInfo class_info(
            env, "network/loki/messenger/libsession_util/util/Contact",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZZLnetwork/loki/messenger/libsession_util/util/UserPic;JJJLnetwork/loki/messenger/libsession_util/util/ExpiryMode;J)V");

    jobject returnObj = env->NewObject(class_info.java_class,
                                       class_info.constructor,
                                       JavaLocalRef(env, env->NewStringUTF(info.session_id.data())).get(),
                                       JavaLocalRef(env, env->NewStringUTF(info.name.data())).get(),
                                       JavaLocalRef(env, env->NewStringUTF(info.nickname.data())).get(),
                                       (jboolean) info.approved,
                                       (jboolean) info.approved_me,
                                       (jboolean) info.blocked,
                                       util::serialize_user_pic(env, info.profile_picture).get(),
                                       (jlong) info.created,
                                       (jlong) (info.profile_updated.time_since_epoch().count()),
                                       (jlong) info.priority,
                                       util::serialize_expiry(env, info.exp_mode, info.exp_timer).get(),
                                       (jlong) info.pro_features);
    return {env, returnObj};
}

session::config::contact_info deserialize_contact(JNIEnv *env, jobject info, session::config::Contacts *conf) {
    struct ClassInfo : JavaClassInfo {
        jmethodID get_id;
        jmethodID get_name;
        jmethodID get_nick;
        jmethodID get_approved;
        jmethodID get_approved_me;
        jmethodID get_blocked;
        jmethodID get_user_pic;
        jmethodID get_priority;
        jmethodID get_expiry;
        jmethodID get_profile_updated;
        jmethodID get_pro_features;

        ClassInfo(JNIEnv *env, jobject obj):
            JavaClassInfo(env, obj),
            get_id(env->GetMethodID(java_class, "getId", "()Ljava/lang/String;")),
            get_name(env->GetMethodID(java_class, "getName", "()Ljava/lang/String;")),
            get_nick(env->GetMethodID(java_class, "getNickname", "()Ljava/lang/String;")),
            get_approved(env->GetMethodID(java_class, "getApproved", "()Z")),
            get_approved_me(env->GetMethodID(java_class, "getApprovedMe", "()Z")),
            get_blocked(env->GetMethodID(java_class, "getBlocked", "()Z")),
            get_user_pic(env->GetMethodID(java_class, "getProfilePicture", "()Lnetwork/loki/messenger/libsession_util/util/UserPic;")),
            get_priority(env->GetMethodID(java_class, "getPriority", "()J")),
            get_expiry(env->GetMethodID(java_class, "getExpiryMode", "()Lnetwork/loki/messenger/libsession_util/util/ExpiryMode;")),
            get_profile_updated(env->GetMethodID(java_class, "getProfileUpdatedEpochSeconds", "()J")),
            get_pro_features(env->GetMethodID(java_class, "getProFeaturesRaw", "()J")) {}
    };

    static ClassInfo class_info(env, info);

    JavaLocalRef account_id(env, static_cast<jstring>(env->CallObjectMethod(info, class_info.get_id)));
    JavaLocalRef name(env, static_cast<jstring>(env->CallObjectMethod(info, class_info.get_name)));
    JavaLocalRef nickname(env, static_cast<jstring>(env->CallObjectMethod(info, class_info.get_nick)));
    JavaLocalRef user_pic(env, env->CallObjectMethod(info, class_info.get_user_pic));
    JavaLocalRef expiry_mode(env, env->CallObjectMethod(info, class_info.get_expiry));

    auto expiry_pair = util::deserialize_expiry(env, expiry_mode.get());
    auto profile_updated_seconds = env->CallLongMethod(info, class_info.get_profile_updated);

    auto contact_info = conf->get_or_construct(JavaStringRef(env, account_id.get()).view());
    if (name.get()) {
        contact_info.name = JavaStringRef(env, name.get()).view();
    }
    if (nickname.get()) {
        contact_info.nickname = JavaStringRef(env, nickname.get()).view();
    }
    contact_info.approved = env->CallBooleanMethod(info, class_info.get_approved);
    contact_info.approved_me = env->CallBooleanMethod(info, class_info.get_approved_me);
    contact_info.blocked = env->CallBooleanMethod(info, class_info.get_blocked);
    contact_info.profile_updated = std::chrono::sys_seconds{std::chrono::seconds{profile_updated_seconds}};
    if (user_pic.get() != nullptr) {
        contact_info.profile_picture = util::deserialize_user_pic(env, user_pic.get());
    }

    contact_info.priority = env->CallLongMethod(info, class_info.get_priority);
    contact_info.exp_mode = expiry_pair.first;
    contact_info.exp_timer = std::chrono::seconds(expiry_pair.second);
    contact_info.pro_features = env->CallLongMethod(info, class_info.get_pro_features);

    return contact_info;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_get(JNIEnv *env, jobject thiz,
                                                          jstring account_id) {
    // If an exception is thrown, return nullptr
    return run_catching_cxx_exception_or_throws<jobject>(
            env,
            [=]() -> jobject {
                auto contacts = ptrToContacts(env, thiz);
                auto contact = contacts->get(JavaStringRef(env, account_id).view());
                if (!contact) return nullptr;
                return serialize_contact(env, contact.value()).release();
            }
    );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_getOrConstruct(JNIEnv *env, jobject thiz,
                                                                     jstring account_id) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto contacts = ptrToContacts(env, thiz);
        auto contact = contacts->get_or_construct(JavaStringRef(env, account_id).view());
        return serialize_contact(env, contact).release();
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_set(JNIEnv *env, jobject thiz,
                                                          jobject contact) {
    run_catching_cxx_exception_or_throws<void>(env, [=] {
        auto contacts = ptrToContacts(env, thiz);
        auto contact_info = deserialize_contact(env, contact, contacts);
        contacts->set(contact_info);
    });
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_erase(JNIEnv *env, jobject thiz,
                                                            jstring account_id) {
    return run_catching_cxx_exception_or_throws<jboolean>(env, [=] {
        auto contacts = ptrToContacts(env, thiz);
        bool result = contacts->erase(JavaStringRef(env, account_id).view());
        return result;
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_all(JNIEnv *env, jobject thiz) {
    return run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        return jlist_from_collection(env, *ptrToContacts(env, thiz), serialize_contact);
    });
}



JavaLocalRef<jobject> serialize_blinded_contact(JNIEnv *env, const session::config::blinded_contact_info &info) {
    static BasicJavaClassInfo class_info(
            env, "network/loki/messenger/libsession_util/util/BlindedContact",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;JJLnetwork/loki/messenger/libsession_util/util/UserPic;JJ)V");

    return {env, env->NewObject(
            class_info.java_class,
            class_info.constructor,
            JavaLocalRef(env, env->NewStringUTF(info.session_id().c_str())).get(),
            JavaLocalRef(env, env->NewStringUTF(info.community_base_url().data())).get(),
            JavaLocalRef(env, env->NewStringUTF(info.community_pubkey_hex().data())).get(),
            JavaLocalRef(env, env->NewStringUTF(info.name.c_str())).get(),
            (jlong) (info.created.time_since_epoch().count()),
            (jlong) (info.profile_updated.time_since_epoch().count()),
            util::serialize_user_pic(env, info.profile_picture).get(),
            (jlong) info.priority,
            (jlong) info.pro_features
    )};
}

session::config::blinded_contact_info deserialize_blinded_contact(JNIEnv *env, jobject jInfo) {
    struct ClassInfo : public JavaClassInfo {
        jmethodID id_getter;
        jmethodID community_server_getter;
        jmethodID community_server_pub_key_getter;
        jmethodID name_getter;
        jmethodID created_epoch_seconds_getter;
        jmethodID profile_updated_epoch_seconds_getter;
        jmethodID profile_pic_getter;
        jmethodID priority_getter;
        jmethodID pro_features_getter;

        ClassInfo(JNIEnv *env, jobject obj)
            : JavaClassInfo(env, obj)
            , id_getter(env->GetMethodID(java_class, "getId", "()Ljava/lang/String;"))
            , community_server_getter(env->GetMethodID(java_class, "getCommunityServer", "()Ljava/lang/String;"))
            , community_server_pub_key_getter(env->GetMethodID(java_class, "getCommunityServerPubKey", "()[B"))
            , name_getter(env->GetMethodID(java_class, "getName", "()Ljava/lang/String;"))
            , created_epoch_seconds_getter(env->GetMethodID(java_class, "getCreatedEpochSeconds", "()J"))
            , profile_updated_epoch_seconds_getter(env->GetMethodID(java_class, "getProfileUpdatedEpochSeconds", "()J"))
            , profile_pic_getter(env->GetMethodID(java_class, "getProfilePicture", "()Lnetwork/loki/messenger/libsession_util/util/UserPic;"))
            , priority_getter(env->GetMethodID(java_class, "getPriority", "()J"))
            , pro_features_getter(env->GetMethodID(java_class, "getProFeaturesRaw", "()J")) {}
    };

    static ClassInfo class_info(env, jInfo);

    JavaLocalRef
        community_server(env, (jstring) env->CallObjectMethod(jInfo, class_info.community_server_getter)),
        id(env, (jstring) env->CallObjectMethod(jInfo, class_info.id_getter));

    JavaLocalRef community_server_pub_key(env, (jbyteArray) env->CallObjectMethod(jInfo, class_info.community_server_pub_key_getter));

    session::config::blinded_contact_info info(
            JavaStringRef(env, community_server.get()).view(),
            JavaByteArrayRef(env, community_server_pub_key.get()).get(),
            JavaStringRef(env, id.get()).view()
    );
    info.created = std::chrono::sys_seconds{std::chrono::seconds{env->CallLongMethod(jInfo, class_info.created_epoch_seconds_getter)}};
    info.profile_picture = util::deserialize_user_pic(env, JavaLocalRef(env, env->CallObjectMethod(jInfo, class_info.profile_pic_getter)).get());
    info.name = JavaStringRef(env, JavaLocalRef(env, (jstring) env->CallObjectMethod(jInfo, class_info.name_getter)).get()).view();
    info.profile_updated = std::chrono::sys_seconds{std::chrono::seconds{env->CallLongMethod(jInfo, class_info.profile_updated_epoch_seconds_getter)}};
    info.priority = env->CallLongMethod(jInfo, class_info.priority_getter);
    info.pro_features = env->CallLongMethod(jInfo, class_info.pro_features_getter);

    return info;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_getOrConstructBlinded(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jstring community_server_url,
                                                                            jstring community_server_pub_key_hex,
                                                                            jstring blinded_id) {
    return serialize_blinded_contact(env, ptrToContacts(env, thiz)->get_or_construct_blinded(
            JavaStringRef(env, community_server_url).view(),
            JavaStringRef(env, community_server_pub_key_hex).view(),
            JavaStringRef(env, blinded_id).view()
    )).release();
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_setBlinded(JNIEnv *env, jobject thiz,
                                                                 jobject contact) {
    ptrToContacts(env, thiz)->set_blinded(
        deserialize_blinded_contact(env, contact)
    );
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_eraseBlinded(JNIEnv *env, jobject thiz,
                                                                   jstring community_server_url,
                                                                   jstring blinded_id) {
    ptrToContacts(env, thiz)->erase_blinded(
            JavaStringRef(env, community_server_url).view(),
            JavaStringRef(env, blinded_id).view()
    );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_allBlinded(JNIEnv *env, jobject thiz) {
    return jlist_from_collection(
            env,
            ptrToContacts(env, thiz)->blinded(),
            serialize_blinded_contact
    );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_getBlinded(JNIEnv *env,
                                                                 jobject thiz,
                                                                 jstring blinded_id) {
    auto result = ptrToContacts(env, thiz)->get_blinded(JavaStringRef(env, blinded_id).view());

    if (result) {
        return serialize_blinded_contact(env, *result).release();
    } else {
        return nullptr;
    }
}