package network.loki.messenger.libsession_util.protocol

import kotlinx.serialization.Serializable


enum class ProFeature(internal val bitIndex: Int) {
    HIGHER_CHARACTER_LIMIT(0),
    PRO_BADGE(1),
    ANIMATED_AVATAR(2),
}

@Serializable
@JvmInline
value class ProFeatures(val rawValue: Long) {
    companion object {
        val NONE = ProFeatures(0L)
    }

    fun contains(feature: ProFeature): Boolean {
        return (rawValue and (1L shl feature.bitIndex)) != 0L
    }

    fun toSet(): Set<ProFeature> {
        return rawValue.toFeatures()
    }
}

internal fun Long.toFeatures(): Set<ProFeature> {
    return buildSet(ProFeature.entries.size) {
        for (entry in ProFeature.entries) {
            if (this@toFeatures and (1L shl entry.bitIndex) != 0L) {
                add(entry)
            }
        }
    }
}

internal fun Collection<ProFeature>.toLong(): Long {
    return fold(0L) { acc, entry ->
        acc or (1L shl entry.bitIndex)
    }
}