#include <jni.h>
#include <memory>
#include <chrono>

#include "jni_utils.h"

#include <cgif.h>
#include <gif.h>
#include <libyuv.h>

template<typename T>
using GifPtr = std::unique_ptr<T, void (*)(T *)>;

void destroyGifContext(GIF_Context *ctx) {
    gif_close(ctx);
    delete ctx;
}

static void throwIfGifError(
        int result,
        const char *message) {
    if (result != GIF_SUCCESS) {
        throw std::runtime_error(std::string(message) + ": " + gif_get_error_string(result));
    }
}


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_network_loki_messenger_libsession_1util_image_GifUtils_reencodeGif(JNIEnv *env, jobject thiz,
                                                                        jbyteArray input,
                                                                        jlong timeout_mills,
                                                                        jint target_width,
                                                                        jint target_height) {
    return jni_utils::run_catching_cxx_exception_or_throws<jbyteArray>(env, [=]() -> jbyteArray {
        jni_utils::JavaByteArrayRef input_array(env, input);

        std::vector<uint8_t> scratch_buffer(GIF_SCRATCH_BUFFER_REQUIRED_SIZE);
        GifPtr<GIF_Context> decoder(new GIF_Context(), &destroyGifContext);

        throwIfGifError(
                gif_init(decoder.get(), input_array.bytes(), input_array.size(),
                         scratch_buffer.data(), scratch_buffer.size()),
                "Failed to decode GIF"
        );

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
                .numLoops = static_cast<uint16_t>(decoder->loop_count),
                .width = static_cast<uint16_t>(target_width),
                .height = static_cast<uint16_t>(target_height),
        };

        GifPtr<CGIFrgb> encoder(cgif_rgb_newgif(&encode_config), [](CGIFrgb* ptr) { cgif_rgb_close(ptr); });
        if (!encoder) {
            throw std::runtime_error("Failed to create GIF encoder");
        }


        int src_width, src_height;
        throwIfGifError(
                gif_get_info(decoder.get(), &src_width, &src_height),
                "Failed to get GIF info"
        );

        const auto needRescale = (src_width != target_width) || (src_height != target_height);
        std::vector<uint8_t> decode_buffer(src_width * src_height * 3); // RGB888
        const auto deadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout_mills);
        int delay_ms;

        auto is_timeout = [&]() {
            if (std::chrono::high_resolution_clock::now() > deadline) {
                env->ThrowNew(env->FindClass("java/util/concurrent/TimeoutException"),
                              "GIF re-encoding timed out");
                return true;
            }
            return false;
        };

        std::vector<uint8_t> decode_argb_buffer, encode_argb_buffer, encode_rgb_buffer;
        if (needRescale) {
            decode_argb_buffer.resize(src_width * src_height * 4);
            encode_argb_buffer.resize(target_width * target_height * 4);
            encode_rgb_buffer.resize(target_width * target_height * 3);
        }

        while (!is_timeout()) {
            auto r = gif_next_frame(decoder.get(), decode_buffer.data(), &delay_ms);
            if (r == 0) {
                break; // Animation finished
            } else if (r < 0) {
                throw std::runtime_error("Failed to decode GIF frame: " + std::string(gif_get_error_string(r)));
            }

            // Here we would add the frame to the encoder
            CGIFrgb_FrameConfig config = {
                    .pImageData = decode_buffer.data(),
                    .fmtChan = CGIF_CHAN_FMT_RGB,
                    .attrFlags = 0,
                    .genFlags = 0,
                    .delay = static_cast<uint16_t>(delay_ms / 10), // in 0.01s
            };

            if (needRescale) {
                // First we need to convert our frame data from RGB24 to RGBA32
                libyuv::RGB24ToARGB(
                        decode_buffer.data(), src_width * 3,
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
                libyuv::ARGBToRGB24(
                        encode_argb_buffer.data(), target_width * 4,
                        encode_rgb_buffer.data(), target_width * 3,
                        target_width, target_height
                );

                // Override the image data pointer to the rescaled data
                config.pImageData = encode_rgb_buffer.data();
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