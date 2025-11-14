package network.loki.messenger.libsession_util.pro

import network.loki.messenger.libsession_util.LibSessionUtilCApi

typealias PaymentProvider = Int

object BackendRequests : LibSessionUtilCApi() {
    const val PAYMENT_PROVIDER_GOOGLE_PLAY: PaymentProvider = 1
    const val PAYMENT_PROVIDER_APP_STORE: PaymentProvider = 2

    external fun buildAddProPaymentRequestJson(
        version: Int,
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        paymentProvider: PaymentProvider,
        paymentId: String,
        orderId: String,
    ): String

    external fun buildGenerateProProofRequestJson(
        version: Int,
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        nowMs: Long,
    ): String

    external fun buildGetProDetailsRequestJson(
        version: Int,
        proMasterPrivateKey: ByteArray,
        nowMs: Long,
        count: Int,
    ): String
}
