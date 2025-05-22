package network.loki.messenger.libsession_util.util

import network.loki.messenger.libsession_util.LibSessionUtilCApi


object MultiEncrypt : LibSessionUtilCApi() {

    const val KICKED_DOMAIN = "SessionGroupKickedMessage"

    external fun encryptForMultipleSimple(
        messages: Array<ByteArray>,
        recipients: Array<ByteArray>,
        ed25519SecretKey: ByteArray,
        domain: String
    ): ByteArray

    external fun decryptForMultipleSimple(
        encoded: ByteArray,
        ed25519SecretKey: ByteArray,
        senderPubKey: ByteArray,
        domain: String,
    ): ByteArray?
}