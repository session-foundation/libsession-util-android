plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    id("maven-publish")
}

android {
    namespace = "org.sessionfoundation.libsession_util"
    compileSdk = 35

    defaultConfig {
        minSdk = 24

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
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
                    connection = "scm:git:https://github.com/session-foundation/libsession-android"
                    url = "https://github.com/session-foundation/libsession-android"
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

            val githubUser = System.getenv("GITHUB_USER").orEmpty()
            val githubToken = System.getenv("GITHUB_TOKEN").orEmpty()

            // Only enable the github remote repository if credentials are given
            if (githubUser.isNotBlank() && githubToken.isNotBlank()) {
                maven {
                    name = "github"
                    url =
                        uri("https://maven.pkg.github.com/session-foundation/libsession-android")
                    credentials {
                        username = githubUser
                        password = githubToken
                    }
                }
            }
        }
    }
}

dependencies {
}