package network.loki.messenger.libsession_util.util

import kotlinx.serialization.Serializable

interface BitSetEntry {
    val bitIndex: Int
}


@Serializable
@JvmInline
value class BitSet<in T>(val rawValue: Long = 0L) where T: BitSetEntry {
    val isEmpty: Boolean
        get() = rawValue == 0L

    fun contains(feature: T): Boolean {
        return (rawValue and (1L shl feature.bitIndex)) != 0L
    }
}

fun <T> Iterable<T>.toBitSet(): BitSet<T> where T: BitSetEntry {
    return BitSet(this.fold(0L) { acc, entry ->
        acc or (1L shl entry.bitIndex)
    })
}

inline fun <reified T> BitSet<T>.asSequence(): Sequence<T> where T: BitSetEntry, T: Enum<T> {
    return enumValues<T>()
        .asSequence()
        .filter { contains(it) }
}

