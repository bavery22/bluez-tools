LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bluetooth
LOCAL_SRC_FILES := uuid.c \
	bluetooth.c \
	sdp.c \
	hci.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../

LOCAL_CFLAGS:= \
	-DVERSION=\"4.93\" \
	-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
	-DNEED_PPOLL \
	-D__ANDROID__ \
	-DCONFIGDIR=\"/etc/bluetooth\" \
	-DSERVICEDIR=\"/system/bin\" \
	-DPLUGINDIR=\"/system/lib/bluez-plugin\" \
	-DANDROID_SET_AID_AND_CAP \
	-DANDROID_EXPAND_NAME \
	-DOUIFILE=\"/data/misc/bluetoothd/ouifile\"

include $(BUILD_STATIC_LIBRARY)
