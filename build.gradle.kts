// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
    val kotlinVersion = "2.3.20"

    id("com.android.library") version "9.1.0" apply false
    id("org.jetbrains.kotlin.plugin.serialization") version kotlinVersion apply false
}