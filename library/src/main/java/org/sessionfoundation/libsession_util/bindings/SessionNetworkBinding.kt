package org.sessionfoundation.libsession_util.bindings

import com.sun.jna.Callback
import com.sun.jna.NativeLong
import com.sun.jna.Pointer
import com.sun.jna.Structure
import com.sun.jna.Structure.FieldOrder
import com.sun.jna.ptr.PointerByReference

internal interface SessionNetworkBinding {
    fun network_init(
        obj: PointerByReference,
        cache_dir: String,
        use_testnet: Boolean,
        single_path_mode: Boolean,
        pre_build_paths: Boolean,
        error_out: ByteArray, // Needs at least 256 bytes
    ): Boolean

    interface GetSwarmCallback : Callback {
        fun invoke(
            nodes: NetworkServiceNode.ByReference,
            nodeLen: NativeLong,
            data: Pointer,
        )
    }

    @FieldOrder("ip", "quic_port", "ed25519_pubkey_hex")
    open class NetworkServiceNode : Structure() {
        val ip: ByteArray = ByteArray(4) // ipv4
        val quic_port: Short = 0
        val ed25519_pubkey_hex: ByteArray = ByteArray(65)

        class ByReference : NetworkServiceNode(), Structure.ByReference
        class ByValue : NetworkServiceNode(), Structure.ByValue
    }

    fun network_get_swarm(
        obj: Pointer,
        callback: GetSwarmCallback,
        data: Pointer,
    )


    fun network_send_onion_request_to_snode_destination(
        obj: Pointer,
        node: NetworkServiceNode.ByValue,
        body: ByteArray,
        bodyLen: NativeLong,
        swarmPubKeyHex: String,
        requestTimeoutMills: Long,
        requestAndPathBuildTimeoutMills: Long,
    )

    fun network_free(obj: Pointer)
}