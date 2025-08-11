package network.loki.messenger.libsession_util

import androidx.test.ext.junit.runners.AndroidJUnit4
import network.loki.messenger.libsession_util.encrypt.DecryptionStream
import network.loki.messenger.libsession_util.encrypt.EncryptionStream
import org.junit.Assert.assertArrayEquals
import org.junit.Test
import org.junit.runner.RunWith
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.security.SecureRandom

@RunWith(AndroidJUnit4::class)
class EncryptionStreamTest {

    private fun testEncryptionCase(
        chunkSize: Int,
        dataSize: Int,
    ) {
        try {
            val key = ByteArray(32)
            SecureRandom().nextBytes(key)

            val expectData = ByteArray(dataSize)
            SecureRandom().nextBytes(expectData)

            val encrypted = ByteArrayOutputStream().let { outputStream ->
                EncryptionStream(outputStream, key, chunkSize).use {
                    it.write(expectData)
                }

                outputStream.toByteArray()
            }

            val actualData = DecryptionStream(
                ByteArrayInputStream(encrypted),
                key
            ).use { it.readAllBytes() }

            assertArrayEquals(expectData, actualData)
        } catch (e: Exception) {
            throw RuntimeException("Encryption/Decryption failed for chunkSize: $chunkSize, dataSize: $dataSize", e)
        }
    }

    @Test
    fun shouldEncryptDecrypt() {
        testEncryptionCase(chunkSize = 24, dataSize = 25)
        testEncryptionCase(chunkSize = 24, dataSize = 24)
        testEncryptionCase(chunkSize = 24, dataSize = 12)
        testEncryptionCase(chunkSize = 24, dataSize = 48)
    }
}