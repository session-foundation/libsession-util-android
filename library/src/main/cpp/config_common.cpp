#include <jni.h>
#include "util.h"
#include "jni_utils.h"

#include <session/config/contacts.hpp>
#include <session/config/user_groups.hpp>
#include <session/config/user_profile.hpp>
#include <session/config/convo_info_volatile.hpp>

extern "C"
JNIEXPORT jlong JNICALL
Java_network_loki_messenger_libsession_1util_ConfigKt_createConfigObject(
        JNIEnv *env,
        jclass _clazz,
        jstring java_config_name,
        jbyteArray ed25519_secret_key,
        jbyteArray initial_dump) {
    return jni_utils::run_catching_cxx_exception_or_throws<jlong>(env, [=] {
        jni_utils::JavaStringRef config_name(env, java_config_name);
        jni_utils::JavaByteArrayRef secret_key(env, ed25519_secret_key);
        auto initial = initial_dump
                       ? std::optional(util::vector_from_bytes(env, initial_dump))
                       : std::nullopt;


        if (config_name.view() == "Contacts") {
            return reinterpret_cast<jlong>(new session::config::Contacts(secret_key.get(), initial));
        } else if (config_name.view() == "UserProfile") {
            return reinterpret_cast<jlong>(new session::config::UserProfile(secret_key.get(), initial));
        } else if (config_name.view() == "UserGroups") {
            return reinterpret_cast<jlong>(new session::config::UserGroups(secret_key.get(), initial));
        } else if (config_name.view() == "ConvoInfoVolatile") {
            return reinterpret_cast<jlong>(new session::config::ConvoInfoVolatile(secret_key.get(), initial));
        } else {
            throw std::invalid_argument("Unknown config name: " + std::string(config_name.view()));
        }
    });
}