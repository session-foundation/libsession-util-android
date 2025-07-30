package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.GroupMember

class GroupMembersConfig private constructor(pointer: Long): ConfigBase(pointer), MutableGroupMembersConfig {
    companion object {
        private external fun newInstance(
            pubKey: ByteArray,
            secretKey: ByteArray?,
            initialDump: ByteArray?
        ): Long
    }

    constructor(groupPubKey: ByteArray, groupAdminKey: ByteArray?, initialDump: ByteArray?)
            : this(newInstance(groupPubKey, groupAdminKey, initialDump))

    override fun namespace() = Namespace.GROUP_MEMBERS()

    external override fun all(): List<GroupMember>
    external override fun erase(pubKeyHex: String): Boolean
    external override fun get(pubKeyHex: String): GroupMember?
    external override fun getOrConstruct(pubKeyHex: String): GroupMember
    external override fun set(groupMember: GroupMember)
    external override fun setPendingSend(pubKeyHex: String, pending: Boolean)

    private external fun statusInt(groupMember: GroupMember): Int
    override fun status(groupMember: GroupMember): GroupMember.Status {
        val statusInt = statusInt(groupMember)
        return GroupMember.Status.entries.first { it.nativeValue == statusInt }
    }
}