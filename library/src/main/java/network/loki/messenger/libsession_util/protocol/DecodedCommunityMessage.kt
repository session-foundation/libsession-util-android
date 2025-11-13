package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.pro.ProProof
import network.loki.messenger.libsession_util.util.Bytes
import java.util.EnumSet

data class DecodedCommunityMessage(
    val proStatus: ProProof.Status?,
    val proProof: ProProof?,
    val proFeatures: ProFeatures,
    val contentPlainText: Bytes,
) {
    @Keep
    constructor(
        status: Int,
        proProof: ProProof?,
        proFeatures: Long,
        contentPlainText: ByteArray,
    ): this(
        proStatus = ProProof.Status.fromNativeValueOrNull(status),
        proProof = proProof,
        proFeatures = ProFeatures(proFeatures),
        contentPlainText = Bytes(contentPlainText),
    )

    init {
        check(proProof == null || proStatus in EnumSet.of(ProProof.Status.Expired, ProProof.Status.Valid)) {
            "proProof must be null unless proStatus is Expired or Valid"
        }
    }
}
