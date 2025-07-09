package network.loki.messenger.libsession_util.util

import network.loki.messenger.libsession_util.LibSessionUtilCApi

object BlindKeyAPI : LibSessionUtilCApi() {
    external fun blindVersionKeyPair(ed25519SecretKey: ByteArray): KeyPair
    external fun blindVersionSign(ed25519SecretKey: ByteArray, timestamp: Long): ByteArray
    external fun blindVersionSignRequest(
        ed25519SecretKey: ByteArray,
        timestamp: Long,
        method: String,
        path: String,
        body: ByteArray?): ByteArray

    /**
     * Generate a 15-blinded key pair
     *
     * @param ed25519SecretKey The Ed25519 secret key, 32/64 bytes
     * @param serverPubKey The server public key, 32 bytes
     */
    external fun blind15KeyPair(
        ed25519SecretKey: ByteArray,
        serverPubKey: ByteArray,
    ): KeyPair

    /**
     * Generate a 15-blinded key pair, returning null on failure
     */
    fun blind15KeyPairOrNull(
        ed25519SecretKey: ByteArray,
        serverPubKey: ByteArray,
    ): KeyPair? = kotlin.runCatching { blind15KeyPair(ed25519SecretKey, serverPubKey) }.getOrNull()

    /**
     * Sign a message with a 15 blinded key.
     *
     * @param ed25519SecretKey The Ed25519 secret key, 32/64 bytes
     * @param serverPubKey The server public key in hex
     * @param message The message to sign
     */
    external fun blind15Sign(
        ed25519SecretKey: ByteArray,
        serverPubKey: String,
        message: ByteArray,
    ): ByteArray

    external fun blind15Ids(
        sessionId: String,
        serverPubKey: String,
    ): List<String>

    external fun blind25Id(
        sessionId: String,
        serverPubKey: String,
    ): String

    /**
     * Takes in a standard sessionId and returns a flag indicating whether it matches the given
     * blindedId for a given serverPubKey
     *
     * @param sessionId The session ID to check, 66bytes string
     * @param blindedId The blinded ID to check against, 66bytes string
     * @param serverPubKey The server public key, 64bytes string
     */
    external fun sessionIdMatchesBlindedId(
        sessionId: String,
        blindedId: String,
        serverPubKey: String,
    ): Boolean
}