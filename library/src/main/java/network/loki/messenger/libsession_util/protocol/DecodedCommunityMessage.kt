package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.util.Bytes

data class DecodedCommunityMessage(
    val proStatus: ProStatus,
    val contentPlainText: Bytes,
) {
    @Keep
    constructor(
        proStatus: ProStatus,
        contentPlainText: ByteArray,
    ): this(
        proStatus = proStatus,
        contentPlainText = Bytes(contentPlainText),
    )
}
