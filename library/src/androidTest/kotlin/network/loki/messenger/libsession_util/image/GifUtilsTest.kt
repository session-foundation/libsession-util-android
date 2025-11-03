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
class GifUtilsTest {
    @Test
    fun testReencodeGif() {

        for (outputSize in listOf(200, 400, 600)) {
            val output = InstrumentationRegistry.getInstrumentation()
                .targetContext
                .applicationContext
                .resources
                .openRawResource(R.raw.earth).use { input ->
                    GifUtils.reencodeGif(
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
            }
        }
    }

    @Test
    fun testCheckAnimatedGifWorks() {
        assertTrue(
            InstrumentationRegistry.getInstrumentation()
                .targetContext
                .applicationContext
                .resources
                .openRawResource(R.raw.earth)
                .use(GifUtils::isAnimatedGif)
        )

        assertTrue(
            InstrumentationRegistry.getInstrumentation()
                .targetContext
                .applicationContext
                .resources
                .openRawResource(R.raw.earth)
                .use {
                    GifUtils.isAnimatedGif(it.readBytes())
                }
        )

        assertFalse(
            InstrumentationRegistry.getInstrumentation()
                .targetContext
                .applicationContext
                .resources
                .openRawResource(R.raw.sunflower_noanim)
                .use(GifUtils::isAnimatedGif)
        )

        assertFalse(
            InstrumentationRegistry.getInstrumentation()
                .targetContext
                .applicationContext
                .resources
                .openRawResource(R.raw.sunflower_noanim)
                .use {
                    GifUtils.isAnimatedGif(it.readBytes())
                }
        )
    }
}