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

    external fun encryptedSize(plaintextSize: Long): Long
    fun decryptedMaxSizeOrNull(ciphertextSize: Long): Long? = runCatching {
        decryptedMaxSize(ciphertextSize)
    }.getOrNull()


    private external fun decryptedMaxSize(ciphertextSize: Long): Long

    private external fun encryptBytes(
        seed: ByteArray,
        plaintextIn: ByteArray,
        cipherOut: ByteArray,
        domain: Int
    ): ByteArray

    fun encryptBytes(
        seed: ByteArray,
        plaintextIn: ByteArray,
        cipherOut: ByteArray,
        domain: Domain
    ): ByteArray = encryptBytes(seed, plaintextIn, cipherOut, domain.nativeValue)

    external fun decryptBytes(
        key: ByteArray,
        cipherIn: ByteArray,
        plainOut: ByteArray,
    ): Long
}