package network.loki.messenger.libsession_util.util

data class ConfigPush(val messages: List<Bytes>, val seqNo: Long, val obsoleteHashes: List<String>)

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

data class KeyPair(val pubKey: Bytes, val secretKey: Bytes) {
    constructor(pubKey: ByteArray, secretKey: ByteArray)
        : this(Bytes(pubKey), Bytes(secretKey))
}
