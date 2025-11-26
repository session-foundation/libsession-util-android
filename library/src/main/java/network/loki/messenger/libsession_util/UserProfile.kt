package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.pro.ProConfig
import network.loki.messenger.libsession_util.pro.ProProof
import network.loki.messenger.libsession_util.protocol.ProProfileFeatures
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
    external override fun getProfileUpdatedSeconds(): Long
    external override fun setReuploadedPic(userPic: UserPic)
    external override fun setNtsPriority(priority: Long)
    external override fun getNtsPriority(): Long
    external override fun setNtsExpiry(expiryMode: ExpiryMode)
    external override fun getNtsExpiry(): ExpiryMode
    external override fun getCommunityMessageRequests(): Boolean
    external override fun setCommunityMessageRequests(blocks: Boolean)
    external override fun isBlockCommunityMessageRequestsSet(): Boolean

    external override fun removeProConfig()

    private external fun setProConfig(
        proof: ProProof,
        rotatingPrivateKey: ByteArray
    )

    override fun setProConfig(proConfig: ProConfig) = setProConfig(
        proConfig.proProof,
        proConfig.rotatingPrivateKey.data
    )

    external override fun setProBadge(proBadge: Boolean)
    external override fun setAnimatedAvatar(animatedAvatar: Boolean)
    external override fun setProAccessExpiryMs(epochMills: Long)
    external override fun removeProAccessExpiry()
    private external fun getProFeaturesRaw(): Long
    override fun getProFeatures(): ProProfileFeatures = ProProfileFeatures(getProFeaturesRaw())
    external override fun getProConfig(): ProConfig?

    private external fun getProAccessExpiryMsOrZero(): Long
    override fun getProAccessExpiryMs(): Long? = getProAccessExpiryMsOrZero().takeIf { it != 0L }
}