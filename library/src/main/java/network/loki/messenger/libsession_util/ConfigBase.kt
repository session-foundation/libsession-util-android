package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.ConfigPush

sealed class ConfigBase(pointer: Long): Config(pointer), MutableConfig {
    external override fun dirty(): Boolean
    external override fun needsPush(): Boolean
    external override fun needsDump(): Boolean
    external override fun push(): ConfigPush
    external override fun dump(): ByteArray
    external override fun encryptionDomain(): String
    external override fun confirmPushed(seqNo: Long, hashes: Array<String>)
    external fun merge(toMerge: Array<Pair<String, ByteArray>>): List<String>
    external override fun activeHashes(): List<String>
}