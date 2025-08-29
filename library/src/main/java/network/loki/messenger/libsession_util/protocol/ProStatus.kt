package network.loki.messenger.libsession_util.protocol

import java.time.Instant


sealed interface ProStatus {
    data object None : ProStatus
    data object Invalid : ProStatus

    data class Valid(
        val expiresAt: Instant,
        val proFeatures: Set<ProFeature>
    ) {
        constructor(
            expiresAtEpochSeconds: Long,
            proFeatures: Long
        ): this(
            expiresAt = Instant.ofEpochSecond(expiresAtEpochSeconds),
            proFeatures = proFeatures.toFeatures()
        )
    }
}