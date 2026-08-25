package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep
import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable
import java.time.Instant

typealias ProProofStatus = Int

/**
 * Represents a proof of Pro. This class is marked as @Serializable to represent the JSON structure
 * received from the Pro Backend.
 *
 * There is deliberately no `version` field. A proof's format is bound into its signature by the
 * 16-byte domain prefix that format selects (`ProProof_v0_____`), so the version was never part of
 * the signed bytes and carrying it proved nothing. A future format arrives as its own field rather
 * than a version bump, so to a client that does not know it the proof simply does not appear —
 * reported as [STATUS_INVALID]. Protobuf tag 1 held the old field and must never be reused.
 */
@Serializable
data class ProProof(
    @SerialName("revocation_tag")
    val revocationTagHex: String,

    @SerialName("rotating_pkey")
    val rotatingPubKeyHex: String,

    @SerialName("expiry_ts")
    val expirySeconds: Long,

    @SerialName("sig")
    val signatureHex: String
) {
    @Keep
    constructor(
        revocationTag: ByteArray,
        rotatingPubKey: ByteArray,
        expirySeconds: Long,
        signature: ByteArray
    ): this(
        revocationTagHex = revocationTag.toHexString(),
        rotatingPubKeyHex = rotatingPubKey.toHexString(),
        expirySeconds = expirySeconds,
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
            nowUnixTs = now.epochSecond,
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

        /**
         * Nothing could be evaluated, so the decoded proof is not to be trusted: the message
         * carried no proof this client can read — either none was attached, or it is in a format
         * this client does not know. Such a message is delivered as non-Pro rather than dropped.
         */
        const val STATUS_INVALID: ProProofStatus = 5
    }
}