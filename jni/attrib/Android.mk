LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gatttool-btle
LOCAL_SRC_FILES := gatttool.c \
	gatt.c \
	gattrib.c \
	att.c \
	utils.c \
	interactive.c \
	../src/log.c \
	../btio/btio.c

LOCAL_STATIC_LIBRARIES := bluetooth bluetoothd libhistory libreadline libtermcap glib
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../src/ \
	$(LOCAL_PATH)/../btio/ \
	$(LOCAL_PATH)/../glib/ \
	$(LOCAL_PATH)/../external/ \
	$(LOCAL_PATH)/../

LOCAL_CFLAGS:= \
        -DVERSION=\"4.98\" \
		-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
		-DNEED_PPOLL \
		-D__ANDROID__

include $(BUILD_EXECUTABLE)
