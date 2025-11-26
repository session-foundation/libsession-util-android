package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.util.Bytes

data class DecodedCommunityMessage(
    val decodedPro: DecodedPro?,
    val contentPlainText: Bytes,
) {
    @Keep
    constructor(
        decodedPro: DecodedPro?,
        contentPlainText: ByteArray,
    ): this(
        decodedPro = decodedPro,
        contentPlainText = Bytes(contentPlainText),
    )
}