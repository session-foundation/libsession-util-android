package org.sessionfoundation.libsession_util

import androidx.annotation.VisibleForTesting
import com.sun.jna.Native
import com.sun.jna.NativeLong
import com.sun.jna.Pointer
import com.sun.jna.ptr.PointerByReference
import java.io.File
import java.net.Inet4Address
import java.net.InetAddress
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

class Network internal constructor(
    private val library: SessionUtilLibrary,
    cacheDir: File,
    useTestnet: Boolean,
    singlePathMode: Boolean,
    preBuildPaths: Boolean
) {
    private val ptr: Pointer

    init {
        val ref = PointerByReference()
        val error = ByteArray(256)
        if (!library.network_init(
                ref,
                cacheDir.absolutePath,
                useTestnet,
                singlePathMode,
                preBuildPaths,
                error
            )
        ) {
            throw RuntimeException("Failed to initialize network: ${Native.toString(error)}")
        }

        ptr = ref.value
        check(ptr.getLong(0) != 0L) { "Null pointer from libsession" }
    }

    // This method should not be used in application code as it's hard to
    // control the lifecycle of the native resources. It's only used in tests to test the destruction
    // of this object.
    @VisibleForTesting
    internal fun destroy() {
        library.network_free(ptr)
    }

    suspend fun getSwarm(pubKeyHex: String): List<ServiceNode> {
        return suspendCoroutine { cont ->
            library.network_get_swarm(ptr, object : SessionUtilLibrary.GetSwarmCallback {
                override fun invoke(
                    nodes: SessionUtilLibrary.NetworkServiceNode.ByReference,
                    nodeLen: NativeLong,
                    data: Pointer
                ) {
                    cont.resume(
                        nodes.toArray(nodeLen.toInt())
                            .map { node -> ServiceNode(node as SessionUtilLibrary.NetworkServiceNode) }
                    )
                }
            }, Pointer.NULL)
        }
    }

    data class ServiceNode(
        val address: Inet4Address,
        val quicPort: Short,
        val ed25519PubKeyHex: String,
    ) {
        internal constructor(node: SessionUtilLibrary.NetworkServiceNode)
                : this(
            address = InetAddress.getByAddress(node.ip) as Inet4Address,
            quicPort = node.quic_port,
            ed25519PubKeyHex = Native.toString(node.ed25519_pubkey_hex)
        )
    }

    protected fun finalize() {
        destroy()
    }
}