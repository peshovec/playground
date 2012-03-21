LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := strbench
LOCAL_SRC_FILES := strbench.c 


LOCAL_SHARED_LIBRARIES := \
	    liblog \
	    libcutils \
            libbinder \
	    libutils
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := eng,debug

include $(BUILD_EXECUTABLE)
