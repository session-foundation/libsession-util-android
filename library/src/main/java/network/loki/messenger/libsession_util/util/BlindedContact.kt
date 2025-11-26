package network.loki.messenger.libsession_util.util

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.ConversationPriority
import network.loki.messenger.libsession_util.protocol.ProProfileFeatures

data class BlindedContact(
    val id: String,
    val communityServer: String,
    val communityServerPubKeyHex: String,
    var name: String,
    var createdEpochSeconds: Long,
    var profileUpdatedEpochSeconds: Long,
    var profilePic: UserPic,
    var priority: ConversationPriority,
    var proFeatures: ProProfileFeatures,
) {
    @OptIn(ExperimentalStdlibApi::class)
    val communityServerPubKey: ByteArray
        get() = communityServerPubKeyHex.hexToByteArray()

    @get:Keep
    private val proFeaturesRaw: Long get() = proFeatures.rawValue
}