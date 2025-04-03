package org.sessionfoundation.libsession_util

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class CryptoTest {
    lateinit var crypto: Crypto

    @Before
    fun setUp() {
        val library = SessionUtilLibrary.load()
        crypto = Crypto(libc = library, binding = library)
    }

    @Test
    fun randomBytesWorks() {
        val bytes = crypto.randomBytes(32)
        assertEquals(32, bytes.size)
    }
}