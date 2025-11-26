package network.loki.messenger.libsession_util.protocol

import network.loki.messenger.libsession_util.util.BitSet
import network.loki.messenger.libsession_util.util.BitSetEntry

sealed interface ProFeature : BitSetEntry

enum class ProMessageFeature(override val bitIndex: Int): ProFeature {
    HIGHER_CHARACTER_LIMIT(0)
}

enum class ProProfileFeature(override val bitIndex: Int): ProFeature {
    PRO_BADGE(0),
    ANIMATED_AVATAR(1),
}

typealias ProMessageFeatures = BitSet<ProMessageFeature>
typealias ProProfileFeatures = BitSet<ProProfileFeature>

