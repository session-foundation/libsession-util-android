package network.loki.messenger.libsession_util.util

import network.loki.messenger.libsession_util.ConversationPriority
import network.loki.messenger.libsession_util.PRIORITY_VISIBLE

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
) {
    val displayName: String
        get() = nickname.ifEmpty { name }
}