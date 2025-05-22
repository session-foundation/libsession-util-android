package network.loki.messenger.libsession_util.util

import network.loki.messenger.libsession_util.LibSessionUtilCApi


object Sodium : LibSessionUtilCApi() {

    const val KICKED_DOMAIN = "SessionGroupKickedMessage"

    external fun ed25519KeyPair(seed: ByteArray): KeyPair
    external fun ed25519PkToCurve25519(pk: ByteArray): ByteArray

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