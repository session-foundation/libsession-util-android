package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.Conversation

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
    external override fun setOneToOne(o: Conversation.OneToOne)

    external override fun getCommunity(baseUrl: String, room: String): Conversation.Community?
    external override fun getOrConstructCommunity(baseUrl: String, room: String, pubKeyHex: String): Conversation.Community
    external override fun getOrConstructCommunity(baseUrl: String, room: String, pubKey: ByteArray): Conversation.Community
    external override fun eraseCommunity(community: Conversation.Community): Boolean
    external override fun eraseCommunity(baseUrl: String, room: String): Boolean
    external override fun setCommunity(o: Conversation.Community)

    external override fun getLegacyClosedGroup(groupId: String): Conversation.LegacyGroup?
    external override fun getOrConstructLegacyGroup(groupId: String): Conversation.LegacyGroup
    external override fun eraseLegacyClosedGroup(groupId: String): Boolean
    external override fun setLegacyGroup(o: Conversation.LegacyGroup)

    external override fun getClosedGroup(sessionId: String): Conversation.ClosedGroup?
    external override fun getOrConstructClosedGroup(sessionId: String): Conversation.ClosedGroup
    external override fun eraseClosedGroup(sessionId: String): Boolean
    external override fun setClosedGroup(o: Conversation.ClosedGroup)

    external override fun setBlindedOneToOne(o: Conversation.BlindedOneToOne)
    external override fun getOrConstructedBlindedOneToOne(blindedId: String): Conversation.BlindedOneToOne
    external override fun eraseBlindedOneToOne(blindedId: String): Boolean

    override fun set(conv: Conversation) {
        when (conv) {
            is Conversation.BlindedOneToOne -> setBlindedOneToOne(conv)
            is Conversation.ClosedGroup -> setClosedGroup(conv)
            is Conversation.Community -> setCommunity(conv)
            is Conversation.LegacyGroup -> setLegacyGroup(conv)
            is Conversation.OneToOne -> setOneToOne(conv)
        }
    }

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