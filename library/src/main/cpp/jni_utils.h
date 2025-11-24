#ifndef SESSION_ANDROID_JNI_UTILS_H
#define SESSION_ANDROID_JNI_UTILS_H

#include <jni.h>
#include <exception>
#include <ranges>
#include <type_traits>

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

    /**
     * A RAII wrapper for a local reference to a Java object. This will automatically delete the local reference
     * @tparam JNIType Type of JNI object
     */
    template <typename JNIType>
    class JavaLocalRef {
        JNIEnv *env_;
        JNIType ref_;

        template<typename>
        friend class JavaLocalRef;

    public:
        JavaLocalRef(JNIEnv *env, JNIType ref) : env_(env), ref_(ref) {}
        ~JavaLocalRef() {
            if (ref_) {
                env_->DeleteLocalRef(ref_);
            }
        }

        template<typename OtherJNIType>
        JavaLocalRef(JavaLocalRef<OtherJNIType> && other)  noexcept : env_(other.env_), ref_(other.ref_) {
            other.ref_ = nullptr;
        }

        JavaLocalRef(const JavaLocalRef&) = delete;

        void reset(JNIType new_ref) {
            if (ref_ != new_ref) {
                env_->DeleteLocalRef(ref_);
            }
            ref_ = new_ref;
        }

        inline JNIType get() const {
            return ref_;
        }

        JNIType leak() {
            auto r = ref_;
            ref_ = nullptr;
            return r;
        }
    };

    struct JavaClassInfo {
        jclass const java_class;

        JavaClassInfo(JNIEnv *env, const char *class_name)
                :java_class((jclass) env->NewGlobalRef(JavaLocalRef(env, env->FindClass(class_name)).get())) {}

        JavaClassInfo(JNIEnv *env, jobject classInstance)
                :java_class((jclass) env->NewGlobalRef(JavaLocalRef(env, env->GetObjectClass(classInstance)).get())) {}
    };

    /**
     * Convenient way to store information to create a generic Java object
     */
    struct BasicJavaClassInfo : public JavaClassInfo {
        jmethodID constructor;

        BasicJavaClassInfo(JNIEnv *env, const char *class_name, const char *constructor_signature)
                :JavaClassInfo(env, class_name),
                 constructor(env->GetMethodID(java_class, "<init>", constructor_signature)) {
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                throw std::runtime_error("Failed to find constructor for class " + std::string(class_name));
            }
        }
    };


    /**
     * Convenient way to store information to create a Java ArrayList object
     */
    struct ArrayListClassInfo: public BasicJavaClassInfo {
        jmethodID add_method;

        static const ArrayListClassInfo & get(JNIEnv *env);

    private:
        explicit ArrayListClassInfo(JNIEnv *env);
    };

    /**
     * Create a Java List from an iterator.
     *
     * @tparam IterBegin The type of beginning iterator
     * @tparam IterEnd The type of end iterator. It must be comparable to IterBegin
     * @tparam ConvertItemFunc The lambda to convert the item of the iterator into jobject/std::optional<jobject>. If an std::nullopt is returned, the item will be skipped
     * @param env
     * @param begin
     * @param end
     * @param func
     * @return The java list
     */
    template <typename IterBegin, typename IterEnd, typename ConvertItemFunc>
    jobject jlist_from_iterator(JNIEnv *env, IterBegin begin, IterEnd end, const ConvertItemFunc &func) {
        auto info = ArrayListClassInfo::get(env);

        auto our_list = env->NewObject(info.java_class, info.constructor);

        for (auto iter = begin; iter != end; ++iter) {
            std::optional<JavaLocalRef<jobject>> item_java= func(env, *iter);
            if (item_java.has_value()) {
                env->CallBooleanMethod(our_list, info.add_method, item_java->get());
            }
        }

        return our_list;
    }

    inline JavaLocalRef<jstring> jstring_from_optional(JNIEnv* env, std::optional<std::string_view> optional) {
        return {env, optional ? env->NewStringUTF(optional->data()) : nullptr};
    }

    /**
     * Convenience function to create a Java List from a collection, using the lambda to convert
     * each item.
     *
     * See [jlist_from_iterator] for more info.
     */
    template <typename Collection, typename ConvertItemFunc>
    jobject jlist_from_collection(JNIEnv *env, const Collection& obj, const ConvertItemFunc &func) {
        return jlist_from_iterator(env, obj.begin(), obj.end(), func);
    }

    /**
     * Convenience function to create a Java List from a collection of strings
     *
     * See [jlist_from_iterator] for more info.
     */
    template <typename Collection>
    jobject jstring_list_from_collection(JNIEnv *env, const Collection& obj) {
        return jlist_from_collection(env, obj, jstring_from_optional);
    }

    /**
     * Create a Java Bytes class from the collection. The collection must be continous range of byte data
     */
    template <typename Collection>
    JavaLocalRef<jobject> session_bytes_from_range(JNIEnv *env, const Collection &obj) {
        static BasicJavaClassInfo bytes_class(
                env, "network/loki/messenger/libsession_util/util/Bytes", "([B)V");

        static_assert(sizeof(*obj.data()) == sizeof(jbyte));

        auto bytes_array = JavaLocalRef(env, env->NewByteArray(static_cast<jsize>(obj.size())));
        env->SetByteArrayRegion(bytes_array.get(), 0, static_cast<jsize>(obj.size()), reinterpret_cast<const jbyte *>(obj.data()));
        return {env, env->NewObject(bytes_class.java_class, bytes_class.constructor, bytes_array.get())};
    }

    /**
     * Create a Kotlin Pair object
     */
    jobject new_kotlin_pair(JNIEnv *env, jobject first, jobject second);

    /**
     * Create a new KeyPair object
     */
    jobject new_key_pair(JNIEnv *env, jbyteArray pubKey, jbyteArray secKey);

    class JavaStringRef {
        JNIEnv *env;
        jstring s;
        std::span<char> data;

        public:
            JavaStringRef(JNIEnv *env, jstring s) : env(env), s(s) {
                const char *c_str = env->GetStringUTFChars(s, nullptr);
                data = std::span<char>(const_cast<char *>(c_str), env->GetStringUTFLength(s));
            }

            JavaStringRef(const JavaStringRef &) = delete;

            ~JavaStringRef() {
                env->ReleaseStringUTFChars(s, data.data());
            }

            // Get the data as a string view. Only valid during the lifetime of this object.
            std::string_view view() const {
                return std::string_view(data.data(), data.size());
            }

            std::string copy() const {
                return std::string(view());
            }

            // Get the data as a span. Only valid during the lifetime of this object.
            std::span<char> get() const {
                return data;
            }

            std::span<uint8_t> get_raw() const {
                return std::span<uint8_t>(reinterpret_cast<uint8_t *>(data.data()), data.size());
            }
    };

    class JavaCharsRef {
        JNIEnv *env;
        jstring s;
        std::span<jchar> data;

    public:
        JavaCharsRef(JNIEnv *env, jstring s) : env(env), s(s) {
            const jchar *c_str = env->GetStringChars(s, nullptr);
            data = std::span<jchar>(const_cast<jchar *>(c_str), env->GetStringLength(s));
        }

        JavaCharsRef(const JavaCharsRef &) = delete;

        ~JavaCharsRef() {
            env->ReleaseStringChars(s, data.data());
        }

        const jchar* chars() const {
            return data.data();
        }

        size_t size() const {
            return data.size();
        }

        // Get the data as a span. Only valid during the lifetime of this object.
        std::span<jchar> get() const {
            return data;
        }
    };

    /**
     * A RAII wrapper for a Java byte array. This will automatically release the byte array when it goes out of scope.
     */
    class JavaByteArrayRef {
        JNIEnv *env;
        jbyteArray byte_array;
        std::span<unsigned char> data;

        public:
            JavaByteArrayRef(JNIEnv *env, jbyteArray byte_array) : env(env), byte_array(byte_array) {
                if (byte_array) {
                    jsize length = env->GetArrayLength(byte_array);
                    data = std::span<unsigned char>(
                            reinterpret_cast<unsigned char *>(env->GetByteArrayElements(byte_array,
                                                                                        nullptr)),
                            length);
                }
            }

            JavaByteArrayRef(const JavaByteArrayRef &) = delete;

            JavaByteArrayRef(JavaByteArrayRef&& other) : env(other.env), byte_array(other.byte_array), data(other.data) {
                other.byte_array = nullptr;
                other.data = {};
            }

            ~JavaByteArrayRef() {
                if (byte_array) {
                    env->ReleaseByteArrayElements(byte_array,
                                                  reinterpret_cast<jbyte *>(data.data()), 0);
                }
            }

            jbyteArray java_array() const {
                return byte_array;
            }

            const unsigned char * bytes() const {
                return data.data();
            }

            unsigned char *bytes() {
                return data.data();
            }

            size_t size() const {
                return data.size();
            }

            // Get the data as a span. Only valid during the lifetime of this object.
            std::span<unsigned char> get() const {
                return data;
            }

            std::span<std::byte> get_as_bytes() const {
                return std::span<std::byte>(reinterpret_cast<std::byte *>(data.data()), data.size());
            }

            std::vector<unsigned char> copy() const {
                return std::vector(data.begin(), data.end());
            }
    };

    template <size_t N>
    static std::optional<std::array<unsigned char, N>> java_to_cpp_array(JNIEnv *env, jbyteArray array) {
        if (!array) {
            return std::nullopt;
        }

        JavaByteArrayRef bytes(env, array);
        auto span = bytes.get();
        if (span.size() != N) {
            throw std::runtime_error("Invalid byte array length from java, expecting " + std::to_string(N) + " got " + std::to_string(span.size()));
        }

        std::array<unsigned char, N> out;
        std::copy(span.begin(), span.end(), out.begin());
        return out;
    }

}

#endif //SESSION_ANDROID_JNI_UTILS_H
