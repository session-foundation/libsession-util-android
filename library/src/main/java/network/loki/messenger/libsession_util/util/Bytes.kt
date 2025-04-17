package network.loki.messenger.libsession_util.util

/**
 * A wrapper class for ByteArray to provide the content equals and hashCode implementation,
 * so that we can safely use the byte array in data classes.
 */
class Bytes(val data: ByteArray) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is Bytes) return false

        if (!data.contentEquals(other.data)) return false

        return true
    }

    override fun hashCode(): Int {
        return data.contentHashCode()
    }

    companion object {
        fun ByteArray.toBytes(): Bytes {
            return Bytes(this)
        }
    }
}