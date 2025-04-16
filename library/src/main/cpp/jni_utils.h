#ifndef SESSION_ANDROID_JNI_UTILS_H
#define SESSION_ANDROID_JNI_UTILS_H

#include <jni.h>
#include <exception>
#include <ranges>
#include <type_traits>

#include "util.h"

namespace jni_utils {
    /**
     * Run a C++ function and catch any exceptions, throwing a Java exception if one is caught,
     * and returning a default-constructed value of the specified type.
     *
     * @tparam RetT The return type of the function
     * @tparam Func The function type
     * @param f The function to run
     * @param fallbackRun The function to run if an exception is caught. The optional exception message reference will be passed to this function.
     * @return The return value of the function, or the return value of the fallback function if an exception was caught
     */
    template<class RetT, class Func, class FallbackRun>
    RetT run_catching_cxx_exception_or(Func f, FallbackRun fallbackRun) {
        try {
            return f();
        } catch (const std::exception &e) {
            return fallbackRun(e.what());
        } catch (...) {
            return fallbackRun(nullptr);
        }
    }

    /**
     * Run a C++ function and catch any exceptions, throwing a Java exception if one is caught.
     *
     * @tparam RetT The return type of the function
     * @tparam Func The function type
     * @param env The JNI environment
     * @param f The function to run
     * @return The return value of the function, or a default-constructed value of the specified type if an exception was caught
     */
    template<class RetT, class Func>
    RetT run_catching_cxx_exception_or_throws(JNIEnv *env, Func f) {
        return run_catching_cxx_exception_or<RetT>(f, [env](const char *msg) {
            jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
            if (msg) {
                auto formatted_message = std::string("libsession: C++ exception: ") + msg;
                env->ThrowNew(exceptionClass, formatted_message.c_str());
            } else {
                env->ThrowNew(exceptionClass, "libsession: Unknown C++ exception");
            }

            return RetT();
        });
    }

    template <typename IterBegin, typename IterEnd, typename ConvertItemFunc>
    jobject jlist_from_iterator(JNIEnv *env, IterBegin begin, IterEnd end, const ConvertItemFunc &func) {
        jclass list_clazz = env->FindClass("java/util/ArrayList");
        jmethodID init = env->GetMethodID(list_clazz, "<init>", "()V");
        jobject our_list = env->NewObject(list_clazz, init);
        jmethodID push = env->GetMethodID(list_clazz, "add", "(Ljava/lang/Object;)Z");

        for (auto iter = begin; iter != end; ++iter) {
            jobject item_java = func(env, *iter);
            env->CallBooleanMethod(our_list, push, item_java);
            env->DeleteLocalRef(item_java);
        }

        env->DeleteLocalRef(list_clazz);
        return our_list;
    }

    template <typename Collection, typename ConvertItemFunc>
    jobject jlist_from_collection(JNIEnv *env, const Collection& obj, const ConvertItemFunc &func) {
        return jlist_from_iterator(env, obj.begin(), obj.end(), func);
    }

    template <typename Collection>
    jobject jstring_list_from_collection(JNIEnv *env, const Collection& obj) {
        return jlist_from_collection(env, obj, util::jstringFromOptional);
    }

    template <typename Collection>
    jobject session_bytes_from_range(JNIEnv *env, const Collection &obj) {
        jclass bytes_clazz = env->FindClass("network/loki/messenger/libsession_util/util/Bytes");
        jmethodID init = env->GetMethodID(bytes_clazz, "<init>", "([B)V");

        auto bytes_array = env->NewByteArray(static_cast<jsize>(obj.size()));
        env->SetByteArrayRegion(bytes_array, 0, static_cast<jsize>(obj.size()), reinterpret_cast<const jbyte *>(obj.data()));
        return env->NewObject(bytes_clazz, init, bytes_array);
    }
}

#endif //SESSION_ANDROID_JNI_UTILS_H
