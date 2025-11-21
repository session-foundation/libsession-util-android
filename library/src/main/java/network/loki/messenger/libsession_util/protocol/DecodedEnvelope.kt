package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.pro.ProProof
import network.loki.messenger.libsession_util.util.Bytes
import java.time.Instant
import java.util.EnumSet

data class DecodedEnvelope(
    val envelope: Envelope,
    val proStatus: ProProof.Status?,
    val proProof: ProProof?,
    val proFeatures: ProFeatures,
    val contentPlainText: Bytes,
    val senderEd25519PubKey: Bytes,
    val senderX25519PubKey: Bytes,
    val timestamp: Instant
) {
    @Keep
    constructor(
        envelope: Envelope,
        proStatus: Int,
        proProof: ProProof?,
        proFeatures: Long,
        contentPlainText: ByteArray,
        senderEd25519PubKey: ByteArray,
        senderX25519PubKey: ByteArray,
        timestampEpochMills: Long
    ): this(
        envelope = envelope,
        proStatus = ProProof.Status.fromNativeValueOrNull(proStatus),
        proProof = proProof,
        proFeatures = ProFeatures(proFeatures),
        contentPlainText = Bytes(contentPlainText),
        senderEd25519PubKey = Bytes(senderEd25519PubKey),
        senderX25519PubKey = Bytes(senderX25519PubKey),
        timestamp = Instant.ofEpochMilli(timestampEpochMills)
    )

    init {
        check(proProof == null || proStatus in EnumSet.of(ProProof.Status.Expired, ProProof.Status.Valid)) {
            "proProof must be null unless proStatus is Expired or Valid"
        }
    }
}
