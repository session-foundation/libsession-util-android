package network.loki.messenger.libsession_util.util

import network.loki.messenger.libsession_util.ConversationPriority

data class BlindedContact(
    val id: String,
    val communityServer: String,
    val communityServerPubKeyHex: String,
    var name: String,
    var createdEpochSeconds: Long,
    var profileUpdatedEpochSeconds: Long,
    var profilePic: UserPic,
    var priority: ConversationPriority,
) {
    @OptIn(ExperimentalStdlibApi::class)
    val communityServerPubKey: ByteArray
        get() = communityServerPubKeyHex.hexToByteArray()
}