package org.sessionfoundation.libsession_util.util

import com.sun.jna.Pointer
import com.sun.jna.ptr.PointerByReference
import org.sessionfoundation.libsession_util.bindings.LibCBinding

inline fun <T> PointerByReference.autoFree(
    libc: LibCBinding,
    block: (PointerByReference) -> T
): T {
    try {
        return block(this)
    } finally {
        if (pointer != null && pointer != Pointer.NULL) {
            libc.free(pointer)
        }
    }
}