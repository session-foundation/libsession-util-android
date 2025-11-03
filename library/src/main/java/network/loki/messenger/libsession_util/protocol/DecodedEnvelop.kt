package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.util.Bytes
import java.time.Instant

data class DecodedEnvelop(
    val proStatus: ProStatus,
    val contentPlainText: Bytes,
    val senderEd25519PubKey: Bytes,
    val senderX25519PubKey: Bytes,
    val timestamp: Instant
) {
    @Keep
    constructor(
        proStatus: ProStatus,
        contentPlainText: ByteArray,
        senderEd25519PubKey: ByteArray,
        senderX25519PubKey: ByteArray,
        timestampEpochMills: Long
    ): this(
        proStatus = proStatus,
        contentPlainText = Bytes(contentPlainText),
        senderEd25519PubKey = Bytes(senderEd25519PubKey),
        senderX25519PubKey = Bytes(senderX25519PubKey),
        timestamp = Instant.ofEpochMilli(timestampEpochMills)
    )
}
