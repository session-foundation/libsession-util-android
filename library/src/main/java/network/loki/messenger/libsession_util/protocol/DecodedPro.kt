package network.loki.messenger.libsession_util.protocol

import network.loki.messenger.libsession_util.pro.ProProof
import network.loki.messenger.libsession_util.pro.ProProofStatus

data class DecodedPro(
    val status: ProProofStatus,
    val proof: ProProof?,
    val proMessageFeatures: ProMessageFeatures,
    val proProfileFeatures: ProProfileFeatures,
)
