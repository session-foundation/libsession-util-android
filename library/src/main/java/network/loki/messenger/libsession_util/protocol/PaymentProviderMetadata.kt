package network.loki.messenger.libsession_util.protocol

import androidx.annotation.Keep

data class PaymentProviderMetadata @Keep constructor(
    val device: String,
    val store: String,
    val platform: String,
    val platformAccount: String,
    val refundPlatformUrl: String,
    val refundSupportUrl: String,
    val refundStatusUrl: String,
    val updateSubscriptionUrl: String,
    val cancelSubscriptionUrl: String,
)