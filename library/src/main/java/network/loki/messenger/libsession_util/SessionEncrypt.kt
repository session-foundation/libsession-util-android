package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.Bytes

private typealias SessionId = String

object SessionEncrypt : LibSessionUtilCApi() {
    /**
     * Decrypts data using the Session protocol for a blinded recipient.
     *
     * @param ciphertext The data to decrypt.
     * @param myEd25519Privkey The ED25519 private key to use for decryption. Could be 32 bytes seed or libsodium-style 64 bytes.
     * @param openGroupPubkey The public key of the open group. Must be 32 bytes unprefixed pub key.
     * @param senderBlindedId The sender's blinded ID. Must be 33 bytes prefixed with either 0x15 or 0x25.
     * @param recipientBlindId The recipient's blinded ID. Must be 33 bytes prefixed with either 0x15 or 0x25.
     * @return A pair of the sender's session ID and the decrypted message.
     */
    external fun decryptForBlindedRecipient(
        ciphertext: ByteArray,
        myEd25519Privkey: ByteArray,
        openGroupPubkey: ByteArray,
        senderBlindedId: ByteArray,
        recipientBlindId: ByteArray
    ): Pair<SessionId, Bytes>

    /**
     * Encrypts data using the Session protocol for a blinded recipient.
     *
     * @param message The plaintext message to encrypt.
     * @param myEd25519Privkey The ED25519 private key to use for signing the message. Could be 32 bytes seed or libsodium-style 64 bytes.
     * @param serverPubKey The public key of the server. Must be 32 bytes unprefixed pub key.
     * @param recipientBlindId The recipient's blinded ID. Must be 33 bytes prefixed with either 0x15 or 0x25.
     */
    external fun encryptForBlindedRecipient(
        message: ByteArray,
        myEd25519Privkey: ByteArray,
        serverPubKey: ByteArray,
        recipientBlindId: ByteArray,
    ): Bytes

    /**
     * Encrypts data using the Session protocol for `recipientX25519PublicKey`.
     *
     * @param ed25519PrivateKey The ED25519 private key to use for signing the message.
     *                          Could be 32bytes seed or libsodium-style 64 bytes
     * @param recipientX25519PublicKey Recipient's x25519 pub key. Must be 32 bytes unprefixed pub key.
     * @param message The plaintext message to encrypt.
     */
    external fun encryptForRecipient(
        ed25519PrivateKey: ByteArray,
        recipientX25519PublicKey: ByteArray,
        message: ByteArray,
    ): Bytes

    /**
     * Decrypt data using the Session protocol.
     *
     * @param x25519PubKey The X25519 public key of what the sender used to encrypt the message. 32 bytes.
     * @param x25519PrivKey The X25519 private key to the public key used to encrypt the message. 32/64 bytes.
     * @param ciphertext The data to decrypt
     *
     * @return A pair of the sender's session ID and the decrypted message.
     */
    external fun decryptIncoming(
        x25519PubKey: ByteArray,
        x25519PrivKey: ByteArray,
        ciphertext: ByteArray
    ): Pair<SessionId, Bytes>

    external fun decryptPushNotification(
        message: ByteArray,
        secretKey: ByteArray,
    ): Bytes

    external fun decryptOnsResponse(
        lowercaseName: String,
        ciphertext: ByteArray,
        nonce: ByteArray?,
    ): SessionId
}