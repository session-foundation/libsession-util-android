package network.loki.messenger.libsession_util.protocol


sealed interface Destination {
    val proSignature: ByteArray?
    val sentTimestampMs: Long

    fun toNativeDestination(nativePtr: Long)

    class Contact(
        // 33 ByteArray prefixed key
        val recipientPubKey: ByteArray,
        override val proSignature: ByteArray?,
        override val sentTimestampMs: Long
    ) : Destination {
        init {
            check(recipientPubKey.size == 33) {
                "recipientPubKey must be 33 ByteArray prefixed key"
            }

            check(proSignature == null || proSignature.size == 64) {
                "proSignature must be null or 64 ByteArray"
            }
        }

        external override fun toNativeDestination(nativePtr: Long)
    }

    class Sync(
        // 33 ByteArray prefixed key
        val myPubKey: ByteArray,
        override val proSignature: ByteArray?,
        override val sentTimestampMs: Long
    ) : Destination {
        init {
            check(myPubKey.size == 33) {
                "myPubKey must be 33 ByteArray prefixed key"
            }

            check(proSignature == null || proSignature.size == 64) {
                "proSignature must be null or 64 ByteArray"
            }
        }

        external override fun toNativeDestination(nativePtr: Long)
    }

    class Group(
        val ed25519PubKey: ByteArray,
        val ed25519PrivKey: ByteArray,
        override val proSignature: ByteArray?,
        override val sentTimestampMs: Long
    ) : Destination {
        init {
            check(ed25519PubKey.size == 33) {
                "groupPubKey must be 33 ByteArray unprefixed key"
            }

            check(ed25519PrivKey.size == 32) {
                "groupPrivKey must be 32 ByteArray unprefixed key"
            }

            check(proSignature == null || proSignature.size == 64) {
                "proSignature must be null or 64 ByteArray"
            }
        }

        external override fun toNativeDestination(nativePtr: Long)
    }

    class Community(
        override val proSignature: ByteArray?,
        override val sentTimestampMs: Long
    ) : Destination {
        init {
            check(proSignature == null || proSignature.size == 64) {
                "proSignature must be null or 64 ByteArray"
            }
        }

        external override fun toNativeDestination(nativePtr: Long)
    }

    class CommunityInbox(
        val communityPubKey: ByteArray,
        val recipientPubKey: ByteArray,
        override val proSignature: ByteArray?,
        override val sentTimestampMs: Long
    ) : Destination {
        init {
            check(communityPubKey.size == 33) {
                "communityInboxPubKey must be 33 ByteArray unprefixed key"
            }

            check(recipientPubKey.size == 33) {
                "recipientPubKey must be 33 ByteArray prefixed key"
            }

            check(proSignature == null || proSignature.size == 64) {
                "proSignature must be null or 64 ByteArray"
            }
        }

        external override fun toNativeDestination(nativePtr: Long)
    }
}