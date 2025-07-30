package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.BaseCommunityInfo
import network.loki.messenger.libsession_util.util.GroupInfo

class UserGroupsConfig private constructor(pointer: Long): ConfigBase(pointer), MutableUserGroupsConfig {
    constructor(ed25519SecretKey: ByteArray, initialDump: ByteArray? = null) : this(
        createConfigObject(
            "UserGroups",
            ed25519SecretKey,
            initialDump
        )
    )

    override fun namespace() = Namespace.USER_GROUPS()

    external override fun getCommunityInfo(baseUrl: String, room: String): GroupInfo.CommunityGroupInfo?
    external override fun getLegacyGroupInfo(accountId: String): GroupInfo.LegacyGroupInfo?
    external override fun getClosedGroup(accountId: String): GroupInfo.ClosedGroupInfo?
    external override fun getOrConstructCommunityInfo(baseUrl: String, room: String, pubKeyHex: String): GroupInfo.CommunityGroupInfo
    external override fun getOrConstructLegacyGroupInfo(accountId: String): GroupInfo.LegacyGroupInfo
    external override fun getOrConstructClosedGroup(accountId: String): GroupInfo.ClosedGroupInfo
    external override fun set(groupInfo: GroupInfo)
    external override fun erase(groupInfo: GroupInfo)
    external override fun eraseCommunity(baseCommunityInfo: BaseCommunityInfo): Boolean
    external override fun eraseCommunity(server: String, room: String): Boolean
    external override fun eraseLegacyGroup(accountId: String): Boolean
    external override fun eraseClosedGroup(accountId: String): Boolean
    external override fun sizeCommunityInfo(): Long
    external override fun sizeLegacyGroupInfo(): Long
    external override fun sizeClosedGroup(): Long
    external override fun size(): Long
    external override fun all(): List<GroupInfo>
    external override fun allCommunityInfo(): List<GroupInfo.CommunityGroupInfo>
    external override fun allLegacyGroupInfo(): List<GroupInfo.LegacyGroupInfo>
    external override fun allClosedGroupInfo(): List<GroupInfo.ClosedGroupInfo>
    external override fun createGroup(): GroupInfo.ClosedGroupInfo
}