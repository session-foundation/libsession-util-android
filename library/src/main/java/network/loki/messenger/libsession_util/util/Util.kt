package network.loki.messenger.libsession_util.util

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object Util : LibSessionUtilCApi() {
    private external fun lengthForCodepoints(str: String, maxCodepoints: Int): Int

    fun truncateCodepoints(str: String, maxCodepoints: Int): String =
        str.take(lengthForCodepoints(str, maxCodepoints))

    external fun countCodepoints(str: String): Int
}