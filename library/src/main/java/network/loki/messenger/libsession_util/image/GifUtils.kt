package network.loki.messenger.libsession_util.image

import network.loki.messenger.libsession_util.LibSessionUtilCApi
import java.io.InputStream

object GifUtils : LibSessionUtilCApi() {
    /**
     * Re-encodes an input GIF to a target width and height.
     *
     * @param input The input stream of the GIF to be re-encoded. The caller is responsible for closing the stream.
     * @param timeoutMills The maximum time in milliseconds to allow for the re-encoding process.
     *  If the process exceeds this time, a [java.util.concurrent.TimeoutException] will be thrown.
     * @param targetWidth The desired width of the output GIF.
     * @param targetHeight The desired height of the output GIF.
     * @return A byte array containing the re-encoded GIF data.
     */
    external fun reencodeGif(
        input: InputStream,
        timeoutMills: Long,
        targetWidth: Int,
        targetHeight: Int
    ): ByteArray

    /**
     * Determines if the input stream contains an animated GIF.
     *
     * An animated GIF is defined as a GIF file with more than one frame.
     *
     * @param input The input stream of the GIF to be checked. The caller is responsible for closing the stream.
     */
    external fun isAnimatedGif(
        input: InputStream
    ): Boolean
}