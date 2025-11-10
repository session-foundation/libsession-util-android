package network.loki.messenger.libsession_util.pro

import androidx.annotation.Keep
import network.loki.messenger.libsession_util.LibSessionUtilCApi


/**
 * Represents the response that returns a [ProProof]
 */
sealed interface ProProofResponse<out StatusCode> {
    data class Success @Keep constructor(val proof: ProProof) : ProProofResponse<Nothing>
    data class Failure<T>(val status: T, val errors: List<String>) : ProProofResponse<T>

    companion object : LibSessionUtilCApi() {
        /**
         * Parses a raw JSON string into a [ProProofResponse] with an integer status code.
         */
        private external fun nativeParseRaw(json: String): ProProofResponse<Int>

        internal fun parseRaw(json: String): ProProofResponse<Int> {
            return nativeParseRaw(json)
        }
    }
}