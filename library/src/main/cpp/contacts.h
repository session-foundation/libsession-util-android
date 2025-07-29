#ifndef SESSION_ANDROID_CONTACTS_H
#define SESSION_ANDROID_CONTACTS_H

#include <jni.h>
#include "session/config/contacts.hpp"

session::config::Contacts *ptrToContacts(JNIEnv *env, jobject obj);
jobject serialize_contact(JNIEnv *env, session::config::contact_info info);
session::config::contact_info deserialize_contact(JNIEnv *env, jobject info, session::config::Contacts *conf);


#endif //SESSION_ANDROID_CONTACTS_H
