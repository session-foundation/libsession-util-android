package org.sessionfoundation.libsession_util.bindings

import com.sun.jna.NativeLong
import com.sun.jna.Pointer
import com.sun.jna.ptr.NativeLongByReference
import com.sun.jna.ptr.PointerByReference

internal interface SessionCryptoBinding {
    fun session_random(size: NativeLong): Pointer

    fun session_blind15_key_pair(
        ed25519_seckey: ByteArray, // 64 bytes in
        server_pk: ByteArray, // 32 bytes in
        blinded_pk_out: ByteArray, // 32 bytes out
        blinded_sk_out: ByteArray, // 32 bytes out
    )

    fun session_blind25_key_pair(
        ed25519_seckey: ByteArray, // 64 bytes in
        server_pk: ByteArray, // 32 bytes in
        blinded_pk_out: ByteArray, // 32 bytes out
        blinded_sk_out: ByteArray, // 32 bytes out
    )

    fun session_ed25519_sign(
        ed25519_seckey: ByteArray, // 64 bytes in
        message: ByteArray, // message in
        message_len: NativeLong, // message length
        signature_out: ByteArray, // 64 bytes out
    ): Boolean

    fun session_decrypt_push_notification(
        payload: ByteArray, // encrypted payload in
        payload_len: NativeLong, // payload length in
        key: ByteArray, // 32 bytes key, in
        plaintext_out: PointerByReference, // decrypted payload out
        plaintext_len_out: NativeLongByReference, // decrypted payload length out
    ): Boolean

    fun session_decrypt_incoming(
        payload: ByteArray, //in
        payload_len: NativeLong, //in
        ed25519_seckey: ByteArray,  //in 64bytes
        session_id_out: ByteArray, // 67 bytes out
        plaintext_out: PointerByReference, //out
        plaintext_len_out: NativeLongByReference, //out
    ): Boolean

    fun session_decrypt_for_blinded_recipient(
        payload: ByteArray, //in
        payload_len: NativeLong, //in
        ed25519_seckey: ByteArray,  //in 64bytes
        community_pubkey: ByteArray, //in 32bytes
        sender_id: ByteArray, //in 33bytes
        recipient_id: ByteArray, //in 33bytes
        session_id_out: ByteArray, // 67 bytes out
        plaintext_out: PointerByReference, //out
        plaintext_len_out: NativeLongByReference, //out
    ): Boolean
}