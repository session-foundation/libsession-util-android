# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("session_util")

# Compiles in C++20 mode
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS -Wno-deprecated-declarations)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(STATIC_BUNDLE ON)
set(ENABLE_ONIONREQ OFF)
add_subdirectory(../../../../libsession-util libsession)

include(FetchContent)


## Catering for libwebp
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_ANIM_UTILS ON CACHE BOOL "" FORCE)
set(WEBP_BUILD_LIBWEBPMUX ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        webp
        GIT_REPOSITORY https://chromium.googlesource.com/webm/libwebp
        GIT_TAG        v1.6.0
)

FetchContent_MakeAvailable(webp)

## Catering for libyuv - to rescale images
FetchContent_Declare(
        libyuv
        GIT_REPOSITORY https://chromium.googlesource.com/libyuv/libyuv
        GIT_TAG stable
)
FetchContent_MakeAvailable(libyuv)

## cgif
FetchContent_Declare(
        cgif
        GIT_REPOSITORY https://github.com/dloebl/cgif.git
        GIT_TAG 4ddc417
)
FetchContent_MakeAvailable(cgif)

## giflib (only to encode)
FetchContent_Declare(
        giflib
        GIT_REPOSITORY https://git.code.sf.net/p/giflib/code
        GIT_TAG 5.2.2
)
FetchContent_MakeAvailable(giflib)

## EasyGifReader (on top of giflib for easier reading)
FetchContent_Declare(
        easy_gif_reader
        GIT_REPOSITORY https://github.com/Chlumsky/EasyGifReader
        GIT_TAG eaaa794
)
FetchContent_MakeAvailable(easy_gif_reader)


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
        encryption.cpp
        jni_utils.cpp
        ed25519.cpp
        curve25519.cpp
        hash.cpp
        protocol.cpp
        attachments.cpp
        webp_utils.cpp
        gif_utils.cpp
        pro_backend.cpp
        pro_proof.cpp

        ${cgif_SOURCE_DIR}/src/cgif.c
        ${cgif_SOURCE_DIR}/src/cgif_rgb.c
        ${cgif_SOURCE_DIR}/src/cgif_raw.c

        ${giflib_SOURCE_DIR}/gifalloc.c
        ${giflib_SOURCE_DIR}/gif_err.c
        ${giflib_SOURCE_DIR}/dgif_lib.c
        ${giflib_SOURCE_DIR}/egif_lib.c
        ${giflib_SOURCE_DIR}/gif_hash.c
        ${giflib_SOURCE_DIR}/gif_font.c
        ${giflib_SOURCE_DIR}/openbsd-reallocarray.c

        ${easy_gif_reader_SOURCE_DIR}/EasyGifReader.cpp
)

add_library( # Sets the name of the library.
        session_util
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        ${SOURCES})

target_include_directories(session_util
        PRIVATE
            ${cgif_SOURCE_DIR}/inc
            ${libyuv_SOURCE_DIR}/include
            ${giflib_SOURCE_DIR}
            ${easy_gif_reader_SOURCE_DIR}
)


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
        libsodium::sodium-internal
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        webp
        libwebpmux
        webpdemux
        webpdecoder
        yuv
)