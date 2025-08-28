package network.loki.messenger.libsession_util.protocol

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object SessionProtocol : LibSessionUtilCApi() {
    external fun encryptForDestination(
        message: ByteArray,
        myEd25519PrivKey: ByteArray,
        destination: Destination,
        namespace: Int
    ): ByteArray?

    external fun decryptEnvelope(
        key: DecryptEnvelopeKey,
        payload: ByteArray,
        nowEpochSeconds: Long,
        proBackendPubKey: ByteArray, // 32 bytes backend key
    ): DecryptedEnvelope
}