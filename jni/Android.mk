LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bluetooth
LOCAL_SRC_FILES := lib/uuid.c \
	lib/bluetooth.c \
	lib/sdp.c \
	lib/hci.c

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

include $(CLEAR_VARS)

LOCAL_MODULE := bluetoothd
LOCAL_SRC_FILES := src/oui.c \
	src/textfile.c

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

include $(CLEAR_VARS)

LOCAL_MODULE := hcitool
LOCAL_SRC_FILES := tools/hcitool.c
LOCAL_STATIC_LIBRARIES := bluetooth bluetoothd
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/

LOCAL_CFLAGS:= \
        -DVERSION=\"4.98\" \
		-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
		-DNEED_PPOLL \
		-D__ANDROID__

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := gatttool
LOCAL_SRC_FILES := attrib/gatttool.c
LOCAL_STATIC_LIBRARIES := bluetooth bluetoothd
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/

LOCAL_CFLAGS:= \
        -DVERSION=\"4.98\" \
		-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
		-DNEED_PPOLL \
		-D__ANDROID__

include $(BUILD_EXECUTABLE)
