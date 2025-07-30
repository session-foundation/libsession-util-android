package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.BaseCommunityInfo
import network.loki.messenger.libsession_util.util.BlindedContact
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


interface ReadableContacts: ReadableConfig {
    fun get(accountId: String): Contact?
    fun all(): List<Contact>

    fun allBlinded(): List<BlindedContact>

    fun getBlinded(blindedId: String): BlindedContact?
}

interface MutableContacts : ReadableContacts, MutableConfig {
    fun getOrConstruct(accountId: String): Contact
    fun set(contact: Contact)
    fun erase(accountId: String): Boolean

    fun getOrConstructBlinded(
        communityServerUrl: String,
        communityServerPubKeyHex: String,
        blindedId: String
    ): BlindedContact
    fun setBlinded(contact: BlindedContact)
    fun eraseBlinded(communityServerUrl: String, blindedId: String)
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
    fun setOneToOne(o: Conversation.OneToOne)

    fun getOrConstructCommunity(baseUrl: String, room: String, pubKeyHex: String): Conversation.Community
    fun getOrConstructCommunity(baseUrl: String, room: String, pubKey: ByteArray): Conversation.Community
    fun eraseCommunity(community: Conversation.Community): Boolean
    fun eraseCommunity(baseUrl: String, room: String): Boolean
    fun setCommunity(o: Conversation.Community)

    fun getOrConstructLegacyGroup(groupId: String): Conversation.LegacyGroup
    fun eraseLegacyClosedGroup(groupId: String): Boolean
    fun setLegacyGroup(o: Conversation.LegacyGroup)

    fun getOrConstructClosedGroup(sessionId: String): Conversation.ClosedGroup
    fun eraseClosedGroup(sessionId: String): Boolean
    fun setClosedGroup(o: Conversation.ClosedGroup)

    fun getOrConstructedBlindedOneToOne(blindedId: String): Conversation.BlindedOneToOne
    fun eraseBlindedOneToOne(blindedId: String): Boolean
    fun setBlindedOneToOne(o: Conversation.BlindedOneToOne)

    fun set(conv: Conversation)
    fun eraseAll(predicate: (Conversation) -> Boolean): Int
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

external fun createConfigObject(
    configName: String,
    ed25519SecretKey: ByteArray,
    initialDump: ByteArray?
): Long