plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    id("maven-publish")
}

android {
    namespace = "org.sessionfoundation.libsession_util"
    compileSdk = 35
    ndkVersion = "26.3.11579264" // r27c can't compile gnutls due to mktime_z: make sure you check before updating this

    defaultConfig {
        minSdk = 24

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                targets("session_util")
                arguments("-GNinja")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1+"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    kotlinOptions {
        jvmTarget = "11"
    }

    publishing {
        singleVariant("release") {
            withSourcesJar()
        }
    }
}

publishing {
    publications {
        create<MavenPublication>("release") {
            groupId = "org.sessionfoundation"
            artifactId = "libsession-util-android"
            version = System.getenv("VERSION") ?: "dev-snapshot"

            pom {
                url = "getsession.org"

                licenses {
                    license {
                        name = "GNU GENERAL PUBLIC LICENSE, Version 3"
                        url = "https://www.gnu.org/licenses/gpl-3.0.txt"
                    }
                }

                scm {
                    connection = "scm:git:https://github.com/session-foundation/libsession-util-android"
                    url = "https://github.com/session-foundation/libsession-util-android"
                }
            }

            afterEvaluate {
                from(components["release"])
            }
        }

        repositories {
            maven {
                name = "local"
                url = uri(layout.buildDirectory.dir("repo"))
            }
        }
    }
}

dependencies {
    implementation(libs.androidx.annotation.jvm)
    implementation(libs.jna) {
        artifact {
            type = "aar"
        }
    }

    testImplementation(libs.junit)

    androidTestImplementation(libs.androidx.junit.ktx)
    androidTestImplementation(libs.junit)
    androidTestImplementation(libs.espresso.core)
    androidTestImplementation(libs.coroutines.test)
    testImplementation(kotlin("test"))
}