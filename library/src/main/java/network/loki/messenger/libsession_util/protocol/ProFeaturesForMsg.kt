package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep

/**
 * Represents the result of trying to augment a message with Pro features.
 *
 * @param status The status of the augmentation attempt.
 * @param error An optional error message if the augmentation failed.
 * @param features The Pro features that were successfully applied.
 * @param codepointCount The number of codepoints in the message
 */
data class ProFeaturesForMsg(
    val status: Status,
    val error: String?,
    val features: ProFeatures,
    val codepointCount: Int,
) {
    @Keep
    constructor(
        statusNativeValue: Int,
        error: String?,
        featuresNativeValue: Long,
        codepointCount: Int,
    ) : this(
        status = Status.entries.first { it.nativeValue == statusNativeValue },
        error = error,
        features = ProFeatures(featuresNativeValue),
        codepointCount = codepointCount,
    )

    enum class Status(internal val nativeValue: Int) {
        Success(0),
        UTFDecodingError(1),
        ExceedsCharacterLimit(2),
    }
}
