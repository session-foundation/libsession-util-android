package network.loki.messenger.libsession_util.encrypt

import java.io.OutputStream
import java.nio.ByteBuffer

/**
 * An [OutputStream] that encrypts data on the fly, using libsodium's encryption stream API.
 *
 * Note that you must ensure this stream is flushed at the end, or you can call [close] (which you
 * should always do) to make sure all data is written out.
 *
 * @param out the underlying output stream to write encrypted data to.
 * @param key the encryption key to use for encrypting data, must be 32 bytes long.
 * @param chunkSize the size of chunks to write to the output stream. The bigger the chunk size,
 * the more memory is used, but the less overhead there is for each write operation.
 * @param autoClose whether to automatically close the underlying output stream when this stream is closed.
 */
class EncryptionStream(
    private val out: OutputStream,
    key: ByteArray,
    val chunkSize: Int = 4096,
    private val autoClose: Boolean = true
) : OutputStream() {
    private val nativeStatePtr: Long
    private val plaintextBuffer by lazy(LazyThreadSafetyMode.NONE) {
        ByteBuffer.allocate(chunkSize)
    }

    private val cipherBuffer = ByteBuffer.allocate(chunkSize + encryptionStreamChunkOverhead())

    init {
        try {
            // Write the chunk size
            out.write(ByteBuffer.allocate(4).putInt(chunkSize).array())

            // Write initial header to the output stream
            nativeStatePtr = createEncryptionStreamState(key, cipherBuffer.array())
            out.write(cipherBuffer.array(), 0, encryptionStreamHeaderSize())
        } catch (e: Exception) {
            if (autoClose) {
                out.close()
            }

            throw e
        }
    }

    protected fun finalize() {
        destroyEncryptionStreamState(nativeStatePtr)
    }

    private fun flushWhenFull() {
        if (!plaintextBuffer.hasRemaining()) {
            flush()
        }
    }

    override fun write(b: Int) {
        flushWhenFull()
        plaintextBuffer.put(b.toByte())
        flushWhenFull()
    }

    override fun write(b: ByteArray, off: Int, len: Int) {
        var from = off
        val to = off + len
        while (from < to) {
            flushWhenFull()
            val toWrite = minOf(to - from, chunkSize - plaintextBuffer.position()).coerceAtLeast(0)
            plaintextBuffer.put(b, from, toWrite)
            from += toWrite
            flushWhenFull()
        }
    }

    override fun flush() {
        super.flush()

        // Flip the buffer to prepare for reading
        plaintextBuffer.flip()

        if (plaintextBuffer.hasRemaining()) {
            cipherBuffer.clear()
            val cipherLength = encryptStreamPush(nativeStatePtr, plaintextBuffer.array(), plaintextBuffer.remaining(), cipherBuffer.array())
            out.write(cipherBuffer.array(), 0, cipherLength)
            plaintextBuffer.clear()
        }

        out.flush()
    }

    override fun close() {
        flush()
        if (autoClose) {
            out.close()
        }
        super.close()
    }


    companion object {
        init {
            System.loadLibrary("session_util")
        }

        external fun encryptionStreamHeaderSize(): Int
        external fun encryptionStreamChunkOverhead(): Int

        private external fun createEncryptionStreamState(key: ByteArray, headerOut: ByteArray): Long
        private external fun encryptStreamPush(statePtr: Long, inBuf: ByteArray, inBufLen: Int, outBuf: ByteArray): Int

        external fun destroyEncryptionStreamState(statePtr: Long)
    }
}