package network.loki.messenger.libsession_util.protocol


sealed interface DecryptEnvelopeKey {

    class Group(
        val groupEd25519PubKey: ByteArray,
        val groupKeys: Array<ByteArray>
    ) : DecryptEnvelopeKey {
        constructor(
            groupEd25519PubKey: ByteArray,
            groupKeys: Collection<ByteArray>
        ) : this(
            groupEd25519PubKey,
            groupKeys.toTypedArray()
        )
    }

    class Regular(
        val ed25519PrivKey: ByteArray
    ) : DecryptEnvelopeKey
}