package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep
import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable
import java.time.Instant

/**
 * Represents a proof of Pro. This class is marked as @Serializable to represent the JSON structure
 * received from the Pro Backend.
 */
@Serializable
data class ProProof(
    val version: Int,

    @SerialName("gen_index_hash")
    val genIndexHashHex: String,

    @SerialName("rotating_pkey")
    val rotatingPubKeyHex: String,

    @SerialName("expiry_unix_ts_ms")
    val expiryMs: Long,

    @SerialName("sig")
    val signatureHex: String
) {
    @Keep
    constructor(
        version: Int,
        genIndexHash: ByteArray,
        rotatingPubKey: ByteArray,
        expiryMs: Long,
        signature: ByteArray
    ): this(
        version = version,
        genIndexHashHex = genIndexHash.toHexString(),
        rotatingPubKeyHex = rotatingPubKey.toHexString(),
        expiryMs = expiryMs,
        signatureHex = signature.toHexString()
    )


    init {
        check(rotatingPubKeyHex.length == 64) {
            "Rotating public key must be 32 bytes"
        }

        check(signatureHex.length == 128) {
            "Signature must be 64 bytes"
        }
    }

    enum class Status(internal val nativeValue: Int) {
        InvalidProBackendSignature(1),
        InvalidUserSignature(2),
        Valid(3),
        Expired(4),

        ;
        companion object {
            internal fun fromNativeValue(value: Int): Status {
                return entries.first { it.nativeValue == value }
            }

            internal fun fromNativeValueOrNull(value: Int): Status? {
                return entries.firstOrNull { it.nativeValue == value }
            }
        }
    }

    class ProSignedMessage(
        val data: ByteArray,
        val signature: ByteArray,
    )

    fun status(
        verifyPubKey: ByteArray,
        signedMessage: ProSignedMessage?,
        now: Instant = Instant.now(),
    ): Status {
        val signedMessageData = signedMessage?.data
        val signedMessageSignature = signedMessage?.signature
        val statusValue = nativeStatus(
            version = version,
            genIndexHash = genIndexHashHex.hexToByteArray(),
            rotatingPubKey = rotatingPubKeyHex.hexToByteArray(),
            expiryMs = expiryMs,
            signature = signatureHex.hexToByteArray(),
            nowUnixTs = now.toEpochMilli(),
            verifyPubKey = verifyPubKey,
            signedMessageData = signedMessageData,
            signedMessageSignature = signedMessageSignature
        )

        return Status.fromNativeValue(statusValue)
    }

    companion object {
        private external fun nativeStatus(
            version: Int,
            genIndexHash: ByteArray,
            rotatingPubKey: ByteArray,
            expiryMs: Long,
            signature: ByteArray,
            nowUnixTs: Long,
            verifyPubKey: ByteArray,
            signedMessageData: ByteArray?,
            signedMessageSignature: ByteArray?
        ): Int
    }
}