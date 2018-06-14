LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES= $(call all-java-files-under, com)
LOCAL_MODULE = TestJni

include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES= jni/appServer_jni.cpp
LOCAL_MODULE = libappServer_jni

LOCAL_SHARED_LIBRARIES != libandroid_runtime libnativehelper libutils libcutils liblog libdl

LOCAL_C_INCLUDES:= \
    $(ANDROID_BUILD_TOP)/external/jsoncpp/chromium-overrides/include \
    $(ANDROID_BUILD_TOP)/external/jsoncpp/include \
    $(ANDROID_BUILD_TOP)/external/jsoncpp/src/lib_json

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(ANDROID_BUILD_TOP)/external/jsoncpp/chromium-overrides/include \
    $(ANDROID_BUILD_TOP)/external/jsoncpp/include
    
LOCAL_CFLAGS := \
    -DJSON_USE_EXCEPTION=0

LOCAL_STATIC_LIBRARIES += libjsoncpp liblog

include external/stlport/libstlport.mk

include $(BUILD_SHARED_LIBRARY)