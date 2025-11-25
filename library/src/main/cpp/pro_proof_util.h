#pragma once

#include <session/session_protocol.hpp>
#include <jni.h>

#include "jni_utils.h"

session::ProProof java_to_cpp_proof(JNIEnv *, jobject proof);
jni_utils::JavaLocalRef<jobject> cpp_to_java_proof(JNIEnv *, const session::ProProof &proof);