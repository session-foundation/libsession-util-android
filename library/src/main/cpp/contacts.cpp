#include "contacts.h"
#include "util.h"
#include "jni_utils.h"

session::config::Contacts *ptrToContacts(JNIEnv *env, jobject obj) {
    auto contactsClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/Contacts"));
    jfieldID pointerField = env->GetFieldID(contactsClass.get(), "pointer", "J");
    return (session::config::Contacts *) env->GetLongField(obj, pointerField);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_get(JNIEnv *env, jobject thiz,
                                                          jstring account_id) {
    // If an exception is thrown, return nullptr
    return jni_utils::run_catching_cxx_exception_or<jobject>(
            [=]() -> jobject {
                std::lock_guard lock{util::util_mutex_};
                auto contacts = ptrToContacts(env, thiz);
                auto contact = contacts->get(jni_utils::JavaStringRef(env, account_id).view());
                if (!contact) return nullptr;
                jobject j_contact = serialize_contact(env, contact.value());
                return j_contact;
            },
            [](const char *) -> jobject { return nullptr; }
    );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_getOrConstruct(JNIEnv *env, jobject thiz,
                                                                     jstring account_id) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        std::lock_guard lock{util::util_mutex_};
        auto contacts = ptrToContacts(env, thiz);
        auto contact = contacts->get_or_construct(jni_utils::JavaStringRef(env, account_id).view());
        return serialize_contact(env, contact);
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_set(JNIEnv *env, jobject thiz,
                                                          jobject contact) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        std::lock_guard lock{util::util_mutex_};
        auto contacts = ptrToContacts(env, thiz);
        auto contact_info = deserialize_contact(env, contact, contacts);
        contacts->set(contact_info);
    });
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_erase(JNIEnv *env, jobject thiz,
                                                            jstring account_id) {
    return jni_utils::run_catching_cxx_exception_or_throws<jboolean>(env, [=] {
        std::lock_guard lock{util::util_mutex_};
        auto contacts = ptrToContacts(env, thiz);
        bool result = contacts->erase(jni_utils::JavaStringRef(env, account_id).view());
        return result;
    });
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_all(JNIEnv *env, jobject thiz) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        std::lock_guard lock{util::util_mutex_};
        return jni_utils::jlist_from_collection(env, *ptrToContacts(env, thiz), serialize_contact);
    });
}

jobject serialize_contact(JNIEnv *env, session::config::contact_info info) {
    auto contactClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Contact"));
    jmethodID constructor = env->GetMethodID(contactClass.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZZLnetwork/loki/messenger/libsession_util/util/UserPic;JJJLnetwork/loki/messenger/libsession_util/util/ExpiryMode;)V");
    jobject returnObj = env->NewObject(contactClass.get(),
                                       constructor,
                                       jni_utils::JavaLocalRef(env, env->NewStringUTF(info.session_id.data())).get(),
                                       jni_utils::JavaLocalRef(env, env->NewStringUTF(info.name.data())).get(),
                                       jni_utils::JavaLocalRef(env, env->NewStringUTF(info.nickname.data())).get(),
                                       (jboolean) info.approved,
                                       (jboolean) info.approved_me,
                                       (jboolean) info.blocked,
                                       jni_utils::JavaLocalRef(env, util::serialize_user_pic(env, info.profile_picture)).get(),
                                       (jlong) info.created,
                                       (jlong) (info.profile_updated.time_since_epoch().count()),
                                       (jlong) info.priority,
                                       util::serialize_expiry(env, info.exp_mode, info.exp_timer));
    return returnObj;
}

session::config::contact_info deserialize_contact(JNIEnv *env, jobject info, session::config::Contacts *conf) {
    jclass contactClass = env->FindClass("network/loki/messenger/libsession_util/util/Contact");

    jfieldID getId, getName, getNick, getApproved, getApprovedMe, getBlocked, getUserPic, getPriority, getExpiry, getHidden, profileUpdatedField;
    getId = env->GetFieldID(contactClass, "id", "Ljava/lang/String;");
    getName = env->GetFieldID(contactClass, "name", "Ljava/lang/String;");
    getNick = env->GetFieldID(contactClass, "nickname", "Ljava/lang/String;");
    getApproved = env->GetFieldID(contactClass, "approved", "Z");
    getApprovedMe = env->GetFieldID(contactClass, "approvedMe", "Z");
    getBlocked = env->GetFieldID(contactClass, "blocked", "Z");
    getUserPic = env->GetFieldID(contactClass, "profilePicture",
                                 "Lnetwork/loki/messenger/libsession_util/util/UserPic;");
    getPriority = env->GetFieldID(contactClass, "priority", "J");
    getExpiry = env->GetFieldID(contactClass, "expiryMode", "Lnetwork/loki/messenger/libsession_util/util/ExpiryMode;");
    profileUpdatedField = env->GetFieldID(contactClass, "profileUpdatedEpochSeconds", "J");

    jni_utils::JavaLocalRef account_id(env, static_cast<jstring>(env->GetObjectField(info, getId)));
    jni_utils::JavaLocalRef name(env, static_cast<jstring>(env->GetObjectField(info, getName)));
    jni_utils::JavaLocalRef nickname(env, static_cast<jstring>(env->GetObjectField(info, getNick)));
    jni_utils::JavaLocalRef user_pic(env, env->GetObjectField(info, getUserPic));
    jni_utils::JavaLocalRef expiry_mode(env, env->GetObjectField(info, getExpiry));

    auto expiry_pair = util::deserialize_expiry(env, expiry_mode.get());
    auto profile_updated_seconds = env->GetLongField(info, profileUpdatedField);

    auto contact_info = conf->get_or_construct(jni_utils::JavaStringRef(env, account_id.get()).view());
    if (name.get()) {
        contact_info.name = jni_utils::JavaStringRef(env, name.get()).view();
    }
    if (nickname.get()) {
        contact_info.nickname = jni_utils::JavaStringRef(env, nickname.get()).view();
    }
    contact_info.approved = env->GetBooleanField(info, getApproved);
    contact_info.approved_me = env->GetBooleanField(info, getApprovedMe);
    contact_info.blocked = env->GetBooleanField(info, getBlocked);
    contact_info.profile_updated = std::chrono::sys_seconds{std::chrono::seconds{profile_updated_seconds}};
    if (user_pic.get() != nullptr) {
        contact_info.profile_picture = util::deserialize_user_pic(env, user_pic.get());
    }

    contact_info.priority = env->GetLongField(info, getPriority);
    contact_info.exp_mode = expiry_pair.first;
    contact_info.exp_timer = std::chrono::seconds(expiry_pair.second);

    return contact_info;
}

jobject serialize_blinded_contact(JNIEnv *env, const session::config::blinded_contact_info &info) {
    jni_utils::JavaLocalRef<jclass> clazz(env, env->FindClass("network/loki/messenger/libsession_util/util/BlindedContact"));
    auto constructor = env->GetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;JLnetwork/loki/messenger/libsession_util/util/UserPic;)V");

    return env->NewObject(
            clazz.get(),
            constructor,
            jni_utils::JavaLocalRef(env, env->NewStringUTF(info.session_id().c_str())).get(),
            jni_utils::JavaLocalRef(env, env->NewStringUTF(info.community_base_url().data())).get(),
            jni_utils::JavaLocalRef(env, env->NewStringUTF(info.community_pubkey_hex().data())).get(),
            jni_utils::JavaLocalRef(env, env->NewStringUTF(info.name.c_str())).get(),
            (jlong) (info.created.time_since_epoch().count()),
            jni_utils::JavaLocalRef(env, util::serialize_user_pic(env, info.profile_picture)).get()
    );
}

session::config::blinded_contact_info deserialize_blinded_contact(JNIEnv *env, jobject jInfo) {
    jni_utils::JavaLocalRef<jclass> clazz(env, env->GetObjectClass(jInfo));
    auto idField = env->GetFieldID(clazz.get(), "id", "Ljava/lang/String;");
    auto communityServerField = env->GetFieldID(clazz.get(), "communityServer", "Ljava/lang/String;");
    auto getCommunityServerPubKey = env->GetMethodID(clazz.get(), "getCommunityServerPubKey", "()[B");
    auto nameField = env->GetFieldID(clazz.get(), "name", "Ljava/lang/String;");
    auto createdEpochSecondsField = env->GetFieldID(clazz.get(), "createdEpochSeconds", "J");
    auto profilePicField = env->GetFieldID(clazz.get(), "profilePic", "Lnetwork/loki/messenger/libsession_util/util/UserPic;");

    session::config::blinded_contact_info info(
            jni_utils::JavaStringRef(env, (jstring) env->GetObjectField(jInfo, communityServerField)).view(),
            jni_utils::JavaByteArrayRef(env, (jbyteArray) env->CallObjectMethod(jInfo, getCommunityServerPubKey)).get(),
            jni_utils::JavaStringRef(env, (jstring) env->GetObjectField(jInfo, idField)).view()
    );
    info.created = std::chrono::sys_seconds{std::chrono::seconds{env->GetLongField(jInfo, createdEpochSecondsField)}};
    info.profile_picture = util::deserialize_user_pic(env, jni_utils::JavaLocalRef(env, env->GetObjectField(jInfo, profilePicField)).get());

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
            jni_utils::JavaStringRef(env, community_server_url).view(),
            jni_utils::JavaStringRef(env, community_server_pub_key_hex).view(),
            jni_utils::JavaStringRef(env, blinded_id).view()
    ));
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
            jni_utils::JavaStringRef(env, community_server_url).view(),
            jni_utils::JavaStringRef(env, blinded_id).view()
    );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_Contacts_allBlinded(JNIEnv *env, jobject thiz) {
    return jni_utils::jlist_from_collection(
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
    auto result = ptrToContacts(env, thiz)->get_blinded(jni_utils::JavaStringRef(env, blinded_id).view());

    if (result) {
        return serialize_blinded_contact(env, *result);
    } else {
        return nullptr;
    }
}