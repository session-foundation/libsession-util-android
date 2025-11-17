package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.KeyPair

object ED25519 : LibSessionUtilCApi() {
    /**
     * Sign a message using the ed25519 private key
     *
     * @param ed25519PrivateKey 64 bytes ed25519 private key
     * @param message Message to sign
     * @return 64 bytes signature
     */
    external fun sign(
        ed25519PrivateKey: ByteArray,
        message: ByteArray,
    ): ByteArray

    /**
     * Verify a message using the ed25519 public key
     *
     * @param ed25519PublicKey 32 bytes ed25519 public key
     * @param message Message to verify
     * @param signature 64 bytes signature
     */
    external fun verify(
        ed25519PublicKey: ByteArray,
        message: ByteArray,
        signature: ByteArray,
    ): Boolean

    external fun generate(seed: ByteArray?): KeyPair

    /**
     * Generate the deterministic Master Session Pro key for signing requests to interact with the
     * Session Pro features of the protocol.
     *
     * @param ed25519Seed The seed the user uses to generate their session id
     * @return The libsodium-style Master Session Pro Ed25519 secret key, 64 bytes.
     */
    external fun generateProMasterKey(ed25519Seed: ByteArray): ByteArray

    private external fun positiveEd25519PubKeyFromCurve25519(curve25519PubKey: ByteArray): ByteArray

    fun ed25519PubKeysFromCurve25519(curve25519PubKey: ByteArray): List<ByteArray> {
        val positive = positiveEd25519PubKeyFromCurve25519(curve25519PubKey)
        val negative = positive.clone()
        negative[31] = (negative[31].toInt() xor 0x80).toByte()
        return listOf(positive, negative)
    }
}