package network.loki.messenger.libsession_util.util

data class ConfigPush(val messages: List<Bytes>, val seqNo: Long, val obsoleteHashes: List<String>)