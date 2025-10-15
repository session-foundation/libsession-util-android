package network.loki.messenger.libsession_util.image

object WebPUtils {
    /**
     * Re-encode the webP animation, resizing each frame to scale to the target width and height.
     * This can serve two purposes:
     * 1. Getting rid of any extra metadata that might be present in the original file.
     * 2. Reducing the dimensions of the animation to fit within specified bounds.
     *
     * If you only want to remove metadata, you can give the original size as targetWidth and targetHeight,
     * this function will then not try to resize the image.
     */
    external fun reencodeWebPAnimation(
        input: ByteArray,
        targetWidth: Int,
        targetHeight: Int,
    ): ByteArray

    /**
     * Check if the given byte array contains a valid WebP animation.
     */
    external fun isWebPAnimation(input: ByteArray): Boolean

    /**
     * Get the dimensions of the WebP image.
     *
     * @return null if the input is not a valid WebP image. or an IntArray of size 2 with width and height.
     */
    external fun getWebPDimensions(input: ByteArray): IntArray?
}