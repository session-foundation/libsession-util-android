package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep
import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable
import java.time.Instant

typealias ProProofStatus = Int

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

    class ProSignedMessage(
        val data: ByteArray,
        val signature: ByteArray,
    )

    /**
     * Checks the status of the Pro proof.
     *
     * @param senderED25519PubKey The sender (proof generator)'s ED25519 public key.
     * @param signedMessage An optional signed message to verify against the proof.
     * @param now The current time to use for expiry checks. Defaults to
     */
    fun status(
        senderED25519PubKey: ByteArray,
        now: Instant,
        signedMessage: ProSignedMessage? = null,
    ): ProProofStatus {
        val signedMessageData = signedMessage?.data
        val signedMessageSignature = signedMessage?.signature
        return nativeStatus(
            nowUnixTs = now.toEpochMilli(),
            verifyPubKey = senderED25519PubKey,
            signedMessageData = signedMessageData,
            signedMessageSignature = signedMessageSignature
        )
    }

    private external fun nativeStatus(
        nowUnixTs: Long,
        verifyPubKey: ByteArray,
        signedMessageData: ByteArray?,
        signedMessageSignature: ByteArray?
    ): Int

    companion object {
        const val STATUS_INVALID_PRO_BACKEND_SIGNATURE: ProProofStatus = 1
        const val STATUS_INVALID_USER_SIGNATURE: ProProofStatus = 2
        const val STATUS_VALID: ProProofStatus = 3
        const val STATUS_EXPIRED: ProProofStatus = 4
    }
}