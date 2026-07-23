package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep
import kotlinx.serialization.KSerializer
import kotlinx.serialization.Serializable
import kotlinx.serialization.descriptors.PrimitiveKind
import kotlinx.serialization.descriptors.PrimitiveSerialDescriptor
import kotlinx.serialization.encoding.Decoder
import kotlinx.serialization.encoding.Encoder
import java.time.Duration
import java.time.Instant

/**
 * Kotlin mirrors of the `session::pro_backend` response structs. libsession-util owns request
 * construction and response parsing (the single source of truth for the wire shape); these types are
 * what the glue hands the app so it never re-parses the wire itself.
 *
 * These are the app-facing shapes: dates are [Instant], intervals are [Duration], and opaque codes
 * (plan/provider/status) stay [String] slugs — no magic-int constants. Because the JNI boundary can
 * only marshal primitives, each type keeps a `@Keep` secondary constructor taking the raw epoch
 * integers (the shape the C++ builds via `NewObject`); it converts once — here at the glue boundary —
 * into the typed primary constructor, mapping the `0` "unset" sentinel to null. The types are
 * `@Serializable` so the app can persist them directly (e.g. its SQLite pro-status cache).
 */

/** Epoch **seconds** as an [Instant], or null when unset (`0`). */
private fun Long.secondsToInstantOrNull(): Instant? =
    if (this == 0L) null else Instant.ofEpochSecond(this)

/** Epoch **milliseconds** as an [Instant], or null when unset (`0`). */
private fun Long.msToInstantOrNull(): Instant? =
    if (this == 0L) null else Instant.ofEpochMilli(this)

/** Serializes an [Instant] as epoch-millis, for the app's local persistence of these types. */
private object InstantAsEpochMillisSerializer : KSerializer<Instant> {
    override val descriptor = PrimitiveSerialDescriptor("Instant", PrimitiveKind.LONG)
    override fun serialize(encoder: Encoder, value: Instant) = encoder.encodeLong(value.toEpochMilli())
    override fun deserialize(decoder: Decoder): Instant = Instant.ofEpochMilli(decoder.decodeLong())
}

/** Serializes a [Duration] as whole seconds. */
private object DurationAsSecondsSerializer : KSerializer<Duration> {
    override val descriptor = PrimitiveSerialDescriptor("Duration", PrimitiveKind.LONG)
    override fun serialize(encoder: Encoder, value: Duration) = encoder.encodeLong(value.seconds)
    override fun deserialize(decoder: Decoder): Duration = Duration.ofSeconds(decoder.decodeLong())
}

/** Route + content-type + body for a request to POST to the Pro backend (from the `*Request` builders). */
@Keep
data class ProRequest(
    val endpoint: String,
    val contentType: String,   // relay verbatim as the Content-Type header; don't assume a format
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
 * Outcome category for a Pro backend response (Delta #12). CLOSED set: [Ok] = success (payload fields
 * set); [Fail] = rejected on client input / a precondition; [Error] = backend fault (the same request
 * may succeed later, i.e. retryable). Ordinals match the C `SESSION_PRO_BACKEND_RESPONSE_STATUS` enum.
 */
@Serializable
enum class ProResponseStatus { Ok, Fail, Error }

/**
 * Common response header (Delta #12). Check [isSuccess] first.
 *
 * On non-[ProResponseStatus.Ok]: [errorCode] is a stable machine-readable slug (spec §5.1) — map known
 * ones to a localized (Crowdin) string; an unknown slug is forward-compatible (falls through). [error] is
 * an English diagnostic — NOT user-facing text; show it only when the slug has no i18n entry at all, and
 * it's always safe to log. Both are null on success.
 */
@Serializable
@Keep
data class ProResponseHeader(
    val status: ProResponseStatus,
    val errorCode: String?,
    val error: String?,
) {
    val isSuccess: Boolean get() = status == ProResponseStatus.Ok

    /** JNI-facing constructor: status as the C enum ordinal, nullable slug + diagnostic. */
    @Keep
    constructor(statusOrdinal: Int, errorCode: String?, error: String?) : this(
        status = ProResponseStatus.values().getOrElse(statusOrdinal) { ProResponseStatus.Error },
        errorCode = errorCode,
        error = error,
    )
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

/** One payment/subscription record from get-pro-status. */
@Serializable
@Keep
data class ProPaymentItem(
    val status: String,            // opaque per-payment status slug: unredeemed/redeemed/expired/revoked
    val planCount: Int,            // parsed billing-period count; >= 1 for periodic units, 0 for "lifetime"
    val planUnit: String,          // parsed unit name: second/day/week/month/year/lifetime (never canonicalized)
    val paymentProvider: String,   // provider slug, e.g. "google_play"
    val autoRenewing: Boolean,
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val purchased: Instant,                  // provider purchase instant; always set
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val redeemed: Instant?,                  // when activated; null if not activated
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val expiry: Instant?,                    // access expiry for this payment; null if not activated
    @Serializable(with = DurationAsSecondsSerializer::class)
    val gracePeriod: Duration,               // grace beyond [expiry] before access is really lost
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val platformRefundExpiry: Instant?,      // deadline for a platform ("quick") refund; null if n/a
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val revoked: Instant?,                   // when revoked; null if not revoked
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val refundRequested: Instant?,           // when a refund was requested; null if none
    val paymentId: String,         // opaque; confidential
) {
    /** Raw-epoch constructor used by the JNI layer (see the file header); converts to the typed fields. */
    @Keep
    constructor(
        status: String,
        planCount: Int,
        planUnit: String,
        paymentProvider: String,
        autoRenewing: Boolean,
        purchasedUnixTsMs: Long,
        redeemedUnixTs: Long,
        expiryUnixTs: Long,
        gracePeriodDurationSeconds: Long,
        platformRefundExpiryUnixTs: Long,
        revokedUnixTsMs: Long,
        refundRequestedUnixTs: Long,
        paymentId: String,
    ) : this(
        status = status,
        planCount = planCount,
        planUnit = planUnit,
        paymentProvider = paymentProvider,
        autoRenewing = autoRenewing,
        purchased = Instant.ofEpochMilli(purchasedUnixTsMs),
        redeemed = redeemedUnixTs.secondsToInstantOrNull(),
        expiry = expiryUnixTs.secondsToInstantOrNull(),
        gracePeriod = Duration.ofSeconds(gracePeriodDurationSeconds),
        platformRefundExpiry = platformRefundExpiryUnixTs.secondsToInstantOrNull(),
        revoked = revokedUnixTsMs.msToInstantOrNull(),
        refundRequested = refundRequestedUnixTs.secondsToInstantOrNull(),
        paymentId = paymentId,
    )
}

/**
 * Response to get-pro-status (endpoint `get_pro_status`, Delta #15 — the split-out "am I Pro?" call).
 * Carries the account status plus its single most-recent payment; the full payment history is a
 * separate (library-only) query and is not wired here.
 */
@Serializable
@Keep
data class GetProStatusResponse(
    override val header: ProResponseHeader,
    val userStatus: String,        // opaque account-status slug: never/active/expired
    val latestPayment: ProPaymentItem?,      // the single most-recent payment, or null when none
    val errorReport: Int,          // SESSION_PRO_BACKEND_GET_PRO_STATUS_ERROR_REPORT
    val autoRenewing: Boolean,
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val expiry: Instant?,                    // account access expiry (incl. grace); null if never subscribed
    @Serializable(with = DurationAsSecondsSerializer::class)
    val gracePeriod: Duration,               // grace included in [expiry]
    @Serializable(with = InstantAsEpochMillisSerializer::class)
    val refundRequested: Instant?,           // when a refund was requested; null if none
) : ProResponse {
    /** Raw-epoch constructor used by the JNI layer (see the file header); converts to the typed fields. */
    @Keep
    constructor(
        header: ProResponseHeader,
        userStatus: String,
        hasLatestPayment: Boolean,
        latestPayment: ProPaymentItem?,
        errorReport: Int,
        autoRenewing: Boolean,
        expiryUnixTs: Long,
        gracePeriodDurationSeconds: Long,
        refundRequestedUnixTs: Long,
    ) : this(
        header = header,
        userStatus = userStatus,
        latestPayment = if (hasLatestPayment) latestPayment else null,
        errorReport = errorReport,
        autoRenewing = autoRenewing,
        expiry = expiryUnixTs.secondsToInstantOrNull(),
        gracePeriod = Duration.ofSeconds(gracePeriodDurationSeconds),
        refundRequested = refundRequestedUnixTs.secondsToInstantOrNull(),
    )
}

/** One revocation-list entry. */
@Keep
data class ProRevocationItem(
    val revocationTagHex: String,  // 32-byte opaque tag, hex
    val effectiveUnixTs: Long,     // seconds; revoked only once client clock >= this
) {
    /** The revocation only takes effect once the client clock reaches this instant. */
    val effective: Instant get() = Instant.ofEpochSecond(effectiveUnixTs)
}

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
