package network.loki.messenger.libsession_util.image

import android.graphics.ImageDecoder
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.sessionfoundation.libsession_util.test.R

@RunWith(AndroidJUnit4::class)
class WebUtilsTest {
    @Test
    fun testReencodeWebP() {

        for (outputSize in listOf(200, 400, 600)) {
            val output = InstrumentationRegistry.getInstrumentation()
                .targetContext
                .applicationContext
                .resources
                .openRawResource(R.raw.webp_animated).use { input ->
                    WebPUtils.reencodeWebPAnimation(
                        input = input.readAllBytes(),
                        timeoutMills = 100_000L,
                        targetWidth = outputSize,
                        targetHeight = outputSize
                    )
                }

            ImageDecoder.decodeDrawable(
                ImageDecoder.createSource(output)
            ) { decoder, info, source ->
                assertEquals(outputSize, info.size.width)
                assertEquals(outputSize, info.size.height)
                assertTrue(info.isAnimated)
                assertEquals("image/webp", info.mimeType)
            }
        }
    }

    @Test
    fun testEncodeGIFToWebP() {
        for (outputSize in listOf(200, 400, 600)) {
            val output = InstrumentationRegistry.getInstrumentation()
                .targetContext
                .applicationContext
                .resources
                .openRawResource(R.raw.earth).use { input ->
                    WebPUtils.encodeGifToWebP(
                        input = input.readAllBytes(),
                        timeoutMills = 100_000L,
                        targetWidth = outputSize,
                        targetHeight = outputSize
                    )
                }

            ImageDecoder.decodeDrawable(
                ImageDecoder.createSource(output)
            ) { decoder, info, source ->
                assertEquals("image/webp", info.mimeType)
                assertEquals(outputSize, info.size.width)
                assertEquals(outputSize, info.size.height)
                assertTrue(info.isAnimated)
            }
        }
    }
}