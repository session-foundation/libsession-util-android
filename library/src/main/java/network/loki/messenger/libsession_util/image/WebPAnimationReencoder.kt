package network.loki.messenger.libsession_util.image

object WebPUtils {
    external fun resizeWebPAnimation(
        input: ByteArray,
        targetWidth: Int,
        targetHeight: Int,
    ): ByteArray
}