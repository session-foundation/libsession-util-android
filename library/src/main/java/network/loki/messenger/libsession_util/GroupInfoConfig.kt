package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.UserPic

class GroupInfoConfig private constructor(pointer: Long): ConfigBase(pointer), MutableGroupInfoConfig {
    constructor(groupPubKey: ByteArray, groupAdminKey: ByteArray?, initialDump: ByteArray?)
            : this(newInstance(groupPubKey, groupAdminKey, initialDump))

    companion object {
        private external fun newInstance(
            pubKey: ByteArray,
            secretKey: ByteArray?,
            initialDump: ByteArray?
        ): Long
    }

    override fun namespace() = Namespace.GROUP_INFO()

    external override fun id(): String
    external override fun destroyGroup()
    external override fun getCreated(): Long?
    external override fun getDeleteAttachmentsBefore(): Long?
    external override fun getDeleteBefore(): Long?
    external override fun getExpiryTimer(): Long
    external override fun getName(): String?
    external override fun getProfilePic(): UserPic
    external override fun isDestroyed(): Boolean
    external override fun setCreated(createdAt: Long)
    external override fun setDeleteAttachmentsBefore(deleteBefore: Long)
    external override fun setDeleteBefore(deleteBefore: Long)
    external override fun setExpiryTimer(expireSeconds: Long)
    external override fun setName(newName: String)
    external override fun getDescription(): String
    external override fun setDescription(newDescription: String)
    external override fun setProfilePic(newProfilePic: UserPic)
    external override fun storageNamespace(): Long
}