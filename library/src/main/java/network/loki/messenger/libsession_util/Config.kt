package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.BaseCommunityInfo
import network.loki.messenger.libsession_util.util.ConfigPush
import network.loki.messenger.libsession_util.util.Contact
import network.loki.messenger.libsession_util.util.Conversation
import network.loki.messenger.libsession_util.util.ExpiryMode
import network.loki.messenger.libsession_util.util.GroupInfo
import network.loki.messenger.libsession_util.util.GroupMember
import network.loki.messenger.libsession_util.util.UserPic
import java.io.Closeable

sealed class Config(initialPointer: Long): Closeable, LibSessionUtilCApi() {
    var pointer = initialPointer
        private set

    init {
        check(pointer != 0L) { "Pointer is null" }
    }

    abstract fun namespace(): Int

    private external fun free()

    final override fun close() {
        if (pointer != 0L) {
            free()
            pointer = 0L
        }
    }
}

interface ReadableConfig {
    fun namespace(): Int
    fun needsPush(): Boolean
    fun needsDump(): Boolean
    fun activeHashes(): List<String>
}

interface MutableConfig : ReadableConfig {
    fun push(): ConfigPush
    fun dump(): ByteArray
    fun encryptionDomain(): String
    fun confirmPushed(seqNo: Long, hashes: Array<String>)
    fun dirty(): Boolean
}

sealed class ConfigBase(pointer: Long): Config(pointer), MutableConfig {
    companion object {
        init {
            System.loadLibrary("session_util")
        }
        external fun kindFor(configNamespace: Int): Class<ConfigBase>

        const val PRIORITY_HIDDEN = -1L
        const val PRIORITY_VISIBLE = 0L
        const val PRIORITY_PINNED = 1L

    }

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


interface ReadableContacts: ReadableConfig {
    fun get(accountId: String): Contact?
    fun all(): List<Contact>
}

interface MutableContacts : ReadableContacts, MutableConfig {
    fun getOrConstruct(accountId: String): Contact
    fun set(contact: Contact)
    fun erase(accountId: String): Boolean
}

class Contacts private constructor(pointer: Long) : ConfigBase(pointer), MutableContacts {
    constructor(ed25519SecretKey: ByteArray, initialDump: ByteArray? = null) : this(
        createConfigObject(
            "Contacts",
            ed25519SecretKey,
            initialDump
        )
    )

    override fun namespace() = Namespace.CONTACTS()

    external override fun get(accountId: String): Contact?
    external override fun getOrConstruct(accountId: String): Contact
    external override fun all(): List<Contact>
    external override fun set(contact: Contact)
    external override fun erase(accountId: String): Boolean
}

interface ReadableUserProfile: ReadableConfig {
    fun getName(): String?
    fun getPic(): UserPic
    fun getNtsPriority(): Long
    fun getNtsExpiry(): ExpiryMode
    fun getCommunityMessageRequests(): Boolean
    fun isBlockCommunityMessageRequestsSet(): Boolean
}

interface MutableUserProfile : ReadableUserProfile, MutableConfig {
    fun setName(newName: String)
    fun setPic(userPic: UserPic)
    fun setNtsPriority(priority: Long)
    fun setNtsExpiry(expiryMode: ExpiryMode)
    fun setCommunityMessageRequests(blocks: Boolean)
}

class UserProfile private constructor(pointer: Long) : ConfigBase(pointer), MutableUserProfile {
    constructor(ed25519SecretKey: ByteArray, initialDump: ByteArray? = null) : this(
        createConfigObject(
            "UserProfile",
            ed25519SecretKey,
            initialDump
        )
    )

    override fun namespace() = Namespace.USER_PROFILE()

    external override fun setName(newName: String)
    external override fun getName(): String?
    external override fun getPic(): UserPic
    external override fun setPic(userPic: UserPic)
    external override fun setNtsPriority(priority: Long)
    external override fun getNtsPriority(): Long
    external override fun setNtsExpiry(expiryMode: ExpiryMode)
    external override fun getNtsExpiry(): ExpiryMode
    external override fun getCommunityMessageRequests(): Boolean
    external override fun setCommunityMessageRequests(blocks: Boolean)
    external override fun isBlockCommunityMessageRequestsSet(): Boolean
}

interface ReadableConversationVolatileConfig: ReadableConfig {
    fun getOneToOne(pubKeyHex: String): Conversation.OneToOne?
    fun getCommunity(baseUrl: String, room: String): Conversation.Community?
    fun getLegacyClosedGroup(groupId: String): Conversation.LegacyGroup?
    fun getClosedGroup(sessionId: String): Conversation.ClosedGroup?
    fun sizeOneToOnes(): Int
    fun sizeCommunities(): Int
    fun sizeLegacyClosedGroups(): Int
    fun size(): Int

    fun empty(): Boolean

    fun allOneToOnes(): List<Conversation.OneToOne>
    fun allCommunities(): List<Conversation.Community>
    fun allLegacyClosedGroups(): List<Conversation.LegacyGroup>
    fun allClosedGroups(): List<Conversation.ClosedGroup>
    fun all(): List<Conversation?>
}

interface MutableConversationVolatileConfig : ReadableConversationVolatileConfig, MutableConfig {
    fun getOrConstructOneToOne(pubKeyHex: String): Conversation.OneToOne
    fun eraseOneToOne(pubKeyHex: String): Boolean

    fun getOrConstructCommunity(baseUrl: String, room: String, pubKeyHex: String): Conversation.Community
    fun getOrConstructCommunity(baseUrl: String, room: String, pubKey: ByteArray): Conversation.Community
    fun eraseCommunity(community: Conversation.Community): Boolean
    fun eraseCommunity(baseUrl: String, room: String): Boolean

    fun getOrConstructLegacyGroup(groupId: String): Conversation.LegacyGroup
    fun eraseLegacyClosedGroup(groupId: String): Boolean

    fun getOrConstructClosedGroup(sessionId: String): Conversation.ClosedGroup
    fun eraseClosedGroup(sessionId: String): Boolean

    fun erase(conversation: Conversation): Boolean
    fun set(toStore: Conversation)

    fun eraseAll(predicate: (Conversation) -> Boolean): Int
}


class ConversationVolatileConfig private constructor(pointer: Long): ConfigBase(pointer), MutableConversationVolatileConfig {
    constructor(ed25519SecretKey: ByteArray, initialDump: ByteArray? = null) : this(
        createConfigObject(
            "ConvoInfoVolatile",
            ed25519SecretKey,
            initialDump
        )
    )

    override fun namespace() = Namespace.CONVO_INFO_VOLATILE()

    external override fun getOneToOne(pubKeyHex: String): Conversation.OneToOne?
    external override fun getOrConstructOneToOne(pubKeyHex: String): Conversation.OneToOne
    external override fun eraseOneToOne(pubKeyHex: String): Boolean

    external override fun getCommunity(baseUrl: String, room: String): Conversation.Community?
    external override fun getOrConstructCommunity(baseUrl: String, room: String, pubKeyHex: String): Conversation.Community
    external override fun getOrConstructCommunity(baseUrl: String, room: String, pubKey: ByteArray): Conversation.Community
    external override fun eraseCommunity(community: Conversation.Community): Boolean
    external override fun eraseCommunity(baseUrl: String, room: String): Boolean

    external override fun getLegacyClosedGroup(groupId: String): Conversation.LegacyGroup?
    external override fun getOrConstructLegacyGroup(groupId: String): Conversation.LegacyGroup
    external override fun eraseLegacyClosedGroup(groupId: String): Boolean

    external override fun getClosedGroup(sessionId: String): Conversation.ClosedGroup?
    external override fun getOrConstructClosedGroup(sessionId: String): Conversation.ClosedGroup
    external override fun eraseClosedGroup(sessionId: String): Boolean

    external override fun erase(conversation: Conversation): Boolean
    external override fun set(toStore: Conversation)

    /**
     * Erase all conversations that do not satisfy the `predicate`, similar to [MutableList.removeAll]
     */
    external override fun eraseAll(predicate: (Conversation) -> Boolean): Int

    external override fun sizeOneToOnes(): Int
    external override fun sizeCommunities(): Int
    external override fun sizeLegacyClosedGroups(): Int
    external override fun size(): Int

    external override fun empty(): Boolean

    external override fun allOneToOnes(): List<Conversation.OneToOne>
    external override fun allCommunities(): List<Conversation.Community>
    external override fun allLegacyClosedGroups(): List<Conversation.LegacyGroup>
    external override fun allClosedGroups(): List<Conversation.ClosedGroup>
    external override fun all(): List<Conversation?>
}

interface ReadableUserGroupsConfig : ReadableConfig {
    fun getCommunityInfo(baseUrl: String, room: String): GroupInfo.CommunityGroupInfo?
    fun getLegacyGroupInfo(accountId: String): GroupInfo.LegacyGroupInfo?
    fun getClosedGroup(accountId: String): GroupInfo.ClosedGroupInfo?
    fun sizeCommunityInfo(): Long
    fun sizeLegacyGroupInfo(): Long
    fun sizeClosedGroup(): Long
    fun size(): Long
    fun all(): List<GroupInfo>
    fun allCommunityInfo(): List<GroupInfo.CommunityGroupInfo>
    fun allLegacyGroupInfo(): List<GroupInfo.LegacyGroupInfo>
    fun allClosedGroupInfo(): List<GroupInfo.ClosedGroupInfo>
    fun createGroup(): GroupInfo.ClosedGroupInfo
}

interface MutableUserGroupsConfig : ReadableUserGroupsConfig, MutableConfig {
    fun getOrConstructCommunityInfo(baseUrl: String, room: String, pubKeyHex: String): GroupInfo.CommunityGroupInfo
    fun getOrConstructLegacyGroupInfo(accountId: String): GroupInfo.LegacyGroupInfo
    fun getOrConstructClosedGroup(accountId: String): GroupInfo.ClosedGroupInfo
    fun set(groupInfo: GroupInfo)
    fun erase(groupInfo: GroupInfo)
    fun eraseCommunity(baseCommunityInfo: BaseCommunityInfo): Boolean
    fun eraseCommunity(server: String, room: String): Boolean
    fun eraseLegacyGroup(accountId: String): Boolean
    fun eraseClosedGroup(accountId: String): Boolean
}

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

interface ReadableGroupInfoConfig: ReadableConfig {
    fun id(): String
    fun getDeleteAttachmentsBefore(): Long?
    fun getDeleteBefore(): Long?
    fun getExpiryTimer(): Long
    fun getName(): String?
    fun getCreated(): Long?
    fun getProfilePic(): UserPic
    fun isDestroyed(): Boolean
    fun getDescription(): String
    fun storageNamespace(): Long
}

interface MutableGroupInfoConfig : ReadableGroupInfoConfig, MutableConfig {
    fun setCreated(createdAt: Long)
    fun setDeleteAttachmentsBefore(deleteBefore: Long)
    fun setDeleteBefore(deleteBefore: Long)
    fun setExpiryTimer(expireSeconds: Long)
    fun setName(newName: String)
    fun setDescription(newDescription: String)
    fun setProfilePic(newProfilePic: UserPic)
    fun destroyGroup()
}

class GroupInfoConfig private constructor(pointer: Long): ConfigBase(pointer), MutableGroupInfoConfig {
    constructor(groupPubKey: ByteArray, groupAdminKey: ByteArray?, initialDump: ByteArray?)
            : this(newInstance(groupPubKey, groupAdminKey, initialDump))

    companion object {
        private external fun newInstance(
            pubKey: ByteArray,
            secretKey: ByteArray?,
            initialDump: ByteArray?
        ): Long
    }

    override fun namespace() = Namespace.GROUP_INFO()

    external override fun id(): String
    external override fun destroyGroup()
    external override fun getCreated(): Long?
    external override fun getDeleteAttachmentsBefore(): Long?
    external override fun getDeleteBefore(): Long?
    external override fun getExpiryTimer(): Long
    external override fun getName(): String?
    external override fun getProfilePic(): UserPic
    external override fun isDestroyed(): Boolean
    external override fun setCreated(createdAt: Long)
    external override fun setDeleteAttachmentsBefore(deleteBefore: Long)
    external override fun setDeleteBefore(deleteBefore: Long)
    external override fun setExpiryTimer(expireSeconds: Long)
    external override fun setName(newName: String)
    external override fun getDescription(): String
    external override fun setDescription(newDescription: String)
    external override fun setProfilePic(newProfilePic: UserPic)
    external override fun storageNamespace(): Long
}

interface ReadableGroupMembersConfig: ReadableConfig {
    fun all(): List<GroupMember>

    /**
     * Returns the [GroupMember] for the given [pubKeyHex] or null if it doesn't exist.
     * Note: exception will be thrown if the [pubKeyHex] is invalid. You can opt to use [getOrNull] instead
     */
    fun get(pubKeyHex: String): GroupMember?
    fun status(groupMember: GroupMember): GroupMember.Status
}

fun ReadableGroupMembersConfig.allWithStatus(): Sequence<Pair<GroupMember, GroupMember.Status>> {
    return all().asSequence().map { it to status(it) }
}

/**
 * Returns the [GroupMember] for the given [pubKeyHex] or null if it doesn't exist or is invalid
 */
fun ReadableGroupMembersConfig.getOrNull(pubKeyHex: String): GroupMember? {
    return runCatching {
        get(pubKeyHex)
    }.getOrNull()
}

interface MutableGroupMembersConfig : ReadableGroupMembersConfig, MutableConfig {
    fun getOrConstruct(pubKeyHex: String): GroupMember
    fun set(groupMember: GroupMember)
    fun erase(pubKeyHex: String): Boolean

    fun setPendingSend(pubKeyHex: String, pending: Boolean)
}

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

sealed class ConfigSig(pointer: Long) : Config(pointer)

interface ReadableGroupKeysConfig {
    fun groupKeys(): List<ByteArray>
    fun needsDump(): Boolean
    fun dump(): ByteArray
    fun needsRekey(): Boolean
    fun pendingKey(): ByteArray?
    fun supplementFor(userSessionIds: List<String>): ByteArray
    fun pendingConfig(): ByteArray?
    fun activeHashes(): List<String>
    fun encrypt(plaintext: ByteArray): ByteArray
    fun decrypt(ciphertext: ByteArray): Pair<ByteArray, String>?
    fun keys(): List<ByteArray>
    fun subAccountSign(message: ByteArray, signingValue: ByteArray): GroupKeysConfig.SwarmAuth
    fun getSubAccountToken(sessionId: String, canWrite: Boolean = true, canDelete: Boolean = false): ByteArray
    fun currentGeneration(): Int
    fun size(): Int
}

interface MutableGroupKeysConfig : ReadableGroupKeysConfig {
    fun makeSubAccount(sessionId: String, canWrite: Boolean = true, canDelete: Boolean = false): ByteArray
    fun loadKey(message: ByteArray, hash: String, timestampMs: Long): Boolean
    fun loadAdminKey(adminKey: ByteArray)
}

class GroupKeysConfig private constructor(
    pointer: Long,
    private val info: GroupInfoConfig,
    private val members: GroupMembersConfig
): ConfigSig(pointer), MutableGroupKeysConfig {
    companion object {
        private external fun newInstance(
            userSecretKey: ByteArray,
            groupPublicKey: ByteArray,
            groupSecretKey: ByteArray? = null,
            initialDump: ByteArray?,
            infoPtr: Long,
            members: Long
        ): Long
    }

    constructor(
        userSecretKey: ByteArray,
        groupPublicKey: ByteArray,
        groupAdminKey: ByteArray?,
        initialDump: ByteArray?,
        info: GroupInfoConfig,
        members: GroupMembersConfig
    ) : this(
        newInstance(
            userSecretKey,
            groupPublicKey,
            groupAdminKey,
            initialDump,
            info.pointer,
            members.pointer
        ),
        info,
        members
    )

    override fun namespace() = Namespace.GROUP_KEYS()

    external override fun groupKeys(): List<ByteArray>
    external override fun needsDump(): Boolean
    external override fun dump(): ByteArray
    external fun loadKey(message: ByteArray,
                         hash: String,
                         timestampMs: Long,
                         infoPtr: Long,
                         membersPtr: Long): Boolean

    override fun loadKey(message: ByteArray, hash: String, timestampMs: Long): Boolean {
        return loadKey(message, hash, timestampMs, info.pointer, members.pointer)
    }

    override fun loadAdminKey(adminKey: ByteArray) {
        loadAdminKey(adminKey, info.pointer, members.pointer)
    }

    private external fun loadAdminKey(adminKey: ByteArray, infoPtr: Long, membersPtr: Long)

    external override fun needsRekey(): Boolean
    external override fun pendingKey(): ByteArray?
    private external fun supplementFor(userSessionIds: Array<String>): ByteArray
    override fun supplementFor(userSessionIds: List<String>): ByteArray {
        return supplementFor(userSessionIds.toTypedArray())
    }

    external override fun pendingConfig(): ByteArray?
    external override fun activeHashes(): List<String>
    external fun rekey(infoPtr: Long, membersPtr: Long): ByteArray

    external override fun encrypt(plaintext: ByteArray): ByteArray
    external override fun decrypt(ciphertext: ByteArray): Pair<ByteArray, String>?

    external override fun keys(): List<ByteArray>

    external override fun makeSubAccount(sessionId: String, canWrite: Boolean, canDelete: Boolean): ByteArray
    external override fun getSubAccountToken(sessionId: String, canWrite: Boolean, canDelete: Boolean): ByteArray

    external override fun subAccountSign(message: ByteArray, signingValue: ByteArray): SwarmAuth

    external override fun currentGeneration(): Int
    external fun admin(): Boolean
    external override fun size(): Int

    data class SwarmAuth(
        val subAccount: String,
        val subAccountSig: String,
        val signature: String
    )
}

private external fun createConfigObject(
    configName: String,
    ed25519SecretKey: ByteArray,
    initialDump: ByteArray?
): Long