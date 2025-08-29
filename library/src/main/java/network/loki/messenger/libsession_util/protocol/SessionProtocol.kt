package network.loki.messenger.libsession_util.protocol

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object SessionProtocol : LibSessionUtilCApi() {
    external fun encryptFor1o1(
        plaintext: ByteArray,
        myEd25519PrivKey: ByteArray,
        timestampMs: Long,
        recipientPubKey: ByteArray, // 33 bytes prefixed key
        proSignature: ByteArray?, // 64 bytes
    ): ByteArray

    external fun encryptForCommunityInbox(
        plaintext: ByteArray,
        myEd25519PrivKey: ByteArray,
        timestampMs: Long,
        recipientPubKey: ByteArray, // 33 bytes prefixed key
        communityServerPubKey: ByteArray, // 32 bytes key
        proSignature: ByteArray?, // 64 bytes
    ): ByteArray

    external fun encryptForGroup(
        plaintext: ByteArray,
        myEd25519PrivKey: ByteArray,
        timestampMs: Long,
        groupEd25519PublicKey: ByteArray, // 33 bytes 03 prefixed key
        groupEd25519PrivateKey: ByteArray, // 32 bytes group "encryption" key
        proSignature: ByteArray?, // 64 bytes
    ): ByteArray

    external fun decryptEnvelope(
        key: DecryptEnvelopeKey,
        payload: ByteArray,
        nowEpochSeconds: Long,
        proBackendPubKey: ByteArray, // 32 bytes backend key
    ): DecryptedEnvelope
}