package network.loki.messenger.libsession_util.util

data class UserPic(val url: String, val key: Bytes) {
    constructor(url: String, key: ByteArray)
        :this(url, Bytes(key))

    companion object {
        val DEFAULT = UserPic("", Bytes(byteArrayOf()))
    }

    // Convenience method to get the key as a ByteArray from native side
    val keyAsByteArray: ByteArray
        get() = key.data
}