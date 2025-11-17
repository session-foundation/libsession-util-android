package network.loki.messenger.libsession_util.protocol

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class SessionProtocolTest {

    @Test
    fun proFeaturesForMessageWorks() {
        val proposedFeatures = ProFeatures.from(listOf(ProFeature.PRO_BADGE))
        val result = SessionProtocol.proFeaturesForMessage(
            messageBody = "Hello, Session Pro!",
            proposedFeatures
        )

        assertEquals(ProFeaturesForMsg.Status.Success, result.status)
        assertEquals(19, result.codepointCount)
        assertEquals(proposedFeatures, result.features)
    }
}