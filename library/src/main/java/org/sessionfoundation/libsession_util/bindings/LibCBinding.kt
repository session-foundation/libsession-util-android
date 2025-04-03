package org.sessionfoundation.libsession_util.bindings

import com.sun.jna.Pointer

interface LibCBinding {
    fun free(obj: Pointer)
}