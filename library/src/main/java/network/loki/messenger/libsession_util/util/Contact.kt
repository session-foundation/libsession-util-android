package network.loki.messenger.libsession_util.util

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.ConversationPriority
import network.loki.messenger.libsession_util.PRIORITY_VISIBLE
import network.loki.messenger.libsession_util.protocol.ProProfileFeatures

data class Contact(
    val id: String,
    var name: String = "",
    var nickname: String = "",
    var approved: Boolean = false,
    var approvedMe: Boolean = false,
    var blocked: Boolean = false,
    var profilePicture: UserPic = UserPic.DEFAULT,
    var createdEpochSeconds: Long = 0,
    var profileUpdatedEpochSeconds: Long = 0,
    var priority: ConversationPriority = PRIORITY_VISIBLE,
    var expiryMode: ExpiryMode = ExpiryMode.NONE,
    var proFeatures: ProProfileFeatures = ProProfileFeatures(),
) {

    val displayName: String
        get() = nickname.ifEmpty { name }

    @get:Keep
    private val proFeaturesRaw: Long get() = proFeatures.rawValue
}