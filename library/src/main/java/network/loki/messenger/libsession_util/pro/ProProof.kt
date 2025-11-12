package network.loki.messenger.libsession_util.pro

import java.time.Instant

class ProProof(
    val version: Int,
    val genIndexHash: ByteArray,
    val rotatingPubKey: ByteArray,
    val expiryMs: Long,
    val signature: ByteArray
) {
    init {
        check(rotatingPubKey.size == 32) {
            "Rotating public key must be 32 bytes"
        }

        check(signature.size == 64) {
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

    private external fun status(
        verifyPubKey: ByteArray,
        nowUnixTs: Long,
        signedMessageData: ByteArray?,
        signedMessageSignature: ByteArray?
    ): Int

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
        val statusValue = status(
            verifyPubKey = verifyPubKey,
            nowUnixTs = now.epochSecond,
            signedMessageData = signedMessageData,
            signedMessageSignature = signedMessageSignature
        )
        return Status.fromNativeValue(statusValue)
    }
}