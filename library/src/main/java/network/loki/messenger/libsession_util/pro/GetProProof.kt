package network.loki.messenger.libsession_util.pro

import network.loki.messenger.libsession_util.LibSessionUtilCApi

enum class GetProProofError {
    Generic,
}

object GetProProofRequests : LibSessionUtilCApi() {
    external fun buildJson(
        version: Int,
        /**
         * 64-byte libsodium style or 32 byte Ed25519 master private key
         */
        masterPrivateKey: ByteArray,

        /**
         * 64-byte libsodium style or 32 byte Ed25519 rotating private key
         */
        rotatingPrivateKey: ByteArray,
        nowUnixTs: Long,
    ): String
}

object GetProProofResponses {
    fun parse(json: String): ProProofResponse<GetProProofError> {
        return when (val r = ProProofResponse.parseRaw(json)) {
            is ProProofResponse.Success -> ProProofResponse.Success(r.proof)
            is ProProofResponse.Failure -> {
                ProProofResponse.Failure(GetProProofError.Generic, r.errors)
            }
        }
    }
}
