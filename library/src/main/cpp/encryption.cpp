#include <jni.h>

#include <session/session_encrypt.hpp>

#include "jni_utils.h"

using jni_utils::JavaByteArrayRef;

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_SessionEncrypt_decryptForBlindedRecipient(
        JNIEnv *env,
        jobject _thiz,
        jbyteArray ciphertext,
        jbyteArray my_ed25519_privte_key,
        jbyteArray open_group_public_key,
        jbyteArray sender_blinded_id,
        jbyteArray recipient_blind_id) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto [plain_text, session_id] = session::decrypt_from_blinded_recipient(
            JavaByteArrayRef(env, my_ed25519_privte_key).get(),
            JavaByteArrayRef(env, open_group_public_key).get(),
            JavaByteArrayRef(env, sender_blinded_id).get(),
            JavaByteArrayRef(env, recipient_blind_id).get(),
            JavaByteArrayRef(env, ciphertext).get()
        );

        return jni_utils::new_kotlin_pair(
                env,
                util::jstringFromOptional(env, session_id),
                jni_utils::session_bytes_from_range(env, plain_text)
        );
    });
}
