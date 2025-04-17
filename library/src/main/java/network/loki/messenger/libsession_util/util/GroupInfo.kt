package network.loki.messenger.libsession_util.util

sealed class GroupInfo {

    data class CommunityGroupInfo(val community: BaseCommunityInfo, val priority: Long) : GroupInfo()

    data class ClosedGroupInfo(
        val groupAccountId: String,
        val adminKey: Bytes?,
        val authData: Bytes?,
        val priority: Long,
        val invited: Boolean,
        val name: String,
        val kicked: Boolean,
        val destroyed: Boolean,
        val joinedAtSecs: Long
    ): GroupInfo() {
        init {
            require(adminKey == null || adminKey.data.isNotEmpty()) {
                "Admin key must be non-empty if present"
            }

            require(authData == null || authData.data.isNotEmpty()) {
                "Auth data must be non-empty if present"
            }
        }

        // For native code
        val authDataAsByteArray: ByteArray?
            get() = authData?.data

        // For native code
        val adminKeyAsByteArray: ByteArray?
            get() = adminKey?.data

        fun hasAdminKey() = adminKey != null

        val shouldPoll: Boolean
            get() = !invited && !kicked && !destroyed

        companion object {
            /**
             * Generate the group's admin key(64 bytes) from seed (32 bytes, normally used
             * in group promotions).
             *
             * Use of JvmStatic makes the JNI signature less esoteric.
             */
            @JvmStatic
            external fun adminKeyFromSeed(seed: ByteArray): ByteArray
        }
    }

    data class LegacyGroupInfo(
        val accountId: String,
        val name: String,
        val members: Map<String, Boolean>,
        val encPubKey: Bytes,
        val encSecKey: Bytes,
        val priority: Long,
        val disappearingTimer: Long,
        val joinedAtSecs: Long
    ): GroupInfo() {
        companion object {
            @Suppress("FunctionName")
            external fun NAME_MAX_LENGTH(): Int
        }

        // For native code
        val encPubKeyAsByteArray: ByteArray
            get() = encPubKey.data

        // For native code
        val encSecKeyAsByteArray: ByteArray
            get() = encSecKey.data
    }
}