package org.sessionfoundation.libsession_util

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.sun.jna.Native
import kotlinx.coroutines.test.runTest
import org.junit.After
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class NetworkTest {
    lateinit var network: Network

    @Before
    fun setUp() {
        network = Network(
            library = SessionUtilLibrary.load(),
            cacheDir = InstrumentationRegistry.getInstrumentation().targetContext.cacheDir,
            useTestnet = true,
            singlePathMode = false,
            preBuildPaths = false,
        )
    }

    @After
    fun tearDown() {
        if (::network.isInitialized) {
            network.destroy()
        }
    }

    @Test
    fun testGetSwarmWorks() = runTest {
        val nodes = network.getSwarm("0538e63512fd78c04d45b83ec7f0f3d593f60276ce535d1160eb589a00cca7db58")
        assertTrue(nodes.isNotEmpty())
    }
}