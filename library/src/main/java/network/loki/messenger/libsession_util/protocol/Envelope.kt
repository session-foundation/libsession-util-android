package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.util.Bytes
import java.time.Instant

data class Envelope(
    val timestamp: Instant,
    val source: Bytes?, // 33 bytes prefixed public key
    val serverTimestamp: Instant?,
    val proSignature: Bytes, // 64 bytes
) {
    @Keep
    constructor(
        timestampMs: Long,
        source: ByteArray?,
        serverTimestampMs: Long,
        proSignature: ByteArray
    ): this(
        timestamp = Instant.ofEpochMilli(timestampMs),
        source = source?.let { Bytes(it) },
        serverTimestamp = if (serverTimestampMs != 0L) Instant.ofEpochMilli(serverTimestampMs) else null,
        proSignature = Bytes(proSignature)
    )
}