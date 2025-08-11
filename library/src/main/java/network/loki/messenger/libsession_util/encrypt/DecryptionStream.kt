package network.loki.messenger.libsession_util.encrypt

import java.io.InputStream
import java.nio.ByteBuffer

class DecryptionStream(
    private val inStream: InputStream,
    key: ByteArray,
    private val autoClose: Boolean = true
) : InputStream() {
    private val nativeStatePtr: Long
    private val chunkSize: Int
    private val cipherBuffer: ByteArray
    private val plaintextBuffer: ByteBuffer

    init {
        try {
            // Read chunk size from the input stream
            val chunkSizeBuffer = ByteBuffer.allocate(4)
            check(inStream.read(chunkSizeBuffer.array(), 0, 4) == 4) {
                "Failed to read the chunk size from the input stream."
            }

            chunkSize = chunkSizeBuffer.int

            cipherBuffer = ByteArray((chunkSize + EncryptionStream.encryptionStreamChunkOverhead()).coerceAtLeast(
                EncryptionStream.encryptionStreamHeaderSize()
            ))

            plaintextBuffer = ByteBuffer.allocate(chunkSize)

            // Read the initial header from the input stream
            check(inStream.read(cipherBuffer, 0, EncryptionStream.encryptionStreamHeaderSize())
                == EncryptionStream.encryptionStreamHeaderSize()) {
                "Failed to read the initial header from the input stream."
            }

            // Initialize the native decryption stream state
            nativeStatePtr = createDecryptionStreamState(key, cipherBuffer)
        } catch (e: Exception) {
            if (autoClose) {
                inStream.close()
            }

            throw e
        }
    }

    protected fun finalize() {
        EncryptionStream.destroyEncryptionStreamState(nativeStatePtr)
    }

    private fun readChunkIfNeeded() {
        if (!plaintextBuffer.hasRemaining()) {
            // Read the next chunk of encrypted data from the input stream
            val bytesRead = inStream.read(cipherBuffer)
            if (bytesRead == -1) {
                // End of stream reached
                return
            }

            // Decrypt the chunk and fill the plaintext buffer
            plaintextBuffer.clear()
            val decryptedBytes = decryptionStreamPull(nativeStatePtr, cipherBuffer, bytesRead, plaintextBuffer.array())
            if (decryptedBytes < 0) {
                throw IllegalStateException("Decryption failed with error code: $decryptedBytes")
            }

            plaintextBuffer.limit(decryptedBytes)
        }
    }

    override fun read(): Int {
        readChunkIfNeeded()

        if (plaintextBuffer.hasRemaining()) {
            return plaintextBuffer.get().toInt()
        }

        return -1 // End of stream
    }

    override fun read(b: ByteArray, off: Int, len: Int): Int {
        readChunkIfNeeded()

        if (!plaintextBuffer.hasRemaining()) {
            return -1 // End of stream
        }

        val bytesToRead = minOf(len, plaintextBuffer.remaining())
        plaintextBuffer.get(b, off, bytesToRead)
        return bytesToRead
    }

    override fun close() {
        if (autoClose) {
            inStream.close()
        }

        super.close()
    }

    companion object {
        private external fun createDecryptionStreamState(key: ByteArray, header: ByteArray): Long
        private external fun decryptionStreamPull(nativeStatePtr: Long, inBuf: ByteArray, inLen: Int, outBuf: ByteArray): Int
    }
}