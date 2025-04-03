package org.sessionfoundation.libsession_util

import com.sun.jna.NativeLong
import com.sun.jna.Pointer
import com.sun.jna.ptr.NativeLongByReference
import com.sun.jna.ptr.PointerByReference
import network.loki.messenger.libsession_util.util.KeyPair
import org.sessionfoundation.libsession_util.bindings.LibCBinding
import org.sessionfoundation.libsession_util.bindings.SessionCryptoBinding
import org.sessionfoundation.libsession_util.util.autoFree

class Crypto internal constructor(
    private val libc: LibCBinding,
    private val binding: SessionCryptoBinding,
) {
    fun randomBytes(size: Int): ByteArray {
        require(size > 0) {
            "size must be greater than 0"
        }

        val bytes = binding.session_random(NativeLong(size.toLong()))

        try {
            return bytes.getByteArray(0, size)
        } finally {
            libc.free(bytes)
        }
    }

    fun blind15KeyPair(ed25519SecKey: ByteArray, serverPk: ByteArray): KeyPair {
        require(ed25519SecKey.size == 64) {
            "ed25519SecKey must be 64 bytes"
        }

        require(serverPk.size == 32) {
            "serverPk must be 32 bytes"
        }

        val blindedPkOut = ByteArray(32)
        val blindedSkOut = ByteArray(32)
        binding.session_blind15_key_pair(
            ed25519_seckey = ed25519SecKey,
            server_pk = serverPk,
            blinded_pk_out = blindedPkOut,
            blinded_sk_out = blindedSkOut
        )

        return KeyPair(blindedPkOut, blindedSkOut)
    }

    fun ed25519Sign(message: ByteArray, ed25519Seckey: ByteArray): ByteArray {
        require(ed25519Seckey.size == 64) {
            "ed25519_seckey must be 64 bytes"
        }

        val signatureOut = ByteArray(64)
        check(binding.session_ed25519_sign(ed25519Seckey, message, NativeLong(message.size.toLong()), signatureOut)) {
            "Failed to sign message"
        }

        return signatureOut
    }

    fun decryptPushNotification(payload: ByteArray, key: ByteArray): ByteArray {
        require(key.size == 32) {
            "key must be 32 bytes"
        }

        return PointerByReference(Pointer.NULL).autoFree(libc) { plaintextOut ->
            val plaintextLenOut = NativeLongByReference()

            check(
                binding.session_decrypt_push_notification(
                    payload = payload,
                    payload_len = NativeLong(payload.size.toLong()),
                    key = key,
                    plaintext_out = plaintextOut,
                    plaintext_len_out = plaintextLenOut
                )
            ) {
                "Failed to decrypt push notification"
            }

            plaintextOut.value.getByteArray(0, plaintextLenOut.value.toInt())
        }
    }

}