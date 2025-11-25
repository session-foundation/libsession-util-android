package network.loki.messenger.libsession_util.protocol

import androidx.test.ext.junit.runners.AndroidJUnit4
import network.loki.messenger.libsession_util.pro.BackendRequests
import org.junit.Assert.assertNotNull
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class BackendRequestsTest {

    @Test
    fun getProviderMetadataWorks() {
        val metadata = BackendRequests.getPaymentProviderMetadata(BackendRequests.PAYMENT_PROVIDER_GOOGLE_PLAY)
        assertNotNull(metadata)
    }
}