package network.loki.messenger.libsession_util.image

import network.loki.messenger.libsession_util.LibSessionUtilCApi
import java.io.InputStream

object WebPUtils : LibSessionUtilCApi() {
    /**
     * Re-encode the webP animation, resizing each frame to scale to the target width and height.
     * This can serve two purposes:
     * 1. Getting rid of any extra metadata that might be present in the original file.
     * 2. Reducing the dimensions of the animation to fit within specified bounds.
     *
     * If you only want to remove metadata, you can give the original size as targetWidth and targetHeight,
     * this function will then not try to resize the image.
     *
     * @throws java.util.concurrent.TimeoutException if the operation takes longer than timeoutMills milliseconds.
     */
    external fun reencodeWebPAnimation(
        input: ByteArray,
        timeoutMills: Long,
        targetWidth: Int,
        targetHeight: Int,
    ): ByteArray

    external fun encodeGifToWebP(
        input: InputStream,
        timeoutMills: Long,
        targetWidth: Int,
        targetHeight: Int,
    ): ByteArray
}