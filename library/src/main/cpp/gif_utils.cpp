#include <jni.h>
#include <memory>
#include <chrono>

#include "jni_utils.h"
#include "jni_input_stream.h"

#include <cgif.h>
#include <libyuv.h>
#include <android/log.h>
#include <gif_lib.h>

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
        jni_utils::JavaByteArrayRef input_ref(env, input);

        EasyGifReader decoder = EasyGifReader::openMemory(input_ref.bytes(), input_ref.size());

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


        std::vector<uint8_t> decode_argb_buffer, encode_argb_buffer, encode_rgba_buffer;
        if (needRescale) {
            decode_argb_buffer.resize(src_width * src_height * 4);
            encode_argb_buffer.resize(target_width * target_height * 4);
            encode_rgba_buffer.resize(target_width * target_height * 4);
        }

        for (auto frame = decoder.begin(); frame != decoder.end(); ++frame) {
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

                // Convert the scaled ARGB32 back to RGBA for encoding
                libyuv::ARGBToRGBA(
                        encode_argb_buffer.data(), target_width * 4,
                        encode_rgba_buffer.data(), target_width * 4,
                        target_width, target_height
                );

                // Override the image data pointer to the rescaled data
                config.pImageData = encode_rgba_buffer.data();
            }

            if (cgif_rgb_addframe(encoder.get(), &config) != CGIF_OK) {
                throw std::runtime_error("Failed to encode GIF frame");
            }

            if (std::chrono::high_resolution_clock::now() > deadline) {
                env->ThrowNew(env->FindClass("java/util/concurrent/TimeoutException"),
                              "GIF re-encoding timed out");
                return nullptr;
            }
        }

        // Close the encoder to finalize the GIF
        encoder.reset();

        jni_utils::JavaByteArrayRef output_array(env, env->NewByteArray(output_buffer.size()));
        std::memcpy(output_array.bytes(), output_buffer.data(), output_buffer.size());
        return output_array.java_array();
    });
}

static bool isAnimatedGif(GifFileType *gif_file) {
    GifRecordType record_type;
    std::vector<GifByteType> line_buf;
    int image_count = 0;

    while (DGifGetRecordType(gif_file, &record_type) == GIF_OK) {
        switch (record_type) {
            case IMAGE_DESC_RECORD_TYPE: {
                if (DGifGetImageDesc(gif_file) != GIF_OK) {
                    throw std::runtime_error("Failed to read GIF image descriptor");
                }

                image_count++;

                if (image_count > 1) {
                    return true;
                }

                line_buf.resize(gif_file->Image.Width);
                for (int i = 0; i < gif_file->Image.Height; ++i) {
                    if (DGifGetLine(gif_file, line_buf.data(), gif_file->Image.Width) != GIF_OK) {
                        throw std::runtime_error("Failed to read GIF image line");
                    }
                }
                break;
            }

            case SCREEN_DESC_RECORD_TYPE: {
                if (DGifGetScreenDesc(gif_file) != GIF_OK) {
                    throw std::runtime_error("Failed to read GIF screen descriptor");
                }

                break;
            }

            case TERMINATE_RECORD_TYPE: return false;

            case EXTENSION_RECORD_TYPE: {
                GifByteType *e = nullptr;
                int ext_code;
                if (DGifGetExtension(gif_file, &ext_code, &e) != GIF_OK) {
                    throw std::runtime_error("Failed to read GIF extension");
                }

                while (e) {
                    DGifGetExtensionNext(gif_file, &e);
                }

                break;
            }

            default: break;
        }
    }

    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_image_GifUtils_isAnimatedGifForStream(JNIEnv *env, jobject thiz,
                                                                                   jobject input) {
    return jni_utils::run_catching_cxx_exception_or_throws<jboolean>(env, [=]() {
        JniInputStream input_stream(env, input);

        int error = D_GIF_SUCCEEDED;
        GifPtr<GifFileType> gif_file(DGifOpen(
                    &input_stream,
                    [](GifFileType *f, GifByteType *out_data, int size) -> int {
                        return reinterpret_cast<JniInputStream *>(f->UserData)->read_fully(out_data, size);
                    },
                    &error
                ),
                [](GifFileType *ptr) { DGifCloseFile(ptr, nullptr); }
        );

        if (!gif_file) {
            throw std::runtime_error(("Failed to open GIF for reading: " + std::to_string(error)).c_str());
        }

        return isAnimatedGif(gif_file.get());
    });
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_network_loki_messenger_libsession_1util_image_GifUtils_isAnimatedGifForBytes(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jbyteArray input) {
    return jni_utils::run_catching_cxx_exception_or_throws<jboolean>(env, [=]() {
        jni_utils::JavaByteArrayRef input_ref(env, input);
        auto input_data = input_ref.get();

        int error = D_GIF_SUCCEEDED;
        GifPtr<GifFileType> gif_file(DGifOpen(
                                             &input_data,
                                             [](GifFileType *f, GifByteType *out_data, int size) -> int {
                                                 auto input = reinterpret_cast<std::span<unsigned char> *>(f->UserData);
                                                 auto copy_count = std::min<size_t>(input->size(), static_cast<size_t>(size));
                                                 std::memcpy(out_data, input->data(), copy_count);
                                                 *input = input->subspan(copy_count);
                                                 return copy_count;
                                             },
                                             &error
                                     ),
                                     [](GifFileType *ptr) { DGifCloseFile(ptr, nullptr); }
        );

        if (!gif_file) {
            throw std::runtime_error(("Failed to open GIF for reading: " + std::to_string(error)).c_str());
        }

        return isAnimatedGif(gif_file.get());
    });

}