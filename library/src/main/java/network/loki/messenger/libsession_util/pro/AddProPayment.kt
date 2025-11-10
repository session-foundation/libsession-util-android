package network.loki.messenger.libsession_util.pro

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object AddProPaymentRequests : LibSessionUtilCApi() {
    private external fun buildJson(
        version: Int,
        masterPrivateKey: ByteArray,
        rotatingPrivateKey: ByteArray,
        paymentProvider: Int,
        paymentTransactionId: String,
        paymentOrderId: String,
    ): String

    fun buildJson(
        /**
         * Version of the request to build a hash for
         */
        version: Int,
        /**
         * 64-byte libsodium style or 32 byte Ed25519 master private key
         */
        masterPrivateKey: ByteArray,

        /**
         * 64-byte libsodium style or 32 byte Ed25519 rotating private key
         */
        rotatingPrivateKey: ByteArray,

        /**
         * Provider that the payment to register is coming from
         */
        paymentProvider: PaymentProvider,

        /**
         * ID that is associated with the payment from the payment provider
         */
        paymentTransactionId: String,

        /**
         * Order ID that is associated with the payment
         */
        paymentOrderId: String,

    ): String = buildJson(
        version = version,
        masterPrivateKey = masterPrivateKey,
        rotatingPrivateKey = rotatingPrivateKey,
        paymentProvider = paymentProvider.nativeValue,
        paymentTransactionId = paymentTransactionId,
        paymentOrderId = paymentOrderId,
    )
}


enum class AddProPaymentError(internal val nativeValue: Int) {
    Generic(1),
    AlreadyRedeemed(2),
    UnknownPayment(3),
}



object AddProPaymentResponses {
    fun parse(jsonResponse: String): ProProofResponse<AddProPaymentError> {
        return when (val r = ProProofResponse.parseRaw(jsonResponse)) {
            is ProProofResponse.Failure<Int> -> {
                ProProofResponse.Failure(
                    status = AddProPaymentError.entries.firstOrNull {
                        it.nativeValue == r.status
                    } ?: AddProPaymentError.Generic,
                    errors = r.errors,
                )
            }

            is ProProofResponse.Success -> {
                ProProofResponse.Success(r.proof)
            }
        }
    }
}