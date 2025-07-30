#ifndef SESSION_ANDROID_CONVERSATION_H
#define SESSION_ANDROID_CONVERSATION_H

#include <jni.h>
#include <android/log.h>
#include "util.h"
#include "session/config/convo_info_volatile.hpp"
#include "jni_utils.h"

inline session::config::ConvoInfoVolatile *ptrToConvoInfo(JNIEnv *env, jobject obj) {
    auto contactsClass = jni_utils::JavaLocalRef(env, env->FindClass("network/loki/messenger/libsession_util/ConversationVolatileConfig"));
    jfieldID pointerField = env->GetFieldID(contactsClass.get(), "pointer", "J");
    return (session::config::ConvoInfoVolatile *) env->GetLongField(obj, pointerField);
}


#endif //SESSION_ANDROID_CONVERSATION_H