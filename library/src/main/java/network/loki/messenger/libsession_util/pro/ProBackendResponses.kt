package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep

/**
 * Kotlin mirrors of the `session::pro_backend` request/response structs. libsession-util owns both
 * request construction and response parsing (the single source of truth for the wire shape); these
 * types are what the JNI layer hands back so the app never re-parses the JSON itself.
 *
 * Timestamp conventions match the C++ structs: fields named `...UnixTs` are whole **unix seconds**;
 * `...UnixTsMs` are **unix milliseconds** (the provider's sub-second precision, kept as ms); duration
 * fields are **seconds**. A `0` timestamp means "not set".
 */

/** Route + body for a request to POST to the Pro backend (from the `*Request` builders). */
@Keep
data class ProRequest(
    val endpoint: String,
    val body: String,
)

/** Per-provider support/management URLs — libsession is the source of truth for these. */
@Keep
data class ProviderUrls(
    val refundPlatformUrl: String,
    val refundSupportUrl: String,
    val refundStatusUrl: String,
    val updateSubscriptionUrl: String,
    val cancelSubscriptionUrl: String,
)

/**
 * Common response header. `status` is 0 for success (or a request-specific status enum, e.g.
 * add-payment); a non-empty [errors] means the parse failed or was partial — always check first.
 */
@Keep
data class ProResponseHeader(
    val status: Int,
    val errors: List<String>,
) {
    val isSuccess: Boolean get() = errors.isEmpty()
}

/** Common interface for the parsed Pro backend responses; check [header] before reading the data. */
interface ProResponse {
    val header: ProResponseHeader
}

/** Response to add-payment / generate-proof: carries the freshly-issued proof (null on error). */
@Keep
data class ProProofResponse(
    override val header: ProResponseHeader,
    val proof: ProProof?,
) : ProResponse

/** One payment/subscription record from get-details. */
@Keep
data class ProPaymentItem(
    val status: String,            // opaque status code: unredeemed/redeemed/expired/revoked
    val plan: String,              // period code, e.g. "1m"/"3m"/"1y"; opaque
    val paymentProvider: String,   // provider slug, e.g. "google_play"; opaque
    val autoRenewing: Boolean,
    val purchasedUnixTsMs: Long,   // sys_ms (provider purchase instant); always set
    val redeemedUnixTs: Long,      // seconds; 0 if not activated
    val expiryUnixTs: Long,        // seconds; 0 if not activated
    val gracePeriodDurationSeconds: Long,
    val platformRefundExpiryUnixTs: Long, // seconds
    val revokedUnixTsMs: Long,     // sys_ms; 0 if not revoked
    val refundRequestedUnixTs: Long, // seconds; 0 if none
    val paymentId: String,         // opaque; confidential
)

/** Response to get-details. */
@Keep
data class GetProDetailsResponse(
    override val header: ProResponseHeader,
    val items: List<ProPaymentItem>,
    val userStatus: String,        // opaque status code: never/active/expired
    val errorReport: Int,          // SESSION_PRO_BACKEND_GET_PRO_DETAILS_ERROR_REPORT
    val autoRenewing: Boolean,
    val expiryUnixTs: Long,        // seconds; includes grace period; may be in the past
    val gracePeriodDurationSeconds: Long,
    val refundRequestedUnixTs: Long, // seconds; 0 if none
    val paymentsTotal: Int,
) : ProResponse

/** One revocation-list entry. */
@Keep
data class ProRevocationItem(
    val revocationTagHex: String,  // 32-byte opaque tag, hex
    val effectiveUnixTs: Long,     // seconds; revoked only once client clock >= this
)

/** Response to get-revocations. */
@Keep
data class GetProRevocationsResponse(
    override val header: ProResponseHeader,
    val ticket: Long,
    val retryInSeconds: Long,
    val retainForSeconds: Long,
    val items: List<ProRevocationItem>,
) : ProResponse

/** Response to set-refund-requested. */
@Keep
data class SetPaymentRefundRequestedResponse(
    override val header: ProResponseHeader,
    val updated: Boolean,
) : ProResponse
