package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.KeyPair

object Curve25519 : LibSessionUtilCApi() {
    private external fun fromED25519(
        ed25519PublicKey: ByteArray,
        ed25519PrivateKey: ByteArray,
    ): KeyPair

    fun fromED25519(keyPair: KeyPair): KeyPair =
        fromED25519(
            ed25519PublicKey = keyPair.pubKey.data,
            ed25519PrivateKey = keyPair.secretKey.data
        )

    external fun pubKeyFromED25519(
        ed25519PublicKey: ByteArray,
    ): ByteArray

    external fun generateKeyPair(): KeyPair
}
