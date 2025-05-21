package network.loki.messenger.libsession_util

object ED25519 {
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
}