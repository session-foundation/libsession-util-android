package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.Bytes

private typealias SessionId = String

object SessionEncrypt {
    external fun decryptForBlindedRecipient(
        ciphertext: ByteArray,
        myEd25519Privkey: ByteArray,
        openGroupPubkey: ByteArray,
        senderBlindedId: ByteArray,
        recipientBlindId: ByteArray
    ): Pair<SessionId, Bytes>
}