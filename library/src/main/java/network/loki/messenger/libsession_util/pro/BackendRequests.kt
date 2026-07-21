package network.loki.messenger.libsession_util.pro

import network.loki.messenger.libsession_util.LibSessionUtilCApi

/**
 * Session Pro backend request builders + response parsers. libsession-util owns the wire format on
 * both sides — the single source of truth — so the app never builds or parses the JSON itself: each
 * `build*Request` signs internally and returns a [ProRequest] (endpoint + body) to POST, and each
 * `parse*Response` turns the raw reply JSON into a typed struct (check `header.errors` first).
 */
object BackendRequests : LibSessionUtilCApi() {
    // Canonical provider code slugs; must match libsession's
    // SESSION_PRO_BACKEND_PAYMENT_PROVIDER_CODE_* constants (opaque wire codes).
    const val PAYMENT_PROVIDER_GOOGLE_PLAY: String = "google_play"
    const val PAYMENT_PROVIDER_APP_STORE: String = "app_store"
    const val PAYMENT_PROVIDER_RANGEPROOF: String = "rangeproof"

    // --- Request builders (sign internally; return endpoint + JSON body) ---

    external fun buildAddProPaymentRequest(
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        providerCode: String,
        paymentId: String,
    ): ProRequest

    external fun buildGenerateProProofRequest(
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        nowSeconds: Long,
    ): ProRequest

    external fun buildGetProDetailsRequest(
        masterPrivateKey: ByteArray,
        nowSeconds: Long,
        count: Int,
    ): ProRequest

    external fun buildRefundRequest(
        masterPrivateKey: ByteArray,
        nowSeconds: Long,
        refundRequestedSeconds: Long,
        providerCode: String,
        paymentId: String,
    ): ProRequest

    external fun buildRevocationsRequest(ticket: Long): ProRequest

    // --- Response parsers (typed structs; check header.errors first) ---

    external fun parseAddPaymentResponse(json: String): ProProofResponse
    external fun parseProProofResponse(json: String): ProProofResponse
    external fun parsePaymentDetailsResponse(json: String): GetProDetailsResponse
    external fun parseRefundResponse(json: String): SetPaymentRefundRequestedResponse
    external fun parseRevocationsResponse(json: String): GetProRevocationsResponse

    // --- Provider URLs (libsession is the source of truth; null if none, e.g. rangeproof/unknown) ---

    external fun providerUrls(providerCode: String): ProviderUrls?

    /**
     * The purchasable payment-provider slugs to surface to users (single source of truth in libsession;
     * excludes non-purchasable providers like `rangeproof`). Order is not significant — the client
     * applies its own ordering and skips slugs it has no display translation for.
     */
    external fun visiblePlatforms(): Array<String>

    // --- Backend identity (single source of truth in libsession; read these, don't hard-code) ---

    /** The Session Pro backend base URL (overridable prod/default), e.g. "https://pro.session.codes". */
    external fun proBackendUrl(): String

    /** The Session Pro backend Ed25519 signing pubkey, hex (64 chars). */
    external fun proBackendPubKeyHex(): String
}
