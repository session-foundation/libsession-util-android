#include <jni.h>
#include "jni_utils.h"

#include <webp/mux.h>
#include <webp/demux.h>
#include <webp/decode.h>
#include <webp/encode.h>

template<typename T>
using WebPPtr = std::unique_ptr<T, void (*)(T *)>;

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_image_WebPUtils_resizeWebPAnimation(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jbyteArray input,
                                                                                 jint target_width,
                                                                                 jint target_height) {
    jni_utils::JavaByteArrayRef input_ref(env, input);
    WebPData input_data = {
        .bytes = input_ref.bytes(),
        .size = input_ref.size(),
    };

    WebPAnimDecoderOptions opts;
    WebPAnimDecoderOptionsInit(&opts);

    opts.color_mode = MODE_ARGB;
    opts.use_threads = 1;

    WebPPtr<WebPAnimDecoder> decoder(WebPAnimDecoderNew(&input_data, &opts), &WebPAnimDecoderDelete);

    if (!decoder) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                      "Failed to create animation decoder");
        return nullptr;
    }

    WebPAnimInfo info;
    if (!WebPAnimDecoderGetInfo(decoder.get(), &info)) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                      "Failed to get animation info");
        return nullptr;
    }

    WebPPtr<WebPAnimEncoder> encoder(WebPAnimEncoderNew(target_width, target_height, nullptr), &WebPAnimEncoderDelete);
    if (!encoder) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                      "Failed to create animation encoder");
        return 0;
    }


    int ts_mills = 0;
    uint8_t *frame = nullptr;
    WebPPicture pic;
    if (!WebPPictureInit(&pic)) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                      "Failed to initialize WebPPicture");
        return nullptr;
    }

    // First, point the pic to the decoder' frame
    pic.width = info.canvas_width;
    pic.height = info.canvas_height;
    pic.use_argb = 1;

    while (WebPAnimDecoderGetNext(decoder.get(), reinterpret_cast<uint8_t **>(&pic.argb), &ts_mills)) {
        // Re-scale the picture to the target size
        if (!WebPPictureRescale(&pic, target_width, target_height)) {
            env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                          "Failed to rescale picture");
            return nullptr;
        }

        auto encode_succeeded = WebPAnimEncoderAdd(encoder.get(), &pic, ts_mills, nullptr);

        // Free the ARGB buffer allocated by "rescale" as soon as it's used
        WebPPictureFree(&pic);

        if (!encode_succeeded) {
            env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                          "Failed to add frame to encoder");
            return nullptr;
        }

        // Reset pic to get ready for the next decoded frame
        pic.argb = nullptr;
        pic.width = info.canvas_width;
        pic.height = info.canvas_height;
        pic.use_argb = 1;
    }

    WebPData out;
    WebPDataInit(&out);

    if (!WebPAnimEncoderAssemble(encoder.get(), &out)) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                      "Failed to assemble animation");
        return nullptr;
    }

    jni_utils::JavaByteArrayRef out_ref(env, env->NewByteArray(out.size));
    std::memcpy(out_ref.bytes(), out.bytes, out.size);
    WebPDataClear(&out);

    return out_ref.java_array();
}