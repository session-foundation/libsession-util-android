package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.LibSessionUtilCApi

typealias PaymentProvider = Int

object BackendRequests : LibSessionUtilCApi() {
    const val PAYMENT_PROVIDER_GOOGLE_PLAY: PaymentProvider = 1
    const val PAYMENT_PROVIDER_APP_STORE: PaymentProvider = 2

    class MasterRotatingSignatures @Keep constructor(
        val masterSignature: ByteArray,
        val rotatingSignature: ByteArray,
    )

    external fun signAddProPaymentRequest(
        version: Int,
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        paymentProvider: PaymentProvider,
        paymentId: String,
        orderId: String,
    ): MasterRotatingSignatures

    external fun signGetProProofRequest(
        version: Int,
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        nowMs: Long,
    ): MasterRotatingSignatures

    external fun signGetProStatusRequest(
        version: Int,
        masterPrivateKey: ByteArray,
        nowMs: Long,
        count: Int,
    ): ByteArray
}
