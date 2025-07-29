package network.loki.messenger.libsession_util

import network.loki.messenger.libsession_util.util.BlindedContact
import network.loki.messenger.libsession_util.util.Contact

class Contacts private constructor(pointer: Long) : ConfigBase(pointer), MutableContacts {
    constructor(ed25519SecretKey: ByteArray, initialDump: ByteArray? = null) : this(
        createConfigObject(
            "Contacts",
            ed25519SecretKey,
            initialDump
        )
    )

    override fun namespace() = Namespace.CONTACTS()

    external override fun get(accountId: String): Contact?
    external override fun getOrConstruct(accountId: String): Contact
    external override fun all(): List<Contact>
    external override fun set(contact: Contact)
    external override fun erase(accountId: String): Boolean
    external override fun getOrConstructBlinded(
        communityServerUrl: String,
        communityServerPubKeyHex: String,
        blindedId: String
    ): BlindedContact

    external override fun setBlinded(contact: BlindedContact)

    external override fun eraseBlinded(
        communityServerUrl: String,
        blindedId: String
    )

    external override fun allBlinded(): List<BlindedContact>

    external override fun getBlinded(
        blindedId: String
    ): BlindedContact?
}