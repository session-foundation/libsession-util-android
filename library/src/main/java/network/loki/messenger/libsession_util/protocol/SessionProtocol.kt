package network.loki.messenger.libsession_util.protocol

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object SessionProtocol : LibSessionUtilCApi() {
    external fun encodeFor1o1(
        plaintext: ByteArray,
        myEd25519PrivKey: ByteArray,
        timestampMs: Long,
        recipientPubKey: ByteArray, // 33 bytes prefixed key
        proRotatingEd25519PrivKey: ByteArray?, // 64 bytes
    ): ByteArray

    external fun decodeFor1o1(
        myEd25519PrivKey: ByteArray,
        payload: ByteArray,
        nowEpochMs: Long,
        proBackendPubKey: ByteArray, // 32 bytes backend key
    ): DecodedEnvelop

    external fun encodeForCommunityInbox(
        plaintext: ByteArray,
        myEd25519PrivKey: ByteArray,
        timestampMs: Long,
        recipientPubKey: ByteArray, // 33 bytes prefixed key
        communityServerPubKey: ByteArray, // 32 bytes key
        proRotatingEd25519PrivKey: ByteArray?, // 64 bytes
    ): ByteArray

    external fun encodeForCommunity(
        plaintext: ByteArray,
        proRotatingEd25519PrivKey: ByteArray?, // 64 bytes
    ): ByteArray

    external fun encodeForGroup(
        plaintext: ByteArray,
        myEd25519PrivKey: ByteArray,
        timestampMs: Long,
        groupEd25519PublicKey: ByteArray, // 33 bytes 03 prefixed key
        groupEd25519PrivateKey: ByteArray, // 32 bytes group "encryption" key
        proRotatingEd25519PrivKey: ByteArray?, // 64 bytes
    ): ByteArray

    external fun decodeEnvelope(
        key: DecryptEnvelopeKey,
        payload: ByteArray,
        nowEpochMs: Long,
        proBackendPubKey: ByteArray, // 32 bytes backend key
    ): DecodedEnvelop

    external fun decodeForCommunity(
        payload: ByteArray,
        nowEpochMs: Long,
        proBackendPubKey: ByteArray, // 32 bytes backend key
    ): DecodedCommunityMessage
}