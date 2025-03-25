# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("session_util")

# Compiles in C++17 mode
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS -Wno-deprecated-declarations)

#set(CMAKE_BUILD_TYPE Release)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(STATIC_BUNDLE ON)
set(ENABLE_ONIONREQ ON)

set(LIBIDN2_VERSION 2.3.7)
set(LIBIDN2_HASH SHA512=eab5702bc0baed45492f8dde43a4d2ea3560ad80645e5f9e0cfa8d3b57bccd7fd782d04638e000ba07924a5d9f85e760095b55189188c4017b94705bef9b4a66)

set(GMP_MIRROR https://ftp.gnu.org/gnu/gmp)

add_subdirectory(../../../../libsession-util libsession)

set(SOURCES
        user_profile.cpp
        user_groups.cpp
        config_base.cpp
        contacts.cpp
        conversation.cpp
        blinded_key.cpp
        util.cpp
        group_members.cpp
        group_keys.cpp
        group_info.cpp
        config_common.cpp
        logging.cpp
)

add_library( # Sets the name of the library.
        session_util
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        ${SOURCES})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        session_util
        PUBLIC
        libsession::util
        libsession::config
        libsession::crypto
        libsession::onionreq
        quic
        libsodium::sodium-internal
        # Links the target library to the log library
        # included in the NDK.
        log)
