package network.loki.messenger.libsession_util.pro

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object BackendRequests : LibSessionUtilCApi() {
    // Canonical provider code slugs; must match libsession's
    // SESSION_PRO_BACKEND_PAYMENT_PROVIDER_CODE_* constants (opaque wire codes).
    const val PAYMENT_PROVIDER_GOOGLE_PLAY: String = "google_play"
    const val PAYMENT_PROVIDER_APP_STORE: String = "app_store"
    const val PAYMENT_PROVIDER_RANGEPROOF: String = "rangeproof"

    external fun buildAddProPaymentRequestJson(
        version: Int,
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        providerCode: String,
        paymentId: String,
    ): String

    external fun buildGenerateProProofRequestJson(
        version: Int,
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        nowSeconds: Long,
    ): String

    external fun buildGetProDetailsRequestJson(
        version: Int,
        proMasterPrivateKey: ByteArray,
        nowSeconds: Long,
        count: Int,
    ): String
}
