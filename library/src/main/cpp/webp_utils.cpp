#include <jni.h>
#include "jni_utils.h"

#include <chrono>

#include <webp/mux.h>
#include <webp/demux.h>
#include <webp/decode.h>
#include <webp/encode.h>


template<typename T>
using WebPPtr = std::unique_ptr<T, void (*)(T *)>;

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_image_WebPUtils_reencodeWebPAnimation(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jbyteArray input,
                                                                                   jlong timeout_ms,
                                                                                   jint target_width,
                                                                                   jint target_height) {
    jni_utils::JavaByteArrayRef input_ref(env, input);
    WebPData input_data = {
        .bytes = input_ref.bytes(),
        .size = input_ref.size(),
    };

    WebPAnimDecoderOptions opts;
    WebPAnimDecoderOptionsInit(&opts);

    opts.color_mode = MODE_RGBA;
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
    uint8_t *frame_data = nullptr;
    const auto deadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout_ms);

    while (WebPAnimDecoderGetNext(decoder.get(), &frame_data, &ts_mills)) {
        WebPPicture pic;
        WebPPictureInit(&pic);

        // First, import the frame into a picture
        pic.width = info.canvas_width;
        pic.height = info.canvas_height;
        pic.use_argb = 0;

        if (!WebPPictureImportRGBA(&pic, frame_data, info.canvas_width * 4)) {
            env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                          "Failed to import frame into picture");
            return nullptr;
        }

        // If the target size is different, rescale the picture
        if (target_width != info.canvas_width || target_height != info.canvas_height) {
            // Re-scale the picture to the target size
            if (!WebPPictureRescale(&pic, target_width, target_height)) {
                WebPPictureFree(&pic);
                env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                              "Failed to rescale picture");
                return nullptr;
            }
        }

        // Now add the picture to the encoder
        WebPConfig encode_config;
        WebPConfigInit(&encode_config);
        encode_config.quality = 95.f;

        auto encode_succeeded = WebPAnimEncoderAdd(encoder.get(), &pic, ts_mills, &encode_config);

        // Free the picture as sonn as we're done with it
        WebPPictureFree(&pic);

        if (!encode_succeeded) {
            env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                          "Failed to add frame to encoder");
            return nullptr;
        }

        if (std::chrono::high_resolution_clock::now() > deadline) {
            env->ThrowNew(env->FindClass("java/util/concurrent/TimeoutException"),
                          "Re-encoding animation timed out");
            return nullptr;
        }
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
