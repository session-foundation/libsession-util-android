package network.loki.messenger.libsession_util.encrypt

import network.loki.messenger.libsession_util.LibSessionUtilCApi

/**
 * Utilities for encrypting and decrypting attachments and profile pictures uploaded to the FileServer.
 */
object Attachments : LibSessionUtilCApi() {
    enum class Domain(val nativeValue: Int) {
        Attachment(0),
        ProfilePic(1),
    }

    /**
     * Returns the size of the encrypted data given the size of the plaintext.
     */
    external fun encryptedSize(plaintextSize: Long): Long

    /**
     * Returns the size of the encrypted data given the size of the plaintext, or null if the size
     * given is smaller than the minimum encryption overhead.
     */
    fun decryptedMaxSizeOrNull(ciphertextSize: Long): Long? = runCatching {
        decryptedMaxSize(ciphertextSize)
    }.getOrNull()


    private external fun decryptedMaxSize(ciphertextSize: Long): Long

    private external fun encryptBytes(
        seed: ByteArray,
        plaintextIn: ByteArray,
        plaintextInOffset: Int,
        plaintextInLen: Int,
        cipherOut: ByteArray,
        cipherOutOffset: Int,
        cipherOutLen: Int,
        domain: Int
    ): ByteArray

    /**
     * Encrypts the given plaintext using a key derived from the given seed and the given domain.
     *
     * The output needs to be allocated by the caller, its size can be determined by calling
     * [encryptedSize] with the size of the plaintext.
     *
     * @return The key used to encrypt the data.
     */
    fun encryptBytes(
        seed: ByteArray,
        plaintextIn: ByteArray,
        cipherOut: ByteArray,
        domain: Domain
    ): ByteArray = encryptBytes(
        seed = seed,
        plaintextIn = plaintextIn,
        plaintextInLen = plaintextIn.size,
        plaintextInOffset = 0,
        cipherOut = cipherOut,
        cipherOutOffset = 0,
        cipherOutLen = cipherOut.size,
        domain = domain.nativeValue
    )

    /**
     * Decrypts the given ciphertext using the given key.
     *
     * The output needs to be allocated by the caller, its size can be determined by calling
     * [decryptedMaxSizeOrNull] with the size of the ciphertext.
     *
     * Will throw if there's any error during decryption.
     *
     * @return The size of the decrypted data.
     */
    external fun decryptBytes(
        key: ByteArray,
        cipherIn: ByteArray,
        cipherInOffset: Int,
        cipherInLen: Int,
        plainOut: ByteArray,
        plainOutOffset: Int,
        plainOutLen: Int,
    ): Long

    /**
     * Decrypts the given ciphertext using the given key.
     *
     * The output needs to be allocated by the caller, its size can be determined by calling
     * [decryptedMaxSizeOrNull] with the size of the ciphertext.
     * Will throw if there's any error during decryption.
     *
     * @return The size of the decrypted data.
     */
    fun decryptBytes(
        key: ByteArray,
        cipherIn: ByteArray,
        plainOut: ByteArray
    ): Long = decryptBytes(
        key = key,
        cipherIn = cipherIn,
        cipherInOffset = 0,
        cipherInLen = cipherIn.size,
        plainOut = plainOut,
        plainOutOffset = 0,
        plainOutLen = plainOut.size
    )
}