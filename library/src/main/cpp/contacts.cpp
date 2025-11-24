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

static JavaLocalRef<jobject> serialize_contact(JNIEnv *env, session::config::contact_info info) {
    static BasicJavaClassInfo class_info(
            env, "network/loki/messenger/libsession_util/util/Contact",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZZLnetwork/loki/messenger/libsession_util/util/UserPic;JJJLnetwork/loki/messenger/libsession_util/util/ExpiryMode;)V");

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
                                       util::serialize_expiry(env, info.exp_mode, info.exp_timer).get());
    return {env, returnObj};
}

session::config::contact_info deserialize_contact(JNIEnv *env, jobject info, session::config::Contacts *conf) {
    struct ClassInfo {
        jclass java_class;
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

        ClassInfo(JNIEnv *env, jclass clazz):
            java_class((jclass) env->NewGlobalRef(clazz)),
            get_id(env->GetMethodID(clazz, "getId", "()Ljava/lang/String;")),
            get_name(env->GetMethodID(clazz, "getName", "()Ljava/lang/String;")),
            get_nick(env->GetMethodID(clazz, "getNickname", "()Ljava/lang/String;")),
            get_approved(env->GetMethodID(clazz, "getApproved", "()Z")),
            get_approved_me(env->GetMethodID(clazz, "getApprovedMe", "()Z")),
            get_blocked(env->GetMethodID(clazz, "getBlocked", "()Z")),
            get_user_pic(env->GetMethodID(clazz, "getProfilePicture", "()Lnetwork/loki/messenger/libsession_util/util/UserPic;")),
            get_priority(env->GetMethodID(clazz, "getPriority", "()J")),
            get_expiry(env->GetMethodID(clazz, "getExpiryMode", "()Lnetwork/loki/messenger/libsession_util/util/ExpiryMode;")),
            get_profile_updated(env->GetMethodID(clazz, "getProfileUpdatedEpochSeconds", "()J")) {}
    };

    static ClassInfo class_info(env, JavaLocalRef(env, env->GetObjectClass(info)).get());

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
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;JJLnetwork/loki/messenger/libsession_util/util/UserPic;J)V");

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
            (jlong) info.priority
    )};
}

session::config::blinded_contact_info deserialize_blinded_contact(JNIEnv *env, jobject jInfo) {
    JavaLocalRef<jclass> clazz(env, env->GetObjectClass(jInfo));
    auto idField = env->GetFieldID(clazz.get(), "id", "Ljava/lang/String;");
    auto communityServerField = env->GetFieldID(clazz.get(), "communityServer", "Ljava/lang/String;");
    auto getCommunityServerPubKey = env->GetMethodID(clazz.get(), "getCommunityServerPubKey", "()[B");
    auto nameField = env->GetFieldID(clazz.get(), "name", "Ljava/lang/String;");
    auto createdEpochSecondsField = env->GetFieldID(clazz.get(), "createdEpochSeconds", "J");
    auto profileUpdatedEpochSecondsField = env->GetFieldID(clazz.get(), "profileUpdatedEpochSeconds", "J");
    auto profilePicField = env->GetFieldID(clazz.get(), "profilePic", "Lnetwork/loki/messenger/libsession_util/util/UserPic;");
    auto priorityField = env->GetFieldID(clazz.get(), "priority", "J");

    session::config::blinded_contact_info info(
            JavaStringRef(env, (jstring) env->GetObjectField(jInfo, communityServerField)).view(),
            JavaByteArrayRef(env, (jbyteArray) env->CallObjectMethod(jInfo, getCommunityServerPubKey)).get(),
            JavaStringRef(env, (jstring) env->GetObjectField(jInfo, idField)).view()
    );
    info.created = std::chrono::sys_seconds{std::chrono::seconds{env->GetLongField(jInfo, createdEpochSecondsField)}};
    info.profile_picture = util::deserialize_user_pic(env, JavaLocalRef(env, env->GetObjectField(jInfo, profilePicField)).get());
    info.name = JavaStringRef(env, JavaLocalRef(env, (jstring) env->GetObjectField(jInfo, nameField)).get()).view();
    info.profile_updated = std::chrono::sys_seconds{std::chrono::seconds{env->GetLongField(jInfo, profileUpdatedEpochSecondsField)}};
    info.priority = env->GetLongField(jInfo, priorityField);

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