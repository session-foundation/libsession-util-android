#ifndef SESSION_ANDROID_CONTACTS_H
#define SESSION_ANDROID_CONTACTS_H

#include <jni.h>
#include <vector>
#include "session/config/contacts.hpp"
#include "util.h"
#include "jni_utils.h"

inline session::config::Contacts *ptrToContacts(JNIEnv *env, jobject obj) {
    auto contactsClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/Contacts"));
    jfieldID pointerField = env->GetFieldID(contactsClass.get(), "pointer", "J");
    return (session::config::Contacts *) env->GetLongField(obj, pointerField);
}

inline jobject serialize_contact(JNIEnv *env, session::config::contact_info info) {
    auto contactClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/util/Contact"));
    jmethodID constructor = env->GetMethodID(contactClass.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZZLnetwork/loki/messenger/libsession_util/util/UserPic;JLnetwork/loki/messenger/libsession_util/util/ExpiryMode;)V");
    auto id = jni_utils::JavaLocalRef(env, env->NewStringUTF(info.session_id.data()));
    auto name = jni_utils::JavaLocalRef(env, env->NewStringUTF(info.name.data()));
    auto nickname = jni_utils::JavaLocalRef(env, env->NewStringUTF(info.nickname.data()));
    jboolean approved, approvedMe, blocked;
    approved = info.approved;
    approvedMe = info.approved_me;
    blocked = info.blocked;
    auto created = info.created;
    auto profilePic = jni_utils::JavaLocalRef(env, util::serialize_user_pic(env, info.profile_picture));
    jobject returnObj = env->NewObject(contactClass.get(), constructor, id.get(), name.get(), nickname.get(), approved,
                                       approvedMe, blocked, profilePic.get(), (jlong)info.priority,
                                       util::serialize_expiry(env, info.exp_mode, info.exp_timer));
    return returnObj;
}

inline session::config::contact_info deserialize_contact(JNIEnv *env, jobject info, session::config::Contacts *conf) {
    jclass contactClass = env->FindClass("network/loki/messenger/libsession_util/util/Contact");

    jfieldID getId, getName, getNick, getApproved, getApprovedMe, getBlocked, getUserPic, getPriority, getExpiry, getHidden;
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
    auto account_id = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, getId)));
    auto name = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, getName)));
    auto nickname = jni_utils::JavaLocalRef(env, static_cast<jstring>(env->GetObjectField(info, getNick)));
    bool approved, approvedMe, blocked, hidden;
    int priority = env->GetLongField(info, getPriority);
    approved = env->GetBooleanField(info, getApproved);
    approvedMe = env->GetBooleanField(info, getApprovedMe);
    blocked = env->GetBooleanField(info, getBlocked);
    auto user_pic = jni_utils::JavaLocalRef(env, env->GetObjectField(info, getUserPic));
    auto expiry_mode = jni_utils::JavaLocalRef(env, env->GetObjectField(info, getExpiry));

    auto expiry_pair = util::deserialize_expiry(env, expiry_mode.get());

    std::string url;
    std::vector<unsigned char> key;

    if (user_pic.get() != nullptr) {
        auto deserialized_pic = util::deserialize_user_pic(env, user_pic.get());
        auto url_jstring = deserialized_pic.first;
        auto url_bytes = env->GetStringUTFChars(url_jstring, nullptr);
        url = std::string(url_bytes);
        env->ReleaseStringUTFChars(url_jstring, url_bytes);
        key = util::vector_from_bytes(env, deserialized_pic.second);
    }

    auto account_id_bytes = env->GetStringUTFChars(account_id.get(), nullptr);
    auto name_bytes = name.get() ? env->GetStringUTFChars(name.get(), nullptr) : nullptr;
    auto nickname_bytes = nickname.get() ? env->GetStringUTFChars(nickname.get(), nullptr) : nullptr;

    auto contact_info = conf->get_or_construct(account_id_bytes);
    if (name_bytes) {
        contact_info.name = name_bytes;
    }
    if (nickname_bytes) {
        contact_info.nickname = nickname_bytes;
    }
    contact_info.approved = approved;
    contact_info.approved_me = approvedMe;
    contact_info.blocked = blocked;
    if (!url.empty() && !key.empty()) {
        contact_info.profile_picture = session::config::profile_pic(url, key);
    } else {
        contact_info.profile_picture = session::config::profile_pic();
    }

    env->ReleaseStringUTFChars(account_id.get(), account_id_bytes);
    if (name_bytes) {
        env->ReleaseStringUTFChars(name.get(), name_bytes);
    }
    if (nickname_bytes) {
        env->ReleaseStringUTFChars(nickname.get(), nickname_bytes);
    }

    contact_info.priority = priority;
    contact_info.exp_mode = expiry_pair.first;
    contact_info.exp_timer = std::chrono::seconds(expiry_pair.second);

    return contact_info;
}


#endif //SESSION_ANDROID_CONTACTS_H
