package network.loki.messenger.libsession_util.protocol

typealias ProFeaturesForMsgStatus = Int

/**
 * Represents the result of trying to augment a message with Pro features.
 *
 * @param status The status of the augmentation attempt.
 * @param error An optional error message if the augmentation failed.
 * @param features The Pro features that were successfully applied.
 * @param codepointCount The number of codepoints in the message
 */
data class ProFeaturesForMsg(
    val status: ProFeaturesForMsgStatus,
    val error: String?,
    val features: ProMessageFeatures,
    val codepointCount: Int,
) {
    companion object {
        const val SUCCESS: ProFeaturesForMsgStatus = 0
        const val UTF_DECODING_ERROR: ProFeaturesForMsgStatus = 1
        const val EXCEEDS_CHARACTER_LIMIT: ProFeaturesForMsgStatus = 2
    }
}
