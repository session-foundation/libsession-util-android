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
        plaintextInOffset: Int,
        plaintextInLen: Int,
        cipherOut: ByteArray,
        cipherOutOffset: Int,
        cipherOutLen: Int,
        domain: Int
    ): ByteArray

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

    external fun decryptBytes(
        key: ByteArray,
        cipherIn: ByteArray,
        cipherInOffset: Int,
        cipherInLen: Int,
        plainOut: ByteArray,
        plainOutOffset: Int,
        plainOutLen: Int,
    ): Long

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