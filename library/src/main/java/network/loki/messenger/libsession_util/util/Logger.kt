package network.loki.messenger.libsession_util.util

typealias LogLevel = Int

interface Logger {
    fun log(message: String, category: String, level: LogLevel)

    companion object {
        const val LOG_LEVEL_TRACE: LogLevel = 0
        const val LOG_LEVEL_DEBUG: LogLevel = 1
        const val LOG_LEVEL_INFO: LogLevel = 2
        const val LOG_LEVEL_WARN: LogLevel = 3
        const val LOG_LEVEL_ERROR: LogLevel = 4
        const val LOG_LEVEL_CRITICAL: LogLevel = 5
        const val LOG_LEVEL_OFF: LogLevel = 6

        init {
            System.loadLibrary("session_util")
        }

        external fun addLogger(logger: Logger)
    }
}