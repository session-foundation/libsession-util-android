package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep
import java.time.Instant


sealed interface ProStatus {
    data object None : ProStatus
    data object Invalid : ProStatus

    data class Valid(
        val expiresAt: Instant,
        val proFeatures: Set<ProFeature>
    ) {
        @Keep
        constructor(
            expiresAtEpochMills: Long,
            proFeatures: Long
        ): this(
            expiresAt = Instant.ofEpochMilli(expiresAtEpochMills),
            proFeatures = proFeatures.toFeatures()
        )
    }
}