package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.util.Bytes
import java.time.Instant

data class DecodedEnvelope(
    val envelope: Envelope,
    val decodedPro: DecodedPro?,
    val contentPlainText: Bytes,
    val senderEd25519PubKey: Bytes,
    val senderX25519PubKey: Bytes,
    val timestamp: Instant
) {
    @Keep
    constructor(
        envelope: Envelope,
        decodedPro: DecodedPro?,
        contentPlainText: ByteArray,
        senderEd25519PubKey: ByteArray,
        senderX25519PubKey: ByteArray,
        timestampEpochMills: Long
    ): this(
        envelope = envelope,
        contentPlainText = Bytes(contentPlainText),
        senderEd25519PubKey = Bytes(senderEd25519PubKey),
        senderX25519PubKey = Bytes(senderX25519PubKey),
        timestamp = Instant.ofEpochMilli(timestampEpochMills),
        decodedPro = decodedPro
    )
}
