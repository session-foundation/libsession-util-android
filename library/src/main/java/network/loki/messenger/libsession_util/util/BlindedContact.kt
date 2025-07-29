package network.loki.messenger.libsession_util.util

data class BlindedContact(
    val id: String,
    val communityServer: String,
    val communityServerPubKeyHex: String,
    var name: String,
    var createdEpochSeconds: Long,
    var profilePic: UserPic
)