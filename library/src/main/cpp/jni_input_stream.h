#ifndef LIBSESSION_UTIL_ANDROID_JNI_INPUT_STREAM_H
#define LIBSESSION_UTIL_ANDROID_JNI_INPUT_STREAM_H

#include <jni.h>

#include "jni_utils.h"

class JniInputStream {
private:
    JNIEnv *env;
    jobject input_stream;
    jmethodID read_method;

public:
    JniInputStream(JNIEnv *env, jobject input_stream)
        : env(env), input_stream(input_stream) {
        jni_utils::JavaLocalRef<jclass> clazz(env, env->GetObjectClass(input_stream));
        read_method = env->GetMethodID(clazz.get(), "read", "([B)I");
    }

    size_t read(uint8_t *buffer, size_t size) {
        jni_utils::JavaLocalRef<jbyteArray> byte_array(env, env->NewByteArray(static_cast<jsize>(size)));
        jint bytes_read = env->CallIntMethod(input_stream, read_method, byte_array.get());

        if (env->ExceptionCheck()) {
            throw std::runtime_error("Exception occurred while reading from InputStream");
        }

        if (bytes_read > 0) {
            env->GetByteArrayRegion(byte_array.get(), 0, bytes_read, reinterpret_cast<jbyte *>(buffer));
        }

        return bytes_read;
    }

    void read_fully(uint8_t *buffer, size_t size) {
        size_t total_bytes_read = 0;
        while (total_bytes_read < size) {
            size_t bytes_read = read(buffer + total_bytes_read, size - total_bytes_read);
            if (bytes_read == 0) {
                throw std::runtime_error("EOF reached");
            }
            total_bytes_read += bytes_read;
        }
    }
};

#endif //LIBSESSION_UTIL_ANDROID_JNI_INPUT_STREAM_H
