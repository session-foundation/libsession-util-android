#include <jni.h>
#include <memory>
#include <chrono>

#include "jni_utils.h"

#include <cgif.h>
#include <libyuv.h>
#include <android/log.h>

#include <EasyGifReader.h>

template<typename T>
using GifPtr = std::unique_ptr<T, void (*)(T *)>;


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_image_GifUtils_reencodeGif(JNIEnv *env, jobject thiz,
                                                                        jbyteArray input,
                                                                        jlong timeout_mills,
                                                                        jint target_width,
                                                                        jint target_height) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=]() -> jbyteArray {
        jni_utils::JavaByteArrayRef input_array(env, input);

        EasyGifReader decoder = EasyGifReader::openMemory(input_array.bytes(), input_array.size());

        std::vector<uint8_t> output_buffer;

        CGIFrgb_Config encode_config = {
                .pWriteFn = [](void* pContext, const uint8_t* pData, const size_t numBytes) -> int {
                    auto* output_buffer = static_cast<std::vector<uint8_t>*>(pContext);
                    output_buffer->insert(output_buffer->end(), pData, pData + numBytes);
                    return 0; // success
                },
                .pContext = &output_buffer,
                .path = nullptr,
                .attrFlags = 0,
                .genFlags = 0,
                .numLoops = static_cast<uint16_t>(decoder.repeatsInfinitely() ? 0 : decoder.repeatCount()),
                .width = static_cast<uint16_t>(target_width),
                .height = static_cast<uint16_t>(target_height),
        };

        GifPtr<CGIFrgb> encoder(cgif_rgb_newgif(&encode_config), [](CGIFrgb* ptr) { cgif_rgb_close(ptr); });
        if (!encoder) {
            throw std::runtime_error("Failed to create GIF encoder");
        }

        const auto src_width = decoder.width();
        const auto src_height = decoder.height();

        const auto needRescale = (src_width != target_width) || (src_height != target_height);
        const auto deadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout_mills);

        auto is_timeout = [&]() {
            if (std::chrono::high_resolution_clock::now() > deadline) {
                env->ThrowNew(env->FindClass("java/util/concurrent/TimeoutException"),
                              "GIF re-encoding timed out");
                return true;
            }
            return false;
        };

        std::vector<uint8_t> decode_argb_buffer, encode_argb_buffer, encode_rgba_buffer;
        if (needRescale) {
            decode_argb_buffer.resize(src_width * src_height * 4);
            encode_argb_buffer.resize(target_width * target_height * 4);
            encode_rgba_buffer.resize(target_width * target_height * 4);
        }

        for (auto frame = decoder.begin(); frame != decoder.end() && !is_timeout(); ++frame) {
            // Here we would add the frame to the encoder
            CGIFrgb_FrameConfig config = {
                    .pImageData = const_cast<uint8_t*>(frame->pixels()),
                    .fmtChan = CGIF_CHAN_FMT_RGBA,
                    .attrFlags = 0,
                    .genFlags = 0,
                    .delay = static_cast<uint16_t>(frame.rawDuration().centiseconds), // in 0.01s
            };

            if (needRescale) {
                // First we need to convert our frame data from RGBA to ARGB
                libyuv::RGBAToARGB(
                        frame->pixels(), src_width * 4,
                        decode_argb_buffer.data(), src_width * 4,
                        src_width, src_height
                );

                // Scale it to target size
                libyuv::ARGBScale(
                        decode_argb_buffer.data(), src_width * 4,
                        src_width, src_height,
                        encode_argb_buffer.data(), target_width * 4,
                        target_width, target_height,
                        libyuv::kFilterBox
                );

                // Convert the scaled ARGB32 back to RGB24 for encoding
                libyuv::ARGBToRGBA(
                        encode_argb_buffer.data(), target_width * 4,
                        encode_rgba_buffer.data(), target_width * 4,
                        target_width, target_height
                );

                // Override the image data pointer to the rescaled data
                config.pImageData = encode_rgba_buffer.data();
            }

            if (is_timeout()) {
                return nullptr;
            }

            if (cgif_rgb_addframe(encoder.get(), &config) != CGIF_OK) {
                throw std::runtime_error("Failed to encode GIF frame");
            }
        }

        // Close the encoder to finalize the GIF
        encoder.reset();

        jni_utils::JavaByteArrayRef output_array(env, env->NewByteArray(output_buffer.size()));
        std::memcpy(output_array.bytes(), output_buffer.data(), output_buffer.size());
        return output_array.java_array();
    });
}