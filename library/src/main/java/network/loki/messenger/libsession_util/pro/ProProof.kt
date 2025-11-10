package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.LibSessionUtilCApi
import java.time.Instant

class ProProof @Keep private constructor(private val nativeValue: Long) {

    val version: Int get() = nativeGetVersion(nativeValue)
    val expiry: Instant get() = Instant.ofEpochSecond(nativeGetExpiry(nativeValue))
    val rotatingPubKey: ByteArray get() = nativeGetRotatingPubKey(nativeValue)


    /**
     * Serialize the [ProProof] to a byte array for storage or transmission
     */
    fun serialize(): String = nativeSerialize(nativeValue)


    protected fun finalize() {
        nativeDestroy(nativeValue)
    }


    companion object : LibSessionUtilCApi() {
        private external fun nativeGetVersion(nativeValue: Long): Int
        private external fun nativeGetExpiry(nativeValue: Long): Long
        private external fun nativeGetRotatingPubKey(nativeValue: Long): ByteArray
        private external fun nativeSerialize(nativeValue: Long): String
        private external fun nativeDeserialize(data: String): Long
        private external fun nativeDestroy(nativeValue: Long)

        /**
         * Deserialize a [ProProof] from a byte array
         */
        fun deserialize(data: String): ProProof {
            return ProProof(nativeDeserialize(data))
        }
    }
}