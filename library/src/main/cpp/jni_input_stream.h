#ifndef LIBSESSION_UTIL_ANDROID_JNI_INPUT_STREAM_H
#define LIBSESSION_UTIL_ANDROID_JNI_INPUT_STREAM_H

#include <jni.h>

#include "jni_utils.h"

class JniInputStream {
private:
    JNIEnv *env;
    jobject input_stream;
    jmethodID read_method;
    std::optional<jni_utils::JavaLocalRef<jbyteArray>> buffer;

public:
    JniInputStream(JNIEnv *env, jobject input_stream)
        : env(env), input_stream(input_stream) {
        jni_utils::JavaLocalRef<jclass> clazz(env, env->GetObjectClass(input_stream));
        read_method = env->GetMethodID(clazz.get(), "read", "([BII)I");
    }

    JniInputStream(const JniInputStream&) = delete;
    JniInputStream& operator=(const JniInputStream&) = delete;

    size_t read_fully(uint8_t *out, size_t len) {
        if (!buffer.has_value() || env->GetArrayLength(buffer->get()) < len) {
            buffer.emplace(env, env->NewByteArray(std::max<jsize>(len, 512)));
        }

        size_t remaining = len;
        while (remaining > 0) {
            jint bytes_read = env->CallIntMethod(input_stream,
                                                 read_method,
                                                 buffer->get(),
                                                 static_cast<jint>(len - remaining),
                                                 static_cast<jint>(remaining));
            if (env->ExceptionCheck()) {
                throw std::runtime_error("Exception occurred while reading from InputStream");
            }

            if (bytes_read <= 0) {
                throw std::runtime_error("End of stream reached before reading requested number of bytes");
            }

            remaining -= bytes_read;
        }


        env->GetByteArrayRegion(buffer->get(), 0, static_cast<jint>(len), reinterpret_cast<jbyte *>(out));
        return len;
    }
};

#endif //LIBSESSION_UTIL_ANDROID_JNI_INPUT_STREAM_H
