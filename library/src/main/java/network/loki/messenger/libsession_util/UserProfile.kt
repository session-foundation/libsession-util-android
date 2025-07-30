package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.ExpiryMode
import network.loki.messenger.libsession_util.util.UserPic

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