package network.loki.messenger.libsession_util.util

object BlindKeyAPI {
    private val loadLibrary by lazy {
        System.loadLibrary("session_util")
    }

    init {
        // Ensure the library is loaded at initialization
        loadLibrary
    }

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
}