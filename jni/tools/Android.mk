LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hcitool-btle
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := hcitool.c
LOCAL_STATIC_LIBRARIES := bluetooth bluetoothd
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../src \
	$(LOCAL_PATH)/..

LOCAL_CFLAGS:= \
        -DVERSION=\"4.98\" \
		-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
		-DNEED_PPOLL \
		-D__ANDROID__

include $(BUILD_EXECUTABLE)
