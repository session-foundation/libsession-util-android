#include "jni_utils.h"

#include <session/hash.hpp>

extern "C"
JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_Hash_hash(JNIEnv *env, jobject thiz,
                                                       jbyteArray message,
                                                       jbyteArray hashOut,
                                                       jbyteArray key) {
    jni_utils::run_catching_cxx_exception_or_throws<void>(env, [=] {
        session::hash::hash(
                jni_utils::JavaByteArrayRef(env, hashOut).get(),
                jni_utils::JavaByteArrayRef(env, message).get(),
                key ? std::make_optional(jni_utils::JavaByteArrayRef(env, key).get()) : std::nullopt);
    });
}