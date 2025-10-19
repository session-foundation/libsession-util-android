package network.loki.messenger.libsession_util.image

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object GifUtils : LibSessionUtilCApi() {
    external fun reencodeGif(
        input: ByteArray,
        timeoutMills: Long,
        targetWidth: Int,
        targetHeight: Int
    ): ByteArray
}