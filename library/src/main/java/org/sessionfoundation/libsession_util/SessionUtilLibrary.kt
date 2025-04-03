package org.sessionfoundation.libsession_util

import com.sun.jna.Library
import com.sun.jna.Native
import org.sessionfoundation.libsession_util.bindings.LibCBinding
import org.sessionfoundation.libsession_util.bindings.SessionNetworkBinding
import org.sessionfoundation.libsession_util.bindings.SessionCryptoBinding

internal interface SessionUtilLibrary : Library, LibCBinding, SessionCryptoBinding , SessionNetworkBinding {
    companion object {
        fun load(): SessionUtilLibrary {
            return Native.load("session_util", SessionUtilLibrary::class.java)
        }
    }
}