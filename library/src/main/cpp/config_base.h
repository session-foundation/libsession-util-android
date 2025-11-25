#ifndef SESSION_ANDROID_CONFIG_BASE_H
#define SESSION_ANDROID_CONFIG_BASE_H

#include <session/config/base.hpp>

#include <jni.h>

session::config::ConfigBase* ptrToConfigBase(JNIEnv *env, jobject obj);
session::config::ConfigSig* ptrToConfigSig(JNIEnv* env, jobject obj);

#endif