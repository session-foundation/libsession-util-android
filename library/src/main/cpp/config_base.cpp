#include "config_base.h"
#include "util.h"
#include "jni_utils.h"


std::pair<std::string, std::vector<unsigned char>> extractHashAndData(JNIEnv *env, jobject kotlin_pair) {
    jni_utils::JavaLocalRef<jclass> pair(env, env->GetObjectClass(kotlin_pair));
    jfieldID first = env->GetFieldID(pair.get(), "first", "Ljava/lang/Object;");
    jfieldID second = env->GetFieldID(pair.get(), "second", "Ljava/lang/Object;");
    auto hash_as_jstring = jni_utils::JavaLocalRef(env, reinterpret_cast<jstring>(env->GetObjectField(kotlin_pair, first)));
    auto data_as_jbytes = jni_utils::JavaLocalRef(env, reinterpret_cast<jbyteArray>(env->GetObjectField(kotlin_pair, second)));

    return std::make_pair(
            std::string(jni_utils::JavaStringRef(env, hash_as_jstring.get()).view()),
            jni_utils::JavaByteArrayRef(env, data_as_jbytes.get()).copy()
    );
}

struct JavaConfigClassInfo : public jni_utils::JavaClassInfo {
    jfieldID pointer_field;

    JavaConfigClassInfo(JNIEnv *env, jobject obj)
            :JavaClassInfo(env, obj),
             pointer_field(env->GetFieldID(java_class, "pointer", "J")) {}

    static const JavaConfigClassInfo& get(JNIEnv *env, jobject obj) {
        static JavaConfigClassInfo class_info(env, obj);
        return class_info;
    }
};

session::config::ConfigBase* ptrToConfigBase(JNIEnv *env, jobject obj) {
    return reinterpret_cast<session::config::ConfigBase*>(
            env->GetLongField(obj, JavaConfigClassInfo::get(env, obj).pointer_field));
}

session::config::ConfigSig* ptrToConfigSig(JNIEnv* env, jobject obj) {
    return reinterpret_cast<session::config::ConfigSig*>(
            env->GetLongField(obj, JavaConfigClassInfo::get(env, obj).pointer_field));
}

extern "C" {
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_dirty(JNIEnv *env, jobject thiz) {
    auto* configBase = ptrToConfigBase(env, thiz);
    return configBase->is_dirty();
}

JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_needsPush(JNIEnv *env, jobject thiz) {
    auto config = ptrToConfigBase(env, thiz);
    return config->needs_push();
}

JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_needsDump(JNIEnv *env, jobject thiz) {
    auto config = ptrToConfigBase(env, thiz);
    return config->needs_dump();
}

JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_push(JNIEnv *env, jobject thiz) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto config = ptrToConfigBase(env, thiz);
        auto [seq_no, to_push, to_delete] = config->push();

        jobject messages = jni_utils::jlist_from_collection(env, to_push, [](JNIEnv *env, const std::vector<unsigned char> &data) {
            return jni_utils::session_bytes_from_range(env, data);
        });

        jobject obsoleteHashes = jni_utils::jstring_list_from_collection(env, to_delete);

        static jni_utils::BasicJavaClassInfo class_info(
            env,
            "network/loki/messenger/libsession_util/util/ConfigPush",
            "(Ljava/util/List;JLjava/util/List;)V"
        );

        return env->NewObject(class_info.java_class, class_info.constructor,
                              messages, static_cast<jlong>(seq_no), obsoleteHashes);
    });
}

JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_free(JNIEnv *env, jobject thiz) {
    auto config = ptrToConfigBase(env, thiz);
    delete config;
}

JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_dump(JNIEnv *env, jobject thiz) {
    auto config = ptrToConfigBase(env, thiz);
    auto dumped = config->dump();
    return util::bytes_from_vector(env, dumped).release();
}

JNIEXPORT jstring JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_encryptionDomain(JNIEnv *env,
                                                                         jobject thiz) {
    auto conf = ptrToConfigBase(env, thiz);
    return env->NewStringUTF(conf->encryption_domain());
}

JNIEXPORT void JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_confirmPushed(JNIEnv *env, jobject thiz,
                                                                      jlong seq_no,
                                                                      jobjectArray hash_list) {
    auto conf = ptrToConfigBase(env, thiz);
    auto hash_list_size = env->GetArrayLength(hash_list);
    std::unordered_set<std::string> hashes(hash_list_size);

    for (int i = 0; i < hash_list_size; i++) {
        jni_utils::JavaLocalRef hash_jstring(env, (jstring) env->GetObjectArrayElement(hash_list, i));
        hashes.insert(jni_utils::JavaStringRef(env, hash_jstring.get()).copy());
    }

    conf->confirm_pushed(seq_no, hashes);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_merge___3Lkotlin_Pair_2(JNIEnv *env, jobject thiz,
                                                                     jobjectArray to_merge) {
    return jni_utils::run_catching_cxx_exception_or_throws<jobject>(env, [=] {
        auto conf = ptrToConfigBase(env, thiz);
        size_t number = env->GetArrayLength(to_merge);
        std::vector<std::pair<std::string, std::vector<unsigned char>>> configs = {};
        for (int i = 0; i < number; i++) {
            auto jElement = (jobject) env->GetObjectArrayElement(to_merge, i);
            auto pair = extractHashAndData(env, jElement);
            configs.push_back(pair);
        }
        return jni_utils::jstring_list_from_collection(env, conf->merge(configs));
    });
}

#pragma clang diagnostic pop
}

extern "C"
JNIEXPORT jobject JNICALL
Java_network_loki_messenger_libsession_1util_ConfigBase_activeHashes(JNIEnv *env, jobject thiz) {
    auto conf = ptrToConfigBase(env, thiz);
    return jni_utils::jstring_list_from_collection(env, conf->active_hashes());
}