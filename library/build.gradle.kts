import org.jetbrains.kotlin.gradle.dsl.JvmTarget

plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.serialization")
    id("maven-publish")
    id("com.google.protobuf") version "0.9.5"
}

group = "org.sessionfoundation"
version = System.getenv("VERSION") ?: "dev-snapshot"

val protobufVersion = "4.33.1"

android {
    namespace = "org.sessionfoundation.libsession_util"
    compileSdk = 35

    defaultConfig {
        minSdk = 26

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                arguments += listOf("-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON")
                targets("session_util")
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

            externalNativeBuild {
                cmake {
                    arguments += listOf("-DCMAKE_BUILD_TYPE=Release")
                }
            }
        }

        debug {
            packaging {
                jniLibs {
                    keepDebugSymbols += "**/*.so"
                }
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    publishing {
        singleVariant("release") {
            withSourcesJar()
        }
    }
}

kotlin {
    compilerOptions {
        jvmTarget = JvmTarget.JVM_11
    }
}

protobuf {
    protoc {
        artifact = "com.google.protobuf:protoc:$protobufVersion"
    }

    plugins {
        generateProtoTasks {
            all().forEach { task ->
                task.builtins {
                    create("java") {}
                }
            }
        }
    }
}

publishing {
    publications {
        create<MavenPublication>("release") {
            groupId = project.group.toString()
            artifactId = project.name
            version = project.version.toString()

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
        }
    }
}

dependencies {

    androidTestImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test:runner:1.7.0")
    androidTestImplementation("androidx.test:rules:1.7.0")
    androidTestImplementation("androidx.test.ext:junit:1.3.0")

    implementation("androidx.annotation:annotation:1.9.1")
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-core:1.9.0")

    api("com.google.protobuf:protobuf-java:$protobufVersion")

    protobuf(files("../libsession-util/proto/SessionProtos.proto"))
}
