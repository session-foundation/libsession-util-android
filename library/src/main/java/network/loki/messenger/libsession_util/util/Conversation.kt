package network.loki.messenger.libsession_util.util

import androidx.annotation.Keep
import java.time.Instant

sealed interface Conversation {

    var lastRead: Long
    var unread: Boolean

    /**
     * The minimal information about a Pro Proof stored in a Conversation.
     * This is not a full ProProof, it contains just enough for the clients to check it it's revoked.
     */
    data class ProProofInfo(
        val genIndexHash: Bytes,
        val expiry: Instant,
    ) {
        @Keep
        constructor(
            genIndexHash: ByteArray,
            expiryMs: Long,
        ) : this(
            genIndexHash = Bytes(genIndexHash),
            expiry = Instant.ofEpochMilli(expiryMs)
        )
    }

    /**
     * A Conversation that contains Pro information.
     */
    sealed interface WithProProofInfo : Conversation {
        var proProofInfo: ProProofInfo?
    }

    data class OneToOne(
        val accountId: String,
        override var lastRead: Long,
        override var unread: Boolean,
        override var proProofInfo: ProProofInfo?,
    ): Conversation, WithProProofInfo

    data class Community(
        val baseCommunityInfo: BaseCommunityInfo,
        override var lastRead: Long,
        override var unread: Boolean
    ) : Conversation

    data class LegacyGroup(
        val groupId: String,
        override var lastRead: Long,
        override var unread: Boolean
    ): Conversation

    data class ClosedGroup(
        val accountId: String,
        override var lastRead: Long,
        override var unread: Boolean
    ): Conversation

    data class BlindedOneToOne(
        val blindedAccountId: String,
        override var lastRead: Long,
        override var unread: Boolean,
        override var proProofInfo: ProProofInfo?,
    ) : Conversation, WithProProofInfo
}